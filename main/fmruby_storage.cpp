
#include <stdio.h>
#include "fmruby.h"
#include "fmruby_fabgl.h"
#include "fmruby_app.h"


#include "FS.h"
#include "SPIFFS.h"

#include "SD.h"
#include "SPI.h"
SPIClass hspi(HSPI);

FmrbFileService file_service;

#define DEFAULT_TEST_PATH "/spiffs/default.rb"
FmrbFileService::FmrbFileService(){
  m_spiffs_opened=false;
  m_sd_opened=false;
}

#define FORMAT_SPIFFS_IF_FAILED false

/*
static void suspend_int(){
  fabgl::suspendInterrupts();
  vTaskDelay(50);
}
static void resume_int(){
  fabgl::resumeInterrupts();
  vTaskDelay(50);
}
*/

static int init_sd()
{
  //gpio_pullup_en(GPIO_NUM_12);
  hspi.begin(14,12,13,15); 
  //pinMode(15, OUTPUT); //HSPI SS
  vTaskDelay(500);

 if(!SD.begin(15,hspi,4000000,"/sd",1)){
      printf("Card Mount Failed\n");
      return -1;
  }
  uint8_t cardType = SD.cardType();

  if(cardType == CARD_NONE){
      printf("No SD card attached\n");
      return -1;
  }

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
  uint64_t cardSize = SD.cardSize() / (1024 * 1024);
  printf("SD Card Size: %lluMB\n", cardSize);
  return 0;
}

int FmrbFileService::init(){
  AutoSuspendInterrupts autoSuspendInt;
  bool ret = SPIFFS.begin(FORMAT_SPIFFS_IF_FAILED,"/spiffs",1);
  if(!ret){
    printf("SPIFFS Mount Failed\n");
    return -1;
  }
  printf("SPIFFS Mount OK\n");
  m_spiffs_opened=true;

  int sd_stat = init_sd();
  if(sd_stat>=0){
    m_sd_opened = true;
  }else{
    m_sd_opened = false;
  }

  return 0;
}

char* FmrbFileService::load(){
  printf("Reading file: %s\r\n", DEFAULT_TEST_PATH);
  if(!m_spiffs_opened) return NULL;

  AutoSuspendInterrupts autoSuspendInt;
  File file = SPIFFS.open(DEFAULT_TEST_PATH);
  //File file = SD.open("/default.rb");
  if(!file || file.isDirectory()){
    printf("- failed to open file for reading\n");
    return NULL;
  }
  printf("- read from file: size=%d\n",(int)file.size());
  char* buff = (char*)fmrb_spi_malloc((int)file.size()+1);
  if(!buff){
    printf("malloc error\n");
    return NULL;
  }
  file.read((uint8_t*)buff,(size_t)file.size());
  return buff;
}
int FmrbFileService::save(char* buff){
  printf("Writing file: %s\r\n", DEFAULT_TEST_PATH);
  if(!m_spiffs_opened) return -1;

  AutoSuspendInterrupts autoSuspendInt;
  File file = SPIFFS.open(DEFAULT_TEST_PATH, FILE_WRITE);
  //File file = SD.open("/default.rb", FILE_WRITE);
  if(!file){
    printf("- failed to open file for writing\n");
    return -1;
  }
  if(file.print(buff)){
    printf("- file written\n");
  } else {
    printf("- write failed\n");
    return -1;
  }
  printf("- save done\n");
  return 0;
}

