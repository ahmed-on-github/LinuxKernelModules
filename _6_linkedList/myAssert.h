#ifndef __MY_ASSERT_H
#define __MY_ASSERT_H

#include <linux/kernel.h>



#ifdef assert
#undef assert
#endif /* assert */

#ifdef NDEBUG
#define assert(expr) ((void)0)
#else
#define assert(expr)    \
do{                     \
    if(!(expr)){        \
       printk(KERN_ERR "Assertion failed: file: (%s), line: (%d), expression: (%s) evaluated to false\n", __FILE__, __LINE__, #expr );\
       return -1;       \
    }                   \
}while(0);
#endif /* NDEBUG */
#endif /* __MY_ASSERT_H */
