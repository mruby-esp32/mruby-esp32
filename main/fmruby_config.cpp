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
  uint32_t size;
  const char* config_txt = fs->load("/spiffs/config/config.txt",size,true,false);
  if(config_txt){
    FMRB_RCODE rcode = read_value_str(config_txt,"main_mode_line",main_mode_line);
    if(rcode!=FMRB_RCODE::OK){ set_default_value();return FMRB_RCODE::OK; }

    rcode = read_value_int(config_txt,"main_screen_shift_x",&main_screen_shift_x);
    if(rcode!=FMRB_RCODE::OK){ set_default_value();return FMRB_RCODE::OK; }

    rcode = read_value_int(config_txt,"main_screen_shift_y",&main_screen_shift_y);
    if(rcode!=FMRB_RCODE::OK){ set_default_value();return FMRB_RCODE::OK; }


    rcode = read_value_str(config_txt,"mruby_mode_line",mruby_mode_line);
    if(rcode!=FMRB_RCODE::OK){ set_default_value();return FMRB_RCODE::OK; }

    rcode = read_value_int(config_txt,"mruby_screen_shift_x",&mruby_screen_shift_x);
    if(rcode!=FMRB_RCODE::OK){ set_default_value();return FMRB_RCODE::OK; }

    rcode = read_value_int(config_txt,"mruby_screen_shift_y",&mruby_screen_shift_y);
    if(rcode!=FMRB_RCODE::OK){ set_default_value();return FMRB_RCODE::OK; }

  }else{
    set_default_value();
  }
  return FMRB_RCODE::OK;
}

FMRB_RCODE FmrbConfig::save(FmrbFileService *fs)
{
  return FMRB_RCODE::OK;
  
}

FMRB_RCODE FmrbConfig::read_value_str(const char* txt,const char* key,char* out)
{
  FMRB_RCODE rcode = FMRB_RCODE::NOTFOUND_ERROR;
  int base = 0;
  bool key_found = false;
  char* work_key = (char*)fmrb_spi_malloc(FMRB_CONFIG_KEY_MAX+1);
  char* work_val = (char*)fmrb_spi_malloc(FMRB_MODE_LINE_MAX);
  for(int i=0;i<=strlen(txt);i++){
    if( *(txt+i) == ':' ){
      if(i-base>=FMRB_CONFIG_KEY_MAX || i==base){
        FMRB_DEBUG(FMRB_LOG::ERR,"Bad Config Format(%d,%d)\n",i,base);
        rcode = FMRB_RCODE::ERROR;
        goto finish_read;
      }
      memcpy(work_key,txt+base,i-base);
      work_key[i-base]='\0';
      //FMRB_DEBUG(FMRB_LOG::DEBUG,"KEY(%s):%d,%d,len=%d\n",work_key,base,i,i-base);
      key_found = true;
      base = i+1;
    }else if(*(txt+i) == '\n' || *(txt+i) == '\0'){
      if(i-base>=FMRB_MODE_LINE_MAX || i==base){
        FMRB_DEBUG(FMRB_LOG::ERR,"Bad Config Format(%d,%d)\n",i,base);
        rcode = FMRB_RCODE::ERROR;
        goto finish_read;
      }
      if(key_found){
        memcpy(work_val,txt+base,i-base);
        work_val[i-base]='\0';
        //FMRB_DEBUG(FMRB_LOG::DEBUG,"VAL(%s):%d,%d,len=%d\n",work_val,base,i,i-base);
        if(0==strcmp(work_key,key)){
          strcpy(out,work_val);
          rcode = FMRB_RCODE::OK;
          goto finish_read;
        }
      }
      base = i+1;
      key_found=false;
    }
  }
finish_read:
  fmrb_free(work_key);
  fmrb_free(work_val);
  return rcode;
}


FMRB_RCODE FmrbConfig::read_value_int(const char* txt,const char* key,int32_t* out)
{
  char* work_val = (char*)fmrb_spi_malloc(FMRB_MODE_LINE_MAX);
  FMRB_RCODE rcode = read_value_str(txt,key,work_val);
  if(rcode==FMRB_RCODE::OK){
    *out = atoi(work_val);
  }
  fmrb_free(work_val);
  return rcode;
}


void FmrbConfig::set_default_value(void)
{
  strncpy(main_mode_line,VGA_640x350_70HzAlt1,FMRB_MODE_LINE_MAX);
  strncpy(mruby_mode_line,VGA_320x200_75Hz,FMRB_MODE_LINE_MAX);
  main_screen_shift_x = 0;
  main_screen_shift_y = 0;
  mruby_screen_shift_x = 0;
  mruby_screen_shift_y = 0;
}