#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/moduleparam.h>
#include <linux/proc_fs.h>
#include <linux/string.h>

/* For copy_from_user() and copy_to_user()*/
#include <linux/uaccess.h>
/* For kmalloc() and kfree()*/
#include <linux/slab.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Ahmed I. Bashir");
MODULE_VERSION("1.1");
MODULE_DESCRIPTION("Parameterized Module to generate files under /proc");

#define MAX_LEN 4096

static char *fileName = NULL;   /* Filename under /proc */
static char *dirName = NULL;    /* Parent directory under /proc. If NULL, parent dir is /proc itself */
static int bufSize = 2048;      /* Default initial size for file's kernel buffer */

static int data_len = 0;        /* Used to store current data length written by user instead of strlen as user may not include a NULL teminator*/

static char *kernel_buffer = NULL;
static struct proc_dir_entry *proc_file_entry = NULL;
static struct proc_dir_entry *proc_folder_entry = NULL;

module_param(fileName, charp, S_IRUSR|S_IWUSR | S_IRGRP);
module_param(dirName, charp, 0640);
/* module_param(bufSize, int , 0640); */
/* Custom setter that blocks changes */

static int set_bufSize(const char *val, const struct kernel_param *kp)
{
    /* * If the module is already loaded, we return an error
     * to prevent the parameter from being changed.
     */
    if (kernel_buffer != NULL) {
        pr_warn("bufSize is locked and cannot be changed after initialization!\n");
        return -EPERM; // "Operation not permitted"
    }

    /* During initial load, kernel_buffer is NULL, so we allow the assignment */
    return param_set_int(val, kp);
}

/* Standard getter */
static const struct kernel_param_ops bufSize_ops = {
    .set = set_bufSize,
    .get = param_get_int,
};

/* Use the callback macro instead of module_param */
module_param_cb(bufSize, &bufSize_ops, &bufSize, 0640);

static ssize_t myread( struct file * file,  char __user *user_buf, size_t len , loff_t* offset_ptr ){

    if (!offset_ptr || len < 1 ) return -1;
    else if ( (*offset_ptr) < 0 ) return -2; // reading before kernel buffer
    else if ( offset_ptr && (*offset_ptr) >= data_len ) return 0; // EOF reached
    return simple_read_from_buffer(user_buf, len, offset_ptr, kernel_buffer, data_len);

}
static ssize_t mywrite( struct file * file, const char __user *user_buf, size_t len, loff_t* offset_ptr ){

    if( len >= (size_t)bufSize ){
        len = bufSize - 1;
    }

    memset(kernel_buffer, 0 , bufSize);

    if( copy_from_user(kernel_buffer, user_buf,len) ){
        printk(KERN_INFO "copy_from_user: returning %d\n", -EFAULT);
        return -EFAULT;
    }
    /* Extra kernel info */
    if( (len + 0x40 < bufSize) &&
        snprintf(kernel_buffer + len, 0x40 , "\nAddress of kernel Buffer: %p\e[0m\n", kernel_buffer) > 0){
        len += 0x40;
    }
    data_len = len;
    return len;
}
static const struct proc_ops ops = {
    .proc_read = myread,
    .proc_write = mywrite,
};


static int __init module_init_func(void){

    #define fileSize  bufSize

    if ((fileSize < 1 || fileSize > MAX_LEN) && fileName){
        printk(KERN_WARNING "/proc[/?]/%s : kernel buffer size is invalid, must be in [1,%d] bytes\n", fileName, MAX_LEN);
        return -1;
    }

    if ( fileName == NULL ){
        printk(KERN_WARNING "/proc[/?]/? : /procfs Module file is empty\n", fileName);
        return -2;
    }


    if (dirName != NULL){
        proc_folder_entry = proc_mkdir(dirName, NULL);
        if (proc_folder_entry == NULL) {
            printk(KERN_ERR "Failed to create /proc/%s entry\n", dirName);
            return -ENOMEM;
        }

    }

    kernel_buffer = kmalloc(fileSize, GFP_KERNEL);
    if( kernel_buffer == NULL){
        printk(KERN_WARNING "/proc/%s file kernel buffer not allocated, returning -ENOMEM = %d\n", fileName, -ENOMEM);
        return -ENOMEM;
    }

    proc_file_entry = proc_create(fileName, 0640, proc_folder_entry, &ops);
    unsigned char flag = ( ((proc_file_entry != NULL)<<0) | ((dirName != NULL)<<1) );

    switch (flag) {
        case 0x1 :{
            printk(KERN_INFO "/proc/%s file created\n"  ,  fileName);
            break;
        }
        case 0x3:{
            printk(KERN_INFO "/proc/%s/%s file created\n" , dirName ,  fileName);
            break;
        }
        case 0x0:{
            printk(KERN_WARNING "\e[31mError creating /proc/%s !!\e[0m\n"  ,  fileName);
            goto remove_buffer ;
        }
        case 0x2:{
            printk(KERN_WARNING "\e[31mError creating /proc/%s/%s !!\e[0m\n"  , dirName,  fileName);
            goto remove_dir;
        }
        default:{
            return -EINVAL;
        }

    }
    return 0;

    remove_dir :
                    proc_remove(proc_folder_entry);
                    proc_file_entry = NULL;
                    printk(KERN_INFO "/proc/%s directory removed\n", dirName);

    remove_buffer:  kfree(kernel_buffer); kernel_buffer = NULL; printk(KERN_INFO "Kernel buffer freed\n");
    return -EINVAL;
}
static void __exit module_exit_func(void){
    remove_proc_entry(fileName, proc_folder_entry);

    kfree(kernel_buffer);

    if( dirName ){
        printk(KERN_INFO "/proc/%s/%s file removed\n" , dirName ,  fileName);
        /* Remove the directory when the module is unloaded */
        if (proc_folder_entry) {
            proc_remove(proc_folder_entry);
            printk(KERN_INFO "/proc/%s directory removed\n", dirName);
        }
    }
    else{
        printk(KERN_INFO "/proc/%s file removed\n"  ,  fileName);
    }

}


module_init(module_init_func);
module_exit(module_exit_func);
