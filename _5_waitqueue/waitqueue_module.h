#ifndef WAITQUEUE_MODULE_H
#define WAITQUEUE_MODULE_H





/* Callback function and structures */
/*
struct kernel_param_ops{
    // Returns 0, or -errno.  arg is in kp->arg.
    int (*set)(const char *val, const struct kernel_param *kp);

    // Returns length written or -errno.  Buffer is 4k (ie. be short!)
    int (*get)(char *buffer, const struct kernel_param *kp);
    // Optional function to free kp->arg when module unloaded.
    void (*free)(void *arg);
};


struct kernel_param {
	const char *name;
	struct module *mod;
	const struct kernel_param_ops *ops;
	const u16 perm;
	s8 level;
	u8 flags;
	union {
		void *arg;
		const struct kparam_string *str;
		const struct kparam_array *arr;
	};
};
*/

#ifndef EXPAND
#define EXPAND(X) X
#endif /* EXPAND */

#ifndef STRINGIFY
/* The Double-Indirection (Two-Level) Setup */
#ifndef STRINGIFY_1
#define STRINGIFY_1(X) #X
#endif /* STRINGIFY_1 */
#define STRINGIFY(X) STRINGIFY_1(X)
#endif /* STRINGIFY */

#ifndef CONCAT
#ifndef CONCAT_RAW
#define CONCAT_RAW(a,b) a##b
#endif /* CONCAT_RAW */
#define CONCAT(a,b) CONCAT_RAW(a,b)
#endif /* CONCAT */

typedef int (*cb_param_setter_t) (const char *, const struct kernel_param *);
typedef int (*cb_param_getter_t) (char *, const struct kernel_param *);

#define CREATE_CALLBACK_KERN_PARAM_WINITVAL(NAME,TYPE,PERMS,INITVAL,GETTER,SETTER) \
\
TYPE NAME = (INITVAL);\
/* Both must be defined as below*/ \
static int CONCAT(NAME,_setter) (const char *val, const struct kernel_param *kp); \
static int CONCAT(NAME,_getter) (char *val, const struct kernel_param *kp); \
\
static int CONCAT(NAME,_setter) (const char *val, const struct kernel_param *kp){ \
    return ( (SETTER)? (((cb_param_setter_t)(SETTER))(val,kp)) : param_set_uint(val,kp) ); \
} \
static int CONCAT(NAME,_getter) (char *val, const struct kernel_param *kp){ \
    return ( (GETTER)? (((cb_param_getter_t)(GETTER))(val,kp)) : param_get_uint(val,kp) ) ; \
} \
\
static struct kernel_param_ops CONCAT(NAME,_ops)   = { \
    .set = CONCAT(NAME,_setter) , \
    .get = CONCAT(NAME,_getter) , \
}; \
module_param_cb(NAME, &CONCAT(NAME,_ops) , &NAME, (PERMS));

#endif /* WAITQUEUE_MODULE_H */
