#ifndef MISC_MEMORY_CONTROL_H
#define MISC_MEMORY_CONTROL_H

#define MEMORY_ALLOCATE_MAX_TIMES           200

void *omalloc(char *, size_t);
int ofree(void *);
void mem_show(void);


#endif
