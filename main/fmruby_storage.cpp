
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

#include <stdio.h>
#include "fmruby.h"
#include "fmruby_fabgl.h"
#include "fmruby_app.h"

#include "esp_partition.h"

#include "FS.h"
#include "SPIFFS.h"

#include "SD.h"
#include "SPI.h"

SPIClass hspi(HSPI);

FmrbFileService::FmrbFileService():
m_spiffs_opened(false),
m_sd_opened(false),
m_sd_size(0)
{
}

FMRB_RCODE FmrbFileService::init(){  
#if 0 //debug
  const esp_partition_t* partition = esp_partition_find_first(ESP_PARTITION_TYPE_DATA,
                                      ESP_PARTITION_SUBTYPE_DATA_SPIFFS, NULL);
  if(partition){
    FMRB_DEBUG(FMRB_LOG::DEBUG,"label:%s\n",partition->label);
    FMRB_DEBUG(FMRB_LOG::DEBUG,"addr:%X\n",partition->address);
    pFMRB_DEBUG(FMRB_LOG::DEBUG,"size:%d\n",partition->size);
  }else{
    FMRB_DEBUG(FMRB_LOG::DEBUG,"spiffs partition not found\n");
  }
#endif

  mount_spiffs();

  init_sd_spi();

  mount_sd();

  return FMRB_RCODE::OK;
}


void FmrbFileService::init_sd_spi(void)
{
  AutoSuspendInterrupts autoSuspendInt;
  //gpio_pullup_en(GPIO_NUM_12);
  //pinMode(15, OUTPUT); //HSPI SS < Low Active
  hspi.begin(14,12,13,15); 
  vTaskDelay(100);
}

FMRB_RCODE FmrbFileService::mount_spiffs(void)
{
  if(m_spiffs_opened)return FMRB_RCODE::OK;

  AutoSuspendInterrupts autoSuspendInt;
  bool ret = SPIFFS.begin(false,"/spiffs",1);
  if(!ret){
    FMRB_DEBUG(FMRB_LOG::ERR,"SPIFFS Mount Failed\n");
    return FMRB_RCODE::DEVICE_ERROR;
  }
  FMRB_DEBUG(FMRB_LOG::INFO,"SPIFFS Mount OK\n");
  m_spiffs_opened = true;
  return FMRB_RCODE::OK;
}

FMRB_RCODE FmrbFileService::mount_sd(void)
{
  AutoSuspendInterrupts autoSuspendInt;
  if(!SD.begin(15,hspi,4000000,"/sd",1)){
      FMRB_DEBUG(FMRB_LOG::ERR,"Card Mount Failed\n");
      return FMRB_RCODE::DEVICE_ERROR;
  }
  uint8_t cardType = SD.cardType();

  if(cardType == CARD_NONE){
      FMRB_DEBUG(FMRB_LOG::ERR,"No SD card attached\n");
      SD.end();
      return FMRB_RCODE::NOTREADY_ERROR;
  }
  FMRB_DEBUG(FMRB_LOG::DEBUG,"mount SD OK\n");

#if 0
  FMRB_DEBUG(FMRB_LOG::DEBUG,"SD Card Type: ");
  if(cardType == CARD_MMC){
      FMRB_DEBUG(FMRB_LOG::RAW,"MMC\n");
  } else if(cardType == CARD_SD){
      FMRB_DEBUG(FMRB_LOG::RAW,"SDSC\n");
  } else if(cardType == CARD_SDHC){
      FMRB_DEBUG(FMRB_LOG::RAW,"SDHC\n");
  } else {
      FMRB_DEBUG(FMRB_LOG::RAW,"UNKNOWN\n");
  }
#endif

  uint64_t cardSize = SD.cardSize();
  FMRB_DEBUG(FMRB_LOG::INFO,"SD Card Size: %lluMB\n", cardSize / (1024 * 1024));
  m_sd_opened = true;
  return FMRB_RCODE::OK;
}

void FmrbFileService::umount_sd(void)
{
  FMRB_DEBUG(FMRB_LOG::DEBUG,"umount SD\n");
  AutoSuspendInterrupts autoSuspendInt;
  SD.end();
}

FmrbStorageType FmrbFileService::check_stype_path(const char* path)
{
  int len = strnlen(path,FMRB_MAX_PATH_LEN);
  if(len==FMRB_MAX_PATH_LEN) return FmrbStorageType::NONE; // too long or broken
  if(len<5) return FmrbStorageType::NONE; // /sd/X
  const char* cmp = strstr(path,"/sd/");
  if(cmp == path){
    return FmrbStorageType::SD;
  }
  if(len<9) return FmrbStorageType::NONE; // /spiffs/X
  cmp = strstr(path,"/spiffs/");
  if(cmp == path){
    return FmrbStorageType::SPIFFS;
  }
  return FmrbStorageType::NONE;
}

const char* FmrbFileService::to_data_path(const char* path)
{
  // "/sd/data.dat" >> "/data.dat"
  FmrbStorageType stype = check_stype_path(path);
  if(stype==FmrbStorageType::NONE) return nullptr;
  if(stype==FmrbStorageType::SD){
    return path+3;
  }
 return path+7;
}


char* FmrbFileService::load(const char* path,uint32_t &fsize,bool is_text,bool localmem){
  FMRB_DEBUG(FMRB_LOG::DEBUG,"Reading file: %s\n", path);
  FmrbStorageType stype = check_stype_path(path);
  if(stype==FmrbStorageType::NONE){
    FMRB_DEBUG(FMRB_LOG::ERR,"Bad path: %s\n", path);
    return nullptr;
  }
  if(stype==FmrbStorageType::SPIFFS){
    if(!m_spiffs_opened){
      FMRB_DEBUG(FMRB_LOG::ERR,"SPIFFS not mounted: %s\n", path);
      return NULL;
    } 
  }

  if(stype==FmrbStorageType::SD){
    if(!m_sd_opened){
      FMRB_RCODE ret = mount_sd();
      if(ret!=FMRB_RCODE::OK){
        FMRB_DEBUG(FMRB_LOG::ERR,"Cannot mound SD: %s\n", path); 
        return NULL;
      }
    } 
  }

  //---Suspend Interrupt---
  //AutoSuspendInterrupts autoSuspendInt;
  fabgl::suspendInterrupts();
  char* buff = NULL;
  File file;
  size_t rsize = 0;
  int term = 0;
  int size = 0;

  FMRB_DEBUG(FMRB_LOG::DEBUG,"- open file: %s\n",to_data_path(path));
  if(stype==FmrbStorageType::SPIFFS){
    file = SPIFFS.open(to_data_path(path));
  }else{
    file = SD.open(to_data_path(path));
  }
  
  if(!file || file.isDirectory()){
    FMRB_DEBUG(FMRB_LOG::ERR,"- failed to open file for reading\n");
    buff = nullptr;
    goto file_access_end;
  }
  if(is_text) term = 1;
  size = (int)file.size();
  FMRB_DEBUG(FMRB_LOG::DEBUG,"- read from file: size=%d\n",size);

  if(localmem){
    buff = (char*)heap_caps_malloc(size+term,MALLOC_CAP_DMA);
  }else{
    buff = (char*)fmrb_spi_malloc(size+term);
  }
  if(!buff){
    FMRB_DEBUG(FMRB_LOG::ERR,"malloc error\n");
    file.close();
    buff = nullptr;
    goto file_access_end;
  }
  rsize = file.read((uint8_t*)buff,(size_t)size);
  FMRB_DEBUG(FMRB_LOG::DEBUG,"- read done:%d\n",(int)rsize);
  file.close();
  if(rsize==0){
    fmrb_free(buff);
    buff = nullptr;
    goto file_access_end;
  }
  if(is_text){
    fsize = (uint32_t)rsize+term;
    buff[size]=(uint8_t)'\0';
  }else{
    fsize = (uint32_t)rsize;
  }

file_access_end:
  fabgl::resumeInterrupts();

  if(stype==FmrbStorageType::SD){
    umount_sd();
  }
  return buff;
}

char* FmrbFileService::load_bitmap(const char* path,uint16_t &width,uint16_t &height,uint32_t &type){
  uint32_t fsize;
  char* data = load(path,fsize,false,true);
  if(!data) return NULL;
  type = *((uint32_t*)data) ;
  width  = (data[FMRB_BITMAP_HEADER_SIZE])   + (data[FMRB_BITMAP_HEADER_SIZE+1]<<8);
  height = (data[FMRB_BITMAP_HEADER_SIZE+2]) + (data[FMRB_BITMAP_HEADER_SIZE+3]<<8);

  return data;
}

FMRB_RCODE FmrbFileService::save(char* buff,const char* path){
  FMRB_DEBUG(FMRB_LOG::DEBUG,"Writing file: %s\r\n", path);
  if(!m_spiffs_opened) return FMRB_RCODE::ERROR;

  AutoSuspendInterrupts autoSuspendInt;
  File file = SPIFFS.open(path, FILE_WRITE);
  //File file = SD.open("/default.rb", FILE_WRITE);
  if(!file){
    FMRB_DEBUG(FMRB_LOG::ERR,"- failed to open file for writing\n");
    return FMRB_RCODE::ERROR;
  }
  if(file.print(buff)){
    FMRB_DEBUG(FMRB_LOG::DEBUG,"- file written\n");
  } else {
    FMRB_DEBUG(FMRB_LOG::ERR,"- write failed\n");
    file.close();
    return FMRB_RCODE::ERROR;
  }
  FMRB_DEBUG(FMRB_LOG::DEBUG,"- save done\n");
  file.close();
  return FMRB_RCODE::OK;
}

