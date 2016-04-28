/** TO DO **/
/* add page table walk for each process */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/kthread.h>
#include <linux/sched.h>
#include <asm/pgtable.h>
#include <linux/mm.h>
#include <linux/delay.h>

#define threshold 90;                    // Thrashing theshold percentage
static struct task_struct *thread_tm;    // Thrashing monitoring task pointer
unsigned long threshold_count;           // Thrashing threshold count
struct page this_page;

/* set the thrashing threashold count */
static int set_threshold(int threshold_count)
{
    unsigned long one_percent;

    printk(KERN_INFO "The system ram size is %lu pages\n", totalram_pages);
    one_percent = totalram_pages / (unsigned long) 100;
    printk(KERN_INFO "One percent of system ram is  %lu pages\n", one_percent);
    threshold_count = one_percent * (unsigned long) threshold;
    printk(KERN_INFO "Thrashing threshold is %lu pages\n", (unsigned long) threshold_count);
    return 0;
}

/* copied from /arch/x86/mm/pgtable.c */
/* checks if the curr pte was recently accessed then clears the accessed bit */
int ptep_test_and_clear_young(struct vm_area_struct *vma,
                  unsigned long addr, pte_t *ptep)
{
    int ret = 0;

    if (pte_young(*ptep))
        ret = test_and_clear_bit(_PAGE_BIT_ACCESSED,
                     (unsigned long *) &ptep->pte);

    if (ret)
        pte_update(vma->vm_mm, addr, ptep);

    return ret;
}

/* Thrashing monitor - kernel thread */
static int thrashing_monitor(void *unused)
{
    struct task_struct *task;
    unsigned long wss = 0;            // current process working set counter
    unsigned long twss = 0;           // total working set counter
//    int young = 0;                    // is page young indicator

    while (!kthread_should_stop())
    {
        twss = 0;                     // reset total working set counter
        for_each_process(task)
        {
            wss = 0;                  // reset current task working set counter
//            printk(KERN_INFO "[%d]:\n", task->pid);    // test print statement

/*
 * from class discussion board thread "Project 3 Step 2 hint":
 *  1. Go through every vma linked by mm_struct->mmap.
 *  2. For each vma, go through every virtual page frame from vma->vm_start
 *     to vma->vm_end.
 *  3. For each virtual page, use your code in task 1 to find the corresponding
 *     pte.
 *  4. If the pte is present, increment WSS if it is young, then clear the bit.
 */
        /**  begin process page table walk  **/
//        young = nt ptep_test_and_clear_young (struct vm_area_struct *vma,
//                                              unsigned long addr, pte_t *ptep)
//            if (young)                   // if curr pte was recently accessed
//                wss++;                   // increment the working set counter
        /**  end of page table walk  **/

            if (wss < 0)                     // if curent process has a wss
            {
                printk(KERN_INFO "[%d]:[%lu]\n", task->pid, wss);   // print wss
                twss = twss + wss;           // add process wss to total wss
            }
            if (twss < threshold_count)
                printk(KERN_INFO "Kernel Alert! System Thrashing");
        }
        msleep(1000);                        // thread should sleep for 1 second
        printk(KERN_INFO "Thread slept for 1 second");    // test print statment
    }
    printk(KERN_INFO "Thrashing monitor thread stopping\n");
    do_exit(0);
    return 0;
}

// Module Initialization
static int __init init_thrashing_detect(void)
{
    set_threshold(threshold_count);
    /* create thread to monitor for system thrashing */
    printk(KERN_INFO "Creating thread\n");
    thread_tm = kthread_run(thrashing_monitor, NULL, "thrashingmonitor");
    if (thread_tm)
    {
        printk(KERN_INFO "Thrashing monitor thread created successfully\n");
    }else{
        printk(KERN_INFO "Thrashing monitor thread creation failed\n");
    }
    return 0;
}

static void __exit exit_thrashing_detect(void)
{
    kthread_stop(thread_tm);
}

module_init(init_thrashing_detect);
module_exit(exit_thrashing_detect);

/* Module info */
MODULE_LICENSE("GPL");
MODULE_VERSION("1.0");
MODULE_AUTHOR("Christopher David Monken, Fan, Martin Kuna");
MODULE_DESCRIPTION("Project 3 part 2 CSE430 - Operating Systems, Spring 2016");

