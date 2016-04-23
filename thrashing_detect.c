/** TO DO **/
/* add page table walk for each process */
/* add get system memory size - should be in discussion boards */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/kthread.h>
#include <linux/sched.h>
#include <asm/pgtable.h>
#include <linux/mm.h>
#include <linux/delay.h>

#define threshold 90;
static struct task_struct *thread_tm;    /* Thrashing monitoring task
int threshold_count = 0;                 // thrashing threshold
struct page this_page;

/* set the thrashing threashold count */
static int set_threshold(int threshold_count)
{
    int sys_mem = 2147483647;            // total sys mem in bytes (2 GB)
//    int mem_mb = 2048                    // total sys mem in MB (2 GB)
//    int mem_kb;
//    int mem_b;
    int one_percent;
    int page_size;
    int pages_mem;
    
//    tm_kb = tm_mb * 1024;                // total sys mem in KB
//    tm_b = tm_kb * 1024;                 // total sys mem in bytes
    page_size = sizeof(this_page);
    pages_mem = sys_mem / page_size;     // total sys mem in pages
//    pages_mem = tm_b / page_size         // total sys mem in pages
    one_percent = pages_mem / 100;       // 1% of total phy mem in pages
    threshold_count = one_percent * threshold;
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
	        wss = 0;                  // reset wss for the current task
	        
	        /**  walk process' page table  **/
	        
//	        if (pte_young(pte))       // if the pte was recently accessed
	            wss++;                // increment the working set counter
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

