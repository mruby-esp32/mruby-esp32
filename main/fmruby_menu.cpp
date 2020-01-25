#include "fmruby.h"
#include "fmruby_app.h"
#include "fmruby_fabgl.h"


FmrbSystemApp SystemApp;

FmrbSystemApp::FmrbSystemApp()
{
  
}

int FmrbSystemApp::init_terminal(void)
{
  printf("start terminal_init\n");
  fabgl_terminal_mode_init();

  m_terminal.begin(&VGAController);
  m_terminal.connectLocally();
  //Terminal.connectSerialPort(Serial,true);

  m_terminal.setBackgroundColor(Color::Black);
  m_terminal.setForegroundColor(Color::White);
  m_terminal.clear();
  printf("terminal_init() done\n");
  return 0;
}

int FmrbSystemApp::print_system_info()
{
  m_terminal.printf("\e[37m* Family mruby *   Ver. %s (%s)\r\n",FMRB_VERSION,FMRB_RELEASE);
  m_terminal.write ("\e[34m   Powereded by FabGL\e[32m\r\n\n");
  m_terminal.printf("\e[32mScreen Size        :\e[33m %d x %d\r\n", VGAController.getScreenWidth(), VGAController.getScreenHeight());
  m_terminal.printf("\e[32mTerminal Size      :\e[33m %d x %d\r\n", m_terminal.getColumns(), m_terminal.getRows());
  m_terminal.printf("\e[32mKeyboard           :\e[33m %s\r\n", PS2Controller.keyboard()->isKeyboardAvailable() ? "OK" : "Error");
  m_terminal.printf("\e[32mFree DMA Memory    :\e[33m %d\r\n", heap_caps_get_free_size(MALLOC_CAP_DMA));
  m_terminal.printf("\e[32mFree 32 bit Memory :\e[33m %d\r\n\n", heap_caps_get_free_size(MALLOC_CAP_32BIT));
  m_terminal.write("\e[32m >> Press Enter\r\n\n");
  m_terminal.write("\e[37m");
  return 0;
}

static void draw_img(uint16_t x0,uint16_t y0,uint8_t* data){
  const int header = 4;
  uint16_t width  = (data[header]) + (data[header+1]<<8);
  uint16_t height = (data[header+2]) + (data[header+3]<<8);
  FMRB_DEBUG(FMRB_LOG::DEBUG,"img:%d,%d\n",width,height);

  uint8_t* p = data+header+4;
  for(uint16_t y=0;y<height;y++){
    for(uint16_t x=0;x<width;x++){
      if(((*p)&0xC0) == 0 ){ //check alpha
        VGAController.setRawPixel(x0+x,y0+y,
          VGAController.createRawPixel(RGB222((*p)&0x03, ((*p)&0x0C) >> 2, ((*p)&0x30) >> 4)));
      }
      p++;
    }
  }
}

int FmrbSystemApp::show_splash(){
  //uint8_t* img_data = (uint8_t*)file_service.load("/test.img");
  uint32_t fsize;
  uint8_t* img_data = (uint8_t*)file_service.load("/bktest.img",fsize,false,false);
  if(img_data){
    draw_img(0,0,img_data);
    free(img_data);
  }
  print_system_info();
  m_terminal.enableCursor(true);
  return 0;
}


int FmrbSystemApp::run_editor(){
    m_editor.begin(&m_terminal);
    int err = m_editor.run();
    if(err >= 0){
      m_script = m_editor.dump_script();
    }
    m_editor.release();
  return 0;
}

int FmrbSystemApp::run_mruby(){
  if(m_script){
    fabgl_mruby_mode_init();
    mruby_engine(m_script);
    free(m_script);
  }
  return 0;

}

int FmrbSystemApp::run()
{
  static bool first_flag = true;
  m_script = NULL;

  while(true){
    init_terminal();

    //Booting Family mruby
    if(first_flag){
      file_service.init();
      show_splash();
      first_flag=false;
    }

    //Select app
    //1.editor, 2.run script

    run_editor();

    m_terminal.end();
    printf("Terminal.end()\n");

    run_mruby();

  }
  return 0;
}

//#define TEST_SCRIPT
#ifndef TEST_SCRIPT
void menu_app()
{
  SystemApp.run();
}
#else
const char* sample_script2 = 
#include "./mrb/entry_mrb.rb";

void menu_app(){ //Test
  printf("ScriptTest\n");
  const char* scirpt = sample_script2;
  if(scirpt){
    fabgl_mruby_mode_init();
    mruby_engine(scirpt);
  }
}
#endif
