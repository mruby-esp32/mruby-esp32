
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

#include "SPIFFS.h"
#include "SD.h"
#include "SPI.h"

SPIClass hspi(HSPI);

FmrbDir::FmrbDir(FmrbStorageType t):
type(t),
length(0),
dir_path(nullptr)
{
  memset(path_list,0,sizeof(char*)*FMRB_MAX_DIRLIST_LEN);
}

FmrbDir::~FmrbDir()
{
  for(int i=0;i<FMRB_MAX_DIRLIST_LEN;i++){
    if(path_list[i]){
      fmrb_free(path_list[i]);
    }
  }
  if(dir_path)fmrb_free(dir_path);
}

namespace{
  const char* StorageTypeString[] = {
    "",
    "/spiffs",
    "/sd"
  };
}

const char* FmrbDir::type_to_path(FmrbStorageType type)
{
  switch(type){
    case FmrbStorageType::SPIFFS:
    return StorageTypeString[1];
    case FmrbStorageType::SD:
    return StorageTypeString[2];
    default:
    break;
  }
  return StorageTypeString[0];
}

void FmrbDir::set(File *dir,const char* dir_name)
{
  if(dir_path){
    FMRB_DEBUG(FMRB_LOG::ERR,"Already set\n");
    return;
  }
  if(!dir_name) return;

  int index = 0;
  dir_path = (char*)fmrb_spi_malloc(1+strnlen(dir_name,FMRB_MAX_PATH_LEN)); 
  strcpy(dir_path,dir_name);

  while (true) {
    File entry = dir->openNextFile();
    if (!entry) {
      break;
    }
    const char* epath = entry.name();
    if(!epath)continue;
    const char* removed_path = remove_base_dir(epath,dir_name);
    if(removed_path && is_file(removed_path)){
      fmrb_dump_mem_stat();
      path_list[index] = (char*)fmrb_spi_malloc(1+strnlen(removed_path,FMRB_MAX_PATH_LEN)); 
      strcpy(path_list[index],removed_path);
      //FMRB_DEBUG(FMRB_LOG::DEBUG,"File(%s) > %s\n",dir_path,path_list[index]);
      index++;
      if(index>=FMRB_MAX_DIRLIST_LEN){
        FMRB_DEBUG(FMRB_LOG::ERR,"Too many files\n");
        break;
      }
    }
  }
  length = index;
}

const char* FmrbDir::fetch_path(int n)
{
  if(n>=length){
    FMRB_DEBUG(FMRB_LOG::ERR,"bad index(%d)\n",n);
    return nullptr;
  }
  return path_list[n];
}

bool FmrbDir::is_file(const char* path)
{
  if(!path)return false;
  const char* cmp = strstr(path,"/");
  if(cmp==NULL) return true;
  return false;
}

bool FmrbDir::is_dir(const char* path)
{
  // /sd/aaa/ > true,  /sd/aaa > false
  if(!path)return false;
  int len = strnlen(path,FMRB_MAX_PATH_LEN);
  if(len<=0) return false;
  if( *(path+len-1) == '/' ){
    return true;
  }
  return false;
}


const char* FmrbDir::remove_base_dir(const char* in,const char* base)
{
  // "/sd/aaa/bbb/ccc.dat", "/sd/aaa/" > "bbb/ccc.dat"
  const char* cmp = strstr(in,base);
  //FMRB_DEBUG(FMRB_LOG::DEBUG,"%s(%s)\n",in,base);
  if(cmp != in){
    return nullptr;
  }
  //FMRB_DEBUG(FMRB_LOG::DEBUG,">>(%s)\n",in + strnlen(base,FMRB_MAX_PATH_LEN)-1);
  return in + strnlen(base,FMRB_MAX_PATH_LEN);
}


/**
 * FmrbFileService
 **/
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
  /*
  hspi.begin(14,12,13,15); 
  vTaskDelay(100);
  */
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

extern fabgl::VGAController VGAController;

FMRB_RCODE FmrbFileService::mount_sd(void)
{
  //VGAController.setResolution();
  AutoSuspendInterrupts autoSuspendInt;
  
  hspi.begin(14,12,13,15); 
  vTaskDelay(100);

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
  if(len<4) return FmrbStorageType::NONE; // /sd/
  const char* cmp = strstr(path,"/sd/");
  if(cmp == path){
    return FmrbStorageType::SD;
  }
  if(len<8) return FmrbStorageType::NONE; // /spiffs/
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

FMRB_RCODE FmrbFileService::precheck_path(const char* path)
{
  FmrbStorageType stype = check_stype_path(path);
  if(stype==FmrbStorageType::NONE){
    FMRB_DEBUG(FMRB_LOG::ERR,"Bad path: %s\n", path);
    return FMRB_RCODE::ERROR;
  }
  if(stype==FmrbStorageType::SPIFFS){
    if(!m_spiffs_opened){
      FMRB_DEBUG(FMRB_LOG::ERR,"SPIFFS not mounted: %s\n", path);
      return FMRB_RCODE::ERROR;
    } 
  }

  if(stype==FmrbStorageType::SD){
    if(!m_sd_opened){
      FMRB_RCODE ret = mount_sd();
      if(ret!=FMRB_RCODE::OK){
        FMRB_DEBUG(FMRB_LOG::ERR,"Cannot mound SD: %s\n", path); 
        return FMRB_RCODE::ERROR;
      }
    } 
  }
  return FMRB_RCODE::OK;
}

char* FmrbFileService::load(const char* path,uint32_t &fsize,bool is_text,bool localmem){
  FMRB_DEBUG(FMRB_LOG::DEBUG,"Reading file: %s\n", path);
  if(precheck_path(path)!=FMRB_RCODE::OK){
    return nullptr;
  }
  FmrbStorageType stype = check_stype_path(path);

  //---Suspend Interrupt---
  //AutoSuspendInterrupts autoSuspendInt;
  fabgl::suspendInterrupts();
  char* buff = nullptr;
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
  
  if(!file){
    FMRB_DEBUG(FMRB_LOG::ERR,"- failed to open file for reading\n");
    buff = nullptr;
    goto file_access_end;
  }
  if(file.isDirectory()){
    FMRB_DEBUG(FMRB_LOG::ERR,"- file is directory\n");
    buff = nullptr;
    file.close();
    goto file_access_end;
  }
  if(is_text) term = 1;
  size = (int)file.size();
  FMRB_DEBUG(FMRB_LOG::DEBUG,"- read from file: size=%d\n",size);

  if(localmem){
    FMRB_DEBUG(FMRB_LOG::DEBUG,"- malloc local\n");
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
  if(!data) return nullptr;
  type = *((uint32_t*)data) ;
  width  = (data[FMRB_BITMAP_HEADER_SIZE])   + (data[FMRB_BITMAP_HEADER_SIZE+1]<<8);
  height = (data[FMRB_BITMAP_HEADER_SIZE+2]) + (data[FMRB_BITMAP_HEADER_SIZE+3]<<8);

  return data;
}


FmrbDir* FmrbFileService::get_dir_obj(const char* dir_path){
  FmrbDir* dir_obj = nullptr;
  FMRB_DEBUG(FMRB_LOG::DEBUG,"get_list: %s\n", dir_path);
  if(precheck_path(dir_path)!=FMRB_RCODE::OK){
    return dir_obj;
  }
  FmrbStorageType stype = check_stype_path(dir_path);

  FMRB_DEBUG(FMRB_LOG::DEBUG,"- open dir: %s (%d)\n",dir_path,FmrbDir::is_dir(dir_path));
  //---Suspend Interrupt---
  AutoSuspendInterrupts autoSuspendInt;
  File base_dir;
  if(stype==FmrbStorageType::SPIFFS){
    base_dir = SPIFFS.open("/");
    dir_obj = new FmrbDir(FmrbStorageType::SPIFFS);
  }else{
    base_dir = SD.open(to_data_path(dir_path));
    dir_obj = new FmrbDir(FmrbStorageType::SD);
  }
  
  if(!base_dir || !base_dir.isDirectory()){
    FMRB_DEBUG(FMRB_LOG::ERR,"- failed to open dir for reading\n");
    return dir_obj;
  }
  
  dir_obj->set(&base_dir,to_data_path(dir_path));

  base_dir.close();
  if(stype==FmrbStorageType::SD){
    umount_sd();
  }
  return dir_obj;
}


FMRB_RCODE FmrbFileService::save(char* buff,const char* path){
  FMRB_DEBUG(FMRB_LOG::DEBUG,"Writing file: %s\n", path);
  if(precheck_path(path)!=FMRB_RCODE::OK){
    return FMRB_RCODE::ERROR;
  }

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

