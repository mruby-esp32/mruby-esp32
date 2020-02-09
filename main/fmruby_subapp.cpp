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
 *  Resolution test
 **/
static void check_resolution(fabgl::VGAController *vga,fabgl::Canvas *canvas,const char* mode)
{
  FMRB_DEBUG(FMRB_LOG::INFO,"Change resolution:%s\n",mode);
  vga->setResolution(mode);
  int w = vga->getScreenWidth();
  int h = vga->getScreenHeight();
  FMRB_DEBUG(FMRB_LOG::INFO,"ScreenSize:(w:%d,h:%d)\n",w,h);
  canvas->setBrushColor(Color::Blue);
  canvas->clear();
  canvas->setPenColor(Color::BrightCyan);
  canvas->drawRectangle(0,0,w-1,h-1);
  canvas->drawLine(0,0,w-1,h-1);
  canvas->drawLine(0,h-1,w-1,0);
  canvas->drawRectangle(30,30,60,60);

  canvas->setPenColor(Color::White);
  canvas->drawText(10,10,mode);


  vTaskDelay(fabgl::msToTicks(8000));

  FMRB_DEBUG(FMRB_LOG::INFO,"Change resolution to default\n");
  vga->setResolution(VGA_640x350_70HzAlt1);//Default
  canvas->setBrushColor(Color::Black);
  canvas->clear();
  vTaskDelay(fabgl::msToTicks(1000));
}

static FMRB_RCODE select_resolution(uint32_t fid,FmrbMenuModule* menu)
{
  FMRB_RCODE ret = FMRB_RCODE::OK;
  FMRB_DEBUG(FMRB_LOG::DEBUG,">menu select_resolution fid:%d\n",fid);

  switch(fid){
    case 2: check_resolution(menu->m_vga,menu->m_canvas,VGA_320x200_75Hz); break;
    case 3: check_resolution(menu->m_vga,menu->m_canvas,VGA_320x200_75HzRetro); break;
    case 4: check_resolution(menu->m_vga,menu->m_canvas,QVGA_320x240_60Hz); break;

    case 5: check_resolution(menu->m_vga,menu->m_canvas,VGA_640x200_70Hz); break;
    case 6: check_resolution(menu->m_vga,menu->m_canvas,VGA_640x200_70HzRetro); break;
    case 7: check_resolution(menu->m_vga,menu->m_canvas,VGA_640x240_60Hz); break;
    case 8: check_resolution(menu->m_vga,menu->m_canvas,VGA_640x350_70Hz); break;
    case 9: check_resolution(menu->m_vga,menu->m_canvas,VGA_640x350_70HzAlt1); break;

    case 10: check_resolution(menu->m_vga,menu->m_canvas,VGA_640x480_60Hz); break;
    case 11: check_resolution(menu->m_vga,menu->m_canvas,VGA_640x480_60HzAlt1); break;
    case 12: check_resolution(menu->m_vga,menu->m_canvas,VGA_640x480_60HzD); break;
    case 13: check_resolution(menu->m_vga,menu->m_canvas,VGA_640x480_73Hz); break;
    case 14: check_resolution(menu->m_vga,menu->m_canvas,VESA_640x480_75Hz); break;

    default:
    break;
  }

  menu->m_canvas->setPenColor(Color::White);
  menu->m_canvas->setBrushColor(Color::Black);
  menu->m_canvas->setGlyphOptions(GlyphOptions().FillBackground(true));
  return ret;
}

static FMRB_RCODE finish_submenu(uint32_t fid,FmrbMenuModule* menu)
{
  return FMRB_RCODE::OK_DONE;
}

FMRB_RCODE fmrb_subapp_resolution_test(FmrbMenuModule* menu)
{
  FMRB_DEBUG(FMRB_LOG::DEBUG,"esolution test\n");

  FmrbMenuItem *top = new FmrbMenuItem(alloc_menu_text_mem("<Select resolution>"),0,select_resolution,FmrbMenuItemType::TITLE);
  //Main
  FmrbMenuItem::add_item_tail(top,alloc_menu_text_mem(" Quit                  "),1 ,finish_submenu,FmrbMenuItemType::SELECTABLE);

  FmrbMenuItem::add_item_tail(top,alloc_menu_text_mem(" VGA_320x200_75Hz      "),2 ,select_resolution,FmrbMenuItemType::SELECTABLE);
  FmrbMenuItem::add_item_tail(top,alloc_menu_text_mem(" VGA_320x200_75HzRetro "),3 ,select_resolution,FmrbMenuItemType::SELECTABLE);
  FmrbMenuItem::add_item_tail(top,alloc_menu_text_mem(" QVGA_320x240_60Hz     "),4 ,select_resolution,FmrbMenuItemType::SELECTABLE);

  FmrbMenuItem::add_item_tail(top,alloc_menu_text_mem(" VGA_640x200_70Hz      "),5 ,select_resolution,FmrbMenuItemType::SELECTABLE);
  FmrbMenuItem::add_item_tail(top,alloc_menu_text_mem(" VGA_640x200_70HzRetro "),6 ,select_resolution,FmrbMenuItemType::SELECTABLE);
  FmrbMenuItem::add_item_tail(top,alloc_menu_text_mem(" VGA_640x240_60Hz      "),7 ,select_resolution,FmrbMenuItemType::SELECTABLE);
  FmrbMenuItem::add_item_tail(top,alloc_menu_text_mem(" VGA_640x350_70Hz      "),8 ,select_resolution,FmrbMenuItemType::SELECTABLE);
  FmrbMenuItem::add_item_tail(top,alloc_menu_text_mem(" VGA_640x350_70HzAlt1  "),9,select_resolution,FmrbMenuItemType::SELECTABLE);

  FmrbMenuItem::add_item_tail(top,alloc_menu_text_mem(" VGA_640x480_60Hz      "),10,select_resolution,FmrbMenuItemType::SELECTABLE);
  FmrbMenuItem::add_item_tail(top,alloc_menu_text_mem(" VGA_640x480_60HzAlt1  "),11,select_resolution,FmrbMenuItemType::SELECTABLE);
  FmrbMenuItem::add_item_tail(top,alloc_menu_text_mem(" VGA_640x480_60HzD     "),12,select_resolution,FmrbMenuItemType::SELECTABLE);
  FmrbMenuItem::add_item_tail(top,alloc_menu_text_mem(" VGA_640x480_73Hz      "),13,select_resolution,FmrbMenuItemType::SELECTABLE);
  FmrbMenuItem::add_item_tail(top,alloc_menu_text_mem(" VESA_640x480_75Hz     "),14,select_resolution,FmrbMenuItemType::SELECTABLE);

  menu->m_canvas->clear();
  FmrbMenuModule *localMenu = new FmrbMenuModule(menu->m_vga,menu->m_canvas,menu->m_terminal,top);

  //Message and wait

  localMenu->begin(nullptr);

  //end of sub app
  delete localMenu;

  return FMRB_RCODE::OK;
}

