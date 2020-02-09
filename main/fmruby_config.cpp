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


FmrbConfig::FmrbConfig():
main_screen_shift_x(0),
main_screen_shift_y(0),
mruby_screen_shift_x(0),
mruby_screen_shift_y(0)
{
  memset(main_mode_line,0,FMRB_MODE_LINE_MAX);
  memset(mruby_mode_line,0,FMRB_MODE_LINE_MAX);
}

FMRB_RCODE FmrbConfig::load(FmrbFileService *fs)
{
  return FMRB_RCODE::OK;
}

FMRB_RCODE FmrbConfig::save(FmrbFileService *fs)
{
  return FMRB_RCODE::OK;
  
}