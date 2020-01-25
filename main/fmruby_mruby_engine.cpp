
#include "mruby.h"
#include "mruby/irep.h"
#include "mruby/compile.h"
#include "mruby/error.h"
#include "mruby/string.h"

#include "fmruby.h"
#include "fmruby_fabgl.h"
#include "fmruby_app.h"



void* mrb_esp32_psram_allocf(mrb_state *mrb, void *p, size_t size, void *ud)
{
  if (size == 0) {
    free(p);
    return NULL;
  }
  else {
    return heap_caps_realloc(p, size, MALLOC_CAP_SPIRAM);
  }
}

void mruby_engine(char* code_string)
{
  mrb_state *mrb = mrb_open_allocf(mrb_esp32_psram_allocf,NULL);

  int ai = mrb_gc_arena_save(mrb);

  mrbc_context *cxt = mrbc_context_new(mrb);
  mrbc_filename(mrb, cxt, "fmrb");

  mrb_value val = mrb_load_string_cxt(mrb,code_string,cxt);
  if (mrb->exc) {
    //FMRB_DEBUG(FMRB_LOG::DEBUG,"Exception occurred: %s\n", mrb_str_to_cstr(mrb, mrb_inspect(mrb, mrb_obj_value(mrb->exc))));
    FMRB_DEBUG(FMRB_LOG::ERR,"Exception occurred\n");
    if (!mrb_undef_p(val)) {
      mrb_print_error(mrb);
    }
    mrb->exc = 0;
  } else {
    FMRB_DEBUG(FMRB_LOG::INFO,"<Exec mruby completed successfully>\n");
  }

  mrb_gc_arena_restore(mrb, ai);
  mrb_close(mrb);

  FMRB_DEBUG(FMRB_LOG::DEBUG,"End of mruby engine\n");
}
