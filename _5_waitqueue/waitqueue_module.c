#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/moduleparam.h>
#include <linux/wait.h>
#include <linux/kthread.h>
#include <linux/sched.h>
#include <linux/mutex.h> /* To lock the condition flag */
#include "./waitqueue_module.h"

/* // Unused header files --
#include <linux/fs.h>
#include <linux/kdev_t.h>
#include <linux/string.h>
*/

MODULE_AUTHOR("Ahmed I. Bashir");
MODULE_LICENSE("GPL");
MODULE_VERSION("1.0");
MODULE_DESCRIPTION("Device Driver Testing waitqueues on kernel version 5.14.0.701");


static int      module_init_func(void) __init;
static void     module_exit_func(void) __exit;

/* ------------------- Create a kernel parameter with callback function -------------------*/
/* Certain changes on this variable will trigger a callback function
   that will conditionally wake up a kernel thread
   from sleeping in a wait queue
*/

static int callback_uint_param_set(const char *val, const struct kernel_param *kp);

CREATE_CALLBACK_KERN_PARAM_WINITVAL(callback_uint_param, uint32_t,\
                                    S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP,\
                                    0UL, NULL, callback_uint_param_set );

/* Override initial value for getter function -- can not be done globally !!*/
/* CONCAT(callback_uint_param,_ops).get = param_get_uint; // Standard ulong getter */


/* -------------------Create a waitqueue -------------------*/

static struct task_struct *wait_thread = NULL;
static int condition_flag = 0 ;

DECLARE_WAIT_QUEUE_HEAD(waitq);

static int waiting_thread(void *unused_args);

/* -------------------Create a Mutex -------------------*/
static DEFINE_MUTEX(param_lock);

/*------------------- Functions Implementation -------------------*/

static int  __init module_init_func(void) {
        //Create the kernel thread with name 'mythread'
        int retval = 0;
        wait_thread = kthread_create(waiting_thread, NULL, "WaitThread");
        if (wait_thread) {
                pr_info("Thread Created successfully\n");
                wake_up_process(wait_thread);
                printk("Module inserted successfully\n");
        } else{
            pr_info("Thread creation failed\n");
            retval = -1;
        }
    return retval;
}
static void __exit module_exit_func(void) {
    /* Protect the global variable(s) using the muxtex lock */
    mutex_lock(&param_lock);
    condition_flag = 2;
    /* Release the mutex lock*/
    mutex_unlock(&param_lock);
    wake_up_interruptible(&waitq);
    kthread_stop(wait_thread);
    printk("Module removed successfully\n");
}

static int callback_uint_param_set(const char *val, const struct kernel_param *kp){

    int retval = 0;
    uint32_t tmp_val = 0;
    static uint32_t old_val = 0;

    /* // Unsafe if strings like "125\n" or "abc" are pointed to by val
       // by echo: echo "5" >
    memcpy(tmp_buf, val, 10);
    tmp_buf[10] = '\0';
    retval = sscanf(val,"%u",(uint32_t *)(kp->arg));
    */
    if( (retval = kstrtouint(val, 0, &tmp_val)) < 0){
        printk(KERN_ERR "callback_uint_param_setter: Invalid 32 bit unsigned int val %s\n",val);
        goto ret;
    }

    printk("New \"callback_uint_param\" value = %u\n", tmp_val);
    /* Protect the global variables using the muxtex lock */
    mutex_lock(&param_lock);

    *((uint32_t *)(kp->arg)) = tmp_val;
    /*Conditionally wake the kernel thread up from the wait queue.
      Condition: A falling edge detected on bit 0*/
    if( ((tmp_val&0x01) == 0) && (old_val&0x01)){
        condition_flag = 1;
        wake_up_interruptible(&waitq);
        /* pr_info("waiting_thread: A falling edge detected on bit 0\n"); */
    }
    old_val = tmp_val;
    /* Release the mutex lock*/
    mutex_unlock(&param_lock);
ret:
    return retval;
}
static int waiting_thread(void *unused_args){

    char rmmod_flag = 0;
    while(1){

        pr_info("Waiting for an event or the module gets removed... \n");
        /* Sleep ... */
        wait_event_interruptible(waitq, condition_flag != 0 );
        /* Woken up from another function */
        switch (condition_flag){ /* Set by another functions */
            case 1:
                pr_info("waiting_thread: A falling edge detected on bit 0\n");
                break;
            case 2:
                pr_info("waiting_thread: Exiting as the module is being removed\n");
                rmmod_flag = 1;
                break;
            default:
                printk("waiting_thread: Woken up by a signal, current condition = %d\n", condition_flag);
                break;
        }
        if( rmmod_flag ) break; /*From while(1)*/
        /* Protect the global variable(s) using the muxtex lock */
        mutex_lock(&param_lock);
        condition_flag = 0;
        /* Release the mutex lock*/
        mutex_unlock(&param_lock);
    }
    //do_exit(0);
    return 0;
}

module_init(module_init_func);
module_exit(module_exit_func);
