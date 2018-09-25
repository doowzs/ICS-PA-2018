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
	TK_NUM    = 1, // DEC and OCT numbers share same type
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
	{"(0x)?[[:digit:]]+",   "NUM",         TK_NUM   },
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

static bool make_token(char *e, bool *overflow, char *msg) {
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

        Log("Hit rule \"%s\" at position %d with len %d: \"%.*s\"",
             rules[i].description, position, substr_len, substr_len, substr_start);

        position += substr_len;
        switch (rules[i].token_type) {
					case TK_NOTYPE:
						// do nothing
						break;
          default: 
						tokens[nr_token].type = rules[i].token_type;
						if (substr_len > 31) {
							*overflow = true;
							strcpy(msg, "Number token is too long.");
							substr_len = 31;
						}
						strncpy(tokens[i].str, substr_start, substr_len);
						nr_token++;
        }
    
        break;
      }
    }

    if (i == NR_REGEX) {
      printf("No match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

  return true;
}

/* Check whether the expression [p,q]
 * is surrounded by parentheses.
 * Also returns false if parentheses don't match.
 */
bool check_parentheses(int p, int q, bool *isValid) {
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
			return false;
		}
	}

  *isValid = true;
	return lcount == 0;
}

/* Find the main operator and return its index. */
int find_main_operator(int p, int q, bool *success) {
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
		  //case TK_DEREF:
			  if (rcount == 0) {
					if (optype > TK_NEGA) {
						index = i;
						optype = tokens[i].type;
					}
				}
		}
	}
	if (index < 0) {
		*success = false;
		return -1;
	}
	return index; // the index of main operator
}

/* Calculate the value of expression [p,q]. */
int eval(int p, int q, bool *success, bool *overflow, char *msg) {
	{  
		/*   DEBUG   */
		char *express = (char*) malloc(128);
		*express = 0; // empty the temporary string
		for (int i = p; i <= q; ++i) {
			strcat(express, tokens[i].str);
		}
	  Log("Calculating section [%d, %d]: \"%s\"", p, q, express);
		free(express);
	}
	if (p > q) {
		/* Bad expression */
		*success = false;
		strcpy(msg, "Invalid expression. Please check your input.");
		return 0;
	} else if (p == q) {
		/* Single token. Should be a number for now. */
		// TODO: add support for registers and varients.
		if (tokens[p].type == TK_NUM) {
			long res = strtol(tokens[p].str, NULL, 0);
			if (res > UINT32_MAX) {
				*overflow = true;
				strcpy(msg, "Number larger than UINT32_MAX.");
			}
			Log("Returning value for [%d, %d]: %d", p, q, (uint32_t) res);
			return (int) res;
		} else {
			*success = false;
			strcpy(msg, "Cannot calculate a signle non-number token.");
			return 0;
		}
	} else {
		bool isValid = true;
		bool parenthesesCheck = check_parentheses(p, q, &isValid);
		if (!isValid) {
			*success = false;
			strcpy(msg, "Parentheses check failed.");
			return 0;
		}
		if (parenthesesCheck) {
			/* The expression is surrounded by a matched pair of parentheses, throw them. */ 
		  return eval(p + 1, q - 1, success, overflow, msg);
		} else {
			int op = find_main_operator(p, q, success);
			if (op < 0) {
				*success = false;
				strcpy(msg, "Main operator not found.");
				return 0;
			}
	    Log("Found main operator \"%s\" at position %d", tokens[op].str, op);

			/* If op is the sign of number, e.g. "-1", val1 should be zero. */
			int val1 = (op == p ? 0 : eval(p, op - 1, success, overflow, msg));
			if (!*success) {
				// message has been written by callee
				return 0;
			}

			int val2 = eval(op + 1, q, success, overflow, msg);
			if (!*success) {
				// message has been written by callee
				return 0;
			}

			int res = 0;
			switch (tokens[op].type) {
				case TK_POSI:
				case TK_PLUS:
					res = val1 + val2;
					if (res < val1 || res < val2) {
						*overflow = true;
						strcpy(msg, "PLUS overflow.");
					}
					break;
				case TK_NEGA:
				case TK_MINUS:
					res = val1 - val2;
					if ((int) res < 0) {
						*overflow = true;
						strcpy(msg, "MINUS result is negative.");
					}
					break;
				case TK_MUL:
					res = val1 * val2;
					if (!val1 && (res / val1 != val2)) {
						*overflow = true;
						strcpy(msg, "MUL overflow.");
					}
					break;
				case TK_DIV:
					if (val2 == 0) {
						*success = false;
						strcpy(msg, "Dividing zero.");
						return 0;
					}
					res = val1 / val2;
					if (res * val2 != val1) {
						*overflow = true;
						strcpy(msg, "DIV mismatch (remainder is discarded).");
					}
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
				default:
					*success = false;
					strcpy(msg, "Calculation error: unknown operation token.");
					return 0;
			}
			Log("Returning value for [%d, %d] is %d", p, q, res);
			return res;
		} 
	}

	*success = false;
	return 0;
}

/* Create tokens and calculate value. */
uint32_t expr(char *e, bool *success, bool *overflow, char* msg) {
  Log("The expression is \"%s\"", e);
  if (e == NULL || !make_token(e, overflow, msg)) {
    *success = false;
		strcpy(msg, "Failed to match regex. Pleach check your input.");
    return 0;
  }

	for (int i = 0; i < nr_token; ++i) {
		if (tokens[i].type == TK_MUL && (i == 0 || tokens[i - 1].type >= TK_PLUS)) {
			Log("Token at position %d marked as DEREF", i);
			tokens[i].type = TK_DEREF;
		}
		if (tokens[i].type == TK_PLUS && (i == 0 || tokens[i - 1].type >= TK_PLUS)) {
			Log("Token at position %d marked as POSITIVE", i);
			tokens[i].type = TK_POSI;
		}
		if (tokens[i].type == TK_MINUS && (i == 0 || tokens[i - 1].type >= TK_PLUS)) {
			Log("Token at position %d marked as NEGATIVE", i);
			tokens[i].type = TK_NEGA;
		}
	}

  /* TODO: Insert codes to evaluate the expression. */
	*success = true;
  uint32_t ret = (uint32_t) eval(0, nr_token - 1, success, overflow, msg); // indexed from 0 to nr_token-1
  if (*success) {
		return ret;
	} else {
		return 0;
	}
}
