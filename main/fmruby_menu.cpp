#include "fmruby.h"
#include "fmruby_app.h"
#include "fmruby_fabgl.h"

/**
 * FmrbSystemApp
 **/
FmrbSystemApp::FmrbSystemApp()
{
  m_state = FMRB_SYS_STATE::INIT;
  m_terminal_available = false;
}

FMRB_RCODE FmrbSystemApp::init_terminal(void)
{
  FMRB_DEBUG(FMRB_LOG::DEBUG,"start terminal_init\n");
  if(!m_terminal_available){
    fabgl_terminal_mode_init();

    m_terminal.begin(&VGAController);
    m_terminal.connectLocally();

    m_terminal.setBackgroundColor(Color::Black);
    m_terminal.setForegroundColor(Color::White);
    m_terminal.clear();
    //FMRB_DEBUG(FMRB_LOG::DEBUG,"terminal_init() done\n");
    m_terminal_available = true;
  }else{
    FMRB_DEBUG(FMRB_LOG::DEBUG,"terminal is already initialized\n");
  }
  return FMRB_RCODE::OK;
}

FMRB_RCODE FmrbSystemApp::close_terminal(void)
{
  FMRB_DEBUG(FMRB_LOG::DEBUG,"start close_terminal\n");
  if(m_terminal_available)
  {
    m_terminal.end();
    m_terminal_available = false;
  }else{
    //FMRB_DEBUG(FMRB_LOG::DEBUG,"terminal is already closed\n");
    return FMRB_RCODE::ERROR;
  }
  return FMRB_RCODE::OK;
}

void FmrbSystemApp::wait_key(char target,int timeout)
{
  if(!m_terminal_available) return;
  
  uint32_t start_time = xTaskGetTickCount()*portTICK_RATE_MS;

  while(true)
  {
    uint32_t current_time = xTaskGetTickCount()*portTICK_RATE_MS;
    if(timeout>0 && current_time > start_time + timeout){
      return;
    }
    if (m_terminal.available())
    {
      char c = m_terminal.read();
      if(c == target){
        return;
      }
    }
    vTaskDelay(10);
  }
}

FMRB_RCODE FmrbSystemApp::print_system_info()
{
  if(!m_terminal_available) return FMRB_RCODE::ERROR;

  m_terminal.printf("\e[37m* Family mruby *   Ver. %s (%s)\r\n",FMRB_VERSION,FMRB_RELEASE);
  m_terminal.write ("\e[34m   Powereded by FabGL\e[32m\r\n\n");
  m_terminal.printf("\e[32mScreen Size        :\e[33m %d x %d\r\n", VGAController.getScreenWidth(), VGAController.getScreenHeight());
  m_terminal.printf("\e[32mTerminal Size      :\e[33m %d x %d\r\n", m_terminal.getColumns(), m_terminal.getRows());
  m_terminal.printf("\e[32mKeyboard           :\e[33m %s\r\n", PS2Controller.keyboard()->isKeyboardAvailable() ? "OK" : "Error");
  m_terminal.printf("\e[32mFree DMA Memory    :\e[33m %d\r\n", heap_caps_get_free_size(MALLOC_CAP_DMA));
  m_terminal.printf("\e[32mFree 32 bit Memory :\e[33m %d\r\n\n", heap_caps_get_free_size(MALLOC_CAP_32BIT));
  m_terminal.write("\e[32m >> Press Enter\r\n\n");
  m_terminal.write("\e[37m");
  return FMRB_RCODE::OK;
}

static void draw_img(uint16_t x0,uint16_t y0,uint8_t* data,int mode){
  const int header = 4;
  uint16_t width  = (data[header]) + (data[header+1]<<8);
  uint16_t height = (data[header+2]) + (data[header+3]<<8);
  
  int dl = 15;
  if(mode==2) dl = 3;
  uint8_t* p = data+header+4;
  for(uint16_t y=0;y<height;y++){
    bool skip=false;
    if(mode>0){
      if(rand()%dl != 0){
        skip = true;
      }
    }
    for(uint16_t x=0;x<width;x++){
      if(!skip){
        if(((*p)&0xC0) == 0 ){ //check alpha
          VGAController.setRawPixel(x0+x,y0+y,
            VGAController.createRawPixel(RGB222((*p)&0x03, ((*p)&0x0C) >> 2, ((*p)&0x30) >> 4)));
        }
      }
      p++;
    }
  }
}

FMRB_RCODE FmrbSystemApp::show_splash(){
  FMRB_canvas.setBrushColor(Color::Black);
  FMRB_canvas.clear();
  vTaskDelay(500);

  uint32_t fsize;
  //uint8_t* img_data = (uint8_t*)FMRB_storage.load("/bktest.img",fsize,false,false);
  //uint8_t* img_data = (uint8_t*)FMRB_storage.load("/bk_small.img",fsize,false,false);
  uint8_t* img_data = (uint8_t*)FMRB_storage.load("/assets/topimage.img",fsize,false,false);
  //FMRB_DEBUG(FMRB_LOG::DEBUG,"img:%d,%d\n",width,height);
  
  if(img_data){
    for(int i=0;i<15;i++){
      if(i<10){
        draw_img(0,0,img_data,1);
      }else{
        draw_img(0,0,img_data,2);
      }
      vTaskDelay(30);
    }
    draw_img(0,0,img_data,0);

    fmrb_free(img_data);
  }
  //print_system_info();
  return FMRB_RCODE::OK;
}

FMRB_RCODE FmrbSystemApp::clear_splash(){
  int w = VGAController.getScreenWidth();
  int h = VGAController.getScreenHeight();
  FMRB_canvas.setBrushColor(Color::Black);
  for(int i=0;i<h;i++){
    FMRB_canvas.fillRectangle(0,i,w-1,i);
    //vTaskDelay(1);
  }
  return FMRB_RCODE::OK;
}


char* alloc_menu_text_mem(const char* input)
{
  if(!input) return nullptr;
  if(strlen(input)>100)return nullptr; //Too long
  char* buff = (char*)fmrb_spi_malloc(strlen(input)+1);
  strcpy(buff,input);
  return buff;
}



FMRB_RCODE menu_callback(uint32_t fid,FmrbMenuModule* menu)
{
  FMRB_RCODE ret = FMRB_RCODE::OK;
  FMRB_DEBUG(FMRB_LOG::DEBUG,">menu callback fid:%d\n",fid);

  if(fid==13){//resolution test
    ret = fmrb_subapp_resolution_test(menu);
  }

  return ret;
}


FmrbMenuItem* FmrbSystemApp::prepare_top_menu(){
  FmrbMenuItem *top = FmrbMenuItem::create_item(alloc_menu_text_mem("<System menu>"),1,menu_callback,FmrbMenuItemType::TITLE);
  FmrbMenuItem *m1,*m2;
  //Main
       FmrbMenuItem::add_item_tail(top,alloc_menu_text_mem(" Open Editor "),2 ,menu_callback,FmrbMenuItemType::SELECTABLE);
       FmrbMenuItem::add_item_tail(top,alloc_menu_text_mem(" Run script  "),4 ,menu_callback,FmrbMenuItemType::UNSELECTABLE);
  m1 = FmrbMenuItem::add_item_tail(top,alloc_menu_text_mem(" Config      "),3 ,menu_callback,FmrbMenuItemType::SELECTABLE);
     //FmrbMenuItem::add_item_tail(top,alloc_menu_text_mem("")           ,5,menu_callback,FmrbMenuItemType::UNSELECTABLE);
  //Sub Config
  m2 = FmrbMenuItem::add_child_item(m1,alloc_menu_text_mem("<Configuration>"),10,menu_callback,FmrbMenuItemType::TITLE);
       FmrbMenuItem::add_item_tail(m2 ,alloc_menu_text_mem(" Editor Resolution "),11,menu_callback,FmrbMenuItemType::SELECTABLE);
       FmrbMenuItem::add_item_tail(m2 ,alloc_menu_text_mem(" mruby Resolution  "),12,menu_callback,FmrbMenuItemType::SELECTABLE);
       FmrbMenuItem::add_item_tail(m2 ,alloc_menu_text_mem(" Resolution Test   "),13,menu_callback,FmrbMenuItemType::SELECTABLE);

  return top;
}

FMRB_RCODE FmrbSystemApp::run_main_menu(){
  m_main_menu.begin();
  wait_key(0x0D,0);
  m_main_menu.clear();
  return FMRB_RCODE::OK;
}

FMRB_RCODE FmrbSystemApp::run_editor(){
    m_terminal.enableCursor(true);
    m_editor.begin(&m_terminal);
    m_terminal.enableCursor(false);
    int err = m_editor.run(m_script);
    m_script = NULL;
    if(err >= 0){
      m_script = m_editor.dump_script();
    }
    m_editor.release();
  return FMRB_RCODE::OK;
}

FMRB_RCODE FmrbSystemApp::run_mruby(){
  if(m_script){
    fabgl_mruby_mode_init();
    m_mruby_engine.run(m_script);
    FMRB_DEBUG(FMRB_LOG::DEBUG,"m_mruby_engine END\n");
  }
  return FMRB_RCODE::OK;

}

//#define TEST_SCRIPT
#ifdef TEST_SCRIPT
const char* sample_script2 = 
#include "./mrb/entry_mrb.rb";
#endif

extern void mruby_test(const char* code_string);

FMRB_RCODE FmrbSystemApp::run()
{
  #ifdef TEST_SCRIPT
  fabgl_mruby_mode_init();
  FMRB_storage.init();
  //m_mruby_engine.run(sample_script2);
  mruby_test(sample_script2);
  FMRB_DEBUG(FMRB_LOG::DEBUG,"m_mruby_engine END\n");
  #endif
  
  while(true){
    FMRB_DEBUG(FMRB_LOG::MSG,"[AppState:%d]\n",m_state);
    fmrb_dump_mem_stat();

    switch(m_state){
      case FMRB_SYS_STATE::INIT:
      {
        FMRB_storage.init();
        //load config
        init_terminal();
        m_script = NULL;
        m_main_menu.init(&FMRB_canvas,&m_terminal,prepare_top_menu());
        show_splash();
        wait_key(0x0D,3000);
        clear_splash();
        //check_keyboard();PS2Controller.keyboard()->isKeyboardAvailable()
        m_state = FMRB_SYS_STATE::SHOW_MENU;
        break;
      }
      case FMRB_SYS_STATE::SHOW_MENU:
      {
        if(!m_terminal_available){
          init_terminal();
        }
        run_main_menu();
        m_state = FMRB_SYS_STATE::DO_EDIT;
        break;
      }
      case FMRB_SYS_STATE::DO_EDIT:
      {
        if(!m_terminal_available){
          init_terminal();
        }
        run_editor();
        m_state = FMRB_SYS_STATE::EXEC_FROM_EDIT;
        break;
      }
      case FMRB_SYS_STATE::EXEC_FROM_EDIT:
      {
        if(m_terminal_available) close_terminal();
        run_mruby();
        m_state = FMRB_SYS_STATE::DO_EDIT;
        break;
      }
      case FMRB_SYS_STATE::EXEC_FROM_FILE:
      default:
      return FMRB_RCODE::ERROR;
    }
  }
  return FMRB_RCODE::OK;
}


/**
 * FmrbMenuItem
 **/

FmrbMenuItem* FmrbMenuItem::create_item(void)
{
  FmrbMenuItem* new_item = (FmrbMenuItem*)fmrb_spi_malloc(sizeof(FmrbMenuItem));
  if(new_item==nullptr) return nullptr;
  memset(new_item,0,sizeof(FmrbMenuItem));
  return new_item;
}
FmrbMenuItem* FmrbMenuItem::create_item(char* desc, uint32_t fid,FmrbMenuCallback cfunc,FmrbMenuItemType type)
{
  FmrbMenuItem* new_item = create_item();
  if(new_item==nullptr) return nullptr;
  new_item->description = desc;
  new_item->fid = fid;
  new_item->func = cfunc;
  new_item->type = type;
  return new_item;
}

FmrbMenuItem* FmrbMenuItem::add_item_tail(FmrbMenuItem* target, char* desc, uint32_t fid,FmrbMenuCallback cfunc,FmrbMenuItemType type)
{
  if(!target)return nullptr;
  FmrbMenuItem* new_item = create_item(desc,fid,cfunc,type);
  if(new_item==nullptr) return nullptr;

  while(target->m_next != nullptr){ //find a tail
    target = target->m_next;
  }

  target->m_next = new_item;
  new_item->m_prev = target;
  return new_item;
}

FmrbMenuItem* FmrbMenuItem::add_child_item(FmrbMenuItem* target, char* desc, uint32_t fid,FmrbMenuCallback cfunc,FmrbMenuItemType type)
{
  if(!target)return nullptr;
  FmrbMenuItem* new_item = create_item(desc,fid,cfunc,type);
  if(new_item==nullptr) return nullptr;
  target->m_child = new_item;
  new_item->m_parent = target;
  return new_item;
}


void FmrbMenuItem::free(FmrbMenuItem* item){
  FMRB_DEBUG(FMRB_LOG::DEBUG,"FmrbMenuItem::free : %p\n",item);

  while(item != nullptr){
    if(item->description) fmrb_free(item->description);
    fmrb_free(item);
    item = item->m_next;
  }
}

FmrbMenuItem* FmrbMenuItem::retrieve_item(FmrbMenuItem* head_item,int line){
  FmrbMenuItem* item = head_item;
  for(int i=0;i<line;i++){
    if(item==NULL) return NULL;
    item = item->m_next;
  }
  return item;
}


/*****
 *  FmrbMenuModule
 *****/
FmrbMenuModule::FmrbMenuModule(){

}

void FmrbMenuModule::init(fabgl::Canvas* canvas,fabgl::Terminal* terminal,FmrbMenuItem* item){
  m_canvas = canvas;
  m_terminal = terminal;
  m_top = item;

  m_offset_x = 30;
  m_offset_y = 10;
  m_mergin = 3;
}

static char get_cursor_dir(fabgl::Terminal *term){
  int escape = 0;
  char escape_c[4] = {0};
  while(true)
  {
    if (term->available())
    {
      char c = term->read();
      //printf("> %02x\n",c);
      if(!escape)
      {
        if(c == 0x0D){
          return 1;
        }else if(c == 0x1B){ //ESC
          escape = 1;
        }
      }else if(escape==1){ //[
        if(c==0x5B){
          escape = 2;
        }else{
          escape = 0;
        }
      }else if(escape==2){
        switch(c){
          case 0x41:  // ESC[A : UP
          return 2;
          case 0x42:  // ESC[B : DOWN
          return 3;
          case 0x43:  // ESC[C : RIGHT
          return 4;
          case 0x44:  // ESC[D : LEFT
          return 5;
          default:
          escape = 0;
        }
      }else{
        escape = 0;
      }
    }
  }

}

void FmrbMenuModule::begin(){
  uint32_t fsize=0;
  uint8_t* img_data = (uint8_t*)FMRB_storage.load("/assets/2bit_logo.img",fsize,false,false);
  if(img_data) draw_img(400,50,img_data,0);

  //m_canvas->selectFont(&fabgl::FONT_8x8);
  m_canvas->selectFont(fabgl::getPresetFixedFont(8,14));
  m_canvas->setGlyphOptions(GlyphOptions().FillBackground(true));

  clear_draw_area();
  exec_menu(m_top);
  clear_draw_area();
}

void FmrbMenuModule::clear_draw_area(void){
  m_canvas->fillRectangle(0,0,300,300);
}

void FmrbMenuModule::exec_menu(FmrbMenuItem* head_item)
{
  int pos = 0;
  int pos_min = 0;
  int pos_max = draw_menu(head_item);
  if(head_item->type == FmrbMenuItemType::TITLE){
    pos = 1;
    pos_min = 1;
  }
  FMRB_DEBUG(FMRB_LOG::DEBUG,"Pos(%d,%d-%d)\n",pos,pos_min,pos_max);
  draw_item(head_item,pos,true);

  //bool selected = false;
  bool cancelled = false;
  while(!cancelled){
    int c = get_cursor_dir(m_terminal);
    if(c>0){
      switch(c){
        case 1: //RETURN
        {
          FMRB_DEBUG(FMRB_LOG::DEBUG,"Select(%d)\n",pos);
          FmrbMenuItem* item = FmrbMenuItem::retrieve_item(head_item,pos);
          if(item){
            if(item->type==FmrbMenuItemType::SELECTABLE){
              item->func(item->fid,this); // callback
              if(item->m_child){
                clear_draw_area();
                exec_menu(item->m_child); // > sub menu
                clear_draw_area();
                draw_menu(head_item);
                draw_item(head_item,pos,true);
              }else{
                //selected = true;
              }
            }
          }
          break;
        }

        case 2: //UP
        {
          if(pos<=pos_min){
            break;
          }
          draw_item(head_item,pos,false);
          pos--;
          draw_item(head_item,pos,true);
          break;
        }

        case 3: //DOWN
        {
          if(pos>=pos_max-1){
            break;
          }
          draw_item(head_item,pos,false);
          pos++;
          draw_item(head_item,pos,true);
          break;
        }
        case 4: //RIGHT
        {
          break; 
        }
        case 5: //LEFT
        {
          if(head_item->m_parent){
            FMRB_DEBUG(FMRB_LOG::DEBUG,"cancelled\n");
            cancelled = true;
          }
          break;
        }
        default:
        break;
      }
    }
  }

}


void FmrbMenuModule::clear(){
  FmrbMenuItem::free(m_top);
}


void FmrbMenuModule::draw_item(FmrbMenuItem* head_item,int line,bool invert){
  FmrbMenuItem* item = head_item;
  for(int i=0;i<line;i++){
    if(item==nullptr)break;
    item = item->m_next;
  }
  if(!item)return;

  int offs = 0;
  if(item->type!=FmrbMenuItemType::TITLE) offs = 8;

  const fabgl::FontInfo *fontinfo = m_canvas->getFontInfo();
  uint8_t width = fontinfo->width;
  uint8_t height = fontinfo->height;
 
  //FMRB_DEBUG(FMRB_LOG::DEBUG,"Menu(%d) > %s\n",line,item->description);
  if(invert){
    m_canvas->setPenColor(Color::White);
    m_canvas->setBrushColor(Color::Blue);
    m_canvas->drawText(m_offset_x+offs,m_offset_y+line*(height+m_mergin),item->description);
    m_canvas->setPenColor(Color::White);
    m_canvas->setBrushColor(Color::Black);
  }else{
    m_canvas->drawText(m_offset_x+offs,m_offset_y+line*(height+m_mergin),item->description);
  }
}


int FmrbMenuModule::draw_menu(FmrbMenuItem* head_item){
  FmrbMenuItem* item = head_item;
  int line_count = 0;

  while(item!=nullptr)
  {
    item = item->m_next;
    line_count++;
  }

  for(int i=0;i<line_count;i++){
    draw_item(head_item,i,false);
  }

  return line_count;
}
