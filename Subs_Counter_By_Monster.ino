#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino

//Subs Counter
#include "Arduino.h"
#include <ArduinoJson.h>

#define NUM_MAX 4
#define ROTATE 90

// for ESP-01 module
//#define DIN_PIN 2 // D4
//#define CS_PIN  3 // D9/RX
//#define CLK_PIN 0 // D3

// for NodeMCU 1.0
#define DIN_PIN 15  // D8
#define CS_PIN  13  // D7
#define CLK_PIN 12  // D6

#include "max7219.h"
#include "fonts.h"

//Needed for WifiManager
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>         //https://github.com/tzapu/WiFiManager



// =======================================================================
// Your config below!
// =======================================================================
//const char* ssid     = "No Need";                                   // SSID of local network
//const char* password = "noneed";                                    // Password on network
String ytApiV3Key = "AIzaSyBNyZpre4veU2aFi7e59LmsMTwL7KWolt4";        // YouTube Data API v3 key generated here: https://console.developers.google.com
String channelId = "UC2Rwe7qAeQoXXWMrWkzCJQw";                        // YT channel id  
long utcOffset = 6;                                                   // for Warsaw,Poland
// =======================================================================


void configModeCallback (WiFiManager *myWiFiManager) {
  Serial.println("Entered config mode");
  Serial.println(WiFi.softAPIP());
  //if you used auto generated SSID, print it
  Serial.println(myWiFiManager->getConfigPortalSSID());
  printStringWithShift(" Can't connect to wifi!   Switching to Access Point Mode!   Please configure the wifi using our app.     ",40,font,' ');
  printStringWithShift(" ... ... ... ",40,font,' ');
  printStringWithShift(" ... AP ... ",40,font,' ');
}


void setup() {
    // put your setup code here, to run once:
    Serial.begin(115200);
    initMAX7219();
    sendCmdAll(CMD_SHUTDOWN,1);
    sendCmdAll(CMD_INTENSITY,0);
    printStringWithShift(".. WiFi ..",20,font,' ');
    
    //WiFiManager
    //Local intialization. Once its business is done, there is no need to keep it around
    WiFiManager wifiManager;
    
    //reset saved settings for testing
    //wifiManager.resetSettings();

    //wifiManager.setTimeout(200);

    //exit after config instead of connecting
    //wifiManager.setBreakAfterConfig(true);

    //set callback that gets called when connecting to previous WiFi fails, and enters Access Point mode
    wifiManager.setAPCallback(configModeCallback);

    //tries to connect to last known settings
    //if it does not connect it starts an access point with the specified name
    //here  "AutoConnectAP" with password "password"
    //and goes into a blocking loop awaiting configuration
    if (!wifiManager.autoConnect("LiveSubs Counter", "123456789")) {
      ESP.reset();
      delay(5000);
    }
    
    //if you get here you have connected to the WiFi
    //Serial.println("Connected...yeey :)");
    //Serial.println("Getting data ...");
    printStringWithShift("  Done  ",20,font,' ');
    delay(3000);
    printStringWithShift("    Successfully connected to the wifi!   Loading channel data...  ",30,font,' ');
    printStringWithShift(" .. ... ..  ",30,font,' ');
    delay(1500);
}
    
// =======================================================================

//// =========== Unused variables =========== 
// subscriberCount1h=-1, subsGain1h=0 
// viewCount24h=-1, viewsGain24h
// subscriberCount24h=-1, subsGain24h=0
//// ========================================

long viewCount;
long subscriberCount;
long videoCount;
long chanName;
int cnt = 0;
unsigned long time1h, time24h;
long localEpoc = 0;
long localMillisAtUpdate = 0;
int h, m, s;
String date;

void loop()
{


  if (WiFi.status() != WL_CONNECTED) {
    printStringWithShift("    Wifi Disconnected!   Trying to reconnect!    ",30,font,' ');
    printStringWithShift(" !     ",30,font,' ');
    while (WiFi.status() != WL_CONNECTED) {
      Serial.print("."); delay(500);
    }
  }

  
  if(cnt<=0) {
    if(getYTData()==0) {
      cnt = 1;  // data is refreshed every 50 loops
//      if(subscriberCount1h<0) {
//        time1h = time24h = millis();
//        subscriberCount1h = subscriberCount24h = subscriberCount;
//        viewCount24h = viewCount;
//      }
//      if(millis()-time1h>1000*60*60) {
//        time1h = millis();
//        subscriberCount1h = subscriberCount;
//      }
//      if(millis()-time24h>1000*60*60*24) {
//        time24h = millis();
//        subscriberCount24h = subscriberCount;
//        viewCount24h = viewCount;
//      }
//      subsGain1h = subscriberCount-subscriberCount1h;
//      subsGain24h = subscriberCount-subscriberCount24h;
//      viewsGain24h = viewCount-viewCount24h;
    }
  }
  cnt--;
  updateTime();
  int del = 3000;
  int scrollDel = 20;
  char txt[10];
  sprintf(txt,"    %02d:%02d  ",h,m);
  
  //printStringWithShift("  Subscribers: ",scrollDel,font,' '); // eng
  //printStringWithShift("  Subskrybcje: ",scrollDel,font,' '); // pol
  printValueWithShift(subscriberCount,scrollDel,0);
  delay(7000);
  printStringWithShift(txt,scrollDel,font,' '); // real time
  delay(5000);
  
  //printStringWithShift("  Views: ",scrollDel,font,' '); // eng
  //printStringWithShift("  Wyświetlenia: ",scrollDel,font,' '); // pol
  //printValueWithShift(viewCount,scrollDel,0);
  //delay(del);
  //printStringWithShift("  Videos: ",scrollDel,font,' '); // eng
  //printStringWithShift("  Filmy: ",scrollDel,font,' '); // pol
  //printValueWithShift(videoCount,scrollDel,0);
  //delay(del);
}
// =======================================================================

int dualChar = 0;

unsigned char convertPolish(unsigned char _c)
{
  unsigned char c = _c;
  if(c==196 || c==197 || c==195) {
    dualChar = c;
    return 0;
  }
  if(dualChar) {
    switch(_c) {
      case 133: c = 1+'~'; break; // 'ą'
      case 135: c = 2+'~'; break; // 'ć'
      case 153: c = 3+'~'; break; // 'ę'
      case 130: c = 4+'~'; break; // 'ł'
      case 132: c = dualChar==197 ? 5+'~' : 10+'~'; break; // 'ń' and 'Ą'
      case 179: c = 6+'~'; break; // 'ó'
      case 155: c = 7+'~'; break; // 'ś'
      case 186: c = 8+'~'; break; // 'ź'
      case 188: c = 9+'~'; break; // 'ż'
      //case 132: c = 10+'~'; break; // 'Ą'
      case 134: c = 11+'~'; break; // 'Ć'
      case 152: c = 12+'~'; break; // 'Ę'
      case 129: c = 13+'~'; break; // 'Ł'
      case 131: c = 14+'~'; break; // 'Ń'
      case 147: c = 15+'~'; break; // 'Ó'
      case 154: c = 16+'~'; break; // 'Ś'
      case 185: c = 17+'~'; break; // 'Ź'
      case 187: c = 18+'~'; break; // 'Ż'
      default:  break;
    }
    dualChar = 0;
    return c;
  }    
  switch(_c) {
    case 185: c = 1+'~'; break;
    case 230: c = 2+'~'; break;
    case 234: c = 3+'~'; break;
    case 179: c = 4+'~'; break;
    case 241: c = 5+'~'; break;
    case 243: c = 6+'~'; break;
    case 156: c = 7+'~'; break;
    case 159: c = 8+'~'; break;
    case 191: c = 9+'~'; break;
    case 165: c = 10+'~'; break;
    case 198: c = 11+'~'; break;
    case 202: c = 12+'~'; break;
    case 163: c = 13+'~'; break;
    case 209: c = 14+'~'; break;
    case 211: c = 15+'~'; break;
    case 140: c = 16+'~'; break;
    case 143: c = 17+'~'; break;
    case 175: c = 18+'~'; break;
    default:  break;
  }
  return c;
}

// =======================================================================

int charWidth(char ch, const uint8_t *data)
{
  int len = pgm_read_byte(data);
  return pgm_read_byte(data + 1 + ch * len);
}

// =======================================================================

int showChar(char ch, const uint8_t *data)
{
  int len = pgm_read_byte(data);
  int i,w = pgm_read_byte(data + 1 + ch * len);
  scr[NUM_MAX*8] = 0;
  for (i = 0; i < w; i++)
    scr[NUM_MAX*8+i+1] = pgm_read_byte(data + 1 + ch * len + 1 + i);
  return w;
}

// =======================================================================

void printCharWithShift(unsigned char c, int shiftDelay, const uint8_t *data, int offs) 
{
  c = convertPolish(c);
  if(c < offs || c > MAX_CHAR) return;
  c -= offs;
  int w = showChar(c, data);
  for (int i=0; i<w+1; i++) {
    delay(shiftDelay);
    scrollLeft();
    refreshAll();
  }
}

// =======================================================================

void printStringWithShift(const char *s, int shiftDelay, const uint8_t *data, int offs)
{
  while(*s) printCharWithShift(*s++, shiftDelay, data, offs);
}

// =======================================================================
// printValueWithShift():
// converts int to string
// centers string on the display
// chooses proper font for string/number length
// can display sign - or +
void printValueWithShift(long val, int shiftDelay, int sign)
{
  const uint8_t *digits = digits5x7;       // good for max 5 digits
  if(val>1999999) digits = digits3x7;      // good for max 8 digits
  else if(val>99999) digits = digits3x7;   // good for max 6-7 digits (was digits4x7)
  String str = String(val);
  if(sign) {
    if(val<0) str=";"+str; else str="<"+str;
  }
  const char *s = str.c_str();
  int wd = 0;
  while(*s) wd += 1+charWidth(*s++ - '0', digits);
  wd--;
  int wdL = (NUM_MAX*8 - wd)/2;
  int wdR = NUM_MAX*8 - wdL - wd;
  //Serial.println(wd); Serial.println(wdL); Serial.println(wdR);
  s = str.c_str();
  while(wdL>0) { printCharWithShift(':', shiftDelay, digits, '0'); wdL--; }
  while(*s) printCharWithShift(*s++, shiftDelay, digits, '0');
  while(wdR>0) { printCharWithShift(':', shiftDelay, digits, '0'); wdR--; }
}

// =======================================================================

const char *ytHost = "www.googleapis.com";

int getYTData()
{
  WiFiClientSecure client;
  Serial.print("connecting to "); Serial.println(ytHost);
  if (!client.connect(ytHost, 443)) {
    Serial.println("connection failed");
    return -1;
  }
  String cmd = String("GET /youtube/v3/channels?part=statistics&id=") + channelId + "&key=" + ytApiV3Key+ " HTTP/1.1\r\n" +
                "Host: " + ytHost + "\r\nUser-Agent: ESP8266/1.1\r\nConnection: close\r\n\r\n";
  client.print(cmd);

  int repeatCounter = 10;
  while (!client.available() && repeatCounter--) {
    Serial.println("y."); delay(500);
  }
  String line,buf="";
  int startJson=0, dateFound=0;
  while (client.connected() && client.available()) {
    line = client.readStringUntil('\n');
    if(line[0]=='{') startJson=1;
    if(startJson) {
      for(int i=0;i<line.length();i++)
        if(line[i]=='[' || line[i]==']') line[i]=' ';
      buf+=line+"\n";
    }
    if(!dateFound && line.startsWith("Date: ")) {
      dateFound = 1;
      date = line.substring(6, 22);
      h = line.substring(23, 25).toInt();
      m = line.substring(26, 28).toInt();
      s = line.substring(29, 31).toInt();
      localMillisAtUpdate = millis();
      localEpoc = (h * 60 * 60 + m * 60 + s);
    }
  }
  client.stop();

  DynamicJsonBuffer jsonBuf;
  JsonObject &root = jsonBuf.parseObject(buf);
  if (!root.success()) {
    Serial.println("parseObject() failed");
    printStringWithShift(" ... ",20,font,' ');
    delay(10);
    return -1;
  }
  viewCount       = root["items"]["statistics"]["viewCount"];
  subscriberCount = root["items"]["statistics"]["subscriberCount"];
  videoCount      = root["items"]["statistics"]["videoCount"];
  chanName        = root["items"]["snippet"]["title"];
  return 0;
}

// =======================================================================

void updateTime()
{
  long curEpoch = localEpoc + ((millis() - localMillisAtUpdate) / 1000);
  long epoch = round(curEpoch + 3600 * utcOffset + 86400L) % 86400L;
  h = ((epoch  % 86400L) / 3600) % 24;
  if(h>12){
    h = (h-12);
  }
  else if(h==0){
    h = 12;
  }
  
  m = (epoch % 3600) / 60;
  s = epoch % 60;
}

// =======================================================================

