/*
 * filename: device1_module.c
 */

/*----------------------------- Header Files Inclusion -----------------------------*/

#include <linux/init.h>         /* For init and exit functions */
#include <linux/module.h>       /* For module macros */
#include <linux/kernel.h>       /* for printk() or pr_info() ... */

#include <linux/moduleparam.h>  /* For module parameters */

#include <linux/string.h>       /* For kernel string functions */

#include <linux/kdev_t.h>       /* For device number type and allocation functions */
#include <linux/fs.h>           /* For file_operations struct */

#include <linux/cdev.h>         /* For the struct cdev */

#include <linux/uaccess.h>      /* For accessing user space data functions copy_to/from_user() */

#include <linux/sysfs.h>
#include <linux/kobject.h>

#define STRINGIZE(X) #X
#define STRCAT(a, b) a##b
/*----------------------------- Module Macros  -----------------------------*/

MODULE_DESCRIPTION("A Module for a device file /dev/mydevicefile1");
MODULE_AUTHOR("Ahmed I. Bashir");
MODULE_LICENSE("GPL");
MODULE_VERSION("1.0");

/*----------------------------- Init and exit functions Declaration -----------------------------*/

static int  device1_module_init(void) __init;
static void  device1_module_exit(void) __exit;

/*----------------------------- Device Number -----------------------------*/

#define start_minor minor_num
/* Starting minor device number parameter */
static unsigned int minor_num = 0 ;
/* This will create  sysfs entry (/sys/module/device1_module/parameters/minor_num)  */
module_param(minor_num, uint, S_IWUSR|S_IRUSR);

/* Dynamic Major and minor device numbers */
static dev_t dynamic_dev_num = 0;

/*/*----------------------------- Cdev struct -----------------------------*/
static struct cdev device1_cdev ;

/*----------------------------- Device File Operations -----------------------------*/

/* These Functions may be ommitted, but opening and closing the device file
   will not notify the kernel module */
static int dev_open     (struct inode *inode_prt, struct file *file_ptr);
static int dev_release  (struct inode *inode_prt, struct file *file_ptr);

/* These functions will use copy_to/from_user() functions */
static ssize_t dev_read (struct file *file_ptr, char __user *buf,   size_t len, loff_t *off_ptr);

/** We write via echo "Text here" | sudo tee /dev/mydevicefile1 /dev/null */
static ssize_t dev_write(struct file *file_ptr, const char  *buf,   size_t len, loff_t *off_ptr);

/* File operation structure  */
static struct file_operations fops = {
    .owner      = THIS_MODULE,

    .open       = dev_open,
    .release    = dev_release,

    .read       = dev_read,
    .write      = dev_write,
    .llseek     = default_llseek,
};

/* -----------------------------Device file buffer -----------------------------  */
#define BUF_LEN (1UL<<12)

static const size_t max_data_size = BUF_LEN; /* Used to expose kernel info under /sysfs */
static char kern_buf [BUF_LEN]; /* 4 KB buffer*/
static unsigned long int current_data_size = 0;


/* ----------------------------- Exposing module parameters via SysFS ----------------------------- */
static long int sysfs_test_rw_var = 0;
#define SYSFS_DIR ((const char *)"mydevicefile1_sysfs")

#define DECLARE_SYSFS_SHOW_FUNC(VARNAME) \
static ssize_t STRCAT(sysfs_show_,VARNAME)   (struct kobject *dir_ref, struct kobj_attribute *attr_ref, char         *read_buf)

#define DECLARE_SYSFS_STORE_FUNC(VARNAME) \
static ssize_t STRCAT(sysfs_store_,VARNAME)   (struct kobject *dir_ref, struct kobj_attribute *attr_ref, const char   *write_buf, size_t count)


/*This is syntactically wrong as we can not use #if inside a #define*/
/**
#define DEFINE_SYSFS_SHOW_FUNC(VARNAME,VARSIZE) \
static ssize_t sysfs_show_(VARNAME)   (struct kobject *dir_ref, struct kobj_attribute *attr_ref, char         *read_buf){
    #if VARSIZE == 8  \
        return sprintf(read_buf,"%lld", ((long long int)(VARNAME))); \
    #elif VARSIZE == 4 \
        return sprintf(read_buf,"%ld", ((long int)(VARNAME))); \
    #elif VARSIZE == 2 \
        return sprintf(read_buf,"%lh", ((short int)(VARNAME))); \
    #else \
        return sprintf(read_buf,"%c", ((char)(VARNAME))); \
    #endif \
}
*/

#define DEFINE_SYSFS_SHOW_FUNC(VARNAME) \
DECLARE_SYSFS_SHOW_FUNC(VARNAME){                                                                   \
    if( dir_ref && attr_ref ){                                                                      \
        switch(sizeof(VARNAME)){                                                                    \
            case(8):         return sprintf(read_buf,"%lld",((long long int)(VARNAME)));    break;  \
            case(4):         return sprintf(read_buf,"%ld", ((long int)(VARNAME)));         break;  \
            case(2):         return sprintf(read_buf,"%hd", ((short int)(VARNAME)));        break;  \
            default:         return sprintf(read_buf,"%c",  ((char)(VARNAME)));             break;  \
        }                                                                                           \
    }                                                                                               \
    else{                                                                                           \
        return -1;                                                                                  \
    }                                                                                               \
    return 0;                                                                                       \
}


#define DEFINE_SYSFS_STORE_FUNC(VARNAME) \
DECLARE_SYSFS_STORE_FUNC(VARNAME){                                                             \
    if( dir_ref && attr_ref ){                                                                 \
        switch(sizeof(VARNAME)){                                                               \
            case(8):         sscanf(write_buf, "%lld",((long long int *)(&(VARNAME))));  break;\
            case(4):         sscanf(write_buf, "%ld", ((long int *)(&(VARNAME))));       break;\
            case(2):         sscanf(write_buf, "%hd", ((short int *)(&(VARNAME))));      break;\
            default:         sscanf(write_buf, "%c",  ((char *)(&(VARNAME))));           break;\
        }                                                                                      \
    }                                                                                          \
    else{                                                                                      \
        return -1;                                                                             \
    }                                                                                          \
    return count;                                                                              \
}



/* Create sysfs kernel object for file(s) directory */
static struct kobject *sysfs_dir_ref =  NULL;

/* Declare  the read (show) and store (write) functions */
DECLARE_SYSFS_SHOW_FUNC(current_data_size);
DECLARE_SYSFS_SHOW_FUNC(max_data_size);

DECLARE_SYSFS_SHOW_FUNC(sysfs_test_rw_var);
DECLARE_SYSFS_STORE_FUNC(sysfs_test_rw_var);

static struct kobj_attribute current_data_size_attr  = __ATTR(current_data_size, 0440, sysfs_show_current_data_size, NULL);
static struct kobj_attribute max_data_size_attr      = __ATTR(max_data_size,     0440, sysfs_show_max_data_size, NULL);

static struct kobj_attribute sysfs_test_rw_var_attr  = __ATTR(sysfs_test_rw_var, 0660, sysfs_show_sysfs_test_rw_var, sysfs_store_sysfs_test_rw_var);

static struct kobj_attribute* kobj_attribute_arr [3] = { &current_data_size_attr, &max_data_size_attr, &sysfs_test_rw_var_attr };
static const char* kobj_attribute_name_arr       [3] = { STRINGIZE(current_data_size_attr), STRINGIZE(max_data_size), STRINGIZE(sysfs_test_rw_var) };
/* ----------------------------- Init and exit functions Implementation -----------------------------*/


static int __init device1_module_init(){
    int ret = 0 ;
    /* Module Parameters */
    printk(KERN_INFO "Passed starting minor_num = %d  \n", minor_num);

    /* Device Minor and Major number dynamic allocation*/
    if ( alloc_chrdev_region(&dynamic_dev_num, start_minor, 1, "MyDeviceName") < 0 ){
        pr_err("Can not allocate device major number\n");
        return -1;
    }
    printk(KERN_INFO "Major device number = %d, minor device number = %d\n",MAJOR(dynamic_dev_num), MINOR(dynamic_dev_num));

    /* Device File cdev struct initialization*/
    cdev_init(&device1_cdev, &fops);

    /* Device File cdev struct  insertion into kernel*/
    if(cdev_add(&device1_cdev, dynamic_dev_num, 1) < 0){
        pr_err("Can not add the device to the kernel\n");
        ret = -1;
        goto unregister_dev_num;
    }
    /* Create kobject for sysfs parent directory of sysfs data files */
    sysfs_dir_ref = kobject_create_and_add(SYSFS_DIR,NULL ); /* creates /sys/mydevicefile1_sysfs dir */
    if(sysfs_dir_ref == NULL){
        printk(KERN_INFO "Can not create sysfs directory \"/sys/%s\"\n", SYSFS_DIR);
        ret = -2;
        goto free_kobj;
    }

    /* Create sysfs files under directory specified by sysfs_dir_entry kobj from attributes */
    char i = 0;
    for( i = 0 ; i < 3 ; i++){
        if( sysfs_create_file( sysfs_dir_ref , &(kobj_attribute_arr[i]->attr) ) ){
            printk(KERN_INFO "Can not create sysfs file \"/sys/%s/%s\"\n", SYSFS_DIR, kobj_attribute_name_arr[i]);

            if(i > 0){
                char j = 0;
                for(j = i-1 ; j >=0 ; j--){
                    sysfs_remove_file( sysfs_dir_ref, &(kobj_attribute_arr[j]->attr) );
                }
            }
            ret = -3;
            goto free_kobj;
        }
    }

    printk(KERN_INFO "Kernel Module Inserted Successfully...\n");
    return 0;

free_kobj:
    kobject_put(sysfs_dir_ref);
unregister_dev_num:
    unregister_chrdev_region(dynamic_dev_num, 1);
    return ret;
}

static void __exit device1_module_exit(){
    char i = 0 ;
    for(i = 0 ; i < 3 ; i++){
        sysfs_remove_file( sysfs_dir_ref, &(kobj_attribute_arr[i]->attr) );
    }
    kobject_put(sysfs_dir_ref);
    cdev_del(&device1_cdev);
    unregister_chrdev_region(dynamic_dev_num, 1);
    printk(KERN_INFO "Kernel Module Removed Successfully...\n");

}

/*----------------------------- Init and exit functions Registeration -----------------------------*/

module_init(device1_module_init);
module_exit(device1_module_exit);

/*----------------------------- Device File Operations Implementation -----------------------------*/

/* These Functions may be ommitted, but opening and closing the device file
   will not notify the kernel module */
static int dev_open     (struct inode *inode_prt, struct file *file_ptr){
    pr_info("Device file /dev/mydevicefile1 opened\n");
    return 0;

}
static int dev_release  (struct inode *inode_prt, struct file *file_ptr){
    pr_info("Device file /dev/mydevicefile1 closed/released\n");
    return 0;

}

/* These functions will use copy_to_user()/copy_from_user() functions */
static ssize_t dev_read (struct file *file_ptr, char __user *buf,   size_t len, loff_t *off_ptr){
    printk("Before: len = %lu , offset = %lld\n", len, *off_ptr);

    // 1. Check if the offset is already at or past the end of our data
    if (*off_ptr >= current_data_size) {
        return 0; // Return 0 to indicate End of File (EOF)
    }

    // 2. Adjust 'len' so we don't read past the end of the buffer
    if (*off_ptr + len > current_data_size) {
        len = current_data_size - *off_ptr;
    }

    // 3. Copy data to user space starting from the current offset
    if (copy_to_user(buf, kern_buf + *off_ptr, len)) {
        return -EFAULT;
    }

    // 4. Update the offset!
    // This is how 'head -n 4' knows where to stop and where
    // the next read would start.
    *off_ptr += len;
    printk("After: len = %lu , offset = %lld\n", len, *off_ptr);

    pr_info("Device file /dev/device1_file dummy read\n");

    return len; // Return the number of bytes actually read
    // return 0; /*This means EOF and no more reads*/
}

/** We write via echo "Text here" | sudo tee /dev/mydevicefile1 /dev/null */
static ssize_t dev_write(struct file *file_ptr, const char __user *buf,   size_t len, loff_t *off_ptr){
    ssize_t temp = 0 ;

    /* 1- shift the kernel tracking offset to the end of data if we are to append*/
    if( file_ptr->f_flags & O_APPEND ){
        printk("Appending to device file\n");
        *off_ptr /* initially = 0 after each .open() call */ = current_data_size ;
    }
    else{ /* Just overwrting */
        printk("Just writing to device file\n");
        memset(kern_buf, 0, BUF_LEN);         /* Clear the kernel buffer*/
        *off_ptr = 0;
    }
    printk("Before: len = %lu , offset = %lld\n", len, *off_ptr);

    if(*off_ptr >= BUF_LEN){
        printk(KERN_ERR "String to write passes the buffer\n");
        return -ENOSPC;
    }

    /* 2- Truncate data if buffer overflow is detected */
    if( *off_ptr + len >= BUF_LEN ){
        len = BUF_LEN - *off_ptr;
    }
    /* If user-space sends 0 bytes or no memory left (len will be 0), just get out immediately -- never return a 0*/
    if (len == 0) {
        printk(KERN_ERR "String to write length is 0\n");
        return -EINVAL - ENOMEM;
        len = strlen(buf);
    }

    /* 3- Copy data to kernel buffer */
    if( (temp = copy_from_user(kern_buf + *off_ptr, buf, len)) ){
        printk(KERN_ERR"Failed to write %ld bytes out of %ld bytes to device file\n", temp, len);
        return -EFAULT;
    }
    else{


    }
    /* 4- CRITICAL: Update kernel tracking offset -- Critical if user space opens file in O_RDWR mode and multiple reads and writes will occur in the same open() syscall*/
    *off_ptr += len;
    kern_buf[*off_ptr ] = '\0';
    printk("After: len = %lu , offset = %lld\n", len, *off_ptr);
    printk("Kernel buffer: \"%s\"\n",kern_buf);
    printk("Kernel buffer: \"%s\"\n",kern_buf);
    /* 5- Update the internal tracking offset*/
    if( *off_ptr > current_data_size ){ /*Not always true, only true when we are appending or overwritting to a new bigger buffer size*/
        current_data_size = *off_ptr;
    }
    pr_info("Device file /dev/mydevicefile1  write\n");

    return len; /*this means writing all data passed into the kernel buffer*/
}

/* ----------- Implement  the read (show) and store (write) functions ----------- */
DEFINE_SYSFS_SHOW_FUNC(current_data_size);
DEFINE_SYSFS_SHOW_FUNC(max_data_size);

DEFINE_SYSFS_SHOW_FUNC(sysfs_test_rw_var);
DEFINE_SYSFS_STORE_FUNC(sysfs_test_rw_var);
