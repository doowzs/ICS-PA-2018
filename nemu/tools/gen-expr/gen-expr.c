#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <string.h>
#define MAXN 10
// this should be enough
int tot = 0;
bool lastIsNum = false;
static char buf[65536];
static char expr[65536];

char rand_op() {
	switch (rand() % 4) {
		case 0:
			return '+';
		case 1:
			return '-';
		case 2:
			return '*';
		default:
			return '/';
	}
}

static inline void gen_rand_expr() {
	//printf("tot is %d\n", tot);
	if (tot == 0) {
		sprintf(buf, "%d", rand() % MAXN), strcat(expr, buf);
	}
	while (tot > 0) {
    switch(rand()%8) {
			case 0:
			case 1:
			case 2:
			case 3:
				tot -= 1;
				if (lastIsNum) {
				  sprintf(buf, "%d", rand() % MAXN), strcat(expr, buf);
				} else {
					sprintf(buf, "%d", rand() % (MAXN-1) + 1), strcat(expr, buf);
					lastIsNum = true;
				}
				break;
			case 4:
			case 5:
				tot -= 1;
				if (lastIsNum) {
					sprintf(buf, "%c", rand_op()), strcat(expr, buf), lastIsNum = false;
				}
				sprintf(buf, "("), strcat(expr, buf);
				lastIsNum = false;
				gen_rand_expr();
				sprintf(buf, ")"), strcat(expr, buf);
				lastIsNum = false;
				break;
		  default:
				tot -= 1;
				gen_rand_expr();
			  if (rand() % 5 == 0) sprintf(buf, " "), strcat(expr, buf);	
				sprintf(buf, "%c", rand_op()), strcat(expr, buf), lastIsNum = false;
				if (rand() % 5 == 0) sprintf(buf, " "), strcat(expr, buf);
				gen_rand_expr();
		}
	}
}

static char code_buf[65536];
static char *code_format =
"#include <stdio.h>\n"
"int main() { "
"  unsigned result = %s; "
"  printf(\"%%u\", result); "
"  return 0; "
"}";

int main(int argc, char *argv[]) {
  int seed = time(0);
  srand(seed);
  int loop = 1;
  if (argc > 1) {
    sscanf(argv[1], "%d", &loop);
  }
  int i;
  for (i = 0; i < loop; i ++) {
		expr[0] = '\0';
		tot = rand() % 30;
		lastIsNum = false;
    gen_rand_expr();

    sprintf(code_buf, code_format, expr);

    FILE *fp = fopen(".code.c", "w");
    assert(fp != NULL);
    fputs(code_buf, fp);
    fclose(fp);

    int ret = system("gcc .code.c -o .expr");
    if (ret != 0) continue;

    fp = popen("./.expr", "r");
    assert(fp != NULL);

    int result;
    fscanf(fp, "%d", &result);
    pclose(fp);

    printf("%u %s\n", result, expr); 
  }
  return 0;
}
