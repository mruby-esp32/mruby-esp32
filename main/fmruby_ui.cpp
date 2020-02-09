/*
 Created by Katsuhiko KAGEYAMA(@kishima) - <https://silentworlds.info>
 Copyright (c) 2019-2020 Katsuhiko KAGEYAMA.
 All rights reserved.

 This file is part of Family mruby.

 Family mruby is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 Family mruby is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with Family mruby.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "fmruby.h"
#include "fmruby_app.h"
#include "fmruby_fabgl.h"


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
m_param(nullptr),
m_storage(nullptr)
{
  m_canvas_config = new FmrbCanvasConfig(RGB888(255,255,255),RGB888(0,0,0),true,8,14);
}

FmrbMenuModule::~FmrbMenuModule(){
  delete (m_top);
  if(m_canvas_config) delete m_canvas_config;
}

void FmrbMenuModule::begin(uint32_t *param){
  m_canvas_config->set(m_canvas);
  m_param = param;

  clear_draw_area();
  exec_menu(m_top);
  
  m_canvas_config->set(m_canvas);
  m_canvas->clear();
}

void FmrbMenuModule::set_param(uint32_t param){
  if(m_param){
    *m_param = param;
  }
}

void FmrbMenuModule::set_storage(FmrbFileService *storage){
  m_storage = storage;
}

void FmrbMenuModule::clear_draw_area(void){
  m_canvas->setBrushColor(Color::Black);
  m_canvas->clear();
  if(m_storage){
    uint32_t fsize=0;
    uint8_t* img_data = (uint8_t*)m_storage->load("/assets/2bit_logo.img",fsize,false,false);
    if(img_data){
      FmrbSystemApp::draw_img(m_vga,400,50,img_data,0);
      fmrb_free(img_data);
    }
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

