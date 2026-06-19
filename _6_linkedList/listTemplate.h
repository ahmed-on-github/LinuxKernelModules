/*No file guards here to allow re-inclusion*/
#include <linux/list.h>

#ifndef LIST_TYPE
#error "\"LIST_TYPE\" Must be defined"
#else

#define CAT_HELPER(A,B) A##B
#define CAT(A,B) CAT_HELPER(A,B)
#define LIST_NAME(LIST_TYPE) CAT(LIST_TYPE,_list_t)


typedef struct CAT(LIST_TYPE,_list) {
    LIST_TYPE CAT(LIST_TYPE,_data);
    struct list_head node;
} LIST_NAME(LIST_TYPE)  ;


#undef LIST_NAME
#undef CAT
#undef CAT_HELPER
#undef LIST_TYPE
#endif /* LIST_TYPE */
#undef TYPE
