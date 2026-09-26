#include "scheduler.h"
#include "kmalloc.h"
#include "apic.h"
#include "printf.h"
#include "spinlock.h"
#include "interrupt.h"

scheduler_t *scheduler_init() {
  scheduler_t *scheduler = kmalloc(sizeof(scheduler_t));
  scheduler->current = (void*)0;
  scheduler->count = 0;
  scheduler->head = 0;
  scheduler->lock = 0;
  scheduler->started = 0;
  return scheduler;
}

void scheduler_start(scheduler_t *scheduler) {
  if (!scheduler) return;
  cli();
  lock(&scheduler->lock);
  scheduler->started = 1;
  unlock(&scheduler->lock);
  sti();
}

static task_t *scheduler_pick_next(scheduler_t *scheduler) {
  if (!scheduler || scheduler->count == 0) return (void*)0;
  cli();
  lock(&scheduler->lock);
  if (!scheduler->started) {
    unlock(&scheduler->lock);
    sti();
    return (void*)0;
  }
  task_t *next = scheduler->queue[scheduler->head];
  scheduler->head = (scheduler->head + 1) % scheduler->count;
  unlock(&scheduler->lock);
  // NOTE: do not sti() here. scheduler_tick() runs inside the timer
  // interrupt handler; re-enabling IF before the context switch allows a
  // nested timer interrupt, which overwrites scheduler->current->rsp with
  // the nested frame. The outer switch would then consume the same frame
  // twice and iretq into garbage.
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
  cli();
  lock(&scheduler->lock);
  scheduler->queue[scheduler->count++] = task;
  unlock(&scheduler->lock);
  sti();
}