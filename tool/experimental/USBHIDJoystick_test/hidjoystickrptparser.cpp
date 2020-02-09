#include "hidjoystickrptparser.h"

#include <SoftwareSerial.h>
extern SoftwareSerial mySerial;

JoystickReportParser::JoystickReportParser(JoystickEvents *evt) :
joyEvents(evt),
oldHat(0xDE),
oldButtons(0) {
        for (uint8_t i = 0; i < RPT_GEMEPAD_LEN; i++)
                oldPad[i] = 0xD;
}

void JoystickReportParser::Parse(USBHID *hid, bool is_rpt_id, uint8_t len, uint8_t *buf) {
        bool match = true;
        /*
        Serial.println(len);
        for (uint8_t i = 0; i < len; i++){
          Serial.print(buf[i]);
          Serial.print(" ");
        }
          Serial.print("\n");
      */
        // Checking if there are changes in report since the method was last called
        for (uint8_t i = 0; i < RPT_GEMEPAD_LEN; i++)
                if (buf[i] != oldPad[i]) {
                        match = false;
                        break;
                }

        // Calling Game Pad event handler
        if (!match && joyEvents) {
                joyEvents->OnGamePadChanged((const GamePadEventData*)buf);

                for (uint8_t i = 0; i < RPT_GEMEPAD_LEN; i++) oldPad[i] = buf[i];
        }

        uint8_t hat = (buf[5] & 0xF);

        // Calling Hat Switch event handler
        if (hat != oldHat && joyEvents) {
                joyEvents->OnHatSwitch(hat);
                oldHat = hat;
        }

        uint16_t buttons = (0x0000 | buf[6]);
        buttons <<= 4;
        buttons |= (buf[5] >> 4);
        uint16_t changes = (buttons ^ oldButtons);

        // Calling Button Event Handler for every button changed
        if (changes) {
                for (uint8_t i = 0; i < 0x0C; i++) {
                        uint16_t mask = (0x0001 << i);

                        if (((mask & changes) > 0) && joyEvents) {
                                if ((buttons & mask) > 0)
                                        joyEvents->OnButtonDn(i + 1);
                                else
                                        joyEvents->OnButtonUp(i + 1);
                        }
                }
                oldButtons = buttons;
        }
}

#define K_UP    0
#define K_DOWN  1
#define K_LEFT  2
#define K_RIGHT 3

static char kmap[4] = {0};
static const uint8_t kcode[4] = {9,10,11,12};

void check_key(int key,const char *nmap){
  if(kmap[key]!=nmap[key]){
    uint8_t flag = nmap[key]==0 ? 0x80 : 0x00;
    mySerial.write(kcode[key] | flag);

    Serial.print("DIR:");
    Serial.println(kcode[key] | flag, DEC);
  }
}
void send_map(const char* nmap){
  for(int i=0; i< 4; i++){
    check_key(i,nmap);
    kmap[i]=nmap[i];
  }
}

void JoystickEvents::OnGamePadChanged(const GamePadEventData *evt) {
  /*
  上
    X1: 01  Y1: 7F  X2: 7F  Y2: 7F  Rz: 00
    X1: 01  Y1: 7F  X2: 7F  Y2: 7F  Rz: 7F
　右
    X1: 01  Y1: 7F  X2: 7F  Y2: FF  Rz: 7F 
    X1: 01  Y1: 7F  X2: 7F  Y2: 7F  Rz: 7F
  下 
    X1: 01  Y1: 7F  X2: 7F  Y2: 7F  Rz: FF
    X1: 01  Y1: 7F  X2: 7F  Y2: 7F  Rz: 7F
  左
    X1: 01  Y1: 7F  X2: 7F  Y2: 00  Rz: 7F
    X1: 01  Y1: 7F  X2: 7F  Y2: 00  Rz: 00    
   */
   uint8_t k1 = evt->Z2;
   uint8_t k2 = evt->Rz;
   char current_map[4]={0};

   if(k1==0x7F && k2==0x7F){
     send_map(current_map);
     return;
   }

   for(int i=0; i< 4; i++){
    current_map[i]=kmap[i];
   }
   if(k1==0x7F && k2==0x00){
     current_map[K_UP]=1;
   }
   if(k1==0xFF && k2==0x7F){
     current_map[K_RIGHT]=1;
   }
   if(k1==0x7F && k2==0xFF){
     current_map[K_DOWN]=1;
   }
   if(k1==0x00 && k2==0x7F){
     current_map[K_LEFT]=1;
   }
   send_map(current_map);
}

void JoystickEvents::OnHatSwitch(uint8_t hat) {
        Serial.print("Hat Switch: ");
        PrintHex<uint8_t > (hat, 0x80);
        Serial.println("");
}

uint8_t trans_button_id(uint8_t id) {
  uint8_t rid = 0;
  switch(id){
    case 4: rid = 1; break; //A
    case 3: rid = 2; break; //B
    case 2: rid = 3; break; //X
    case 1: rid = 4; break; //Y
    case 5: rid = 5; break; //L
    case 6: rid = 6; break; //R
    case 8: rid = 7; break; //START
    case 7: rid = 8; break; //SELECT
    default:break;    
  }
  return rid;
}

void JoystickEvents::OnButtonUp(uint8_t but_id) {
        Serial.print("Up: ");
        Serial.println(but_id, DEC);
        uint8_t id = trans_button_id(but_id);
        if(id>0) mySerial.write(id | 0x80);
}

void JoystickEvents::OnButtonDn(uint8_t but_id) {
        Serial.print("Dn: ");
        Serial.println(but_id, DEC);
        uint8_t id = trans_button_id(but_id);
        if(id>0) mySerial.write(id);
}
