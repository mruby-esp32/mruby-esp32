#pragma once

#include "fabgl.h"

#define FMRB_VERSION "0.6"
#define FMRB_RELEASE "2020/02/15"

void* fmrb_spi_malloc(size_t size);
void* fmrb_spi_realloc(void* ptr, size_t size);

#define ENABLE_FMRB_LOG

enum class FMRB_LOG{
  INFO=0,
  MSG,
  DEBUG,
};

void fmrb_debug_print(FMRB_LOG lv,const char *fmt,...);

#ifdef ENABLE_FMRB_LOG
  #define FMRB_DEBUG(...) fmrb_debug_print(__VA_ARGS__)
#else
  #define FMRB_DEBUG(...)
#endif
