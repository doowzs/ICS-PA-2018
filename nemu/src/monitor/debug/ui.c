#include "monitor/monitor.h"
#include "monitor/expr.h"
#include "monitor/watchpoint.h"
#include "nemu.h"

#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>

void cpu_exec(uint64_t);

/* We use the `readline' library to provide more flexibility to read from stdin. */
char* rl_gets() {
  static char *line_read = NULL;

  if (line_read) {
    free(line_read);
    line_read = NULL;
  }

  line_read = readline("(nemu) ");

  if (line_read && *line_read) {
    add_history(line_read);
  }

  return line_read;
}

static int cmd_c(char *args);
static int cmd_q(char *args);
static int cmd_help(char *args);
static int cmd_si(char *args);
static int cmd_info(char *args);
static int cmd_p(char *args);
static int cmd_x(char *args);
static int cmd_w(char *args);
static int cmd_d(char *args);
static int cmd_save(char *args);
static int cmd_load(char *args);
#ifdef DIFF_TEST
static int cmd_detach(char *args);
static int cmd_attach(char *args);
#endif
static void cmd_wrong_parameter();

static struct {
  char *name;
  char *description;
  int (*handler) (char *);
} cmd_table [] = {
  /*
	 * This is the list of 9 basic commands for nemu debugger."
	 */
	{ "help", "Display informations about all supported commands", cmd_help },
  { "c", "Continue the execution of the program", cmd_c },
  { "q", "Exit NEMU", cmd_q },
	{ "si", "Continue running and stop after [N] steps. If [N] is not given, debugger will run only one step.", cmd_si },
  { "info", "Print info about the program.	SUBCMD can be given in two types, \'r\' for register and \'w\' for all watchpoints.", cmd_info },
	{ "p", "Usage: p EXPR, Calculate the value of EXPR.", cmd_p },
	{ "x", "Usage: x N EXPR, Calculate EXPR and print 4N bytes of memory data from EXPR.", cmd_x	},
	{ "w", "usage: w EXPR, Watch point - automaticly pause the program when the value stored in EXPR is changed.", cmd_w },
  { "d", "Usage: d N, Delete watchpoint numbered with N.", cmd_d },
  { "save", "Usage: save filename, Save NEMU state to a file.", cmd_save },
  { "load", "Usage: load filename, Load NEMU state from a specific file.", cmd_load },
#ifdef DIFF_TEST
  { "detach", "Usage: detach, stop difftest.", cmd_detach },
  { "attach", "Usage: attach, restart difftest.", cmd_attach },
#endif
};

#define NR_CMD (sizeof(cmd_table) / sizeof(cmd_table[0]))
#define PMEM_SIZE (128 * 1024 * 1024)
#define SAVE_PATH "/home/doowzs/ics2018/savefiles/"

static int cmd_help(char *args) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  int i;

  if (arg == NULL) {
    /* no argument given */
    for (i = 0; i < NR_CMD; i ++) {
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  }
  else {
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(arg, cmd_table[i].name) == 0) {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}

static int cmd_c(char *args) {
	/* pass -1 for no-stop running */
	cpu_exec(-1);
	return 0;
}

static int cmd_q(char *args) {
	/* exit nemu */
	return -1;
}

static int cmd_si(char *args) {
  /* parse the first argument to integer (N) */
  char *arg = strtok(NULL, " ");
	if (arg == NULL) {
		/* no argument given */
		cpu_exec(1);
	} else {
		/* if given 0, run 0 steps. */
		int N = strtol(arg, NULL, 0);
		cpu_exec(N);
	}
	return 0;
}

static int cmd_info(char *args) {
	/* extract the parameter */
	char *arg = strtok(NULL, " ");
	if (arg == NULL || strcmp(arg, "r") == 0) {
		/* r-mode (default), print all registers */
    /* -- GPR+EIP -- */
		printf("[\033[1;36mInfo\033[0m] Printing all registers.\n");
		for (int i = 0; i < 8; ++i) {
			printf("%s = \033[1;33m0x%08x\033[0mH = \033[1;33m%11d\033[0mD = \033[1;33m%11u\033[0mU\n", regsl[i], reg_l(i), reg_l(i), reg_l(i));
		}
    /* -- EFLAGS -- */
		printf("%s = \033[1;33m0x%08x\033[0mH = \033[1;33m%11d\033[0mD = \033[1;33m%11u\033[0mU\n", "eip", cpu.eip, cpu.eip, cpu.eip);
		printf("EFLAGS = ");
		for (int i = 0; i < EFLAGS_SIZE; ++i) {
			if ((cpu.eflags32 >> regse_index[i]) & 1) {
			  printf("[\033[1;32m%s\033[0m] ", regse_upper[i]);
			} else {
				printf(" %s  ", regse_upper[i]);
			}
		}
		printf(" [\033[1;33m0x%08d\033[0m]\n", cpu.eflags32);
    /* -- CR0  -- */
    printf("Paging status %s\n", ((cpu.CR[0] >> 31) & 1) ? "ON" : "OFF");
    /* -- IDTR -- */
    printf("IDTR base \033[1;33m0x%08x\033[0m limit \033[1;33m%d\033[0m\n", cpu.IDTR.base, cpu.IDTR.limit);
	} else if (strcmp(arg, "w") == 0) {
		/* w-mode, list all watchnodes */
		printf("[\033[1;36mInfo\033[0m] Printing all watchpoints.\n");
		list_wp();
	} else {
		/* wrong parameter given, call handler */
		cmd_wrong_parameter(args);
	}
  return 0;
}

static int cmd_p(char *args) {
	bool success = false, overflow = false;
	uint32_t res = expr(args, &success, &overflow);
	if (success) {
		if (overflow) { 
			printf("[\033[1;33mWarning\033[0m] Overflow detected.\n");
		}
		printf("[\033[1;36mPrint\033[0m] The result is \033[1;33m0x%08x\033[0mH = \033[1;33m%11d\033[0mD = \033[1;33m%11u\033[0mU\n", res, res, res);
	} else {
		printf("[\033[1;31mError\033[0m] Calculation failed.\n");
	}
	return 0;
}

static int cmd_x(char *args) {
	/* extract parameter from args (should be N and EXPR) */
	char *arg1 = strtok(NULL, " ");
	char *arg2 = strtok(NULL, "\0");
	if (arg1 == NULL || arg2 == NULL) {
		/* wrong parameters given, call handler */
		cmd_wrong_parameter(args);
	} else {
		/* calculate EXPR */
		bool success = false, overflow = false;
		uint32_t n = strtol(arg1, NULL, 0);
		uint32_t st = expr(arg2, &success, &overflow);

		if (success) {
		  if (overflow) { 
			  printf("[\033[1;33mWarning\033[0m] Overflow detected.\n");
		  }
			int res = 0;
	  	for (uint32_t i = 0; i < n; ++i, st += 4) {
        res = vaddr_read(st, 4);
        printf("0x%08x: \033[1;33m0x%08x\033[0mH = \033[1;33m%11d\033[0mD = \033[1;33m%11u\033[0mU\n", st, res, res, res);
		  }
	  } else {
		  printf("[\033[1;31mError\033[0m] Calculation failed.\n");
	  }
	}
	return 0;
}

static int cmd_w(char *args) {
	if (args == NULL) {
		cmd_wrong_parameter(args);
		return 0;
	}
	WP *wp = new_wp();
	if (wp == NULL) {
		printf("[\033[1;31mError\033[0m] No free watchpoint in pool.\n");
		return 0;
	} else {
	 	char *wp_expr = strtok(NULL, "\0");
		int expr_len = strlen(wp_expr);
		if (expr_len > 63) {
			printf("[\033[1;31mError\033[00m] The expression is too long (>63 characters).\n");
			free_wp(wp->NO);
			return 0;
		}
	
		bool success = false, overflow = false;
		uint32_t wp_val = expr(wp_expr, &success, &overflow);
		if (success) {
			if (overflow) { 
				printf("[\033[1;33mWarning\033[0m] Overflow detected.\n");
			}
		} else {
			printf("[\033[1;31mError\033[0m] Calculation failed.\n");
			free_wp(wp->NO);
			return 0;
		}
	  strcpy(wp->expr, wp_expr);	
		wp->val = wp_val;
		printf("[\033[1;32mSuccess\033[0m] Added watchpoint #%02d: 0x%08xH = \"%s\".\n", wp->NO, wp->val, wp->expr);
		return 0;
	}
}

static int cmd_d(char *args) {
	char *arg = strtok(NULL, " ");
	if (arg == NULL) {
		cmd_wrong_parameter(args);
		return 0;
	}

	int wp_NO = (int) strtol(arg, NULL, 0);
	if (free_wp(wp_NO)) {
		printf("[\033[1;32mSuccess\033[0m] Deleted watchpoint #%02d.\n", wp_NO);
	}
	return 0;
}

static int cmd_save(char *args) {
  char *arg = strtok(NULL, " ");
  if (arg == NULL) {
    cmd_wrong_parameter(args);
    return 0;
  }

  char fn[128] = SAVE_PATH;
  strcat(fn, arg);
  Log("%s", fn);
  FILE *fp = fopen(fn, "r");
  if (fp != NULL) {
    fclose(fp);
    if ((arg = strtok(NULL, " ")) != NULL && strcmp(arg, "-f") == 0) {
      printf("[\033[1;31mSAVE\033[0m] Warning: You are force saving to a existent file.\n");
    } else {
      printf("[\033[1;31mSAVE\033[0m] ERR: The target file exists. Use -f to force save.\n");
      return 0;
    }
  }

  fp = fopen(fn, "w+");
  if (fp == NULL) {
    printf("[\033[1;31mSAVE\033[0m] Open target file failed.\n");
  } else {
    printf("[\033[1;31mSAVE\033[0m] Start writing to file %s\n", fn);
    {
      fwrite(&cpu, sizeof(CPU_state), 1, fp);
      fwrite(&cpu.IDTR.base, sizeof(int), cpu.IDTR.limit, fp);
      fwrite(guest_to_host(0), sizeof(char), PMEM_SIZE + 0x100000, fp);
    }
    printf("[\033[1;31mSAVE\033[0m] NEMU state save finished.\n");
    fclose(fp);
  }
  return 0;
}

static int cmd_load(char *args) {
  char *arg = strtok(NULL, " ");
  if (arg == NULL) {
    cmd_wrong_parameter(args);
    return 0;
  }

  char fn[128] = SAVE_PATH;
  strcat(fn, arg);
  Log("%s", fn);
  FILE *fp = fopen(fn, "r");
  if (fp == NULL) {
    printf("[\033[1;32mLOAD\033[0m] ERR: The target file does not exist.\n");
    return 0;
  }

  int sz = 0;
  printf("[\033[1;32mLOAD\033[0m] Start loading from file %s\n", fn);
  {
    sz += fread(&cpu, sizeof(CPU_state), 1, fp);
    sz += fread(&cpu.IDTR.base, sizeof(int), cpu.IDTR.limit, fp);
    sz += fread(guest_to_host(0), sizeof(char), PMEM_SIZE + 0x100000, fp);
  }
  printf("[\033[1;32mLOAD\033[0m] NEMU state load finished (%u bytes). eip is at 0x%08x.\n", sz, cpu.eip);
  fclose(fp);
  return 0;
}

#ifdef DIFF_TEST
static int cmd_detach(char *args) {
  void difftest_detach();
  difftest_detach();
  return 0;
}

static int cmd_attach(char *args) {
  void difftest_attach();
  difftest_attach();
  return 0;
}
#endif

static void cmd_wrong_parameter(char *args) {
	/* input a prompt message and abort running the command */
	printf("[\033[1;36mCMD\033[0m] Wrong parameter \'%s\'. Please check your input. Type 'help' for usage.\n", args);
}

void ui_mainloop(int is_batch_mode) {
	if (is_batch_mode) {
    cmd_c(NULL);
    return;
  }

  while (1) {
    char *str = rl_gets();
    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    char *cmd = strtok(str, " ");
    if (cmd == NULL) { continue; }

    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    char *args = cmd + strlen(cmd) + 1;
    if (args >= str_end) {
      args = NULL;
    }

#ifdef HAS_IOE
    extern void sdl_clear_event_queue(void);
    sdl_clear_event_queue();
#endif

    int i;
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(cmd, cmd_table[i].name) == 0) {
        if (cmd_table[i].handler(args) < 0) { return; }
        break;
      }
    }

    if (i == NR_CMD) { 
      printf("[\033[1;36mCMD\033[0m] Unknown command '%s'. Type 'help' for usage.\n", cmd); 
#ifndef DIFF_TEST
      printf("[\033[1;36mCMD\033[0m] Note that this version does not contain diff-test.\n");
#endif
    }
  }
}
