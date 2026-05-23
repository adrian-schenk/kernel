#include "scheduler.h"
#include "kmalloc.h"
#include "apic.h"
#include "printf.h"

scheduler_t *scheduler_init() {
  scheduler_t *scheduler = kmalloc(sizeof(scheduler_t));
  scheduler->current = (void*)0;
  scheduler->count = 0;
  scheduler->head = 0;
  return scheduler;
}

static task_t *scheduler_pick_next(scheduler_t *scheduler) {
  if (!scheduler || scheduler->count == 0) return (void*)0;
  task_t *next = scheduler->queue[scheduler->head];
  scheduler->head = (scheduler->head + 1) % scheduler->count;
  return next;
}

void scheduler_tick(scheduler_t *scheduler) {
  if (!scheduler) return;
  task_t *next = scheduler_pick_next(scheduler);
  if (next) {
    apic_write(EOI_REGISTER, 0); // send end of interrupt signal to apic
    task_switch_to(next);
  }
}

void scheduler_add_task(scheduler_t *scheduler, task_t *task) {
  if (!scheduler || !task) return;
  if (scheduler->count >= SCHEDULER_QUEUE_SIZE) return;
  scheduler->queue[scheduler->count++] = task;
}