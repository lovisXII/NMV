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
		

// #define TEST_CUSTOM

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

	uint64_t *tmp = (uint64_t*) pm1; 
	
	for(int i = 0; i < 512; i++)
	{
		//Doing a mask to check if the page is valid
		if(PGT_IS_VALID(tmp[i])) 
		{
			printk("PML%d which is adress 0x%lx contains: PML%d[%d] 0x%lx\n",lvl,tmp,lvl,i,tmp[i]);
			
			// If it's a huge page, and it's not a last level, we don't print it
			if(lvl > 2 || (lvl > 1 && !	PGT_IS_HUGE_PAGE(tmp[i])))
				print_pgt((PGT_ADRESS(tmp[i])),lvl-1);
		}
	}
	
}

__attribute__((noreturn))
void main_multiboot2(void *mb2)
{
	#ifndef TEST_CUSTOM
		struct task fake;
		paddr_t		new;
		fake.pgt = store_cr3();
		new = alloc_page();
	#endif

	#ifdef TEST_CUSTOM
		struct task* fake_task;
		fake_task->pgt = alloc_page();
		fake_task->load_paddr=0x210000;

		fake_task->load_vaddr=0x128000;
		fake_task->bss_end_vaddr = 0x12C000;

		struct task os;
		os.pgt = store_cr3();
		paddr_t random_adr = alloc_page();
		vaddr_t vadr_os = 0x40000000;
	#endif


	clear();                                     /* clear the VGA screen */
	
	printk("Rackdoll OS\n-----------\n\n");                 /* greetings */
	
	#ifdef TEST_CUSTOM
		printk("cr3 value is 0x%lx\n",store_cr3());
		printk("[Debug] load_vaddr : 0x%lx\n", fake_task->load_vaddr);
		printk("[Debug] load_paddr : 0x%lx\n", fake_task->load_paddr);
	#endif

	setup_interrupts();                           /* setup a 64-bits IDT */
	setup_tss();                                  /* setup a 64-bits TSS */
	

	interrupt_vector[INT_PF] = pgfault;      /* setup page fault handler */
	
	print_pgt(store_cr3(),4);
	
	uint64_t address = 0x201000;

	printk("\n\nMaping address 0x%lx onto 0x%lx\n\n",address,store_cr3());
	
	#ifdef TEST_CUSTOM 
		map_page(&os, vadr_os, random_adr);
	#endif

	#ifndef TEST_CUSTOM
		map_page(&fake, address, new);
	#endif

	printk("\n\nPrinting page table for fake task after mapping\n\n");

	#ifdef TEST_CUSTOM
		print_pgt(os.pgt, 4);
	#endif

	#ifndef TEST_CUSTOM
		print_pgt(fake.pgt, 4);
	#endif

	remap_pic();               /* remap PIC to avoid spurious interrupts */
	disable_pic();                         /* disable anoying legacy PIC */
	sti();     

	#ifdef TEST_CUSTOM
		load_task(fake_task);
		printk("fake task cr3 is :0x%lx\n",fake_task->pgt);
		printk("Current CR3 is :0x%lx\n",store_cr3());
		// die();
		set_task(fake_task);
		printk("\n\nPrinting fake_test to check if copy of kernel is ok\n\n");
		// print_pgt(fake_task->pgt, 4);
	#endif

	#ifndef TEST_CUSTOM 
		printk("\n\nLoading\n\n");
		load_tasks(mb2);                         /* load the tasks in memory */
		printk("\n\n Running task\n\n");
		run_tasks();                                 /* run the loaded tasks */
	#endif

	printk("\nGoodbye!\n");                                 /* fairewell */
	die();                        /* the work is done, we can die now... */
}
