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

#include "fabgl.h"
#include "fmruby.h"
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

#define DOUBLEBUFFERING true

// Shared with mgem
fabgl::VGAController VGAController;
fabgl::PS2Controller PS2Controller;
fabgl::Canvas        FMRB_canvas(&VGAController);

void fabgl_init(void)
{
  PS2Controller.begin(PS2Preset::KeyboardPort0);
  VGAController.begin(VGA_RED1, VGA_RED0, VGA_GREEN1, VGA_GREEN0, VGA_BLUE1, VGA_BLUE0, VGA_HSYNC, VGA_VSYNC);
}

void fabgl_terminal_mode_init(FmrbConfig* config)
{
  FMRB_DEBUG(FMRB_LOG::DEBUG,"Main screen: %s\n",config->main_mode_line);
  VGAController.setResolution(config->main_mode_line);
  VGAController.moveScreen(config->main_screen_shift_x, config->main_screen_shift_y);
}

void fabgl_mruby_mode_init(FmrbConfig* config)
{
  FMRB_DEBUG(FMRB_LOG::DEBUG,"Mruby screen: %s\n",config->mruby_mode_line);
  VGAController.setResolution(config->mruby_mode_line, -1, -1, DOUBLEBUFFERING);
  VGAController.moveScreen(config->mruby_screen_shift_x, config->mruby_screen_shift_y);
}
