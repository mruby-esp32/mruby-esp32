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
  m_editor = new FmrbEditor();
}

void FmrbSystemApp::init(){
  FMRB_storage.init();
  m_config = new FmrbConfig(&FMRB_storage);
  strncpy(m_config->main_mode_line,VGA_640x350_70HzAlt1,256);
  strncpy(m_config->mruby_mode_line,VGA_320x200_75Hz,256);
}

FMRB_RCODE FmrbSystemApp::init_terminal(void)
{
  FMRB_DEBUG(FMRB_LOG::DEBUG,"start terminal_init\n");
  if(!m_terminal_available){
    fabgl_terminal_mode_init(m_config);

    m_terminal.begin(&VGAController);
    m_terminal.connectLocally();

    m_terminal.setBackgroundColor(Color::Black);
    m_terminal.setForegroundColor(Color::White);
    m_terminal.clear();
    //FMRB_DEBUG(FMRB_LOG::DEBUG,"terminal_init() done\n");
    m_terminal_available = true;
    //print_system_info();
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
  
  VGAController.processPrimitives();

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
  vTaskDelay(fabgl::msToTicks(34));
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

FMRB_RCODE message_callback(uint32_t fid,FmrbMenuModule* menu)
{
  FmrbDialog* dialog = new FmrbDialog(menu->m_canvas,menu->m_terminal);
  dialog->open_message_dialog("Not supported.",0);
  vTaskDelay(1000);
  delete dialog;
  return FMRB_RCODE::OK;
}

FMRB_RCODE menu_callback(uint32_t fid,FmrbMenuModule* menu)
{
  FMRB_RCODE ret = FMRB_RCODE::OK;
  FMRB_DEBUG(FMRB_LOG::DEBUG,">menu callback fid:%d\n",fid);


  if(fid==2){//begin editor
    menu->set_param(1);
    ret = FMRB_RCODE::OK_DONE;
  }
  if(fid==13){//resolution test
    ret = fmrb_subapp_resolution_test(menu);
  }

  return ret;
}


FmrbMenuItem* FmrbSystemApp::prepare_top_menu(){
  FmrbMenuItem *top = new FmrbMenuItem(alloc_menu_text_mem("<System menu>"),1,menu_callback,FmrbMenuItemType::TITLE);
  FmrbMenuItem *m1,*m2;
  //Main
       FmrbMenuItem::add_item_tail(top,alloc_menu_text_mem(" Open Editor "),2 ,menu_callback,FmrbMenuItemType::SELECTABLE);
       FmrbMenuItem::add_item_tail(top,alloc_menu_text_mem(" Run script  "),4 ,message_callback,FmrbMenuItemType::SELECTABLE);
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
  uint32_t return_info=0;
  m_main_menu->begin(&return_info);
  //m_main_menu.clear();
  if(return_info==1){
    m_state = FMRB_SYS_STATE::DO_EDIT;
  }
  return FMRB_RCODE::OK;
}

FMRB_RCODE FmrbSystemApp::run_editor(){
    m_terminal.enableCursor(true);

    m_editor->begin(&m_terminal);
    fmrb_dump_mem_stat();
    int err = m_editor->run(m_script);

    m_terminal.enableCursor(false);
    m_script = NULL;
    if(err >= 0){
      m_script = m_editor->dump_script();
    }
    m_editor->release();
  return FMRB_RCODE::OK;
}

FMRB_RCODE FmrbSystemApp::run_mruby(){
  if(m_script){
    fabgl_mruby_mode_init(m_config);
    fmrb_dump_mem_stat();
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

  bool skip_splash = true;
  bool skip_menu = false;
  
  while(true){
    FMRB_DEBUG(FMRB_LOG::MSG,"[AppState:%d]\n",m_state);
    fmrb_dump_mem_stat();

    switch(m_state){
      case FMRB_SYS_STATE::INIT:
      {
        //load config
        init_terminal();
        m_script = NULL;
        m_main_menu = new FmrbMenuModule(&FMRB_canvas,&m_terminal,prepare_top_menu());
        if(!skip_splash){
          show_splash();
          wait_key(0x0D,3000);
          clear_splash();
        }
        //check_keyboard();PS2Controller.keyboard()->isKeyboardAvailable()
        m_state = FMRB_SYS_STATE::SHOW_MENU;
        if(skip_menu) m_state = FMRB_SYS_STATE::DO_EDIT;
        break;
      }
      case FMRB_SYS_STATE::SHOW_MENU:
      {
        if(!m_terminal_available){
          init_terminal();
        }
        run_main_menu();
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

FmrbMrubyEngine *FmrbSystemApp::mruby_engign()
{
  return &m_mruby_engine;
}


/**
 * FmrbMenuItem
 **/

FmrbMenuItem::FmrbMenuItem(void):
description(nullptr),
fid(0),
func(nullptr),
type(FmrbMenuItemType::NONE),
m_prev(nullptr),
m_next(nullptr),
m_parent(nullptr),
m_child(nullptr)
{
}

FmrbMenuItem::FmrbMenuItem(char* desc, uint32_t _fid,FmrbMenuCallback _func,FmrbMenuItemType _type):
description(desc),
fid(_fid),
func(_func),
type(_type),
m_prev(nullptr),
m_next(nullptr),
m_parent(nullptr),
m_child(nullptr)
{
}

FmrbMenuItem::~FmrbMenuItem(){
  FmrbMenuItem* item = this;
  while(item != nullptr){
    if(item->description) fmrb_free(item->description);
    if(item->m_child) delete (item->m_child);
    item = item->m_next;
  }
}

FmrbMenuItem* FmrbMenuItem::add_item_tail(FmrbMenuItem* target, char* desc, uint32_t fid,FmrbMenuCallback cfunc,FmrbMenuItemType type)
{
  if(!target)return nullptr;
  FmrbMenuItem* new_item = new FmrbMenuItem(desc,fid,cfunc,type);

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
  FmrbMenuItem* new_item = new FmrbMenuItem(desc,fid,cfunc,type);
  target->m_child = new_item;
  new_item->m_parent = target;
  return new_item;
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

FmrbMenuModule::FmrbMenuModule(fabgl::Canvas* canvas,fabgl::Terminal* terminal,FmrbMenuItem* item):
m_canvas(canvas),
m_terminal(terminal),
m_top(item),
m_offset_x(30),
m_offset_y(10),
m_mergin(3),
m_param(nullptr)
{
}

FmrbMenuModule::~FmrbMenuModule(){
  delete (m_top);
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

void FmrbMenuModule::begin(uint32_t *param){

  //m_canvas->selectFont(&fabgl::FONT_8x8);
  m_canvas->selectFont(fabgl::getPresetFixedFont(8,14));
  m_canvas->setGlyphOptions(GlyphOptions().FillBackground(true));
  m_param = param;

  clear_draw_area();
  exec_menu(m_top);
  
  m_canvas->setBrushColor(Color::Black);
  m_canvas->clear();
}

void FmrbMenuModule::set_param(uint32_t param){
  if(m_param){
    *m_param = param;
  }
}

void FmrbMenuModule::clear_draw_area(void){
  m_canvas->setBrushColor(Color::Black);
  m_canvas->clear();
  uint32_t fsize=0;
  uint8_t* img_data = (uint8_t*)FMRB_storage.load("/assets/2bit_logo.img",fsize,false,false);
  if(img_data) draw_img(400,50,img_data,0);
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
  //FMRB_DEBUG(FMRB_LOG::DEBUG,"Pos(%d,%d-%d)\n",pos,pos_min,pos_max);
  draw_item(head_item,pos,true);

  //bool selected = false;
  bool cancelled = false;
  while(!cancelled){
    int c = get_cursor_dir(m_terminal);
    if(c>0){
      switch(c){
        case 1: //RETURN
        {
          FMRB_DEBUG(FMRB_LOG::DEBUG,"Menu:Select(%d)\n",pos);
          FmrbMenuItem* item = FmrbMenuItem::retrieve_item(head_item,pos);
          if(item){
            if(item->type==FmrbMenuItemType::SELECTABLE){
              if(item->func){
                FMRB_RCODE ret = item->func(item->fid,this); // callback if set
                clear_draw_area();
                draw_menu(head_item);
                draw_item(head_item,pos,true);
                if(ret==FMRB_RCODE::OK_DONE){
                  cancelled=true;
                }
              }
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
    m_canvas->setPenColor(Color::White);
    m_canvas->setBrushColor(Color::Black);
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

/**
 * Dialog 
 **/
FmrbDialog::FmrbDialog(fabgl::Canvas* canvas,fabgl::Terminal *t):
  FmrbTerminalInput(t),
  m_terminal(t),
  m_canvas(canvas),
  m_swap_buff(nullptr),
  m_dialog_width(0),
  m_dialog_height(0),
  m_x(0),
  m_y(0)
{
  m_screen_width = VGAController.getScreenWidth();
  m_screen_height = VGAController.getScreenHeight();
}

FmrbDialog::~FmrbDialog(){
  if(m_swap_buff)fmrb_free(m_swap_buff);
}

void FmrbDialog::open_message_dialog(const char* message,int timeout_sec)
{
  int len = strlen(message);

  /*
  const fabgl::FontInfo *info = m_canvas->getFontInfo();
  uint8_t width_org = info->width;
  uint8_t height_org = info->height;
  m_canvas->selectFont(fabgl::getPresetFixedFont(8,14));
  m_canvas->setGlyphOptions(GlyphOptions().FillBackground(true));
  */
  
  const fabgl::FontInfo *fontinfo = m_canvas->getFontInfo();
  uint8_t font_width = fontinfo->width;
  uint8_t font_height = fontinfo->height;
  FMRB_DEBUG(FMRB_LOG::DEBUG,"Font[%d,%d]\n",font_width,font_height);

  /*
   Width : 80% window, 10% margin, 60% text
   */
  uint16_t window_width = m_screen_width * 80 / 100;
  uint16_t window_height = (font_height+2)*3;
  FMRB_DEBUG(FMRB_LOG::DEBUG,"Window[%d,%d]\n",window_width,window_height);
  m_canvas->setBrushColor(RGB888(0,0,1<<6));
  int s = 6;
  m_canvas->fillRectangle(
    s+ m_screen_width*10/100,
    s+ m_screen_height/2 - window_height/2,
    s+ m_screen_width*90/100,
    s+ m_screen_height/2 + window_height/2
  );
  m_canvas->setBrushColor(Color::Blue);
  m_canvas->fillRectangle(
    m_screen_width*10/100,
    m_screen_height/2 - window_height/2,
    m_screen_width*90/100,
    m_screen_height/2 + window_height/2
  );
  m_canvas->setPenColor(Color::White);
  m_canvas->drawText(
    m_screen_width*20/100,
    m_screen_height/2 - window_height/2 + (font_height+2),
    message);

  //wait_key;
  wait_vkey(VirtualKey::VK_RETURN);
}


FmrbTerminalInput::FmrbTerminalInput(fabgl::Terminal *t):
m_terminal(t)
{
}

FmrbTerminalInput::~FmrbTerminalInput(){}

void FmrbTerminalInput::wait_vkey(VirtualKey k)
{  
  while(true)
  {
    VirtualKey rk = read_vkey();
    if(rk == k){
        return;
    }
  }
}

const VirtualKey _ascii_to_vkey_map[] = {
/*	0x00	*/	VirtualKey::VK_NONE,	
/*	0x01	*/	VirtualKey::VK_NONE,	
/*	0x02	*/	VirtualKey::VK_NONE,	
/*	0x03	*/	VirtualKey::VK_NONE,	
/*	0x04	*/	VirtualKey::VK_NONE,	
/*	0x05	*/	VirtualKey::VK_NONE,	
/*	0x06	*/	VirtualKey::VK_NONE,	
/*	0x07	*/	VirtualKey::VK_NONE,	
/*	0x08	*/	VirtualKey::VK_NONE,	
/*	0x09	*/	VirtualKey::VK_NONE,	
/*	0x0a	*/	VirtualKey::VK_NONE,	
/*	0x0b	*/	VirtualKey::VK_NONE,	
/*	0x0c	*/	VirtualKey::VK_NONE,	
/*	0x0d	*/	VirtualKey::VK_NONE,	
/*	0x0e	*/	VirtualKey::VK_NONE,	
/*	0x0f	*/	VirtualKey::VK_NONE,	
/*	0x10	*/	VirtualKey::VK_NONE,	
/*	0x11	*/	VirtualKey::VK_NONE,	
/*	0x12	*/	VirtualKey::VK_NONE,	
/*	0x13	*/	VirtualKey::VK_NONE,	
/*	0x14	*/	VirtualKey::VK_NONE,	
/*	0x15	*/	VirtualKey::VK_NONE,	
/*	0x16	*/	VirtualKey::VK_NONE,	
/*	0x17	*/	VirtualKey::VK_NONE,	
/*	0x18	*/	VirtualKey::VK_NONE,	
/*	0x19	*/	VirtualKey::VK_NONE,	
/*	0x1a	*/	VirtualKey::VK_NONE,	
/*	0x1b	*/	VirtualKey::VK_NONE,	
/*	0x1c	*/	VirtualKey::VK_NONE,	
/*	0x1d	*/	VirtualKey::VK_NONE,	
/*	0x1e	*/	VirtualKey::VK_NONE,	
/*	0x1f	*/	VirtualKey::VK_NONE,	
/*	0x20	*/	VirtualKey::VK_SPACE,	
/*	0x21	*/	VirtualKey::VK_EXCLAIM,	
/*	0x22	*/	VirtualKey::VK_QUOTEDBL,	
/*	0x23	*/	VirtualKey::VK_HASH,	
/*	0x24	*/	VirtualKey::VK_DOLLAR,
/*	0x25	*/	VirtualKey::VK_PERCENT,	
/*	0x26	*/	VirtualKey::VK_AMPERSAND,	
/*	0x27	*/	VirtualKey::VK_ACUTEACCENT,	
/*	0x28	*/	VirtualKey::VK_LEFTPAREN,	
/*	0x29	*/	VirtualKey::VK_RIGHTPAREN,	
/*	0x2a	*/	VirtualKey::VK_ASTERISK,	
/*	0x2b	*/	VirtualKey::VK_PLUS,	
/*	0x2c	*/	VirtualKey::VK_COMMA,	
/*	0x2d	*/	VirtualKey::VK_MINUS,	
/*	0x2e	*/	VirtualKey::VK_PERIOD,	
/*	0x2f	*/	VirtualKey::VK_SLASH,	
/*	0x30	*/	VirtualKey::VK_0,	
/*	0x31	*/	VirtualKey::VK_1,	
/*	0x32	*/	VirtualKey::VK_2,	
/*	0x33	*/	VirtualKey::VK_3,	
/*	0x34	*/	VirtualKey::VK_4,	
/*	0x35	*/	VirtualKey::VK_5,	
/*	0x36	*/	VirtualKey::VK_6,	
/*	0x37	*/	VirtualKey::VK_7,	
/*	0x38	*/	VirtualKey::VK_8,	
/*	0x39	*/	VirtualKey::VK_9,	
/*	0x3a	*/	VirtualKey::VK_COLON,	
/*	0x3b	*/	VirtualKey::VK_SEMICOLON,	
/*	0x3c	*/	VirtualKey::VK_LESS,	
/*	0x3d	*/	VirtualKey::VK_EQUALS,	
/*	0x3e	*/	VirtualKey::VK_GREATER,	
/*	0x3f	*/	VirtualKey::VK_QUESTION,	
/*	0x40	*/	VirtualKey::VK_AT,	
/*	0x41	*/	VirtualKey::VK_A,	
/*	0x42	*/	VirtualKey::VK_B,	
/*	0x43	*/	VirtualKey::VK_C,
/*	0x44	*/	VirtualKey::VK_D,	
/*	0x45	*/	VirtualKey::VK_E,	
/*	0x46	*/	VirtualKey::VK_F,	
/*	0x47	*/	VirtualKey::VK_G,	
/*	0x48	*/	VirtualKey::VK_H,	
/*	0x49	*/	VirtualKey::VK_I,	
/*	0x4a	*/	VirtualKey::VK_J,	
/*	0x4b	*/	VirtualKey::VK_K,	
/*	0x4c	*/	VirtualKey::VK_L,	
/*	0x4d	*/	VirtualKey::VK_M,	
/*	0x4e	*/	VirtualKey::VK_N,	
/*	0x4f	*/	VirtualKey::VK_O,	
/*	0x50	*/	VirtualKey::VK_P,	
/*	0x51	*/	VirtualKey::VK_Q,	
/*	0x52	*/	VirtualKey::VK_R,	
/*	0x53	*/	VirtualKey::VK_S,	
/*	0x54	*/	VirtualKey::VK_T,	
/*	0x55	*/	VirtualKey::VK_U,	
/*	0x56	*/	VirtualKey::VK_V,	
/*	0x57	*/	VirtualKey::VK_W,	
/*	0x58	*/	VirtualKey::VK_X,	
/*	0x59	*/	VirtualKey::VK_Y,	
/*	0x5a	*/	VirtualKey::VK_Z,	
/*	0x5b	*/	VirtualKey::VK_LEFTBRACKET,	
/*	0x5c	*/	VirtualKey::VK_BACKSLASH,	
/*	0x5d	*/	VirtualKey::VK_RIGHTBRACKET,	
/*	0x5e	*/	VirtualKey::VK_CARET,	
/*	0x5f	*/	VirtualKey::VK_UNDERSCORE,	
/*	0x60	*/	VirtualKey::VK_GRAVEACCENT,	
/*	0x61	*/	VirtualKey::VK_a,	
/*	0x62	*/	VirtualKey::VK_b,	
/*	0x63	*/	VirtualKey::VK_c,	
/*	0x64	*/	VirtualKey::VK_d,	
/*	0x65	*/	VirtualKey::VK_e,	
/*	0x66	*/	VirtualKey::VK_f,	
/*	0x67	*/	VirtualKey::VK_g,	
/*	0x68	*/	VirtualKey::VK_h,	
/*	0x69	*/	VirtualKey::VK_i,	
/*	0x6a	*/	VirtualKey::VK_j,	
/*	0x6b	*/	VirtualKey::VK_k,	
/*	0x6c	*/	VirtualKey::VK_l,	
/*	0x6d	*/	VirtualKey::VK_m,	
/*	0x6e	*/	VirtualKey::VK_n,	
/*	0x6f	*/	VirtualKey::VK_o,	
/*	0x70	*/	VirtualKey::VK_p,	
/*	0x71	*/	VirtualKey::VK_q,	
/*	0x72	*/	VirtualKey::VK_r,	
/*	0x73	*/	VirtualKey::VK_s,	
/*	0x74	*/	VirtualKey::VK_t,	
/*	0x75	*/	VirtualKey::VK_u,	
/*	0x76	*/	VirtualKey::VK_v,	
/*	0x77	*/	VirtualKey::VK_w,	
/*	0x78	*/	VirtualKey::VK_x,	
/*	0x79	*/	VirtualKey::VK_y,	
/*	0x7a	*/	VirtualKey::VK_z,	
/*	0x7b	*/	VirtualKey::VK_LEFTBRACE,	
/*	0x7c	*/	VirtualKey::VK_VERTICALBAR,	
/*	0x7d	*/	VirtualKey::VK_RIGHTBRACE,	
/*	0x7e	*/	VirtualKey::VK_TILDE,	
/*	0x7f	*/	VirtualKey::VK_NONE,	
};


VirtualKey FmrbTerminalInput::read_vkey()
{
  int escape = 0;
  char escape_c[4] = {0};
  while(true)
  {
    if (m_terminal->available())
    {
      char c = m_terminal->read();
      printf("> %02x\n",c);

      if(!escape)
      {
        if(c>=0x20 && c<=0x7E){
          //Visible character
          return _ascii_to_vkey_map[(int)c];
        }else{
          switch(c){
            case 0x7F: // BS
              return VirtualKey::VK_BACKSPACE;
              break;
            case 0x0D: // CR
              return VirtualKey::VK_RETURN;
              break;
            case 0x1A: // Ctrl-z
              break;
            case 0x18: // Ctrl-x
              break;
            case 0x16: // Ctrl-v
              break;
            case 0x03: // Ctrl-c
              break;
            case 0x04: // Ctrl-d
              break;
            case 0x1B: // ESC
              escape = 1;
              break;
          }
        }

      }else{ // Escape

        if(escape==1){
          switch(c){
            case 0x5B: // '[' : Cursor/ 
            case 0x4F: // 'O' : Function key
              escape_c[0] = c;
              escape=2;
              break;
            default:
              escape=0;
              break;
          }
        }else if(escape==2){
          if(escape_c[0]==0x5B){
            switch(c){
              case 0x41:  // ESC[A : UP
              return VirtualKey::VK_UP;
              case 0x42:  // ESC[B : DOWN
              return VirtualKey::VK_DOWN;
              case 0x43:  // ESC[C : RIGHT
              return VirtualKey::VK_RIGHT;
              case 0x44:  // ESC[D : LEFT
              return VirtualKey::VK_LEFT;
                escape = 0;
                break;
              case 0x31:  // ESC[1 : ...
              case 0x32:  // ESC[2 : ...
              case 0x33:  // ESC[3 : ...
              case 0x35:  // ESC[5 : ...
              case 0x36:  // ESC[6 : ...
                escape_c[1] = c;
                escape = 3;
                break;
              default:
                escape = 0;
              break;
            }
          }else if(escape_c[0]==0x4F){
            switch(c){
              case 0x50: // ESC OP : F1
                return VirtualKey::VK_F1;
              case 0x51: // ESC OP : F2
                return VirtualKey::VK_F2;
              case 0x52: // ESC OP : F3
                return VirtualKey::VK_F3;
              case 0x53: // ESC OP : F4
                return VirtualKey::VK_F4;
            }
            escape = 0;
          }else{
            escape = 0;
          }
        }else if(escape==3){
          if(escape_c[1]==0x31){
            switch(c){
              case 0x35: // ESC[15 : ..  F5
                return VirtualKey::VK_F5;
              case 0x37: // ESC[17 : ..  F6
                return VirtualKey::VK_F6;
              case 0x38: // ESC[18 : ..  F7
                return VirtualKey::VK_F7;
              case 0x39: // ESC[19 : ..  F8
                return VirtualKey::VK_F8;
                escape_c[2] = c;
                escape = 4;
                break;
              default:
                escape = 0;
                break;
            }
          }else if(escape_c[1]==0x32){
            switch(c){
              case 0x30: // ESC[20 : ..  F9
                return VirtualKey::VK_F9;
              case 0x31: // ESC[21 : ..  F10
                return VirtualKey::VK_F10;
              case 0x33: // ESC[23 : ..  F11
                return VirtualKey::VK_F11;
              case 0x34: // ESC[24 : ..  F12
                return VirtualKey::VK_F12;
                escape_c[2] = c;
                escape = 4;
                break;
              default:
                escape = 0;
                break;
            }
          }else if(escape_c[1]==0x33){
            if(c==0x7E){ // ESC[3~ : DEL
              return VirtualKey::VK_DELETE;
            }
            escape=0;
          }else if(escape_c[1]==0x35){
            if(c==0x7E){ // ESC[5~ : PageUp
              return VirtualKey::VK_PAGEUP;
            }
            escape=0;
          }else if(escape_c[1]==0x36){
            if(c==0x7E){ // ESC[6~ : PageUp
              return VirtualKey::VK_PAGEDOWN;
            }
            escape=0;
          }else{
            escape=0;
          }
        }else if(escape==4){
          switch(c){
            case 0x7E: //  ESC[1*~ : FN
              break;
            default:
              break;
          }
          escape=0;
        }else{ //escape > 4
          escape=0;
        }

        if(escape==0){
          escape_c[0] = 0;
          escape_c[1] = 0;
          escape_c[2] = 0;
          escape_c[3] = 0;
        }
      }
    }
  }
}

