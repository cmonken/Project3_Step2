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
static struct task_struct *thread_tm;    // Thrashing monitoring task
int threshold_count = 0;                 // Thrashing threshold count
struct page this_page;

/* set the thrashing threashold count */
static int set_threshold(int threshold_count)
{
    int one_percent;
    int pages_mem;
    
    printk(KERN_INFO "The system ram size is %lu pages\n", totalram_pages);
    one_percent = (int) totalram_pages / 100;
    printk(KERN_INFO "One percent of system ram is  %d pages\n", one_percent);
    threshold_count = one_percent * threshold;
    printk(KERN_INFO "Thrashing threshold is %d pages\n", threshold_count);
    return 0;
}

/* Thrashing monitor - kernel thread */
static int thrashing_monitor(void *unused)
{
    struct task_struct *task;
    int wss = 0;                      // current process working set counter
    int twss = 0;                     // total working set counter

    while (!kthread_should_stop())
    {
        twss = 0;                     // reset total working set counter
        for_each_process(task)
	    {
	        wss = 0;                  // reset current task working set counter

/* curr->pte is place holder for the currently accessed pte during the page
 * table walk.  It needs to be changed to properly access the current pte when
 * adding the page talbe walk code.  The if statement checks if the current pte
 * was read since last check, increments the process wss counter if so and then
 * clears the accessed bit in preparation for the next check.
 */
	        /**  begin process page table walk  **/
//	        if (pte_young(curr->pte))        // if curr pte was recently accessed
//            {
//                wss++;                     // increment the working set counter
//                pte_makeold(curr->pte);    // clear the acessed bit 
//            }
            /**  end of page table walk  **/

            if (wss < 0)              // if curent process has a wss
            {
                printk(KERN_INFO "[%d]:[%d]\n", task->pid, wss);    // print wss
                twss = twss + wss;    // add process wss to total wss
            }
            if (twss < threshold_count)
                printk(KERN_INFO "Kernel Alert! System Thrashing");
        }
        msleep(1000);                 // thread should sleep for 1 second
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

