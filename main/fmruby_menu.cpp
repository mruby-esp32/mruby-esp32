#include "fabgl.h"
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
}

FmrbEditor Editor;

void terminal_init(void)
{
  /*
  PS2Controller.begin(PS2Preset::KeyboardPort0);
  */ 
  Terminal.begin();
  //Terminal.loadFont(Canvas.getPresetFontInfo(132, 25));
  Terminal.connectLocally();      // to use Terminal.read(), available(), etc..

  Terminal.setBackgroundColor(Color::Black);
  Terminal.setForegroundColor(Color::BrightGreen);
  Terminal.clear();

  print_info();

  Terminal.enableCursor(true);
}


void terminal_task(void *pvParameter)
{

  Editor.begin();
  return;

  while(true)
  {

    printf("LOOP\n");
      for(int i=0;i<30;i++){
        Terminal.write("\eM");
        vTaskDelay(100 / portTICK_PERIOD_MS);
      }
      for(int i=0;i<30;i++){
        Terminal.write("\eD");
        vTaskDelay(100 / portTICK_PERIOD_MS);
      }


    if (Terminal.available())
    {
      char c = Terminal.read();
      switch (c) {
        case 0x7F:       // DEL -> backspace + ESC[K
          Terminal.write("\b\e[K");
          break;
        case 0x0D:       // CR  -> CR + LF
          Terminal.write("\r\n");
          break;
        default:
          Terminal.write(c);
          break;
      }

    }
  }
}
