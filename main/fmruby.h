#pragma once

#include "fabgl.h"

#define FMRB_VERSION "0.6.0"
#define FMRB_RELEASE "2020/02/15"

#define FMRB_MAIN_TASK_STACK_SIZE (8192*2)

void* fmrb_spi_malloc(size_t size);
void* fmrb_spi_realloc(void* ptr, size_t size);
void fmrb_free(void* ptr);

#define OVERLOAD_SPI_ALLOCATOR \
  void* operator new(std::size_t size){\
  void* ptr = fmrb_spi_malloc(size);\
    if(ptr == nullptr) { throw std::bad_alloc(); }\
    return ptr; }\
  void operator delete(void* ptr){ fmrb_free(ptr); }



#define FMRB_BITMAP_HEADER_SIZE (4)

#define ENABLE_FMRB_LOG

enum class FMRB_LOG{
  DEBUG=0,
  MSG,
  INFO,
  ERR,
  RAW,
};

enum class FMRB_RCODE{
  DEVICE_ERROR=-100,
  MEMALLOC_ERROR,
  ERROR=-1,
  OK=0,
  OK_DONE,
};

void fmrb_debug_print(FMRB_LOG lv,const char *fmt,const char* func,int line,...);
void fmrb_dump_mem_stat();

#ifdef ENABLE_FMRB_LOG
  #define FMRB_DEBUG(lv,fmt, ...) fmrb_debug_print(lv,fmt, __func__ , __LINE__ , ##__VA_ARGS__ )
#else
  #define FMRB_DEBUG(...)
#endif
