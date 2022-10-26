#include <idt.h>                            /* see there for interrupt names */
#include <memory.h>                               /* physical page allocator */
#include <printk.h>                      /* provides printk() and snprintk() */
#include <string.h>                                     /* provides memset() */
#include <syscall.h>                         /* setup system calls for tasks */
#include <task.h>                             /* load the task from mb2 info */
#include <types.h>              /* provides stdint and general purpose types */
#include <vga.h>                                         /* provides clear() */
#include <x86.h>                                    /* access to cr3 and cr2 */

#define PGT_ADRESS_MASK 		0xFFFFFFFFFF000

__attribute__((noreturn))
void die(void)
{
	/* Stop fetching instructions and go low power mode */
	asm volatile ("hlt");

	/* This while loop is dead code, but it makes gcc happy */
	while (1)
		;
}

void print_pgt(paddr_t pm1, uint8_t lvl)
{
	if(lvl < 1)
		return ;
	paddr_t *tmp = pm1; 
	for(int i = 0; i < 512; i++)
	{
		if(tmp[i] & 0x1 == 1)
		{
			printk("PML%d : 0x%lx\n",lvl,tmp[i]);
			print_pgt((tmp[i] & PGT_ADRESS_MASK)>>12,--lvl);
		}
	}
	
}

__attribute__((noreturn))
void main_multiboot2(void *mb2)
{
	struct task fake;
	paddr_t		new;
	fake.pgt = store_cr3();
	new = alloc_page();

	clear();                                     /* clear the VGA screen */
	printk("Rackdoll OS\n-----------\n\n");                 /* greetings */

	setup_interrupts();                           /* setup a 64-bits IDT */
	setup_tss();                                  /* setup a 64-bits TSS */
	interrupt_vector[INT_PF] = pgfault;      /* setup page fault handler */
	print_pgt(store_cr3(), 4);
	// map_page(&fake, 0x201000, new);
	remap_pic();               /* remap PIC to avoid spurious interrupts */
	disable_pic();                         /* disable anoying legacy PIC */
	sti();     

	load_tasks(mb2);                         /* load the tasks in memory */
	run_tasks();                                 /* run the loaded tasks */

	printk("\nGoodbye!\n");                                 /* fairewell */
	die();                        /* the work is done, we can die now... */
}
