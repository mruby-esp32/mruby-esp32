#include "fmruby.h"
#include "fmruby_app.h"
#include "fmruby_fabgl.h"

/**
 * FmrbSystemApp
 **/
FmrbSystemApp::FmrbSystemApp(fabgl::VGAController *v,fabgl::PS2Controller *ps2,fabgl::Canvas *c):
m_vga(v),
m_ps2(ps2),
m_canvas(c)
{
  m_state = FMRB_SYS_STATE::INIT;
  m_terminal_available = false;
  m_editor = nullptr;
  m_main_menu = nullptr;
}

void FmrbSystemApp::init(){
  FMRB_storage.init();
  m_config = new FmrbConfig(&FMRB_storage);
  //load config
  strncpy(m_config->main_mode_line,VGA_640x350_70HzAlt1,256);
  strncpy(m_config->mruby_mode_line,VGA_320x200_75Hz,256);

}

FMRB_RCODE FmrbSystemApp::init_terminal(void)
{
  FMRB_DEBUG(FMRB_LOG::DEBUG,"start terminal_init\n");
  if(!m_terminal_available){
    fabgl_terminal_mode_init(m_config);

    m_terminal.begin(m_vga);
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

FMRB_RCODE FmrbSystemApp::print_system_info()
{
  if(!m_terminal_available) return FMRB_RCODE::ERROR;

  m_terminal.printf("\e[37m* Family mruby *   Ver. %s (%s)\r\n",FMRB_VERSION,FMRB_RELEASE);
  m_terminal.write ("\e[34m   Powereded by FabGL\e[32m\r\n\n");
  m_terminal.printf("\e[32mScreen Size        :\e[33m %d x %d\r\n", m_vga->getScreenWidth(), m_vga->getScreenHeight());
  m_terminal.printf("\e[32mTerminal Size      :\e[33m %d x %d\r\n", m_terminal.getColumns(), m_terminal.getRows());
  m_terminal.printf("\e[32mKeyboard           :\e[33m %s\r\n", m_ps2->keyboard()->isKeyboardAvailable() ? "OK" : "Error");
  m_terminal.printf("\e[32mFree DMA Memory    :\e[33m %d\r\n", heap_caps_get_free_size(MALLOC_CAP_DMA));
  m_terminal.printf("\e[32mFree 32 bit Memory :\e[33m %d\r\n\n", heap_caps_get_free_size(MALLOC_CAP_32BIT));
  m_terminal.write("\e[32m >> Press Enter\r\n\n");
  m_terminal.write("\e[37m");
  return FMRB_RCODE::OK;
}

void FmrbSystemApp::draw_img(fabgl::VGAController *vga,uint16_t x0,uint16_t y0,uint8_t* data,int mode){
  const int header = 4;
  uint16_t width  = (data[header]) + (data[header+1]<<8);
  uint16_t height = (data[header+2]) + (data[header+3]<<8);
  
  vga->processPrimitives();

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
          vga->setRawPixel(x0+x,y0+y,
            vga->createRawPixel(RGB222((*p)&0x03, ((*p)&0x0C) >> 2, ((*p)&0x30) >> 4)));
        }
      }
      p++;
    }
  }
  vTaskDelay(fabgl::msToTicks(34));//wait for 1 frame
}

FMRB_RCODE FmrbSystemApp::show_splash(){
  m_canvas->setBrushColor(Color::Black);
  m_canvas->clear();
  vTaskDelay(500);

  uint32_t fsize;
  //uint8_t* img_data = (uint8_t*)FMRB_storage.load("/bktest.img",fsize,false,false);
  //uint8_t* img_data = (uint8_t*)FMRB_storage.load("/bk_small.img",fsize,false,false);
  uint8_t* img_data = (uint8_t*)FMRB_storage.load("/assets/topimage.img",fsize,false,false);
  //FMRB_DEBUG(FMRB_LOG::DEBUG,"img:%d,%d\n",width,height);
  
  if(img_data){
    for(int i=0;i<15;i++){
      if(i<10){
        draw_img(m_vga,0,0,img_data,1);
      }else{
        draw_img(m_vga,0,0,img_data,2);
      }
      vTaskDelay(30);
    }
    draw_img(m_vga,0,0,img_data,0);

    fmrb_free(img_data);
  }
  //print_system_info();
  return FMRB_RCODE::OK;
}

FMRB_RCODE FmrbSystemApp::clear_splash(){
  int w = m_vga->getScreenWidth();
  int h = m_vga->getScreenHeight();
  m_canvas->setBrushColor(Color::Black);
  for(int i=0;i<h;i++){
    m_canvas->fillRectangle(0,i,w-1,i);
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
  FmrbDialog* dialog = new FmrbDialog(menu->m_vga,menu->m_canvas,menu->m_terminal,menu->m_canvas_config);
  dialog->open_message_dialog("Not supported.",0);
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

    m_editor->begin(&m_mruby_engine);
    fmrb_dump_mem_stat();
    FMRB_RCODE err = m_editor->run(m_script);

    m_terminal.enableCursor(false);
    m_script = NULL;
    if(err == FMRB_RCODE::OK){
      m_script = m_editor->dump_script();
    }
    m_editor->release();
  return err;
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
        init_terminal();
        m_script = NULL;
        if(!m_editor) m_editor = new FmrbEditor(m_vga,m_canvas,&m_terminal);
        if(!m_main_menu) m_main_menu = new FmrbMenuModule(m_vga, m_canvas,&m_terminal,prepare_top_menu());

        if(!skip_splash){
          show_splash();
          m_main_menu->wait_vkey(FmrbVkey::VK_RETURN,3000);
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
        FMRB_RCODE err = run_editor();
        if(err==FMRB_RCODE::OK)
        {
          m_state = FMRB_SYS_STATE::EXEC_FROM_EDIT;
        }else{
          FMRB_DEBUG(FMRB_LOG::ERR,"Editor error!\n");
        }
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

FmrbMenuModule::FmrbMenuModule(fabgl::VGAController* vga,fabgl::Canvas* canvas,fabgl::Terminal* terminal,FmrbMenuItem* item):
FmrbTerminalInput(terminal),
m_vga(vga),
m_canvas(canvas),
m_terminal(terminal),
m_top(item),
m_offset_x(30),
m_offset_y(10),
m_mergin(3),
m_param(nullptr)
{
  m_canvas_config = new FmrbCanvasConfig(RGB888(255,255,255),RGB888(0,0,0),true,8,14);
}

FmrbMenuModule::~FmrbMenuModule(){
  delete (m_top);
  if(m_canvas_config) delete m_canvas_config;
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
  if(img_data){
    FmrbSystemApp::draw_img(m_vga,400,50,img_data,0);
    fmrb_free(img_data);
  }
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
    FmrbVkey k = read_vkey();
    switch(k){
        case FmrbVkey::VK_RETURN: //RETURN
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

        case FmrbVkey::VK_UP: //UP
        {
          if(pos<=pos_min){
            break;
          }
          draw_item(head_item,pos,false);
          pos--;
          draw_item(head_item,pos,true);
          break;
        }

        case FmrbVkey::VK_DOWN: //DOWN
        {
          if(pos>=pos_max-1){
            break;
          }
          draw_item(head_item,pos,false);
          pos++;
          draw_item(head_item,pos,true);
          break;
        }
        case FmrbVkey::VK_RIGHT: //RIGHT
        {
          break; 
        }
        case FmrbVkey::VK_LEFT: //LEFT
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
  //uint8_t width = fontinfo->width;
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
FmrbDialog::FmrbDialog(fabgl::VGAController* vga,fabgl::Canvas* canvas,fabgl::Terminal *t,FmrbCanvasConfig* cconfig):
  FmrbTerminalInput(t),
  m_vga(vga),
  m_canvas(canvas),
  m_terminal(t),
  m_swap_buff(nullptr),
  m_dialog_width(0),
  m_dialog_height(0),
  m_x(0),
  m_y(0),
  m_fg_color (RGB888(255,255,255)),
  m_bg_color1(RGB888(  0,  0,1<<6)),
  m_bg_color2(RGB888(  0,  0,3<<6)),
  m_canva_config_origin(cconfig)
{
  m_screen_width = m_vga->getScreenWidth();
  m_screen_height = m_vga->getScreenHeight();

  m_canvas->setGlyphOptions(GlyphOptions().FillBackground(true));

  const fabgl::FontInfo *fontinfo = m_canvas->getFontInfo();
  m_font_width = fontinfo->width;
  m_font_height = fontinfo->height;
  m_line_height = m_font_height+2;
  //FMRB_DEBUG(FMRB_LOG::DEBUG,"Font[%d,%d]\n",font_width,font_height);
}

FmrbDialog::~FmrbDialog(){
  m_canva_config_origin->set(m_canvas);
  if(m_swap_buff)fmrb_free(m_swap_buff);
}
  
void FmrbDialog::open_message_dialog(const char* message,int timeout_sec)
{
  int msg_len = strlen(message);
  int line_cnt = 0;
  int max_line_chars = m_screen_width/m_font_width - 5;
  //count lines
  for(int lcnt=0,c=0;c<=msg_len;c++){
    if('\n' == message[c] || 0 == message[c] || lcnt>=max_line_chars){
      lcnt=0;
      line_cnt++;
    }
    lcnt++;
  }
  //FMRB_DEBUG(FMRB_LOG::DEBUG,"line_cnt[%d] max=%d\n",line_cnt,max_line_chars);
  uint16_t window_width = m_screen_width * 95 / 100;
  uint16_t window_height = m_line_height*(2+line_cnt);

  int text_top_height = draw_window(line_cnt);

  m_canvas->setPenColor(m_fg_color);
  const char* prev_p = message;
  char* buff = (char*)fmrb_spi_malloc(max_line_chars+2);
  for(int lcnt=0,line=0,c=0;c<=msg_len;c++){
    //FMRB_DEBUG(FMRB_LOG::DEBUG,"[%c] c:%d lcnt:%d line:%d\n",message[c],c,lcnt,line);
    if('\n' == message[c] || 0 == message[c] || lcnt>=max_line_chars){
      memset(buff,0,max_line_chars+2);
      memcpy(buff,prev_p,&message[c]-prev_p);
      if(lcnt>=max_line_chars){
        prev_p = &message[c];
      }else{
        prev_p = &message[c]+1;
      }
      lcnt=0;
      FMRB_DEBUG(FMRB_LOG::DEBUG,">%s\n",buff);
      m_canvas->drawText(
        m_screen_width*7/100,
        text_top_height + line*m_line_height,
        buff,true);
      line++;
    }
    lcnt++;
  }
  fmrb_free(buff);

  //wait_key;
  wait_vkey(FmrbVkey::VK_RETURN);
}


bool FmrbDialog::open_confirmation_dialog(const char* message)
{
  bool select = true;
  const char* _yes = "     <Yes>         No ";
  const char* _no  = "      Yes         <No>";
  int text_top_height = draw_window(3);
  m_canvas->setPenColor(m_fg_color);
  m_canvas->setBrushColor(m_bg_color1);
  m_canvas->drawText( m_screen_width*7/100, text_top_height,message,true);

  while(true){
    if(select){
      m_canvas->drawText(m_screen_width*7/100, text_top_height+m_line_height*2,_yes,true);
    }else{
      m_canvas->drawText(m_screen_width*7/100, text_top_height+m_line_height*2,_no ,true);
    }
    FmrbVkey key = read_vkey();
    if(key==FmrbVkey::VK_LEFT){
      select=true;
    }else if(key==FmrbVkey::VK_RIGHT){
      select=false;
    }else if(key==FmrbVkey::VK_RETURN){
      break;
    }
  }

  return select;
}

int FmrbDialog::draw_window(int line)
{
  const fabgl::FontInfo *fontinfo = m_canvas->getFontInfo();
  uint8_t font_height = fontinfo->height;
  uint8_t line_height = font_height+2;
  uint16_t window_width = m_screen_width * 95 / 100;
  uint16_t window_height = line_height*(2+line);
  FMRB_DEBUG(FMRB_LOG::DEBUG,"Window[%d,%d]\n",window_width,window_height);
  m_canvas->setBrushColor(m_bg_color2);
  int s = 6;
  m_canvas->fillRectangle(
    s+ m_screen_width*5/100,
    s+ m_screen_height/2 - window_height/2,
    s+ m_screen_width*95/100,
    s+ m_screen_height/2 + window_height/2
  );
  m_canvas->setBrushColor(m_bg_color1);
  m_canvas->fillRectangle(
    m_screen_width*5/100,
    m_screen_height/2 - window_height/2,
    m_screen_width*95/100,
    m_screen_height/2 + window_height/2
  );
  return m_screen_height/2 - window_height/2 + line_height;
}


/**
 * FmrbTerminalInput Class
 **/
FmrbTerminalInput::FmrbTerminalInput(fabgl::Terminal *t):
m_terminal(t)
{
}

FmrbTerminalInput::~FmrbTerminalInput(){}

void FmrbTerminalInput::wait_vkey(FmrbVkey k)
{  
  while(true)
  {
    FmrbVkey rk = read_vkey();
    if(rk == k){
        return;
    }
    vTaskDelay(10);
  }
}

void FmrbTerminalInput::wait_vkey(FmrbVkey target,int timeout)
{
  uint32_t start_time = xTaskGetTickCount()*portTICK_RATE_MS;

  while(true)
  {
    uint32_t current_time = xTaskGetTickCount()*portTICK_RATE_MS;
    if(timeout>0 && current_time > start_time + timeout){
      return;
    }
    if (m_terminal->available())
    {
      FmrbVkey in = read_vkey();
      if(in == target){
        return;
      }
    }
    vTaskDelay(10);
  }
}

const FmrbVkey _ascii_to_vkey_map[] = {
/*	0x00	*/	FmrbVkey::VK_NONE,	
/*	0x01	*/	FmrbVkey::VK_NONE,	
/*	0x02	*/	FmrbVkey::VK_NONE,	
/*	0x03	*/	FmrbVkey::VK_NONE,	
/*	0x04	*/	FmrbVkey::VK_NONE,	
/*	0x05	*/	FmrbVkey::VK_NONE,	
/*	0x06	*/	FmrbVkey::VK_NONE,	
/*	0x07	*/	FmrbVkey::VK_NONE,	
/*	0x08	*/	FmrbVkey::VK_NONE,	
/*	0x09	*/	FmrbVkey::VK_NONE,	
/*	0x0a	*/	FmrbVkey::VK_NONE,	
/*	0x0b	*/	FmrbVkey::VK_NONE,	
/*	0x0c	*/	FmrbVkey::VK_NONE,	
/*	0x0d	*/	FmrbVkey::VK_NONE,	
/*	0x0e	*/	FmrbVkey::VK_NONE,	
/*	0x0f	*/	FmrbVkey::VK_NONE,	
/*	0x10	*/	FmrbVkey::VK_NONE,	
/*	0x11	*/	FmrbVkey::VK_NONE,	
/*	0x12	*/	FmrbVkey::VK_NONE,	
/*	0x13	*/	FmrbVkey::VK_NONE,	
/*	0x14	*/	FmrbVkey::VK_NONE,	
/*	0x15	*/	FmrbVkey::VK_NONE,	
/*	0x16	*/	FmrbVkey::VK_NONE,	
/*	0x17	*/	FmrbVkey::VK_NONE,	
/*	0x18	*/	FmrbVkey::VK_NONE,	
/*	0x19	*/	FmrbVkey::VK_NONE,	
/*	0x1a	*/	FmrbVkey::VK_NONE,	
/*	0x1b	*/	FmrbVkey::VK_NONE,	
/*	0x1c	*/	FmrbVkey::VK_NONE,	
/*	0x1d	*/	FmrbVkey::VK_NONE,	
/*	0x1e	*/	FmrbVkey::VK_NONE,	
/*	0x1f	*/	FmrbVkey::VK_NONE,	
/*	0x20	*/	FmrbVkey::VK_SPACE,	
/*	0x21	*/	FmrbVkey::VK_EXCLAIM,	
/*	0x22	*/	FmrbVkey::VK_QUOTEDBL,	
/*	0x23	*/	FmrbVkey::VK_HASH,	
/*	0x24	*/	FmrbVkey::VK_DOLLAR,
/*	0x25	*/	FmrbVkey::VK_PERCENT,	
/*	0x26	*/	FmrbVkey::VK_AMPERSAND,	
/*	0x27	*/	FmrbVkey::VK_ACUTEACCENT,	
/*	0x28	*/	FmrbVkey::VK_LEFTPAREN,	
/*	0x29	*/	FmrbVkey::VK_RIGHTPAREN,	
/*	0x2a	*/	FmrbVkey::VK_ASTERISK,	
/*	0x2b	*/	FmrbVkey::VK_PLUS,	
/*	0x2c	*/	FmrbVkey::VK_COMMA,	
/*	0x2d	*/	FmrbVkey::VK_MINUS,	
/*	0x2e	*/	FmrbVkey::VK_PERIOD,	
/*	0x2f	*/	FmrbVkey::VK_SLASH,	
/*	0x30	*/	FmrbVkey::VK_0,	
/*	0x31	*/	FmrbVkey::VK_1,	
/*	0x32	*/	FmrbVkey::VK_2,	
/*	0x33	*/	FmrbVkey::VK_3,	
/*	0x34	*/	FmrbVkey::VK_4,	
/*	0x35	*/	FmrbVkey::VK_5,	
/*	0x36	*/	FmrbVkey::VK_6,	
/*	0x37	*/	FmrbVkey::VK_7,	
/*	0x38	*/	FmrbVkey::VK_8,	
/*	0x39	*/	FmrbVkey::VK_9,	
/*	0x3a	*/	FmrbVkey::VK_COLON,	
/*	0x3b	*/	FmrbVkey::VK_SEMICOLON,	
/*	0x3c	*/	FmrbVkey::VK_LESS,	
/*	0x3d	*/	FmrbVkey::VK_EQUALS,	
/*	0x3e	*/	FmrbVkey::VK_GREATER,	
/*	0x3f	*/	FmrbVkey::VK_QUESTION,	
/*	0x40	*/	FmrbVkey::VK_AT,	
/*	0x41	*/	FmrbVkey::VK_A,	
/*	0x42	*/	FmrbVkey::VK_B,	
/*	0x43	*/	FmrbVkey::VK_C,
/*	0x44	*/	FmrbVkey::VK_D,	
/*	0x45	*/	FmrbVkey::VK_E,	
/*	0x46	*/	FmrbVkey::VK_F,	
/*	0x47	*/	FmrbVkey::VK_G,	
/*	0x48	*/	FmrbVkey::VK_H,	
/*	0x49	*/	FmrbVkey::VK_I,	
/*	0x4a	*/	FmrbVkey::VK_J,	
/*	0x4b	*/	FmrbVkey::VK_K,	
/*	0x4c	*/	FmrbVkey::VK_L,	
/*	0x4d	*/	FmrbVkey::VK_M,	
/*	0x4e	*/	FmrbVkey::VK_N,	
/*	0x4f	*/	FmrbVkey::VK_O,	
/*	0x50	*/	FmrbVkey::VK_P,	
/*	0x51	*/	FmrbVkey::VK_Q,	
/*	0x52	*/	FmrbVkey::VK_R,	
/*	0x53	*/	FmrbVkey::VK_S,	
/*	0x54	*/	FmrbVkey::VK_T,	
/*	0x55	*/	FmrbVkey::VK_U,	
/*	0x56	*/	FmrbVkey::VK_V,	
/*	0x57	*/	FmrbVkey::VK_W,	
/*	0x58	*/	FmrbVkey::VK_X,	
/*	0x59	*/	FmrbVkey::VK_Y,	
/*	0x5a	*/	FmrbVkey::VK_Z,	
/*	0x5b	*/	FmrbVkey::VK_LEFTBRACKET,	
/*	0x5c	*/	FmrbVkey::VK_BACKSLASH,	
/*	0x5d	*/	FmrbVkey::VK_RIGHTBRACKET,	
/*	0x5e	*/	FmrbVkey::VK_CARET,	
/*	0x5f	*/	FmrbVkey::VK_UNDERSCORE,	
/*	0x60	*/	FmrbVkey::VK_GRAVEACCENT,	
/*	0x61	*/	FmrbVkey::VK_a,	
/*	0x62	*/	FmrbVkey::VK_b,	
/*	0x63	*/	FmrbVkey::VK_c,	
/*	0x64	*/	FmrbVkey::VK_d,	
/*	0x65	*/	FmrbVkey::VK_e,	
/*	0x66	*/	FmrbVkey::VK_f,	
/*	0x67	*/	FmrbVkey::VK_g,	
/*	0x68	*/	FmrbVkey::VK_h,	
/*	0x69	*/	FmrbVkey::VK_i,	
/*	0x6a	*/	FmrbVkey::VK_j,	
/*	0x6b	*/	FmrbVkey::VK_k,	
/*	0x6c	*/	FmrbVkey::VK_l,	
/*	0x6d	*/	FmrbVkey::VK_m,	
/*	0x6e	*/	FmrbVkey::VK_n,	
/*	0x6f	*/	FmrbVkey::VK_o,	
/*	0x70	*/	FmrbVkey::VK_p,	
/*	0x71	*/	FmrbVkey::VK_q,	
/*	0x72	*/	FmrbVkey::VK_r,	
/*	0x73	*/	FmrbVkey::VK_s,	
/*	0x74	*/	FmrbVkey::VK_t,	
/*	0x75	*/	FmrbVkey::VK_u,	
/*	0x76	*/	FmrbVkey::VK_v,	
/*	0x77	*/	FmrbVkey::VK_w,	
/*	0x78	*/	FmrbVkey::VK_x,	
/*	0x79	*/	FmrbVkey::VK_y,	
/*	0x7a	*/	FmrbVkey::VK_z,	
/*	0x7b	*/	FmrbVkey::VK_LEFTBRACE,	
/*	0x7c	*/	FmrbVkey::VK_VERTICALBAR,	
/*	0x7d	*/	FmrbVkey::VK_RIGHTBRACE,	
/*	0x7e	*/	FmrbVkey::VK_TILDE,	
/*	0x7f	*/	FmrbVkey::VK_NONE,	
};

bool FmrbTerminalInput::is_visible(FmrbVkey k)
{
  if(k>=FmrbVkey::VK_SPACE && k<=FmrbVkey::VK_VISIBLE_MAX) return true;
  return false;
}

char FmrbTerminalInput::to_ascii(FmrbVkey k)
{
  switch(k){
    case FmrbVkey::VK_SPACE:
      return ' '; 

    case FmrbVkey::VK_0 ... FmrbVkey::VK_9:
      return ((int)k - (int)FmrbVkey::VK_0)+ '0';

    case FmrbVkey::VK_a ... FmrbVkey::VK_z:
      return ((int)k - (int)FmrbVkey::VK_a)+ 'a';

    case FmrbVkey::VK_A ... FmrbVkey::VK_Z:
      return ((int)k - (int)FmrbVkey::VK_A)+ 'A';

    case FmrbVkey::VK_GRAVEACCENT: return 0x60;// "`"
    case FmrbVkey::VK_ACUTEACCENT: return 0xB4;// "´"
    case FmrbVkey::VK_QUOTE: return '\'';
    case FmrbVkey::VK_QUOTEDBL: return '"';
    case FmrbVkey::VK_EQUALS: return '=';
    case FmrbVkey::VK_MINUS: return '-';
    case FmrbVkey::VK_PLUS: return '+';
    case FmrbVkey::VK_ASTERISK: return '*';
    case FmrbVkey::VK_BACKSLASH: return '\\';
    case FmrbVkey::VK_SLASH: return '/';
    case FmrbVkey::VK_PERIOD: return '.';
    case FmrbVkey::VK_COLON: return ':';
    case FmrbVkey::VK_COMMA: return ',';
    case FmrbVkey::VK_SEMICOLON: return ';';
    case FmrbVkey::VK_AMPERSAND: return '&';
    case FmrbVkey::VK_VERTICALBAR: return '|';
    case FmrbVkey::VK_HASH: return '#';
    case FmrbVkey::VK_AT: return '@';
    case FmrbVkey::VK_CARET: return '^';
    case FmrbVkey::VK_DOLLAR: return '$';
    case FmrbVkey::VK_PERCENT: return '%';
    case FmrbVkey::VK_EXCLAIM: return '!';
    case FmrbVkey::VK_QUESTION: return '?';
    case FmrbVkey::VK_LEFTBRACE: return '{';
    case FmrbVkey::VK_RIGHTBRACE: return '}';
    case FmrbVkey::VK_LEFTBRACKET: return '[';
    case FmrbVkey::VK_RIGHTBRACKET: return ']';
    case FmrbVkey::VK_LEFTPAREN: return '(';
    case FmrbVkey::VK_RIGHTPAREN: return ')';
    case FmrbVkey::VK_LESS: return '<';
    case FmrbVkey::VK_GREATER: return '>';
    case FmrbVkey::VK_UNDERSCORE: return '_';
    case FmrbVkey::VK_TILDE: return '~';
    default: return 0;
  }
  return 0;
}

FmrbVkey FmrbTerminalInput::read_vkey()
{
  int escape = 0;
  char escape_c[4] = {0};
  while(true)
  {
    if (m_terminal->available())
    {
      char c = m_terminal->read();
      FMRB_DEBUG(FMRB_LOG::DEBUG,"> %02x\n",c);

      if(!escape)
      {
        if(c>=0x20 && c<=0x7E){
          //Visible character
          return _ascii_to_vkey_map[(int)c];
        }else{
          switch(c){
            case 0x7F: // BS
              return FmrbVkey::VK_BACKSPACE;
            case 0x0D: // CR
              return FmrbVkey::VK_RETURN;
            case 0x1A: // Ctrl-z
              break;
            case 0x18: // Ctrl-x
              break;
            case 0x16: // Ctrl-v
              break;
            case 0x03: // Ctrl-c
              return FmrbVkey::VK_CTRL_C;
            case 0x04: // Ctrl-d
              return FmrbVkey::VK_CTRL_D;
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
              return FmrbVkey::VK_UP;
              case 0x42:  // ESC[B : DOWN
              return FmrbVkey::VK_DOWN;
              case 0x43:  // ESC[C : RIGHT
              return FmrbVkey::VK_RIGHT;
              case 0x44:  // ESC[D : LEFT
              return FmrbVkey::VK_LEFT;
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
                //m_terminal->read();
                return FmrbVkey::VK_F1;
              case 0x51: // ESC OP : F2
                //m_terminal->read();
                return FmrbVkey::VK_F2;
              case 0x52: // ESC OP : F3
                //m_terminal->read();
                return FmrbVkey::VK_F3;
              case 0x53: // ESC OP : F4
                //m_terminal->read();
                return FmrbVkey::VK_F4;
            }
            escape = 0;
          }else{
            escape = 0;
          }
        }else if(escape==3){
          if(escape_c[1]==0x31){
            switch(c){
              case 0x35: // ESC[15 : ..  F5
                m_terminal->read();
                return FmrbVkey::VK_F5;
              case 0x37: // ESC[17 : ..  F6
                m_terminal->read();
                return FmrbVkey::VK_F6;
              case 0x38: // ESC[18 : ..  F7
                m_terminal->read();
                return FmrbVkey::VK_F7;
              case 0x39: // ESC[19 : ..  F8
                m_terminal->read();
                return FmrbVkey::VK_F8;
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
                return FmrbVkey::VK_F9;
              case 0x31: // ESC[21 : ..  F10
                return FmrbVkey::VK_F10;
              case 0x33: // ESC[23 : ..  F11
                return FmrbVkey::VK_F11;
              case 0x34: // ESC[24 : ..  F12
                return FmrbVkey::VK_F12;
                escape_c[2] = c;
                escape = 4;
                break;
              default:
                escape = 0;
                break;
            }
          }else if(escape_c[1]==0x33){
            if(c==0x7E){ // ESC[3~ : DEL
              return FmrbVkey::VK_DELETE;
            }
            escape=0;
          }else if(escape_c[1]==0x35){
            if(c==0x7E){ // ESC[5~ : PageUp
              return FmrbVkey::VK_PAGEUP;
            }
            escape=0;
          }else if(escape_c[1]==0x36){
            if(c==0x7E){ // ESC[6~ : PageUp
              return FmrbVkey::VK_PAGEDOWN;
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


/**
 * FmrbCanvasConfig
 **/
FmrbCanvasConfig::FmrbCanvasConfig(RGB888 fg,RGB888 bg,bool fillbg, int font_width,int font_height):
  fg(fg),
  bg(bg),
  fill_bg(fillbg),
  font_width(font_width),
  font_height(font_height)
{
}

FmrbCanvasConfig::~FmrbCanvasConfig(){}

void FmrbCanvasConfig::set(fabgl::Canvas* canvas)
{
  canvas->setPenColor(fg);
  canvas->setBrushColor(bg);
  canvas->selectFont(fabgl::getPresetFixedFont(font_width,font_height));
  canvas->setGlyphOptions(GlyphOptions().FillBackground(fill_bg));
}