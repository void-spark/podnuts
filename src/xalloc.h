#ifndef XALLOC_H
#define XALLOC_H

/* #define DEBUG_MALLOC
#define MALLOC_STATS */

void malloc_init();
void dump_meminfo(UR_OBJECT user);
void dump_malloc_table(UR_OBJECT user);
void *xalloc(size_t size, char *desc);
void xfree(void *r);
void *xrealloc(void *old, size_t size);

#endif /* !XALLOC_H */
