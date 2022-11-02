#include <idt.h>                            /* see there for interrupt names */
#include <memory.h>                               /* physical page allocator */
#include <printk.h>                      /* provides printk() and snprintk() */
#include <string.h>                                     /* provides memset() */
#include <syscall.h>                         /* setup system calls for tasks */
#include <task.h>                             /* load the task from mb2 info */
#include <types.h>              /* provides stdint and general purpose types */
#include <vga.h>                                         /* provides clear() */
#include <x86.h>                                    /* access to cr3 and cr2 */
#include <mask_and_coo.h>
		
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
// pm1 : physical adress of a page lvl
// lvl : level of the page table
{
	if(lvl < 1)
		return ;

	paddr_t *tmp = pm1; 
	
	for(int i = 0; i < 512; i++)
	{
		//Doing a mask to check if the page is valid
		if(PGT_IS_VALID(tmp[i]) && !PGT_IS_HUGE_PAGE(tmp[i])) 
		{
			printk("PML%d : 0x%lx\n",lvl,tmp[i]);
			print_pgt((PGT_ADRESS(tmp[i])),--lvl);
		}
		else if(PGT_IS_HUGE_PAGE(tmp[i])){
			printk("Huge Page : 0x%lx\n",tmp[i]);
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
	
	map_page(&fake, 0x201000, new);
	// print_pgt(0x201000, 4);
	
	remap_pic();               /* remap PIC to avoid spurious interrupts */
	disable_pic();                         /* disable anoying legacy PIC */
	sti();     
	load_tasks(mb2);                         /* load the tasks in memory */
	print_pgt(store_cr3(), 4);
	// run_tasks();                                 /* run the loaded tasks */

	printk("\nGoodbye!\n");                                 /* fairewell */
	die();                        /* the work is done, we can die now... */
}
