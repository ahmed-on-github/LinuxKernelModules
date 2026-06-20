#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>

//#include <linux/list.h>


#define LIST_TYPE int
#include "listTemplate.h"
#include "myAssert.h"

#define MODULE_NAME "linkedListTestModule"

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Testing linux kernel linked lists");
MODULE_VERSION("1.0");
MODULE_AUTHOR("Ahmed I. Bashir");


static int  __init module_init_func(void);
static void __exit module_exit_func(void);

/* Declare list array*/
static int_list_t list_node_arr [5] = {0};
//assert(0);


static int  __init module_init_func(void){
    assert(1);
    struct list_head *int_list_head = &(list_node_arr[0].node);
    INIT_LIST_HEAD(int_list_head);
    for(int i = 0 ;  i < 5; i++){
        INIT_LIST_HEAD(&(list_node_arr[i].node));
        assert( (list_node_arr[i].node.next == &(list_node_arr[i].node))
        &&      (list_node_arr[i].node.prev == &(list_node_arr[i].node)) );
        /* OR: use printk()*/
        printk("list_node_arr[%d].node.next = %p", i, list_node_arr[i].node.next);
        printk("list_node_arr[%d].node.prev = %p", i, list_node_arr[i].node.prev);
        printk("&list_node_arr[%d].node = %p\n", i, &(list_node_arr[i].node));

    }

    list_node_arr[0].int_data = 5;
    /* Traverse the linked list via list_for_each()*/
    struct list_head *ptr =  int_list_head; //&(list_node_arr[0].node);

    list_for_each(ptr, int_list_head){ /* Data is head (5) is not printed !!*/
        printk("ptr->int_data = %d\n", container_of(ptr,int_list_t,node)->int_data);
    }


    list_node_arr[1].int_data = 2;
    printk("Inserting %d after head\n", list_node_arr[1].int_data);
    list_add(&(list_node_arr[1].node), int_list_head); /*Insert after head*/
    list_for_each(ptr, int_list_head){ /* Head (5 -- not printed) -> 2 */
        printk("ptr->int_data = %d\n", container_of(ptr,int_list_t,node)->int_data);
    }


    list_node_arr[2].int_data = 7;
    printk("Inserting %d before head\n", list_node_arr[2].int_data);
    list_add_tail(&(list_node_arr[2].node), int_list_head); /*Insert before head (at tail)*/
    list_for_each(ptr, int_list_head){ /* Head (5 -- not printed) -> 2 -> 7 */
        printk("ptr->int_data = %d\n", container_of(ptr,int_list_t,node)->int_data);
    }

    list_node_arr[3].int_data = 4;
    printk("Inserting %d before tail\n", list_node_arr[3].int_data);
    list_add_tail(&(list_node_arr[3].node), int_list_head->prev); /*Insert before head (at tail)*/
    list_for_each(ptr, int_list_head){ /* Head (5 -- not printed) -> 2 -> 4 -> 7 */
        printk("ptr->int_data = %d\n", container_of(ptr,int_list_t,node)->int_data);
    }


    printk("Deleting node after head\n");
    list_del(int_list_head->next); /*delete after head */
    list_for_each(ptr, int_list_head){ /* Head (5 -- not printed) -> 4 -> 7 */
        printk("ptr->int_data = %d\n", container_of(ptr,int_list_t,node)->int_data);
    }

    printk("Deleting node before tail\n");
    list_del(int_list_head->prev->prev); /*delete before tail */
    list_for_each(ptr, int_list_head){ /* Head (5 -- not printed)  -> 7 */
        printk("ptr->int_data = %d\n", container_of(ptr,int_list_t,node)->int_data);
    }

    list_node_arr[1].int_data = 2;
    printk("Inserting %d after head\n", list_node_arr[1].int_data);
    list_add(&(list_node_arr[1].node), int_list_head); /*Insert after head*/
    list_for_each(ptr, int_list_head){ /* Head (5 -- not printed) -> 2 -> 7  */
        printk("ptr->int_data = %d\n", container_of(ptr,int_list_t,node)->int_data);
    }

    printk("Swapping list head previous and next\n");
    list_swap(int_list_head->prev, int_list_head->next );
    list_for_each(ptr, int_list_head){ /* Head (5 -- not printed) -> 7 -> 2 */
        printk("ptr->int_data = %d\n", container_of(ptr,int_list_t,node)->int_data);
    }

    printk("Replacing list tail  with list_node_arr[3]\n");
    list_replace(int_list_head->prev, &(list_node_arr[3].node));
    list_for_each(ptr, int_list_head){ /* Head (5 -- not printed) -> 7 -> 4 */
        printk("ptr->int_data = %d\n", container_of(ptr,int_list_t,node)->int_data);
    }

    list_node_arr[4].int_data = 8;
    list_add(&(list_node_arr[4].node), int_list_head->next); /*Insert after 1st node*/ /* Head (5 -- not printed) -> 7 -> 2 -> 4 */

    /* Testing functions */
    list_for_each(ptr, int_list_head){
        if( list_is_first(ptr, int_list_head)  ){
            printk("%p is 1st node, with data = %d\n", ptr, container_of(ptr,int_list_t,node)->int_data);
        }
        else if( list_is_last(ptr, int_list_head) ){
            printk("%p is last node, with data = %d\n", ptr, container_of(ptr,int_list_t,node)->int_data);
        }
        else{
            printk("%p is middle node, with data = %d\n", ptr, container_of(ptr,int_list_t,node)->int_data);
        }
    }
    printk("list_is_head(int_list_head) = %d, list_is_head(int_list_head->prev) = %d\n",
        list_is_head(int_list_head,int_list_head), list_is_head(int_list_head->prev, int_list_head)
    );

    printk("Module \"%s\" inserted\n", MODULE_NAME);
    return 0;
}
static void __exit module_exit_func(void){
    printk("Module \"%s\" removed\n", MODULE_NAME);
}


module_init(module_init_func);
module_exit(module_exit_func);
