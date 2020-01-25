
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

FmrbFileService file_service;

//#define DEFAULT_TEST_PATH "/test.rb"
FmrbFileService::FmrbFileService(){
  m_spiffs_opened=false;
  m_sd_opened=false;
}

#define FORMAT_SPIFFS_IF_FAILED false

static int init_sd()
{
  //gpio_pullup_en(GPIO_NUM_12);
  hspi.begin(14,12,13,15); 
  //pinMode(15, OUTPUT); //HSPI SS
  vTaskDelay(500);

 if(!SD.begin(15,hspi,4000000,"/sd",1)){
      FMRB_DEBUG(FMRB_LOG::ERR,"Card Mount Failed\n");
      return -1;
  }
  uint8_t cardType = SD.cardType();

  if(cardType == CARD_NONE){
      FMRB_DEBUG(FMRB_LOG::ERR,"No SD card attached\n");
      return -1;
  }

#if 0
  printf("SD Card Type: ");
  if(cardType == CARD_MMC){
      printf("MMC\n");
  } else if(cardType == CARD_SD){
      printf("SDSC\n");
  } else if(cardType == CARD_SDHC){
      printf("SDHC\n");
  } else {
      printf("UNKNOWN\n");
  }
#endif

  uint64_t cardSize = SD.cardSize() / (1024 * 1024);
  FMRB_DEBUG(FMRB_LOG::INFO,"SD Card Size: %lluMB\n", cardSize);
  return 0;
}

int FmrbFileService::init(){
  AutoSuspendInterrupts autoSuspendInt;
  
  /*
  const esp_partition_t* partition = esp_partition_find_first(ESP_PARTITION_TYPE_DATA,
                                      ESP_PARTITION_SUBTYPE_DATA_SPIFFS, NULL);
  if(partition){
    FMRB_DEBUG(FMRB_LOG::DEBUG,"label:%s\n",partition->label);
    FMRB_DEBUG(FMRB_LOG::DEBUG,"addr:%X\n",partition->address);
    pFMRB_DEBUG(FMRB_LOG::DEBUG,size:%d\n",partition->size);
  }else{
    FMRB_DEBUG(FMRB_LOG::DEBUG,"spiffs partition not found\n");
  }
  */

  bool ret = SPIFFS.begin(FORMAT_SPIFFS_IF_FAILED,"/spiffs",1);
  if(!ret){
    FMRB_DEBUG(FMRB_LOG::ERR,"SPIFFS Mount Failed\n");
    return -1;
  }
  FMRB_DEBUG(FMRB_LOG::DEBUG,"SPIFFS Mount OK\n");
  m_spiffs_opened=true;

  int sd_stat = init_sd();
  if(sd_stat>=0){
    m_sd_opened = true;
  }else{
    m_sd_opened = false;
  }

  return 0;
}

char* FmrbFileService::load(const char* path,uint32_t &fsize,bool is_text,bool localmem){
  FMRB_DEBUG(FMRB_LOG::DEBUG,"Reading file: %s\r\n", path);
  if(!m_spiffs_opened) return NULL;

  AutoSuspendInterrupts autoSuspendInt;
  File file = SPIFFS.open(path);
  //File file = SD.open("/default.rb");
  if(!file || file.isDirectory()){
    FMRB_DEBUG(FMRB_LOG::ERR,"- failed to open file for reading\n");
    return NULL;
  }
  int term = 0;
  if(is_text) term = 1;
  int size = (int)file.size();
  FMRB_DEBUG(FMRB_LOG::DEBUG,"- read from file: size=%d\n",size);

  char* buff = NULL;
  if(localmem){
    buff = (char*)malloc(size+term);
  }else{
    buff = (char*)fmrb_spi_malloc(size+term);
  }
  if(!buff){
    FMRB_DEBUG(FMRB_LOG::ERR,"malloc error\n");
    file.close();
    return NULL;
  }
  size_t rsize = file.read((uint8_t*)buff,(size_t)size);
  FMRB_DEBUG(FMRB_LOG::DEBUG,"- read done:%d\n",(int)rsize);
  file.close();
  if(rsize==0){
    free(buff);
    return NULL;
  }
  if(is_text){
    fsize = (uint32_t)rsize+term;
    buff[size]=(uint8_t)'\0';
  }else{
    fsize = (uint32_t)rsize;
  }
  return buff;
}

char* FmrbFileService::load_bitmap(const char* path,uint16_t &width,uint16_t &height,uint32_t &type){
  uint32_t fsize;
  char* data = load(path,fsize,false,true);
  if(!data) return NULL;
  type = *((uint32_t*)data) ;
  width  = (data[FMRB_BITMAP_HEADER_SIZE]) + (data[FMRB_BITMAP_HEADER_SIZE+1]<<8);
  height = (data[FMRB_BITMAP_HEADER_SIZE+2]) + (data[FMRB_BITMAP_HEADER_SIZE+3]<<8);

  return data;
}

int FmrbFileService::save(char* buff,const char* path){
  FMRB_DEBUG(FMRB_LOG::DEBUG,"Writing file: %s\r\n", path);
  if(!m_spiffs_opened) return -1;

  AutoSuspendInterrupts autoSuspendInt;
  File file = SPIFFS.open(path, FILE_WRITE);
  //File file = SD.open("/default.rb", FILE_WRITE);
  if(!file){
    FMRB_DEBUG(FMRB_LOG::ERR,"- failed to open file for writing\n");
    return -1;
  }
  if(file.print(buff)){
    FMRB_DEBUG(FMRB_LOG::DEBUG,"- file written\n");
  } else {
    FMRB_DEBUG(FMRB_LOG::ERR,"- write failed\n");
    file.close();
    return -1;
  }
  FMRB_DEBUG(FMRB_LOG::DEBUG,"- save done\n");
  file.close();
  return 0;
}

