
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


FmrbMrubyEngine::FmrbMrubyEngine(){

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

  for (i=depth-1,loc=&RARRAY_PTR(backtrace)[i]; i>0; i--,loc--) {
    if (mrb_string_p(*loc)) {
      FMRB_DEBUG(FMRB_LOG::ERR,"\t[%d] %.*s\n",
                i, (int)RSTRING_LEN(*loc), RSTRING_PTR(*loc));
    }
  }
  if (mrb_string_p(*loc)) {
    log_len = (int)RSTRING_LEN(*loc);
    FMRB_DEBUG(FMRB_LOG::ERR,"%.*s: ", log_len, RSTRING_PTR(*loc));
  }
  log_len += (int)RSTRING_LEN(s);
  FMRB_DEBUG(FMRB_LOG::RAW,"%s\n", RSTRING_PTR(s));

  if(log_len > DBG_MSG_MAX_LEN){
    log_len = DBG_MSG_MAX_LEN;
  }

}

void FmrbMrubyEngine::run(char* code_string)
{
  FMRB_DEBUG(FMRB_LOG::INFO,"<Execute mruby script>\n\n");

  m_exec_result = 0;
  if(m_error_msg) fmrb_free(m_error_msg);
  m_error_msg = NULL;
  m_error_line = -1;

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
    m_exec_result = -1;
  } else {
    FMRB_DEBUG(FMRB_LOG::INFO,"<Execute mruby completed successfully>\n");
  }

  mrb_gc_arena_restore(mrb, ai);
  mrb_close(mrb);

  FMRB_DEBUG(FMRB_LOG::DEBUG,"End of mruby engine\n");
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
