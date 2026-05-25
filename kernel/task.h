#pragma once
#include <stdint.h>

typedef struct task {
  uint64_t rsp;
  uint64_t id;
  uint64_t rsp_base;
} task_t;

task_t *task_create(uint64_t entry);
void task_switch_to(task_t *next);
extern void _task_switch_to(void* v, task_t *current, task_t *next);
static void task_return(char a, char b, char c, char d, char e, char f, long long task_id);