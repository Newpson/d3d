#include <stddef.h>

/* Single pool model */
size_t memory_allocate(size_t nbytes);
void * memory_take(size_t nbytes);
void memory_free(void);

/* extra */
size_t memory_usage(void);
size_t memory_capacity(void);
size_t memory_available(void);
