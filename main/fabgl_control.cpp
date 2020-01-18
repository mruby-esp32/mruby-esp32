#include "fabgl.h"
#include "fmruby_fabgl.h"

// indicate VGA GPIOs to use for selected color configuration for Narya borad
#define VGA_RED1   GPIO_NUM_22
#define VGA_RED0   GPIO_NUM_21
#define VGA_GREEN1 GPIO_NUM_19
#define VGA_GREEN0 GPIO_NUM_18
#define VGA_BLUE1  GPIO_NUM_5
#define VGA_BLUE0  GPIO_NUM_4
#define VGA_HSYNC  GPIO_NUM_23
#define VGA_VSYNC  GPIO_NUM_27

#define DOUBLEBUFFERING 1

fabgl::VGAController VGAController;
fabgl::PS2Controller PS2Controller;
fabgl::Terminal      Terminal;
fabgl::Canvas        FMRB_canvas(&VGAController);

void fabgl_init(void)
{

  PS2Controller.begin(PS2Preset::KeyboardPort0,KbdMode::GenerateVirtualKeys);

  VGAController.begin(VGA_RED1, VGA_RED0, VGA_GREEN1, VGA_GREEN0, VGA_BLUE1, VGA_BLUE0, VGA_HSYNC, VGA_VSYNC);
  
  //VGAController.setResolution(VGA_320x200_75Hz, 320, 200);
  //VGAController.setResolution(VGA_512x384_60Hz, 512, 384,false);
}

void fabgl_terminal_mode_init(void)
{
  printf("fabgl_terminal_mode_init\n");
  
  VGAController.setResolution(VGA_640x350_70HzAlt1, 640, 350, false);
  //VGAController.setResolution(VGA_512x384_60Hz, 512, 384,false);
  VGAController.moveScreen(20, 0);
  printf("fabgl_terminal_mode_init done\n");
}

void fabgl_mruby_mode_init(void)
{
  printf("fabgl_mruby_mode_init\n");
  VGAController.setResolution(VGA_320x200_75Hz, -1, -1, true);

  //Canvas.selectFont(Canvas.getPresetFontInfo(40, 14)); // get a font for about 40x14 text screen
  //Canvas.setGlyphOptions(GlyphOptions().FillBackground(true));
  printf("fabgl_mruby_mode_init done\n");
}
