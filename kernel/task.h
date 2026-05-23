#pragma once
#include <stdint.h>

typedef struct task {
  uint64_t rsp;
  uint64_t id;
} task_t;

task_t *task_create(uint64_t entry);
void task_switch_to(task_t *next);
extern void _task_switch_to(void* v, task_t *current, task_t *next);