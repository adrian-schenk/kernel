#pragma once
#include <stdint.h>
#include <stddef.h>
#include "apic.h"
#include "page.h"

typedef struct task {
  uint64_t rsp;
  uint64_t id;
  uint64_t rsp_base;
  struct page_table *task_pml4;
} task_t;

task_t *task_create(uint64_t entry);
task_t *task_create_priv(uint64_t entry, char ss, char cs);
void task_switch_to(task_t *next);
extern void _task_switch_to(void* v, task_t *current, task_t *next);
static void task_return(char a, char b, char c, char d, char e, char f, long long task_id);