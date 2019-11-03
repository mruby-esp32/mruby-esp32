#include "fmruby.h"
#include "fmruby_app.h"
#include "fmruby_fabgl.h"
#include "fmruby_editor.h"

void print_info()
{
  Terminal.write("\e[37m* * FabGL - Loopback VT/ANSI Terminal\r\n");
  Terminal.write("\e[34m* * 2019 by Fabrizio Di Vittorio - www.fabgl.com\e[32m\r\n\n");
  Terminal.printf("\e[32mScreen Size        :\e[33m %d x %d\r\n", VGAController.getScreenWidth(), VGAController.getScreenHeight());
  Terminal.printf("\e[32mTerminal Size      :\e[33m %d x %d\r\n", Terminal.getColumns(), Terminal.getRows());
  Terminal.printf("\e[32mKeyboard           :\e[33m %s\r\n", Keyboard.isKeyboardAvailable() ? "OK" : "Error");
  Terminal.printf("\e[32mFree DMA Memory    :\e[33m %d\r\n", heap_caps_get_free_size(MALLOC_CAP_DMA));
  Terminal.printf("\e[32mFree 32 bit Memory :\e[33m %d\r\n\n", heap_caps_get_free_size(MALLOC_CAP_32BIT));
  Terminal.write("\e[32mFree typing test - press ESC to introduce escape VT/ANSI codes\r\n\n");
  Terminal.write("\e[37m");
}

FmrbEditor Editor;

void terminal_init(void)
{
  printf("start terminal_init\n");

  Terminal.begin();
  Terminal.connectLocally();      // to use Terminal.read(), available(), etc..

  Terminal.setBackgroundColor(Color::Black);
  Terminal.setForegroundColor(Color::White);
  Terminal.clear();

  print_info();

  Terminal.enableCursor(true);

}

void menu_app()
{
  printf("terminal_task\n");

  while(true){
    fabgl_terminal_mode_init();
    terminal_init();
    printf("terminal_init() done\n");

    //select app

    //1.editor, 2.run script

    int err_code = Editor.run();
    //vTaskDelay(10000 / portTICK_RATE_MS);
    char* scirpt = Editor.dump_script();
    Terminal.end();
    printf("Terminal.end()\n");
    if(scirpt){
      fabgl_mruby_mode_init();
      mruby_engine(scirpt);
      free(scirpt);
    }
  }
}
