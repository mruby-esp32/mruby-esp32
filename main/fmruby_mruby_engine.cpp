
#include "mruby.h"
#include "mruby/irep.h"
#include "mruby/compile.h"
#include "mruby/error.h"
#include "mruby/string.h"
#include "mruby/array.h"
#include "mruby/variable.h"

#include "fmruby.h"
#include "fmruby_fabgl.h"
#include "fmruby_app.h"


FmrbMrubyEngine::FmrbMrubyEngine():
m_exec_result(FMRB_RCODE::OK),
m_error_line(0)
{
  m_joypad_map = (uint8_t*)fmrb_spi_malloc(FMRB_JOYPAD_MAP_LENGTH);
  memset(m_joypad_map,0,FMRB_JOYPAD_MAP_LENGTH);
  m_error_msg = (char*)fmrb_spi_malloc(FMRB_DBG_MSG_MAX_LEN);
}
FmrbMrubyEngine::~FmrbMrubyEngine()
{
  if(m_joypad_map) fmrb_free( m_joypad_map );
  if(m_error_msg) fmrb_free( m_error_msg );
}

const char *FmrbMrubyEngine::get_error_msg()
{
  return m_error_msg;
}
int FmrbMrubyEngine::get_error_line()
{
  return m_error_line;
}
FMRB_RCODE FmrbMrubyEngine::get_result()
{
  return m_exec_result;
}


void* FmrbMrubyEngine::mrb_esp32_psram_allocf(mrb_state *mrb, void *p, size_t size, void *ud)
{
  if (size == 0) {
    fmrb_free(p);
    return NULL;
  }
  else {
    return fmrb_spi_realloc(p, size);
  }
}

extern "C" mrb_value
mrb_unpack_backtrace(mrb_state *mrb, mrb_value backtrace);

void FmrbMrubyEngine::check_backtrace(mrb_state *mrb) {
  mrb_value exc = mrb_obj_value(mrb->exc);
  
  mrb_value backtrace = mrb_obj_iv_get(mrb, mrb->exc, mrb_intern_lit(mrb, "backtrace"));
  if (mrb_nil_p(backtrace)) return;
  if (!mrb_array_p(backtrace)) backtrace = mrb_unpack_backtrace(mrb, backtrace);

  mrb_int depth = RARRAY_LEN(backtrace);

  mrb_value s = mrb_funcall(mrb, exc, "inspect", 0);
  int i;
  mrb_value *loc;
  int log_len = 0;
  int csr = 0;

  for (i=depth-1,loc=&RARRAY_PTR(backtrace)[i]; i>0; i--,loc--) {
    if (mrb_string_p(*loc)) {
      FMRB_DEBUG(FMRB_LOG::ERR,"  [%d] %.*s\n",
                i, (int)RSTRING_LEN(*loc), RSTRING_PTR(*loc));
      if(FMRB_DBG_MSG_MAX_LEN - csr >1){
        snprintf(m_error_msg+csr,FMRB_DBG_MSG_MAX_LEN-csr-1," [%d] %.*s\n",
                i, (int)RSTRING_LEN(*loc), RSTRING_PTR(*loc));
        csr = strlen(m_error_msg);
      }
    }
  }
  //printf("cst:%d:[%s]\n",csr,m_error_msg);
  if (mrb_string_p(*loc)) {
    log_len = (int)RSTRING_LEN(*loc);
    FMRB_DEBUG(FMRB_LOG::ERR,"%.*s> ", log_len, RSTRING_PTR(*loc));
    if(FMRB_DBG_MSG_MAX_LEN - csr >1){
      snprintf(m_error_msg+csr,FMRB_DBG_MSG_MAX_LEN-csr-1,"%.*s: ",
                log_len, RSTRING_PTR(*loc));
      csr = strlen(m_error_msg);
    }
  //printf("cst:%d:%s\n",csr,m_error_msg);
  }

  //log_len += (int)RSTRING_LEN(s);
  FMRB_DEBUG(FMRB_LOG::RAW,"%s\n", RSTRING_PTR(s));
  if(FMRB_DBG_MSG_MAX_LEN - csr >1){
    log_len = (int)RSTRING_LEN(s);
    snprintf(m_error_msg+csr,FMRB_DBG_MSG_MAX_LEN-csr-1,"%s",RSTRING_PTR(s));
    csr = strlen(m_error_msg);
    printf("cst:%d:%s\n",csr,m_error_msg);
  }

}

TaskHandle_t uartTaskHandle = NULL;
void uartTask(void *pvParameters)
{
  uint8_t *map = (uint8_t*)pvParameters;
  FMRB_DEBUG(FMRB_LOG::INFO,"start uartTask\n");
  memset(map,0,FMRB_JOYPAD_MAP_LENGTH);
  
  while(true){
    while(Serial2.available() > 0){
      uint8_t inChar = Serial2.read();
      //FMRB_DEBUG(FMRB_LOG::DEBUG,"%02X\n",inChar);
      if(inChar==0xFF){
        memset(map,0,FMRB_JOYPAD_MAP_LENGTH);
      }else{
        switch(inChar&0x7F){
          case (int)FMRB_JPAD_KEY::A : map[(int)FMRB_JPAD_KEY::A] = inChar&0x80 ? 0 : 1; break;
          case (int)FMRB_JPAD_KEY::B : map[(int)FMRB_JPAD_KEY::B] = inChar&0x80 ? 0 : 1; break;
          case (int)FMRB_JPAD_KEY::X : map[(int)FMRB_JPAD_KEY::X] = inChar&0x80 ? 0 : 1; break;
          case (int)FMRB_JPAD_KEY::Y : map[(int)FMRB_JPAD_KEY::Y] = inChar&0x80 ? 0 : 1; break;
          case (int)FMRB_JPAD_KEY::L : map[(int)FMRB_JPAD_KEY::L] = inChar&0x80 ? 0 : 1; break;
          case (int)FMRB_JPAD_KEY::R : map[(int)FMRB_JPAD_KEY::R] = inChar&0x80 ? 0 : 1; break;
          case (int)FMRB_JPAD_KEY::START  : map[(int)FMRB_JPAD_KEY::START] = inChar&0x80 ? 0 : 1; break;
          case (int)FMRB_JPAD_KEY::SELECT : map[(int)FMRB_JPAD_KEY::SELECT] = inChar&0x80 ? 0 : 1; break;
          case (int)FMRB_JPAD_KEY::UP     : map[(int)FMRB_JPAD_KEY::UP] = inChar&0x80 ? 0 : 1; break;
          case (int)FMRB_JPAD_KEY::DOWN   : map[(int)FMRB_JPAD_KEY::DOWN] = inChar&0x80 ? 0 : 1; break;
          case (int)FMRB_JPAD_KEY::LEFT   : map[(int)FMRB_JPAD_KEY::LEFT] = inChar&0x80 ? 0 : 1; break;
          case (int)FMRB_JPAD_KEY::RIGHT  : map[(int)FMRB_JPAD_KEY::RIGHT] = inChar&0x80 ? 0 : 1; break;
          default: break;
        }
      }
    }
    vTaskDelay(fabgl::msToTicks(15));
  }
}

uint8_t *FmrbMrubyEngine::get_joypad_map()
{
  return m_joypad_map;
}

void FmrbMrubyEngine::prepare_env()
{
  //create a task for UART input
  xTaskCreateUniversal(uartTask, "uartTask", FMRB_UART_TASK_STACK_SIZE, m_joypad_map, FMRB_UART_TASK_PRIORITY, &uartTaskHandle, CONFIG_ARDUINO_RUNNING_CORE);
  //create sound module

  m_exec_result = FMRB_RCODE::OK;
  m_error_msg[0] = '\0';
  m_error_line = -1;

}
void FmrbMrubyEngine::cleanup_env()
{
  if(uartTaskHandle){
    FMRB_DEBUG(FMRB_LOG::INFO,"delete uartTask\n");
    vTaskDelete(uartTaskHandle);
    uartTaskHandle=NULL;
  }
  //remove sound module
}

void FmrbMrubyEngine::run(char* code_string)
{
  FMRB_DEBUG(FMRB_LOG::INFO,"<Execute mruby script>\n\n");
  prepare_env();

  mrb_state *mrb = mrb_open_allocf(mrb_esp32_psram_allocf,NULL);

  int ai = mrb_gc_arena_save(mrb);

  mrbc_context *cxt = mrbc_context_new(mrb);
  mrbc_filename(mrb, cxt, "fmrb");

  mrb_value val = mrb_load_string_cxt(mrb,code_string,cxt);
  //mrb_value val = mrb_load_string(mrb,code_string);
  if (mrb->exc) {
    //FMRB_DEBUG(FMRB_LOG::DEBUG,"Exception occurred: %s\n", mrb_str_to_cstr(mrb, mrb_inspect(mrb, mrb_obj_value(mrb->exc))));
    FMRB_DEBUG(FMRB_LOG::ERR,"<Exception occurred>\n");
    if (!mrb_undef_p(val)) {
      check_backtrace(mrb);
    }
    mrb->exc = 0;
    m_exec_result = FMRB_RCODE::ERROR;
  } else {
    FMRB_DEBUG(FMRB_LOG::INFO,"<Execute mruby completed successfully>\n");
  }

  mrb_gc_arena_restore(mrb, ai);
  mrb_close(mrb);

  FMRB_DEBUG(FMRB_LOG::DEBUG,"End of mruby engine\n");
  cleanup_env();
  return;
}

void* t_mrb_esp32_psram_allocf(mrb_state *mrb, void *p, size_t size, void *ud)
{
  if (size == 0) {
    fmrb_free(p);
    return NULL;
  }
  else {
    return fmrb_spi_realloc(p, size);
  }
}

void mruby_test(const char* code_string)
{
  FMRB_DEBUG(FMRB_LOG::INFO,"<Execute mruby script2>\n\n");

  mrb_state *mrb = mrb_open_allocf(t_mrb_esp32_psram_allocf,NULL);

  int ai = mrb_gc_arena_save(mrb);

  //mrbc_context *cxt = mrbc_context_new(mrb);
  //mrbc_filename(mrb, cxt, "fmrb");

  //mrb_value val = mrb_load_string_cxt(mrb,code_string,cxt);
  mrb_value val = mrb_load_string(mrb,code_string);
  if (mrb->exc) {
    FMRB_DEBUG(FMRB_LOG::DEBUG,"Exception occurred: %s\n", mrb_str_to_cstr(mrb, mrb_inspect(mrb, mrb_obj_value(mrb->exc))));
    //FMRB_DEBUG(FMRB_LOG::ERR,"<Exception occurred>\n");
    if (!mrb_undef_p(val)) {
      //check_backtrace(mrb);
    }
    mrb->exc = 0;
  } else {
    FMRB_DEBUG(FMRB_LOG::INFO,"<Execute mruby completed successfully>\n");
  }

  mrb_gc_arena_restore(mrb, ai);
  mrb_close(mrb);

  FMRB_DEBUG(FMRB_LOG::DEBUG,"End of mruby engine\n");
  return;
}
