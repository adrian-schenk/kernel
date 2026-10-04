#include "task.h"
#include "kmalloc.h"
#include "printf.h"
#include "cpu.h"
#include "interrupt.h"
#include "spinlock.h"

uint64_t tid = 0;

task_t *task_create(uint64_t entry)
{
  return _task_create(entry, 0x20 | 0x3, 0x18 | 0x3, (uint64_t)_task_return_r3);
}

task_t *task_create_priv(uint64_t entry, char ss, char cs)
{
  return _task_create(entry, ss, cs, (uint64_t)_task_return);
}

task_t *_task_create(uint64_t entry, char ss, char cs, uint64_t ret) {
  task_t *task = (task_t *)kmalloc(sizeof(task_t));
  task->id = tid++;
  task->task_pml4 = &kernel_pml4;

  if (cs & 0x3)
    task->task_pml4 = pt_alloc_page_phys(1);

  task->rsp_base = (uint64_t)kmalloc(4096);
  uint64_t *stack = ((uint8_t *)task->rsp_base) + 4096;

  if (((uint64_t)stack & 0x10) == 0)
    --stack;

  *--stack = ret; // return address

  *--stack = ss;          // ss
  *--stack = stack + 1; // save rsp for later
  *--stack = 0x202;       // rflags
  *--stack = cs;          // CS
  *--stack = entry;       // return address for when the task is switched to

  --stack;
  --stack; // mock interrupt number and error

  *--stack = 0x00; // %rax
  *--stack = 0x00; // %rbx
  *--stack = 0x00; // %rcx
  *--stack = 0x00; // %rdx
  *--stack = 0x00; // %rsi
  *--stack = 0x00; // %rdi
  *--stack = 0x00; // %rbp
  *--stack = 0x00; // %r8
  *--stack = 0x00; // %r9
  *--stack = 0x00; // %r10
  *--stack = 0x00; // %r11
  *--stack = 0x00; // %r12
  *--stack = 0x00; // %r13
  *--stack = 0x00; // %r14
  *--stack = 0x00; // %r15

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

void task_return(uint64_t a)
{
  //kprintf("task returned, exiting cpu %d and %l with %l\n", this_cpu(cpu_id), this_cpu(scheduler)->current->id, a);
  cli();
  for (int i = 0; i < SCHEDULER_QUEUE_SIZE; i++)
  {
    if (this_cpu(scheduler)->queue[i] && this_cpu(scheduler)->queue[i]->id == this_cpu(scheduler)->current->id)
    {
      kfree(this_cpu(scheduler)->queue[i]->rsp_base);
      this_cpu(scheduler)->queue[i] = (void *)0;
      //this_cpu(scheduler)->count--;
      this_cpu(scheduler)->current = (void *)0;
      break;
    }
  }
  sti();
  for (;;)
    ;
}

void task_switch_to(task_t *next)
{
  _task_switch_to(&this_cpu(scheduler)->current, this_cpu(scheduler)->current, next);
}