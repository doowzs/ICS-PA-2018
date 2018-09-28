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

void free_wp(WP *wp) {
	if (head == wp) {
		head = wp->next;
	} else {
		WP* iter = head;
		while (iter->next != NULL && iter->next != wp) {
			iter = iter->next;
		}
		Assert(iter->next != NULL, "The watchpoint to be deleted is not in use.");
		iter->next = wp->next;
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
}

void list_wp() {
	if (head == NULL) {
		printf("No watchpoint to show.\n");
	} else {
		for (WP *iter = head; iter != NULL; iter = iter->next) {
			printf("#%02d: 0x%08x = \"%s\"\n", iter->NO, iter->val, iter->expr);
		}
	}
}
