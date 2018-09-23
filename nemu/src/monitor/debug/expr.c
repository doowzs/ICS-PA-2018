#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>
#include <stdlib.h>

/* DONE: Add more token types */
enum {
  TK_NOTYPE = 256, TK_EQ,
	TK_NUM = 1, // DEC and OCT numbers share same type
	// TK_REG = 2,
	// TK_VAR = 3,
  TK_PLUS = 11,
	TK_MINUS = 12,
	TK_MUL = 21,
	TK_DIV = 22,
	TK_PLEFT = 31,
	TK_PRIGHT = 32
};

static struct rule {
  char *regex;
  int token_type;
} rules[] = {
  /* DONE: Add more rules.
   * Pay attention to the precedence level of different rules.
   */
  {" +", TK_NOTYPE},     // spaces
	{"(0x)?[[:digit:]]+", TK_NUM}, // numbers, TODO: deal with negative numbers.
  {"\\+", TK_PLUS},      // plus
	{"-", TK_MINUS},       // minus
	{"\\*", TK_MUL},       // multiply
	{"\\/", TK_DIV},       // division
	{"\\(", TK_PLEFT},     // left parenthesis
	{"\\)", TK_PRIGHT},    // right parenthesis
  {"==", TK_EQ}          // equal
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

Token tokens[32];
int nr_token;

static bool make_token(char *e) {
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

        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);
        position += substr_len;

        /* FIXME: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */

        switch (rules[i].token_type) {
					case TK_NUM:
						if (substr_len > 31) {
							// TODO: deal with overflow
							substr_len = 31;
						}
						strncpy(tokens[nr_token].str, substr_start, substr_len);
						tokens[nr_token].str[substr_len] = 0; // end of string
          default: 
				    tokens[nr_token].type = rules[i].token_type;
						if (rules[i].token_type != TK_NOTYPE) {
							nr_token++;
						}
        }
    
        break;
      }
    }

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
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
			*isValid = (lcount == 0);
			return false;
		}
	}

  *isValid = true;
	return lcount == 0;
}

/* Find the main operator and return its index. */
int find_main_operator(int p, int q, bool *success) {
	/* Update the return value if found an operator with higher priority. */
	int rcount = 0, index = 0, optype = TK_DIV + 1;
	for (int i = q; i >= p; --i) {
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
			case TK_PLUS:
			case TK_MINUS:
				if (rcount == 0 && optype > TK_MINUS) {
					index = i;
					optype = tokens[i].type;
				}
			case TK_MUL:
			case TK_DIV:
				if (rcount == 0 && optype > TK_DIV) {
					index = i;
					optype = tokens[i].type;
				}
		}
	}
	Log("Main operator at position %d", index);
	return index; // the index of main operator
}

/* Calculate the value of expression [p,q]. */
uint32_t eval(int p, int q, bool *success) {
	Log("Calculating period [%d, %d].", p, q);
	if (p > q) {
		/* Bad expression */
		*success = false;
		return 0;
	} else if (p == q) {
		/* Single token. Should be a number for now. */
		// TODO: add support for registers and varients.
		if (tokens[p].type == TK_NUM) {
			*success = true;
			return (uint32_t) strtol(tokens[p].str, NULL, 0);
		} else {
			assert(false); //TODO: change to *success = false;
			return 0;
		}
	} else {
		bool isValid = true;
		bool parenthesesCheck = check_parentheses(p, q, &isValid);
		if (!isValid) {
			/* invalid expression */
			assert(false); //TODO: change to *success = false;
			return 0;
		}
		if (parenthesesCheck) {
			/* The expression is surrounded by a matched pair of parentheses, throw them. */ 
		  return eval(p + 1, q - 1, success);
		} else {
			// TODO: add more things here.
			int op = find_main_operator(p, q, success);
			assert(*success); //TODO: change assert to human friendly prompt.
			uint32_t val1 = eval(p, op - 1, success);
			assert(*success); //TODO: change assert to human friendly prompt.
			uint32_t val2 = eval(op + 1, q, success);
			assert(*success); //TODO: change assert to human friendly prompt.

			switch (tokens[op].type) {
				case TK_PLUS:
					return val1 + val2;
				case TK_MINUS:
					return val1 - val2; //TODO: deal with overflow
				case TK_MUL:
					return val1 * val2; //TODO: deal with overflow
				case TK_DIV:
					return val1 / val2;
				default:
					assert(false); // invalid token
			}
		} 
	}
	*success = false;
	return 0;
}

/* Create tokens and calculate value. */
uint32_t expr(char *e, bool *success) {
  Log("The expression is \"%s\"", e);
  if (e == NULL || !make_token(e)) {
    *success = false;
    return 0;
  }

  /* TODO: Insert codes to evaluate the expression. */
	*success = true;
  uint32_t ret = eval(0, nr_token - 1, success); // indexed from 0 to nr_token-1
  if (*success) {
		return ret;
	} else {
		return 0;
	}
}
