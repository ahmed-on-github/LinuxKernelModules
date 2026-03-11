#include <linux/init.h>
/*
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
*/
#include <linux/module.h>
#include <linux/kernel.h>

MODULE_AUTHOR("Ahmed I. Bashir");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Exploring Kernel Log Levels Module");
MODULE_VERSION("1.0");

static int  kernel_mod_init(void) __init;
static void  kernel_mod_exit(void) __exit;

static int __init kernel_mod_init(void){
    printk("Kernel Module Inserted\n"); /*Will be logged with the default log level
                                        inside /proc/sys/kernel/printk*/
    printk(KERN_EMERG "Kernel Emergency Message, KERN_EMERG = '%s'\n", KERN_EMERG);
    printk(KERN_ALERT "Kernel Alert Message, KERN_ALERT = '%s'\n", KERN_ALERT);
    printk(KERN_CRIT "Kernel Critical Message, KERN_CRIT = '%s'\n", KERN_CRIT);
    printk(KERN_ERR "Kernel Error Message, KERN_ERR = '%s'\n", KERN_ERR);
    printk(KERN_WARNING "Kernel Warning Message\n");
    printk(KERN_NOTICE "Kernel Notice Message\n");
    printk(KERN_INFO "Kernel Info Message\n");
    printk(KERN_DEBUG "Kernel Debug Message\n");
    return 0;
}

static void __exit kernel_mod_exit(void){
    printk("Kernel Module Removed\n"); /*Will be logged with the default log level
                                        inside /proc/sys/kernel/printk*/
}

module_init(kernel_mod_init);
module_exit(kernel_mod_exit);
                              
