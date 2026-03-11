*
 * Exploring the /proc virtual filesystem in memory
 */

/* for init and exit function */
#include <linux/init.h>
/* For module definitions and macros*/
#include <linux/module.h>
/* For printk()*/
#include <linux/kernel.h>

/* For copy_from_user() and copy_to_user()*/
#include <linux/uaccess.h>
/* For kmalloc() and kfree()*/
#include <linux/slab.h>
/* For procfs defintions and types*/
#include <linux/proc_fs.h>
/* For memset()*/
#include <linux/string.h>

MODULE_AUTHOR("Ahmed I. Bashir");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Exploring proc filesystem");
MODULE_VERSION("1.0");


#define FILENAME "procfs_file.txt"
#define MAX_LEN 1024

static char *kernel_buffer;

static ssize_t myread( struct file * file,  char __user *user_buf, size_t len , loff_t* offset_ptr ){

    return simple_read_from_buffer(user_buf, len, offset_ptr, kernel_buffer, strlen(kernel_buffer));

}
static ssize_t mywrite( struct file * file, const char __user *user_buf, size_t len, loff_t* offset_ptr ){

    if( len > (size_t)MAX_LEN ){
        len = MAX_LEN;
    }

    memset(kernel_buffer, 0 , MAX_LEN);

    if( copy_from_user(kernel_buffer, user_buf,len) ){
        printk(KERN_INFO "copy_from_user: returning %d\n", -EFAULT);
        return -EFAULT;
    }
    /* Extra kernel info*/
    if( (len + 0x40 <= MAX_LEN) &&
        snprintf(kernel_buffer + strlen(kernel_buffer), 0x40 , "\nAddress of kernel Buffer: %p\e[0m\n", kernel_buffer) > 0){
        len += 0x40;
    }
    return len;
}
static const struct proc_ops ops = {
    .proc_read = myread,
    .proc_write = mywrite,
};

static int  procfs_init(void) __init;
static void  procfs_exit(void) __exit;


static int __init procfs_init(void){
    kernel_buffer = kmalloc(MAX_LEN, GFP_KERNEL);
    if( kernel_buffer == NULL){
        printk(KERN_WARNING "/proc/%s file kernel buffer not allocated\n", FILENAME);
        return -ENOMEM;
    }
    proc_create(FILENAME,0666, NULL, &ops);

    printk(KERN_INFO "/proc/%s file created\n", FILENAME);
    return 0;
}

static void __exit procfs_exit(void){
    remove_proc_entry(FILENAME, NULL);

    kfree(kernel_buffer);

    printk(KERN_INFO "/proc/%s file removed\n", FILENAME);
}

/* Register the init and exit functions in the kernel */
module_init(procfs_init);
module_exit(procfs_exit);
