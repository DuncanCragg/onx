#ifndef MEM_H
#define MEM_H

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

void*   Mem_alloc(  char* func, int line, size_t n);
void    Mem_free(   char* func, int line, void* p);
char*   Mem_strdup( char* func, int line, const char* s);
void    Mem_freestr(char* func, int line, char* s);

#define mem_alloc(s)   Mem_alloc(  (char*)__FUNCTION__, __LINE__, s)
#define mem_alloc_p(s) Mem_alloc(  func, line, s)
#define mem_free(s)    Mem_free(   (char*)__FUNCTION__, __LINE__, s)
#define mem_strdup(s)  Mem_strdup ((char*)__FUNCTION__, __LINE__, s)
#define mem_freestr(s) Mem_freestr((char*)__FUNCTION__, __LINE__, s)

extern bool mem_fillin_up;

void     mem_strncpy(char* dst, const char* src, size_t count);
uint32_t mem_used();
void     mem_show_allocated(bool clear);


#endif
