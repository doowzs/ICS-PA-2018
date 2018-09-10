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

static int cmd_c(char *args) {
  cpu_exec(-1);
  return 0;
}

static int cmd_q(char *args) {
  return -1;
}

static int cmd_help(char *args);
static int cmd_si(char *args);
static int cmd_info(char *args);
static int cmd_p(char *args);
static int cmd_x(char *args);
static int cmd_w(char *args);
static int cmd_d(char *args);
static void cmd_wrong_parameters();

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
	{ "p", "Calculate the value of EXPR.", cmd_p },
	{ "x", "Calculate EXPR and print 4 bytes of memory data from EXPR.",
              cmd_x	},
	{ "w", "Watch point - automaticly pause the program when the value stored in EXPR is changed.", cmd_w },
  { "d", "Delete watchpoint numbered with N.", cmd_d },
};

#define NR_CMD (sizeof(cmd_table) / sizeof(cmd_table[0]))

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
	char *arg = strtok(NULL, " ");
	if (arg == NULL || strcmp(arg, "r") == 0) {
		printf("eax = 0x%08xH = %10dD\n", cpu.eax, cpu.eax);
		printf("ecx = 0x%08xH = %10dD\n", cpu.ecx, cpu.ecx);
		printf("edx = 0x%08xH = %10dD\n", cpu.edx, cpu.edx);
		printf("ebx = 0x%08xH = %10dD\n", cpu.ebx, cpu.ebx);
		printf("esp = 0x%08xH = %10dD\n", cpu.esp, cpu.esp);
		printf("ebp = 0x%08xH = %10dD\n", cpu.ebp, cpu.ebp);
		printf("esi = 0x%08xH = %10dD\n", cpu.esi, cpu.esi);
		printf("edi = 0x%08xH = %10dD\n", cpu.edi, cpu.edi);
		printf("eip = 0x%08xH = %10dD\n", cpu.eip, cpu.eip);
	} else if (strcmp(arg, "w") == 0) {
	  //TODO: Implement watchpoint in /monitor/debug/watchpoint.c
	} else {
		cmd_wrong_parameters();
	}
  return 0;
}

static int cmd_p(char *args) {
	//TODO: finish cmd_p function
	return 0;
}

static int cmd_x(char *args) {
	//TODO: implement EXPR fuction and improve cmd_x
	char *arg = strtok(NULL, " ");
	if (arg == NULL) {
		cmd_wrong_parameters();
	} else {
		int st = strtol(arg, NULL, 0); //TODO: replace paddr with EXPR
		int res = paddr_read(st, 4);
		printf("0x%08x: 0x%08x = %10d\n", st, res, res); 
	}
	return 0;
}

static int cmd_w(char *args) {
	//TODO: finish cmd_w function
	return 0;
}

static int cmd_d(char *args) {
	//TODO: finish cmd_d function
	return 0;
}

static void cmd_wrong_parameters() {
	printf("Wrong parameters. Please check your input.\n");
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

    if (i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
  }
}
