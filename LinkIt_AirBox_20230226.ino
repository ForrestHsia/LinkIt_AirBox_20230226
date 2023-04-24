#include <LWiFi.h>
#include <TLSClient.h>
#include <WiFiClient.h>
#include <WiFiServer.h>
#include <WiFiUdp.h>
#include "configuration.h"
/*
 LinkIt 7697空氣盒子~示範程式, 此商品是透過Micro USB供電,
 在供電前請先打開LinkIt 7697 nano Breakout的開關,
 
 如需啟用MCS服務, 請自行前往官方申請並觀看教學,
 並新增四個資料通道id分別為pmat25、pmat100、Temp、Humid,
 於下方填入Wi-Fi與MCS服務資料即可.

 若不需要MCS服務可不用新增,直接下載程式即可.
 
 */

#include <SoftwareSerial.h>
#include "Wire.h"
#include "U8g2lib.h"

SoftwareSerial myMerial(3, 2);
U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, /* clock=*/ SCL, /* data=*/ SDA, /* reset=*/ U8X8_PIN_NONE);

//========================================================================
#include "MCS.h"

char _lwifi_ssid[] = WIFI_SSID; //填入Wi-Fi名稱
char _lwifi_pass[] = WIFI_PASS; //填入Wi-Fi密碼

MCSLiteDevice mcs(DeviceId, DeviceKey, MCSLiteServerIP, MCSLiteServerPort); //填入MCS服務需要之名稱與ID
MCSDisplayFloat MCS_PM25("pmat25");
MCSDisplayFloat MCS_PM10("pmat100");
MCSDisplayFloat MCS_Temp("Temp");
MCSDisplayFloat MCS_Humid("Humid");
//========================================================================

long pmat10 = 0;
long pmat25 = 0;

long pmat100 = 0;

long Temp = 0;
long Humid = 0;
char buf[50];

void setup() {
  Serial.begin(9600);
  myMerial.begin(9600);
  pinMode(2, OUTPUT);
  digitalWrite(2, HIGH);
  u8g2.begin();
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB08_tr);
  u8g2.drawStr(0,10,"Wait...");
  u8g2.sendBuffer();

  mcs.addChannel(MCS_PM25);
  mcs.addChannel(MCS_PM10);
  mcs.addChannel(MCS_Temp);
  mcs.addChannel(MCS_Humid);
  
  WiFi.begin(_lwifi_ssid, _lwifi_pass);
  delay(3000); 
}

void loop() {
  retrievepm25();
  
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB08_tr);
  u8g2.drawStr(0,10,"LinkIt AirBox");
  //u8g2.drawStr(0,10,("PM100 : " + String(pmat100) + " ug/m3").c_str());
  u8g2.drawStr(0,25,("PM2.5 : " + String(pmat25) + " ug/m3").c_str());
  u8g2.drawStr(0,37,("PM10 : " + String(pmat100) + " ug/m3").c_str());
  //u8g2.drawStr(0,49,("PM100 : " + String(pmat100) + " ug/m3").c_str());
  u8g2.drawStr(0,49,("Temp : " + String(Temp) + " *C").c_str());
  u8g2.drawStr(0,61,("Humid : " + String(Humid) + " %RH").c_str());
  u8g2.sendBuffer();
  
  if (!mcs.connected()) {
    mcs.connect();
    if (mcs.connected()) { Serial.println("MCS 已重新連線"); }
  } else {
    mcs.process(1000);

    MCS_Temp.set(Temp);
    MCS_Humid.set(Humid);
    MCS_PM25.set(pmat25);
    MCS_PM10.set(pmat100);
  }
  
  delay(1000);

}


void retrievepm25(){
  int count = 0;
  unsigned char c;
  unsigned char high;
  while (myMerial.available()) { 
    c = myMerial.read();
    if((count==0 && c!=0x42) || (count==1 && c!=0x4d)){
      break;
    }
    if(count > 27){ 
      break;
    }
     else if(count == 10 || count == 12 || count == 14 || count == 24 || count == 26) {
      high = c;
    }
    else if(count == 11){
      pmat10 = 256*high + c;
    }
    else if(count == 13){
      pmat25 = 256*high + c;
    }
    else if(count == 15){
      pmat100 = 256*high + c;
    }
     else if(count == 25){        
      Temp = (256*high + c)/10;
    }
    else if(count == 27){            
      Humid = (256*high + c)/10;
    }   
    count++;
  }
  while(myMerial.available()) myMerial.read();
}
