#include "fabgl.h"
#include "fabutils.h"

#include "fmruby_fabgl.h"

// select one color configuration
#define USE_8_COLORS  0
#define USE_64_COLORS 1

// indicate VGA GPIOs to use for selected color configuration
#define VGA_RED1   GPIO_NUM_22
#define VGA_RED0   GPIO_NUM_21
#define VGA_GREEN1 GPIO_NUM_19
#define VGA_GREEN0 GPIO_NUM_18
#define VGA_BLUE1  GPIO_NUM_5
#define VGA_BLUE0  GPIO_NUM_4
#define VGA_HSYNC  GPIO_NUM_23
#define VGA_VSYNC  GPIO_NUM_27

#define DOUBLEBUFFERING 1


void fabgl_init(void)
{
  VGAController.begin(VGA_RED1, VGA_RED0, VGA_GREEN1, VGA_GREEN0, VGA_BLUE1, VGA_BLUE0, VGA_HSYNC, VGA_VSYNC);

  VGAController.setResolution(VGA_320x200_75Hz, -1, -1, DOUBLEBUFFERING);
  VGAController.moveScreen(20, 0);
  Canvas.selectFont(Canvas.getPresetFontInfo(40, 14)); // get a font for about 40x14 text screen
  Canvas.setGlyphOptions(GlyphOptions().FillBackground(true));
}