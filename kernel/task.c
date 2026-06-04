#include "task.h"
#include "kmalloc.h"
#include "printf.h"
#include "cpu.h"
#include "interrupt.h"
#include "spinlock.h"

task_t *task_create(uint64_t entry)
{
  return task_create_priv(entry, 0x20 | 0x3, 0x18 | 0x3);
}

task_t *task_create_priv(uint64_t entry, char ss, char cs)
{
  task_t *task = (task_t *)kmalloc(sizeof(task_t));
  task->id = 1;
  task->task_pml4 = &kernel_pml4;

  if (cs & 0x3)
    task->task_pml4 = pt_alloc_page_phys(1);

  task->rsp_base = (uint64_t)kmalloc(4096);
  uint64_t *rsp_location;
  uint64_t *stack = ((uint8_t *)task->rsp_base) + 4096;

  *--stack = task->id;
  *--stack = task_return; // fake rbp
  *--stack = task_return; // return address
  *--stack = ss;          // ss
  rsp_location = --stack; // save rsp for later
  *--stack = 0x202;       // rflags
  *--stack = cs;          // CS
  *--stack = entry;       // return address for when the task is switched to

  --stack;
  --stack; // mock interrupt number and error

  for (int i = 0; i < 15; i++)
    *--stack = 0x00; // mock registers

  for (int i = 0; i < (152 / 8); i++)
    *--stack = 0x00; // mock function call stack from interrupt_handler to task_switch_to (152 bytes)

  *rsp_location = (uint64_t *)rsp_location + 2;
  task->rsp = (uint64_t)stack;

  if (cs & 0x3) {
    // TODO: fix this to not identity map the entire memory for every task
    for (int i = 0; i < 0x8000000; i += HUGE_PAGE_SIZE)
    {
      pt_map_page_huge(task->task_pml4, i, i, PAGE_PRESENT | PAGE_WRITABLE | PAGE_USER);
    }

    VbeModeInfoBlock *video_mode = (VbeModeInfoBlock*)boot_info->vesa_info;
    uint64_t framebuffer_base = (uint64_t)video_mode->PhysBasePtr;
    uint64_t framebuffer_size = (uint64_t)video_mode->BytesPerScanLine * video_mode->YResolution;
    for (uint64_t offset = 0; offset < framebuffer_size; offset += PAGE_SIZE) {
        pt_map_page(task->task_pml4,
                    framebuffer_base + offset,
                    framebuffer_base + offset,
                    PAGE_PRESENT | PAGE_WRITABLE | PAGE_USER);
    }

    apic_map_pages(task->task_pml4);
  }

  return task;
}

// TODO: allow threads to return values
static void task_return(char a, char b, char c, char d, char e, char f, long long task_id)
{
  // kprintf("task returned, exiting cpu %d and %l at %p with %d\n", this_cpu(cpu_id), task_id, &task_id, a);
  cli();
  lock(&this_cpu(scheduler)->lock);
  for (int i = 0; i < SCHEDULER_QUEUE_SIZE; i++)
  {
    if (this_cpu(scheduler)->queue[i] && this_cpu(scheduler)->queue[i]->id == task_id)
    {
      kfree(this_cpu(scheduler)->queue[i]->rsp_base);
      this_cpu(scheduler)->queue[i] = (void *)0;
      this_cpu(scheduler)->count--;
      this_cpu(scheduler)->current = (void *)0;
      break;
    }
  }
  unlock(&this_cpu(scheduler)->lock);
  sti();
  for (;;)
    ;
}

void task_switch_to(task_t *next)
{
  _task_switch_to(&this_cpu(scheduler)->current, this_cpu(scheduler)->current, next);
}