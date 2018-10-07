#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>
#include <stdlib.h>

/* DONE: Add more token types */
enum {
  TK_NOTYPE = 256, 
	TK_NUM    = 1, // DEC and HEX numbers share same type
	TK_REG    = 2,
	// TK_VAR = 3,
	TK_PLEFT  = 4,
	TK_PRIGHT = 5,
	TK_EQ     = 11, // EQUAL ==
	TK_NEQ    = 12, // NEQ !=
	TK_AND    = 13, // AND &&
  TK_PLUS   = 21,
	TK_MINUS  = 22,
	TK_MUL    = 31,
	TK_DIV    = 32,
	TK_POSI   = 41, // POSITIVE +
	TK_NEGA   = 42, // NEGATIVE -
	TK_DEREF  = 43
};

static struct rule {
  char *regex;
	char *description;
  int token_type;
} rules[] = {
  /* DONE: Add more rules.
   * Pay attention to the precedence level of different rules.
   */
  {" +",                  "SPACE",       TK_NOTYPE},
	{"0[xX][[:xdigit:]]+",  "HEXNUM",      TK_NUM   },
	{"[[:digit:]]+",        "DECNUM",      TK_NUM   },
	{"\\$[[:alpha:]]+",     "REG",         TK_REG   },
  {"\\+",                 "PLUS",        TK_PLUS  },
	{"-",                   "MINUS",       TK_MINUS },
	{"\\*",                 "MUL/DEREF",   TK_MUL   },
	{"\\/",                 "DIV",         TK_DIV   },
	{"\\(",                 "PLEFT",       TK_PLEFT },
	{"\\)",                 "PRIGHT",      TK_PRIGHT},
  {"==",                  "EQ",          TK_EQ    },
	{"!=",                  "NEQ",         TK_NEQ   },
  {"&&",                  "AND",         TK_AND   }
};

#define NR_REGEX (sizeof(rules) / sizeof(rules[0]) )

static regex_t re[NR_REGEX];

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i ++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

typedef struct token {
  int type;
  char str[32];
} Token;

Token tokens[128];
int nr_token;

static void print_prompt(int pos, bool isError, const char *msg) {
	printf("[\033[1;%dm%s\033[0m] %s\n", isError ? 31 : 33, isError ? "Error" : "Warning", msg);

  int space_cnt = 0, token_len = strlen(tokens[pos].str);
  for (int i = 0; i < nr_token; ++i) {
		printf("%s", tokens[i].str);
		space_cnt += (i < pos) ? strlen(tokens[i].str) : 0;
	}
	printf("\n%*.s\033[1;%dm^", space_cnt, "", isError ? 31 : 33);
	for (int i = 1; i < token_len; ++i) {
		printf("~");
	}
	printf("\033[0m\n");
}

static bool make_token(char *e, bool *overflow) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;
				
			  #ifdef EXPR_DEBUG
          Log("Hit rule \"%s\" at position %d with len %d: \"%.*s\"",
               rules[i].description, position, substr_len, substr_len, substr_start);
				#endif

        position += substr_len;
        switch (rules[i].token_type) {
					case TK_NOTYPE:
						// do nothing
						break;
          default: 
						tokens[nr_token].type = rules[i].token_type;
						if (substr_len > 31) {
							*overflow = true;
							printf("[\033[1;33mWarning\033[0m] Number token is too long.\n");
							printf("%s\n%*.s\033[1;33m^", e, position - substr_len, "");
							for (int j = 1; j < substr_len; ++j) {
								printf("~");
							}
							printf("\033[0m\n");
							substr_len = 31;
						}
						strncpy(tokens[nr_token].str, substr_start, substr_len);
						tokens[nr_token].str[substr_len] = 0; // end of string
						nr_token++;
        }
    
        break;
      }
    }

    if (i == NR_REGEX) {
			printf("[\033[1;31mError\033[0m] No match at position %d\n", position);
      printf("%s\n%*.s\033[1;31m^\033[0m\n", e, position, "");
      return false;
    }
  }

  return true;
}

/* Check whether the expression [p,q]
 * is surrounded by parentheses.
 * Also returns false if parentheses don't match.
 */
static bool check_parentheses(int p, int q, bool *isValid) {
  int lcount = 0;
	for (int i = p; i <= q; ++i) {
		switch (tokens[i].type) {
			case TK_PLEFT:
				lcount++;
				break;
			case TK_PRIGHT:
				lcount--;
				break;
		}

		if (i == q) {
			break;
		}
		if (lcount <= 0) {
			/* check is invalid or ok 
			 * e.g. (a)) is invalid,
			 * while (a+b)+c is ok, but return false.
			 */
			*isValid = (lcount == 0);
			if (!isValid) {
				print_prompt(i, true, "Parentheses check failed.");
			}
			return false;
		}
	}

  *isValid = true;
	return lcount == 0;
}

/* Find the main operator and return its index. */
static int find_main_operator(int p, int q, bool *success) {
	/* Update the return value if found an operator with higher priority.
	 * If the next token is + or - (sign), then update to the current one.
	 */
	int rcount = 0, index = -1, optype = TK_NOTYPE - 1;
	for (int i = q; i >= p; --i) {
		//Log("At %d, %s, rcount = %d.", i, tokens[i].str, rcount);
		switch (tokens[i].type) {
			case TK_PRIGHT:
				rcount++;
				break;
			case TK_PLEFT:
				rcount--;
				if (rcount < 0) {
					/* invalid expression */
					*success = false;
					print_prompt(i, true, "Not a valid expression.");
					return -1;
				}
				break;
			case TK_EQ:
			case TK_NEQ:
				if (rcount == 0) {
					if (optype > TK_NEQ) {
						index = i;
						optype = tokens[i].type;
					}
				}
				break;
			case TK_AND:
				if (rcount == 0) {
					if (optype > TK_AND) {
						index = i;
						optype = tokens[i].type;
					}
				}
				break;
			case TK_PLUS:
			case TK_MINUS:
				if (rcount == 0) {
				 	if (optype > TK_MINUS) {
					  index = i;
					  optype = tokens[i].type;
					}
				}
				break;
			case TK_MUL:
			case TK_DIV:
				if (rcount == 0) {
					if (optype > TK_DIV) {
					  index = i;
					  optype = tokens[i].type;
					}
				}
				break;
			case TK_POSI:
			case TK_NEGA:
		  case TK_DEREF:
			  if (rcount == 0) {
					if (optype >= TK_POSI) { // deal signs in LTR method
						index = i;
						optype = tokens[i].type;
					}
				}
		}
	}
	return index; // the index of main operator, -1 for not found
}

/* Read the value of a given REG token. */
static int read_reg(int pos, bool *success) {
	char *name = tokens[pos].str + 1; // 0 is '$'
	// transform to lower case
	for (int i = 0; i < strlen(name); ++i) {
		if (name[i] >= 'A' && name[i] <= 'Z') {
			name[i] -= 32;
		}
	}

	for (int i = 0; i < 8; ++i) {
		if (strcmp(name, regsl[i]) == 0) {
			return reg_l(i);
		}
	}
	for (int i = 0; i < 8; ++i) {
		if (strcmp(name, regsw[i]) == 0) {
			return reg_w(i);
		}
	}
	for (int i = 0; i < 8; ++i) {
		if (strcmp(name, regsb[i]) == 0) {
			return reg_b(i);
		}
	}
	for (int i = 0; i < 6; ++i) {
		if (strcmp(name, regse[i]) == 0) {
			return reg_e(regse_index[i]);
		}
	}
	if (strcmp(name, "eip") == 0) {
		return cpu.eip;
	} else if (strcmp(name, "eflags") == 0) {
		return cpu.eflags._e32;
	} else {
		*success = false;
		print_prompt(pos, true, "Not a valid register name.");
		return 0;
	}
}

/* Calculate the value of expression [p,q]. */
static int eval(int p, int q, bool *success, bool *overflow) {
	#ifdef EXPR_DEBUG  
		/*   DEBUG   */
		char *express = (char*) malloc(128);
		*express = 0; // empty the temporary string
		for (int i = p; i <= q; ++i) {
			strcat(express, tokens[i].str);
		}
	  Log("Calculating section [%d, %d]: \"%s\"", p, q, express);
		free(express);
	#endif
	if (p > q) {
		/* Bad expression */
		*success = false;
		print_prompt(p, true, "Invalid expression. Please check your input.");
		return 0;
	} else if (p == q) {
		/* Single token. A num or register. */
		int res = 0;
		long res_l = 0;
	  switch (tokens[p].type) {
			case TK_NUM:
				res_l = strtol(tokens[p].str, NULL, 0);
				if (res_l > UINT32_MAX) {
					*overflow = true;
					print_prompt(p, false, "Number larger than UINT32_MAX.");
				}
				res = (int) res_l;
				break;
			case TK_REG:
				res = read_reg(p, success);
				break;
			default:
				*success = false;
				print_prompt(p, true, "Cannot calculate an invalid signle token.");
				res = 0;
		}
		#ifdef EXPR_DEBUG
			Log("Returning value for [%d, %d]: %d", p, q, (uint32_t) res);
		#endif
		return res;
	} else {
		bool isValid = true;
		bool parenthesesCheck = check_parentheses(p, q, &isValid);
		if (!isValid) {
			*success = false;
			return 0;
		}
		if (parenthesesCheck) {
			/* The expression is surrounded by a matched pair of parentheses, throw them. */ 
		  return eval(p + 1, q - 1, success, overflow);
		} else {
			int op = find_main_operator(p, q, success);
			if (op < 0) {
				*success = false;
				print_prompt(p, true, "Main operator not found or parentheses check failed.");
				return 0;
			}
			#ifdef EXPR_DEBUG
				Log("Found main operator \"%s\" at position %d", tokens[op].str, op);
			#endif

			/* If op is the sign of number, e.g. "-1", val1 should be zero. */
			int val1 = (op == p ? 0 : eval(p, op - 1, success, overflow));
			if (!*success) {
				// message has been written by callee
				return 0;
			}

			int val2 = eval(op + 1, q, success, overflow);
			if (!*success) {
				// message has been written by callee
				return 0;
			}

			int res = 0;
			switch (tokens[op].type) {
				case TK_POSI:
				case TK_PLUS:
					res = val1 + val2;
					if (((val1 >= 0) == (val2 >= 0)) &&
							((val1 >= 0) != (res >= 0))) {
						*overflow = true;
						print_prompt(op, false, "PLUS overflow.");
					}
					break;
				case TK_NEGA:
				case TK_MINUS:
					res = val1 - val2;
					if (((val1 >= 0) == (-val2 >= 0)) &&
							((val1 >= 0) != (res >= 0))) {
						*overflow = true;
						print_prompt(op, false, "MINUS overflow.");
					}
					break;
				case TK_MUL:
					res = val1 * val2;
					if (val1 && (res / val1 != val2)) {
						*overflow = true;
						print_prompt(op, false, "MUL overflow.");
					}
					break;
				case TK_DIV:
					if (val2 == 0) {
						*success = false;
						print_prompt(op, true, "Dividing zero.");
						return 0;
					}
					res = val1 / val2;
					/* if (res * val2 != val1) {
					 * *overflow = true;
					 * print_prompt(op, false, "DIV mismatch (remainder is discarded).");
					 * }
					 */
					break;
				case TK_EQ:
					res = (val1 == val2) ? 1 : 0;
					break;
				case TK_NEQ:
					res = (val1 != val2) ? 1 : 0;
					break;
				case TK_AND:
					res = (val1 && val2) ? 1 : 0;
					break;
				case TK_DEREF:
					res = vaddr_read(val2, 4);
					break;
				default:
					*success = false;
					print_prompt(op, true, "Calculation error: unknown operation token.");
					return 0;
			}
			#ifdef EXPR_DEBUG
				/*   DEBUG   */
				char *express = (char*) malloc(128);
				*express = 0; // empty the temporary string
				for (int i = p; i <= q; ++i) {
					strcat(express, tokens[i].str);
				}
				Log("Returning value for [%d, %d]: \"%s\" is %d", p, q, express, res);
				free(express);
			#endif
			return res;
		} 
	}

	*success = false;
	print_prompt(p, true, "Calculation failed. Unknown error.");
	return 0;
}

/* Create tokens and calculate value. */
uint32_t expr(char *e, bool *success, bool *overflow) {
	#ifdef EXPR_DEBUG
		Log("The expression is \"%s\"", e);
	#endif
  if (e == NULL || !make_token(e, overflow)) {
    *success = false;
    return 0;
  }

	for (int i = 0; i < nr_token; ++i) {
		if (tokens[i].type == TK_MUL && (i == 0 || tokens[i - 1].type >= TK_PLUS)) {
			#ifdef EXPR_DEBUG
				Log("Token at position %d marked as DEREF", i);
			#endif
			tokens[i].type = TK_DEREF;
		}
		if (tokens[i].type == TK_PLUS && (i == 0 || tokens[i - 1].type >= TK_PLUS)) {
			#ifdef EXPR_DEBUG
				Log("Token at position %d marked as POSITIVE", i);
			#endif
			tokens[i].type = TK_POSI;
		}
		if (tokens[i].type == TK_MINUS && (i == 0 || tokens[i - 1].type >= TK_PLUS)) {
			#ifdef EXPR_DEBUG
				Log("Token at position %d marked as NEGATIVE", i);
			#endif
			tokens[i].type = TK_NEGA;
		}
	}

  /* TODO: Insert codes to evaluate the expression. */
	*success = true;
  uint32_t ret = (uint32_t) eval(0, nr_token - 1, success, overflow); // indexed from 0 to nr_token-1
  if (*success) {
		return ret;
	} else {
		return 0;
	}
}
