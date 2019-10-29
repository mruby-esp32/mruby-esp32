
#include "mruby.h"
#include "mruby/irep.h"
#include "mruby/compile.h"
#include "mruby/error.h"
#include "mruby/string.h"

#include "fmruby_fabgl.h"
#include "fmruby_app.h"

#include "entry_mrb.h"



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

#define DOUBLEBUFFERING 1
void mruby_init()
{
  VGAController.setResolution(VGA_320x200_75Hz, -1, -1, DOUBLEBUFFERING);

  Canvas.selectFont(Canvas.getPresetFontInfo(40, 14)); // get a font for about 40x14 text screen
  Canvas.setGlyphOptions(GlyphOptions().FillBackground(true));

}


//void mruby_task(void *pvParameter)
void mruby_engine(char* code_string)
{
  mrb_state *mrb = mrb_open_allocf(mrb_esp32_psram_allocf,NULL);
  int ai = mrb_gc_arena_save(mrb);

  mrb_value val = mrb_load_string(mrb,code_string);
  if (mrb->exc) {
    printf("Exception occurred: %s\n", mrb_str_to_cstr(mrb, mrb_inspect(mrb, mrb_obj_value(mrb->exc))));
    mrb->exc = 0;
  } else {
    printf("Success\n");
  }

  mrb_gc_arena_restore(mrb, ai);
  mrb_close(mrb);

/*
  mrb_state *mrb = mrb_open_allocf(mrb_esp32_psram_allocf,NULL);
  int ai = mrb_gc_arena_save(mrb);
  printf("Loading binary...!\n");
  mrb_load_irep(mrb, entry_mrb);
  if (mrb->exc) {
    printf("Exception occurred: %s\n", mrb_str_to_cstr(mrb, mrb_inspect(mrb, mrb_obj_value(mrb->exc))));
    mrb->exc = 0;
  } else {
    printf("Success\n");
  }
  mrb_gc_arena_restore(mrb, ai);
  mrb_close(mrb);
*/
  printf("End of mruby task\n");
  while (1) {
	  vTaskDelay(1);
  }
}
