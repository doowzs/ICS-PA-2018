#ifndef __WATCHPOINT_H__
#define __WATCHPOINT_H__

#include "common.h"

typedef struct watchpoint {
  int NO;
  struct watchpoint *next;
	char expr[64];
	int val;
} WP;

WP* new_wp();
void free_wp(WP *);
void list_wp();
bool check_wp();

#endif
