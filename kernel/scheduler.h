#pragma once
#include "task.h"

#define SCHEDULER_QUEUE_SIZE 128

typedef struct scheduler {
  task_t *current;
  task_t *queue[SCHEDULER_QUEUE_SIZE];
  int count;
  int head;
} scheduler_t;

scheduler_t *scheduler_init();
void scheduler_add_task(scheduler_t *scheduler, task_t *task);

void scheduler_tick(scheduler_t *scheduler);

static task_t *scheduler_pick_next(scheduler_t *scheduler);