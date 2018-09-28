#include "monitor/watchpoint.h"
#include "monitor/expr.h"

#define NR_WP 32

static WP wp_pool[NR_WP];
static WP *head, *free_;

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = &wp_pool[i + 1];
  }
  wp_pool[NR_WP - 1].next = NULL;

  head = NULL;
  free_ = wp_pool;
}

WP* new_wp() {
	Assert(free_ != NULL, "Watchpoint pool is empty.");
	WP *ret = free_;
	free_ = ret->next;
	if (head == NULL || head->NO >= ret->NO) {
		ret->next = head;
		head = ret;
	} else {
		WP *iter = head;
		while (iter->next != NULL && iter->next->NO < ret->NO) {
			iter = iter->next;
		}
		ret->next = iter->next;
		iter->next = ret;
	}
	return ret;
}

bool free_wp(int wp_NO) {
	WP *wp = NULL;
	if (head == NULL) {
		printf("[\033[1;31mError\033[0m] No watchpoint in use.\n");
		return false;
	} else {
		if (head->NO == wp_NO) {
			wp = head;
			head = wp->next;
		} else {
			for (WP *iter = head; iter->next != NULL; iter = iter->next) {
				if (iter->next->NO == wp_NO) {
					wp = iter->next;
					iter->next = wp->next;
					break;
				}
			}
		}
		if (wp == NULL) {
			printf("[\033[1;31mError\033[0m] Watchpoint #%02d is not in use.\n", wp_NO);
			return false;
		}
	}
	if (free_ == NULL || free_->NO >= wp->NO) {
		wp->next = free_;
		free_ = wp;
	} else {
		WP *iter = free_;
		while (iter->next != NULL && iter->next->NO < wp->NO) {
			iter = iter->next;
		}
		wp->next = iter->next;
		iter->next = wp;
	}
	return true;
}

void list_wp() {
	if (head == NULL) {
		printf("No watchpoint to show.\n");
	} else {
		for (WP *iter = head; iter != NULL; iter = iter->next) {
			printf("#%02d: 0x%08xH = \"%s\"\n", iter->NO, iter->val, iter->expr);
		}
	}
}

bool check_wp() {
	int new_val = 0;
	bool changed = false, success = false, overflow = false;
	for (WP *iter = head; iter != NULL; iter = iter->next) {
		new_val = expr(iter->expr, &success, &overflow);
		if (!success || new_val != iter->val) {
			changed = true;
			printf("[\033[1;36mWatchpoint\033[0m] WP #%02d: 0x%08xH -> 0x%08xH %s\n", iter->NO, iter->val, new_val, success ? "" : "[\033[1;31mFATAL\033[0m]");
		}
		iter->val = new_val;
	}
	return changed;
}
