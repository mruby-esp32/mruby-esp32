#pragma once

#include "fabgl.h"
#include "fmruby_virtual_key.h"

#define FMRB_VERSION "0.6.0"
#define FMRB_RELEASE "2020/02/15"

#define FMRB_MAIN_TASK_PRIORITY 1
#define FMRB_UART_TASK_PRIORITY 2

#define FMRB_MAIN_TASK_STACK_SIZE (1024*16)
#define FMRB_UART_TASK_STACK_SIZE (1024*2)

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


class FmrbTerminalInput{
public:
  FmrbTerminalInput(fabgl::Terminal *t);
  ~FmrbTerminalInput();
  fabgl::Terminal *m_terminal;

  OVERLOAD_SPI_ALLOCATOR
  FmrbVkey read_vkey();
  void wait_vkey(FmrbVkey k);
  void wait_vkey(FmrbVkey target,int timeout);

  static bool is_visible(FmrbVkey k);
  static char to_ascii(FmrbVkey k);

};

struct FmrbCanvasConfig {
  RGB888 fg; //pen color
  RGB888 bg; //brush color
  bool fill_bg;
  int font_width;
  int font_height;

  OVERLOAD_SPI_ALLOCATOR
  FmrbCanvasConfig(RGB888 fg,RGB888 bg,bool fillbg, int widht,int height);
  ~FmrbCanvasConfig();
  void set(fabgl::Canvas*);
};



void fmrb_debug_print(FMRB_LOG lv,const char *fmt,const char* func,int line,...);
void fmrb_dump_mem_stat();

#ifdef ENABLE_FMRB_LOG
  #define FMRB_DEBUG(lv,fmt, ...) fmrb_debug_print(lv,fmt, __func__ , __LINE__ , ##__VA_ARGS__ )
#else
  #define FMRB_DEBUG(...)
#endif
