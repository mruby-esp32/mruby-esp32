#include "fmruby.h"
#include "fmruby_app.h"
#include "fmruby_fabgl.h"

/**
 *  Resolution test
 **/
static void check_resolution(const char* mode)
{
  FMRB_DEBUG(FMRB_LOG::INFO,"Change resolution:%s\n",mode);
  VGAController.setResolution(mode);
  int w = VGAController.getScreenWidth();
  int h = VGAController.getScreenHeight();
  FMRB_DEBUG(FMRB_LOG::INFO,"ScreenSize:(w:%d,h:%d)\n",w,h);
  FMRB_canvas.setBrushColor(Color::Blue);
  FMRB_canvas.clear();
  FMRB_canvas.setPenColor(Color::BrightCyan);
  FMRB_canvas.drawRectangle(0,0,w-1,h-1);
  FMRB_canvas.drawLine(0,0,w-1,h-1);
  FMRB_canvas.drawLine(0,h-1,w-1,0);
  FMRB_canvas.drawRectangle(30,30,60,60);

  FMRB_canvas.setPenColor(Color::White);
  FMRB_canvas.drawText(10,10,mode);


  vTaskDelay(fabgl::msToTicks(8000));

  FMRB_DEBUG(FMRB_LOG::INFO,"Change resolution to default\n");
  VGAController.setResolution(VGA_640x350_70HzAlt1);//Default
  FMRB_canvas.setBrushColor(Color::Black);
  FMRB_canvas.clear();
  vTaskDelay(fabgl::msToTicks(1000));
}

static FMRB_RCODE select_resolution(uint32_t fid,FmrbMenuModule* menu)
{
  FMRB_RCODE ret = FMRB_RCODE::OK;
  FMRB_DEBUG(FMRB_LOG::DEBUG,">menu select_resolution fid:%d\n",fid);

  switch(fid){
    case 2: check_resolution(VGA_320x200_75Hz); break;
    case 3: check_resolution(VGA_320x200_75HzRetro); break;
    case 4: check_resolution(QVGA_320x240_60Hz); break;

    case 5: check_resolution(VGA_640x200_70Hz); break;
    case 6: check_resolution(VGA_640x200_70HzRetro); break;
    case 7: check_resolution(VGA_640x240_60Hz); break;
    case 8: check_resolution(VGA_640x350_70Hz); break;
    case 9: check_resolution(VGA_640x350_70HzAlt1); break;

    case 10: check_resolution(VGA_640x480_60Hz); break;
    case 11: check_resolution(VGA_640x480_60HzAlt1); break;
    case 12: check_resolution(VGA_640x480_60HzD); break;
    case 13: check_resolution(VGA_640x480_73Hz); break;
    case 14: check_resolution(VESA_640x480_75Hz); break;

    default:
    break;
  }

  FMRB_canvas.setPenColor(Color::White);
  FMRB_canvas.setBrushColor(Color::Black);
  FMRB_canvas.setGlyphOptions(GlyphOptions().FillBackground(true));
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

  FMRB_canvas.clear();
  FmrbMenuModule *localMenu = new FmrbMenuModule(menu->m_canvas,menu->m_terminal,top);

  //Message and wait

  localMenu->begin(nullptr);

  //end of sub app
  delete localMenu;
  
  return FMRB_RCODE::OK;
}

