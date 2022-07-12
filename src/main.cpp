/*

On Branch: websockets@Dell_7280  !!!!!

*/
#define ESPMINI

#include <Arduino.h>
#ifdef ESP32
#include <WiFi.h>
#include "AsyncTCP.h"
#include <rom/rtc.h>
#else
#include <ESP8266WiFi.h>
#include "ESPAsyncTCP.h"
#endif
#include "FS.h"
#include <LittleFS.h>
#include <WiFiUdp.h>
#include <time.h>
#include <ESPAsyncWebServer.h>
#include <AsyncElegantOTA.h>
#include <cstdarg>
#include <logger.h>
#include <uptime.h>
#include "Node_settings.h"
#include "secrets.h"
#include "switchdev.h"
#include "version.h"

#if defined(MQTT)
#include <PubSubClient.h>
#endif

#if defined(SENSOR_18B20)
#include <OneWire.h>
#include <DallasTemperature.h>
#endif

#if defined(SENSOR_BOSCH)
#include <BMX_sensor.h>
#endif

#if defined(RF24GW)
#include <RF24.h>
#include "dataformat.h"
#endif

#if defined(LEDMATRIX)
#include <LED_Matrix.h>
#endif

#if defined(NEOPIXEL)
#include <Adafruit_NeoPixel.h>
#endif

//Erweiterungen Header


//ENDE Erweiterungen Header

typedef enum {ret_html=0, ret_json, ret_text, ret_stat} call_t;
typedef enum {log_rf24=0, log_sys, log_mqtt, log_sens, log_web, log_sys_crit} log_t;
struct prefs_t {
   char            mqttserver[SERVERNAMESIZE];
   char            mqttclient[SERVERNAMESIZE];
   bool            log_sensor;
   bool            log_mqtt;
   bool            log_webcmd;
   bool            log_sysinfo;
   bool            log_fs_sysinfo;
   bool            log_rf24;
   char            rf24gw_hub_name[SERVERNAMESIZE];
   uint16_t        rf24gw_hub_port;
   uint16_t        rf24gw_gw_port;
   uint16_t        rf24gw_gw_no;
   bool            rf24gw_enable;
   bool            rf24node_enable;
   NODE_DATTYPE    rf24node_node_id;  
   uint8_t         rf24node_val1_channel;
   bool            rf24node_val2_enable;
   uint8_t         rf24node_val2_channel;
   bool            rf24node_val3_enable;
   uint8_t         rf24node_val3_channel;
};
prefs_t preference;


// Create AsyncWebServer object on port 80
AsyncWebServer httpServer(80);
AsyncWebSocket ws("/ws");

WiFiClient mqtt_wifi_client;
WiFiUDP udp;
Logger logger = Logger(LOGGER_NUMLINES,LOGGER_LINESIZE);
log_t logtopic;
bool rebootflag = false;
#if defined(MQTT)
PubSubClient mqttClient(mqtt_wifi_client);
bool send_mqtt_statt = false;
#endif
#if defined(SW1)
Switchdev sw1(SW1, SW1_TYP);
#endif

// Variablen für das Vorhandensein von Messwerten
bool has_temp = false;
bool has_pres = false;
bool has_humi = false;

// Variablen zur generischen Nutzung
char*  info_str;
char*  tmp_str;

//Schleifensteuerung
unsigned long lastSens = 0;
unsigned long lastMsg = 0;
unsigned long lastInfo = 0;
unsigned long lastalive = 0;

// Zeitmanagement
const char* NTP_SERVER = "de.pool.ntp.org";
const char* TZ_INFO    = "CET-1CEST-2,M3.5.0/02:00:00,M10.5.0/03:00:00";  
// enter your time zone (https://remotemonitoringsystems.ca/time-zone-abbreviations.php)

tm timeinfo;
time_t now;
long unsigned lastNTPtime;
unsigned long lastEntryTime;
char timeStr[12];

#if defined(SENSOR_18B20)
OneWire oneWire(SENSOR_18B20);
DallasTemperature sensor(&oneWire);
float temperature;
uint8_t resolution = 0;
#endif

#if defined(SENSOR_BOSCH)
BMX_SENSOR sensor;
float temp, pres, humi;
#endif

#if defined(ANALOGINPUT)
int analog_value;
#else
#ifdef ESP32
// tbd interne Spannung messen
#else
ADC_MODE(ADC_VCC);
#endif
#endif

#if defined(RF24GW) || defined(RF24NODE)
udpdata_t udpdata;
#endif

#if defined(RF24GW)
RF24 radio(RF24GW_RADIO_CE_PIN,RF24GW_RADIO_CSN_PIN);
payload_t payload;
uint8_t  rf24_node2hub[] = RF24_NODE2HUB;
uint8_t  rf24_hub2node[] = RF24_HUB2NODE;
#endif

#if defined(LEDMATRIX)
LED_Matrix matrix(LEDMATRIX_DIN, LEDMATRIX_CLK, LEDMATRIX_CS, LEDMATRIX_DEVICES_X, LEDMATRIX_DEVICES_Y);
#endif

#if defined(NEOPIXEL)
Adafruit_NeoPixel pixels(NEOPIXELNUM, NEOPIXELPIN, NEO_GRB + NEO_KHZ800);
#endif


//Erweiterungen Variablen


//ENDE Erweiterungen Variablen

/************************************************
 * Allgemeine Funktionen
 ***********************************************/
/************************************************
 * Füllt den "timeStr" mit der aktuellen Zeit
 ***********************************************/
void fill_timeStr() {
  time(&now);                       // read the current time
  localtime_r(&now, &timeinfo);           // update the structure tm with the current time
  snprintf(timeStr,11,"[%02d:%02d:%02d]",timeinfo.tm_hour,timeinfo.tm_min,timeinfo.tm_sec);
}
/*************************************************************************
 *  Universelle Prozedur zur Erweiterung (concat) des "info_str"
 *  Übernimmt dabei auch die Prüfung des Restplatzes im Array of Char
 *  Ist "add_komma" = true, dann wird vor dem anzufügenden Text 
 *  ein Komma eingefügt.
 *************************************************************************/
bool add_2_info_str(const char* addstr, bool add_komma) {
  bool retval = true;
  size_t space_left = INFOSIZE - strlen(info_str) -1;
  if ( space_left > strlen(addstr) ) {
    if (add_komma) {
      strncat(info_str, ",", space_left);
      space_left--;
    }
    strncat(info_str, addstr, space_left);
  } else {
    retval = false;
  }
  return retval;
}
/***********************************************************
 * getVcc: liest die aktuelle Betriebsspannung
 * Achtung: ESP32 noch nicht implementiert
 **********************************************************/
float getVcc(void) {
#if defined(ANALOGINPUT)
  return 0;
#else  
#ifdef ESP32
  return 0;
#else
  return ((float)ESP.getVcc()/1000.0);
#endif
#endif
}
/*******************************************************
 * do_restart: Startet das System neu
 ******************************************************/
char* do_restart()
{
  rebootflag = true;
  sprintf(info_str,"Restart");
  return info_str;
}
/*******************************************************
 * write2log: Schreibt Dateien in die LogKanäle
 * 
 * 
 ******************************************************/
void write2log(int count, ...) {
  va_list args;
  int n = 0;
  const char *c[10];

  /* Parameterabfrage initialisieren */
  va_start(args, count);
  while(n < count) {
    c[n] = (const char*)va_arg(args, const char*);
    n++;
  }
  fill_timeStr();
  if (preference.log_fs_sysinfo && logtopic == log_sys_crit) {
    File f = LittleFS.open("/logfile.txt", "a");
    if (f) {
      f.print(timeStr);
      n = 0;
      while(n < count) {
        f.print(c[n]);
        n++;
      }
    }
  }
  if ((preference.log_rf24 && logtopic == log_rf24) || (preference.log_sensor && logtopic == log_sens) || 
      (preference.log_mqtt && logtopic == log_mqtt) || (preference.log_webcmd && logtopic == log_web) || 
      (preference.log_sysinfo && logtopic == log_sys) || (preference.log_sysinfo && logtopic == log_sys_crit)) {
    logger.print(timeStr);
    n = 0;
    while(n < count) {
      logger.print(c[n]);
      n++;
    }
    logger.println();
  }
#if defined(DEBUG_SERIAL)
  Serial.print(timeStr);
  n = 0;
  while(n < count) {
    Serial.print(c[n]);
    n++;
  }
  Serial.println();
#endif
  va_end(args);
}

/******************************************************
 * handleWebSocketMessage: Verarbeitet die Daten, 
 * die von der Webseite auf das Websocket gesendet werden.
 * Daten folgen der Form: <item>:<value>
 ******************************************************/
void handleWebSocketMessage(void *arg, uint8_t *data, size_t len)
{
  AwsFrameInfo *info = (AwsFrameInfo *)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT)
  {
    data[len] = 0;
    String mycmd = (char*)data;
    int pos = mycmd.indexOf(":");
    String item = mycmd.substring(0,pos);
    String value = mycmd.substring(pos+1);
    if ( item == "sw1" ) {
      sw1.set_state(value.toInt());
      sprintf(info_str,"{%s}",sw1.stat_json(false));
      ws.textAll(info_str);
    }
    if ( item == "sw1sl0" ) {
      sw1.set_vol(value.toInt());
      sprintf(info_str,"{%s}",sw1.stat_json(false));
      ws.textAll(info_str);
    }
  }
}

/**********************************************************
 * Eventverwaltung des Websockets
 *********************************************************/
void ws_onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
             void *arg, uint8_t *data, size_t len) {
    switch (type) {
      case WS_EVT_CONNECT:
#ifdef SW1
        sprintf(info_str,"{%s}",sw1.stat_json(false));
        ws.textAll(info_str);
#endif
        break;
      case WS_EVT_DISCONNECT:
        Serial.printf("WebSocket client #%u disconnected\n", client->id());
        break;
      case WS_EVT_DATA:
        handleWebSocketMessage(arg, data, len);
        break;
      case WS_EVT_PONG:
      case WS_EVT_ERROR:
        break;
  }
}

/**********************************************
 * Initialisierung des Websockets
 *********************************************/
void initWebSocket()
{
  ws.onEvent(ws_onEvent);
  httpServer.addHandler(&ws);
}

/***********************************************
 * ServFile bedient die Webserver URL, die nicht direkt
 * per httpServer.on() festgelegt wurden.
 * Ist die geforderte Datei nicht im Dateisystem
 * dann wird die Datei "index.html" gesendet. 
 **********************************************/
void serveFile(AsyncWebServerRequest *request) 
{
  String myurl = request->url();
  String dataType = "text/plain";
  Serial.print("Requested: ");
  Serial.println(myurl);
  if (myurl == "/") myurl="/index.html";
  if (LittleFS.exists(myurl)) {
    Serial.print(myurl);
    Serial.println("  exists");
  } else {
    Serial.print(myurl);
    Serial.println("  does not exist  Send: /index.html");
    Serial.println("Fileliste:");
#ifdef ESP32
    File root = LittleFS.open("/");
    if(!root){
        Serial.println("- failed to open / directory");
        return;
    }
    if(!root.isDirectory()){
        Serial.println(" - not a directory");
        return;
    }

    File file = root.openNextFile();
    while(file){
        if(file.isDirectory()){
            Serial.print("  DIR : ");
            Serial.println(file.name());
/*            if(levels){
                listDir(LittleFS, file.path(), levels -1);
            } */
        } else {
            Serial.print("  FILE: ");
            Serial.print(file.name());
            Serial.print("\tSIZE: ");
            Serial.println(file.size());
        }
        file = root.openNextFile();
    }
#else
    Dir dir = LittleFS.openDir("/");
    if (dir.isDirectory()) {
      Serial.println("/ is a Directory");
    } else {
      Serial.println("/ is NOT a Directory");
    }
    File f;
    while (dir.next()) {
      Serial.print(dir.fileName());
      Serial.print("  ");
      f = dir.openFile("r");
      Serial.println(f.size());
      f.close();
    }
#endif
    myurl="/index.html";
  }
  if (myurl.endsWith(".html") || myurl.endsWith(".htm")) {
    dataType = "text/html";
  } else if (myurl.endsWith(".css")) {
    dataType = "text/css";
  } else if (myurl.endsWith(".js")) {
    dataType = "application/javascript";
  } else if (myurl.endsWith(".png")) {
    dataType = "image/png";
  } else if (myurl.endsWith(".gif")) {
    dataType = "image/gif";
  } else if (myurl.endsWith(".jpg")) {
    dataType = "image/jpeg";
  } else if (myurl.endsWith(".ico")) {
    dataType = "image/x-icon";
  } else if (myurl.endsWith(".xml")) {
    dataType = "text/xml";
  } else if (myurl.endsWith(".pdf")) {
    dataType = "application/pdf";
  } else if (myurl.endsWith(".zip")) {
    dataType = "application/zip";
  }
  request->send(LittleFS, myurl, dataType);
}

bool getNTPtime(long unsigned int sec) 
{
  {
    uint32_t start = millis();
    do {
      time(&now);
      localtime_r(&now, &timeinfo);
      delay(10);
    } while (((millis() - start) <= (1000 * sec)) && (timeinfo.tm_year < (2016 - 1900)));
    if (timeinfo.tm_year <= (2016 - 1900)) return false;  // the NTP call was not successful
  }
  return true;
}

/**********************************
 * Connect or reconnect to WIFI
 *********************************/
void wifi_con(void) 
{
  if (WiFi.status() != WL_CONNECTED) {
    logtopic = log_sys_crit;
    write2log(4,"Try to connect to ",ssid, " with password ",password);
    WiFi.mode(WIFI_STA);
#ifdef ESP32
    WiFi.setHostname(HOSTNAME);
#else
    WiFi.persistent(false);
    WiFi.hostname(HOSTNAME);
#endif
    WiFi.begin(ssid, password);
    logtopic = log_sys_crit;
    write2log(1,"WIFI (re)connect");

    // ... Give ESP 10 seconds to connect to station.
    unsigned int i=0;
    while (WiFi.status() != WL_CONNECTED && i < 100) {
      delay(200);
      i++;
    }
    while (WiFi.status() != WL_CONNECTED) {
      logtopic = log_sys_crit;
      write2log(1, "WIFI reboot");
      delay(3000);
      ESP.restart();
    }
    configTime(0, 0, NTP_SERVER);
    // See https://github.com/nayarsystems/posix_tz_db/blob/master/zones.csv for Timezone codes for your region
    setenv("TZ", TZ_INFO, 1);
    tzset();
    if (getNTPtime(10)) {  // wait up to 10sec to sync
    } else {
      Serial.println("Time not set");
      ESP.restart();
    }
    lastNTPtime = time(&now);
    lastEntryTime = millis();
  }
}

/*
 * read_preferences: 
 * Liest die Voreinstellungen aus dem Filesystem ein.
 */
void read_preferences(void) {
  File myFile = LittleFS.open("/prefs", "r");
  if (myFile) {
    myFile.read((byte *)&preference, sizeof(preference));
    myFile.close();
  }
}

/*
 * read_preferences: 
 * Liest die Voreinstellungen aus dem Filesystem ein.
 */
void save_preferences(void) {
  File myFile = LittleFS.open("/prefs", "w");
  myFile.write((byte *)&preference, sizeof(preference));
  myFile.close();
}

/*
 * Eine Funktion als Dummy die nichts macht
 */
void noop() {}

/****************************************************************
 * Funktionen für den Webserver
 ***************************************************************/

char* mk_cmd(AsyncWebServerRequest *request) {
  bool prefs_change = false;
  int args = request->args();
  for(int argNo=0;argNo<args;argNo++){
#if defined(DEBUG_SERIAL_HTML)
    Serial.println(String("handleCmd: ")+ request->argName(argNo) + String(": ") + request->arg(argNo));
#endif
//  Serial.printf("ARG[%s]: %s\n", request->argName(i).c_str(), request->arg(i).c_str());
/*    if ( request->argName(argNo) == "log_sensor" ) {
      if (request->arg(argNo) == "1" ) {
        if ( ! eepromdata.log_sensor) {
           eepromdata.log_sensor = true;
           eepromchange = true;
        }
      } else {
        if ( eepromdata.log_sensor) {
           eepromdata.log_sensor = false;
           eepromchange = true;
        }
      }
    }
    if ( request->argName(argNo) == "log_mqtt" ) {
      if ( request->arg(argNo) == "1" ) {
        if ( ! eepromdata.log_mqtt) {
           eepromdata.log_mqtt = true;
           eepromchange = true;
        }
      } else {
        if ( eepromdata.log_mqtt) {
           eepromdata.log_mqtt = false;
           eepromchange = true;
        }
      }
    }
    if ( request->argName(argNo) == "log_webcmd" ) {
      if ( request->arg(argNo) == "1" ) {
        if ( ! eepromdata.log_webcmd) {
           eepromdata.log_webcmd = true;
           eepromchange = true;
        }
      } else {
        if ( eepromdata.log_webcmd) {
          eepromdata.log_webcmd = false;
          eepromchange = true;
        }
      }
    }
    if ( request->argName(argNo) == "log_sysinfo" ) {
      if ( request->arg(argNo) == "1" ) {
        if ( ! eepromdata.log_sysinfo) {
           eepromdata.log_sysinfo = true;
           eepromchange = true;
        }
      } else {
        if ( eepromdata.log_sysinfo) {
           eepromdata.log_sysinfo = false;
           eepromchange = true;
        }
      }
    }
    if ( request->argName(argNo) == "log_fs_sysinfo" ) {
      if ( request->arg(argNo) == "1" ) {
        if ( ! eepromdata.log_fs_sysinfo) {
           eepromdata.log_fs_sysinfo = true;
           eepromchange = true;
        }
      } else {
        if ( eepromdata.log_fs_sysinfo) {
           eepromdata.log_fs_sysinfo = false;
           eepromchange = true;
        }
      }
    } */
#if defined(RF24NODE_VAL2)
    if ( request->argName(argNo) == "rf24node_val2_channel" ) {
      if ( eepromdata.rf24node_val2_channel != request->arg(argNo).toInt() ) {
        eepromdata.rf24node_val2_channel = request->arg(argNo).toInt();
        eepromchange = true;
      }
    }
#endif
    if ( request->argName(argNo) == "dellogfile" ) {
      LittleFS.remove("logfile.txt");
    }
    if ( request->argName(argNo) == "deldebugfile" ) {
      LittleFS.remove("debugfile.txt");
    }

    if ( request->argName(argNo) == "saveeprom" ) {
#if defined(DEBUG_SERIAL_HTML)
        Serial.println("CMD: saveeprom");
#endif
 /*     if (eepromchange) {
        EEPROM.put( 0, eepromdata );
        EEPROM.commit();
        eepromchange = false;
        sprintf(info_str,"%s","EEPROM changed");
#if defined(DEBUG_SERIAL_HTML)
        Serial.println("EEPROM saved");
#endif
      } */
    }
    if ( request->argName(argNo) == "mqttserver" ) {
      snprintf(preference.mqttserver,SERVERNAMESIZE-1,request->arg(argNo).c_str());
      prefs_change = true;  
    }
    if ( request->argName(argNo) == "mqttclient" ) {
      snprintf(preference.mqttclient,SERVERNAMESIZE-1,request->arg(argNo).c_str());
      prefs_change = true;  
    }
#if defined(RF24GW)
    if ( request->argName(argNo) == "log_rf24" ) {
      if (request->arg(argNo) == "1" ) {
        if ( ! eepromdata.log_rf24) {
           preference.log_rf24 = true;
           eepromchange = true;
        }
      } else {
        if ( eepromdata.log_rf24) {
           preference.log_rf24 = false;
           eepromchange = true;
        }
      }
    }
#endif
    if ( request->argName(argNo) == "log_sensor" ) {
      if (request->arg(argNo) == "1" ) {
        if ( ! preference.log_sensor) {
           preference.log_sensor = true;
           prefs_change = true;
        }
      } else {
        if ( preference.log_sensor) {
           preference.log_sensor = false;
           prefs_change = true;
        }
      }
    }
    if ( request->argName(argNo) == "log_mqtt" ) {
      if ( request->arg(argNo) == "1" ) {
        if ( ! preference.log_mqtt) {
           preference.log_mqtt = true;
           prefs_change = true;
        }
      } else {
        if ( preference.log_mqtt) {
           preference.log_mqtt = false;
           prefs_change = true;
        }
      }
    }
    if ( request->argName(argNo) == "log_webcmd" ) {
      if ( request->arg(argNo) == "1" ) {
        if ( ! preference.log_webcmd) {
           preference.log_webcmd = true;
           prefs_change = true;
        }
      } else {
        if ( preference.log_webcmd) {
           preference.log_webcmd = false;
           prefs_change = true;
        }
      }
    }
    if ( request->argName(argNo) == "log_sysinfo" ) {
      if ( request->arg(argNo) == "1" ) {
        if ( ! preference.log_sysinfo) {
           preference.log_sysinfo = true;
           prefs_change = true;
        }
      } else {
        if ( preference.log_sysinfo) {
           preference.log_sysinfo = false;
           prefs_change = true;
        }
      }
    }
    if ( request->argName(argNo) == "log_fs_sysinfo" ) {
      if ( request->arg(argNo) == "1" ) {
        if ( ! preference.log_fs_sysinfo) {
           preference.log_fs_sysinfo = true;
           prefs_change = true;
        }
      } else {
        if ( preference.log_fs_sysinfo) {
           preference.log_fs_sysinfo = false;
           prefs_change = true;
        }
      }
    }
#if defined(SW1)
    if ( request->argName(argNo) == "sw1" ) {
      if ( request->arg(argNo) == "0" ) {
        sw1.set_state(0);
      }
      if ( request->arg(argNo) == "1" ) {
        sw1.set_state(1);
      }
      if ( request->arg(argNo) == "2" ) {
        sw1.set_state(2);
#if defined(DEBUG_SERIAL_HTML)
        Serial.println("CMD: sw1 set 2");
#endif
      }
//      call = ret_json;
      sprintf(info_str,"{ %s }",sw1.stat_json(false));
    }
    if ( request->argName(argNo) == "sw1sl0" ) {
      sw1.set_vol(request->arg(argNo).toInt());
    }
    if ( request->argName(argNo) == "sw1sl1" ) {
      sw1.set_R(request->arg(argNo).toInt());
    }
    if ( request->argName(argNo) == "sw1sl2" ) {
      sw1.set_G(request->arg(argNo).toInt());
    }
    if ( request->argName(argNo) == "sw1sl3" ) {
      sw1.set_B(request->arg(argNo).toInt());
    }
#endif    
#if defined(SW2)
    if ( request->argName(argNo) == "sw2" ) {
      if ( request->arg(argNo) == "0" ) {
        sw2.set_state(0);
      }
      if ( request->arg(argNo) == "1" ) {
        sw2.set_state(1);
      }
      if ( request->arg(argNo) == "2" ) {
        sw2.set_state(2);
      }
      call = ret_json;
      sprintf(info_str,"{ %s ]",sw2.stat_json(false));
    }
    if ( request->argName(argNo) == "sw2sl0" ) {
      sw2.set_vol(request->arg(argNo).toInt());
    }
    if ( request->argName(argNo) == "sw2sl1" ) {
      sw2.set_R(request->arg(argNo).toInt());
    }
    if ( request->argName(argNo) == "sw2sl2" ) {
      sw2.set_G(request->arg(argNo).toInt());
    }
    if ( request->argName(argNo) == "sw2sl3" ) {
      sw2.set_B(request->arg(argNo).toInt());
    }
#endif    
#if defined(SW3)
    if ( request->argName(argNo) == "sw3" ) {
      if ( request->arg(argNo) == "0" ) {
        sw3.set_state(0);
      }
      if ( request->arg(argNo) == "1" ) {
        sw3.set_state(1);
      }
      if ( request->arg(argNo) == "2" ) {
        sw3.set_state(2);
      }
      call = ret_json;
      sprintf(info_str,"{ %s }",sw3.stat_json(false));
    }
    if ( request->argName(argNo) == "sw3sl0" ) {
      sw3.set_vol(request->arg(argNo).toInt());
    }
    if ( request->argName(argNo) == "sw3sl1" ) {
      sw3.set_R(request->arg(argNo).toInt());
    }
    if ( request->argName(argNo) == "sw3sl2" ) {
      sw3.set_G(request->arg(argNo).toInt());
    }
    if ( request->argName(argNo) == "sw3sl3" ) {
      sw3.set_B(request->arg(argNo).toInt());
    }
#endif    
#if defined(SW4)
    if ( request->argName(argNo) == "sw4" ) {
      if ( request->arg(argNo) == "0" ) {
        sw4.set_state(0);
      }
      if ( request->arg(argNo) == "1" ) {
        sw4.set_state(1);
      }
      if ( request->arg(argNo) == "2" ) {
        sw4.set_state(2);
      }
      call = ret_json;
      sprintf(info_str,"{ %s }",sw4.stat_json(false));
    }
    if ( request->argName(argNo) == "sw4sl0" ) {
      sw4.set_vol(request->arg(argNo).toInt());
    }
    if ( request->argName(argNo) == "sw4sl1" ) {
      sw4.set_R(request->arg(argNo).toInt());
    }
    if ( request->argName(argNo) == "sw4sl2" ) {
      sw4.set_G(request->arg(argNo).toInt());
    }
    if ( request->argName(argNo) == "sw4sl3" ) {
      sw4.set_B(request->arg(argNo).toInt());
    }
#endif    
#if defined(RF24GW_NO)
    if ( request->argName(argNo) == "rf24gw_enable" ) {
      eepromdata.rf24gw_enable = request->arg(argNo).toInt() == 1;
    }
    if ( request->argName(argNo) == "rf24hubname" ) {
      snprintf(eepromdata.rf24gw_hub_name,SERVERNAMESIZE-1,"%s",request->arg(argNo).c_str());
    }
    if ( request->argName(argNo) == "rf24hubport" ) {
      eepromdata.rf24gw_hub_port = request->arg(argNo).toInt();
    }
    if ( request->argName(argNo) == "rf24gwport" ) {
      eepromdata.rf24gw_gw_port = request->arg(argNo).toInt();
    }
    if ( request->argName(argNo) == "rf24gwno" ) {
      eepromdata.rf24gw_gw_no = request->arg(argNo).toInt();
    }
#endif
    if ( request->argName(argNo) == "help" ) {
      logger.println("Hilfesystem");
      logger.println("Hilfesystem");
      logger.println("Hilfesystem");
    }
    if ( request->argName(argNo) == "saveeprom" ) {
      if (prefs_change) {
        save_preferences();
      }
    }
  }
  return info_str;
}

/****************************************************************
 * mk_webcfg: ERzeugt die initiale Konfiguration für die Webseite
 ***************************************************************/
char* mk_webcfg()
{
  char tmp[100];
#if defined(DEBUG_SERIAL_HTML)
  Serial.println("Generiere webcfg ... ");
#endif
  snprintf(info_str, INFOSIZE-1, "{\"titel1\":\"%s\" ", HOSTNAME);
#if defined(HOST_DISCRIPTION)
  snprintf(tmp,99,"\"titel2\":\"%s\"",HOST_DISCRIPTION);
  add_2_info_str(tmp, true);
#endif
#if defined(SW1)
  add_2_info_str(sw1.web_json(), true);
#endif
#if defined(SW2)
  add_2_info_str(sw2.web_json(), true);
#endif
#if defined(SW3)
  add_2_info_str(sw3.web_json(), true);
#endif
#if defined(SW4)
  add_2_info_str(sw4.web_json(), true);
#endif
#if defined(LEDMATRIX)
  snprintf(tmp,99,"\"ledmatrix_x\":%d",LEDMATRIX_DEVICES_X * 8);
  add_2_info_str(tmp, true);
  snprintf(tmp,99,"\"ledmatrix_y\":%d",LEDMATRIX_DEVICES_Y * 8);
  add_2_info_str(tmp, true);
  snprintf(tmp,99,"\"ledmatrix\":1");
  add_2_info_str(tmp, true);
#else
  snprintf(tmp,99,"\"ledmatrix\":0");
  add_2_info_str(tmp, true);
#endif
  snprintf(tmp,99,"}");
  add_2_info_str(tmp, false);
#if defined(DEBUG_SERIAL_HTML)
  Serial.print(" ok (");
  Serial.print(strlen(info_str));
  Serial.println(" byte)");
#endif
  return info_str;
}

/*
 * Das folgende JSON bildet den Status der Logeinstellungen ab
 * Es wird NUR für die Webseite "settings.html" genutzt
 */
char* mk_logcfg(void) {
  char tmp[100];
#if defined(DEBUG_SERIAL_HTML)
  Serial.print("Generiere loginfo ... ");
#endif  
  snprintf(info_str, INFOSIZE-1, "{");
  snprintf(tmp,99,"\"log_sensor\":%s",preference.log_sensor?"1":"0");
  add_2_info_str(tmp, false);
  snprintf(tmp,99,"\"log_webcmd\":%s",preference.log_webcmd?"1":"0");
  add_2_info_str(tmp, true);
  snprintf(tmp,99,"\"log_sysinfo\":%s",preference.log_sysinfo?"1":"0");
  add_2_info_str(tmp, true);
  snprintf(tmp,99,"\"log_fs_sysinfo\":%s",preference.log_fs_sysinfo?"1":"0");
  add_2_info_str(tmp, true);
#if defined(RF24GW)
  snprintf(tmp,99,"\"log_rf24\":%s",eepromdata.log_rf24?"1":"0");
  add_2_info_str(tmp, true);
#endif
#if defined(MQTT)
  snprintf(tmp,99,"\"log_mqtt\":%s",preference.log_mqtt?"1":"0");
  add_2_info_str(tmp, true);
#endif
  snprintf(tmp,99,"}");
  add_2_info_str(tmp, false);
#if defined(DEBUG_SERIAL_HTML)
  Serial.print(" ok (");
  Serial.print(strlen(info_str));
  Serial.println(" byte)");
#endif
  return info_str;
}
/*
 * Das folgende JSON bildet den Status der Sensoren und Aktoren ab
 * Es wird für die Webseite und für MQTT genutzt
 */
char* mk_setcfg(void) {
  char tmp[100];
#if defined(DEBUG_SERIAL_HTML)
  Serial.println("Generiere setinfo ... ");
#endif
  snprintf(info_str, INFOSIZE-1, "{");
#if defined(MQTT)
  snprintf(tmp,99,"\"mqtt\":1");
  add_2_info_str(tmp, false);
  snprintf(tmp,99,"\"mqttserver\":\"%s\"",preference.mqttserver);
  add_2_info_str(tmp, true);
  snprintf(tmp,99,"\"mqttclient\":\"%s\"",preference.mqttclient);
  add_2_info_str(tmp, true);
#else
  snprintf(tmp,99,"\"mqtt\":0");
  add_2_info_str(tmp, false);
#endif
#if defined(RF24GW)
  if (eepromdata.rf24gw_enable) {
    snprintf(tmp,99,"\"rf24gw\":1");
    add_2_info_str(tmp, strlen(info_str) > 4? true: false);
    snprintf(tmp,99,"\"RF24HUB-Server\":\"%s\"",eepromdata.rf24gw_hub_name);
    add_2_info_str(tmp, true);
    snprintf(tmp,99,"\"RF24HUB-Port\":%d",eepromdata.rf24gw_hub_port);
    add_2_info_str(tmp, true);
    snprintf(tmp,99,"\"RF24GW-Port\":%d",eepromdata.rf24gw_gw_port);
    add_2_info_str(tmp, true);
    snprintf(tmp,99,"\"RF24GW-No\":%d",eepromdata.rf24gw_gw_no);
    add_2_info_str(tmp, true);
  } else {
    add_2_info_str("\"rf24gw\":0", strlen(info_str) > 4? true: false);
  }
#else
    snprintf(tmp,99,"\"rf24gw\":0");
    add_2_info_str(tmp, strlen(info_str) > 4? true: false);
#endif
#if defined(RF24NODE)
  if (eepromdata.rf24node_enable) {
    snprintf(tmp,99,"\"rf24gw\":1");
    add_2_info_str(tmp, strlen(info_str) > 4? true: false);
    snprintf(tmp,99,"\"rf24node_nid\":%d",eepromdata.rf24node_node_id);
    add_2_info_str(tmp, true);
    snprintf(tmp,99,"\"rf24node_m1n\":\"%s\"",eepromdata.rf24node_val1_name);
    add_2_info_str(tmp, true);
    snprintf(tmp,99,"\"rf24node_m1v\":%d",eepromdata.rf24node_val1_channel);
    add_2_info_str(tmp, true);
  } else {
    snprintf(tmp,99,"\"rf24gw\":0");
    add_2_info_str(tmp, strlen(info_str) > 4? true: false);
  }
#else
  snprintf(tmp,99,"\"rf24gw\":0");
  add_2_info_str(tmp, strlen(info_str) > 4? true: false);
#endif
#if defined(RF24NODE_VAL2)
  if (eepromdata.rf24node_val2_enable) {
    snprintf(tmp,99,"\"rf24node_m2\":1");
    add_2_info_str(tmp, strlen(info_str) > 4? true: false);
    snprintf(tmp,99,"\"rf24node_m2n\":\"%s\"",eepromdata.rf24node_val2_name);
    add_2_info_str(tmp, true);
    snprintf(tmp,99,"\"rf24node_m2v\":%d",eepromdata.rf24node_val2_channel);
    add_2_info_str(tmp, true);
  } else {
    add_2_info_str("\"rf24node_m2\":0", true);
  }
#else
    snprintf(tmp,99,"\"rf24node_m2\":0");
    add_2_info_str(tmp, strlen(info_str) > 4? true: false);
#endif
#if defined(RF24NODE_VAL3)
  if (eepromdata.rf24node_val3_enable) {
    snprintf(tmp,99,"\"rf24node_m3\":1");
    add_2_info_str(tmp, strlen(info_str) > 4? true: false);
    snprintf(tmp,99,"\"rf24node_m3n\":\"%s\"",eepromdata.rf24node_val3_name);
    add_2_info_str(tmp, true);
    snprintf(tmp,99,"\"rf24node_m3v\":%d",eepromdata.rf24node_val3_channel);
    add_2_info_str(tmp, true);
  } else {
    snprintf(tmp,99,"\"rf24node_m3\":0");
    add_2_info_str(tmp, strlen(info_str) > 4? true: false);
  }
#else
  snprintf(tmp,99,"\"rf24node_m3\":0");
  add_2_info_str(tmp, strlen(info_str) > 4? true: false);
#endif
  snprintf(tmp,99,"}");
  add_2_info_str(tmp, false);
#if defined(DEBUG_SERIAL_HTML)
  Serial.print(" ok (");
  Serial.print(strlen(info_str));
  Serial.println(" byte)");
#endif
  return info_str;  
}
/*
 * Das folgende JSON bildet den Status der Sensoren und Aktoren ab
 * Es wird für die Webseite und für MQTT genutzt
 */
char* mk_sysinfo1(void) {
#if defined(DEBUG_SERIAL_HTML)
  Serial.print("Generiere sysinfo1 ... ");
#endif
  char tmp[100];
  int rssi = WiFi.RSSI();
  int rssi_quality = 0;
  snprintf (info_str,INFOSIZE, "{");
#ifdef ESP32
  snprintf(tmp,99,"\"Platform\":\"%s\"",ESP.getChipModel());
#else
  snprintf(tmp,99,"\"Platform\":\"%s\"","ESP8266");
#endif
  add_2_info_str(tmp,false);
#ifdef ESP32
  snprintf(tmp,99,"\"Hostname\":\"%s\"",HOSTNAME);
#else
  snprintf(tmp,99,"\"Hostname\":\"%s\"",WiFi.hostname().c_str());
#endif
  add_2_info_str(tmp,true);
#ifdef ESP32
  snprintf(tmp,99,"\"Cores\":\"%d\"",ESP.getChipCores());
#else
  snprintf(tmp,99,"\"Cores\":\"%d\"",1);
#endif
  add_2_info_str(tmp,true);
  snprintf(tmp,99,"\"SSID\":\"%s (%ddBm / %d%%)\"",WiFi.SSID().c_str(), rssi, rssi_quality);
  add_2_info_str(tmp,true);
  snprintf(tmp,99,"\"IP\":\"%s\"",WiFi.localIP().toString().c_str());
  add_2_info_str(tmp,true);
  snprintf(tmp,99,"\"Channel\":\"%d\"",WiFi.channel());
  add_2_info_str(tmp,true);
  snprintf(tmp,99,"\"GW-IP\":\"%s\"",WiFi.gatewayIP().toString().c_str());
  add_2_info_str(tmp,true);
  snprintf(tmp,99,"\"Freespace\":\"%0.0fKB\"",ESP.getFreeSketchSpace() / 1024.0);
  add_2_info_str(tmp,true);
  snprintf(tmp,99,"\"Sketchsize\":\"%0.0fKB\"",ESP.getSketchSize() / 1024.0);
  add_2_info_str(tmp,true);
  snprintf(tmp,99,"\"FlashSize\":\"%dMB\"",(int)(ESP.getFlashChipSize() / 1024 / 1024));
  add_2_info_str(tmp,true);
  snprintf(tmp,99,"\"FlashFreq\":\"%dMHz\"",(int)(ESP.getFlashChipSpeed() / 1000000));
  add_2_info_str(tmp,true);
  snprintf(tmp,99,"\"CpuFreq\":\"%dMHz\"",(int)(F_CPU / 1000000));
  add_2_info_str(tmp,true);
  snprintf(tmp,99,"\"Vcc\":\"%.2fV\"}",getVcc());
  add_2_info_str(tmp,true);
#if defined(DEBUG_SERIAL_HTML)
  Serial.print(" ok (");
  Serial.print(strlen(info_str));
  Serial.println(" byte)");
#endif
  return info_str;
}

#ifdef ESP32
/*
 * Da der ESP Core keine ResetReason zurückgibt
 * wird diese hier nachgebildet
 */
char* getResetReason(char* tmp) {
#if defined(DEBUG_SERIAL_HTML)
  Serial.println("Reset Reason roh:");
  Serial.println(rtc_get_reset_reason(0));
#endif
  switch ( rtc_get_reset_reason(0) ) {
    case 1 : sprintf(tmp,"%s","POWERON_RESET"); break;          /**<1,  Vbat power on reset*/
    case 3 : sprintf(tmp,"%s","SW_RESET");break;               /**<3,  Software reset digital core*/
    case 4 : sprintf(tmp,"%s","OWDT_RESET");break;             /**<4,  Legacy watch dog reset digital core*/
    case 5 : sprintf(tmp,"%s","DEEPSLEEP_RESET");break;        /**<5,  Deep Sleep reset digital core*/
    case 6 : sprintf(tmp,"%s","SDIO_RESET");break;             /**<6,  Reset by SLC module, reset digital core*/
    case 7 : sprintf(tmp,"%s","TG0WDT_SYS_RESET");break;       /**<7,  Timer Group0 Watch dog reset digital core*/
    case 8 : sprintf(tmp,"%s","TG1WDT_SYS_RESET");break;       /**<8,  Timer Group1 Watch dog reset digital core*/
    case 9 : sprintf(tmp,"%s","RTCWDT_SYS_RESET");break;       /**<9,  RTC Watch dog Reset digital core*/
    case 10 : sprintf(tmp,"%s","INTRUSION_RESET");break;       /**<10, Instrusion tested to reset CPU*/
    case 11 : sprintf(tmp,"%s","TGWDT_CPU_RESET");break;       /**<11, Time Group reset CPU*/
    case 12 : sprintf(tmp,"%s","SW_CPU_RESET");break;          /**<12, Software reset CPU*/
    case 13 : sprintf(tmp,"%s","RTCWDT_CPU_RESET");break;      /**<13, RTC Watch dog Reset CPU*/
    case 14 : sprintf(tmp,"%s","EXT_CPU_RESET");break;         /**<14, for APP CPU, reseted by PRO CPU*/
    case 15 : sprintf(tmp,"%s","RTCWDT_BROWN_OUT_RESET");break;/**<15, Reset when the vdd voltage is not stable*/
    case 16 : sprintf(tmp,"%s","RTCWDT_RTC_RESET");break;      /**<16, RTC Watch dog reset digital core and rtc module*/
    default : sprintf(tmp,"%s","NO_MEAN");
  }
#if defined(DEBUG_SERIAL_HTML)
  Serial.println("Reset Reason:");
  Serial.println(tmp);
#endif
  return tmp;
}
#endif

/*
 * Das folgende JSON bildet den Status der Sensoren und Aktoren ab
 * Es wird für die Webseite und für MQTT genutzt
 */
char* mk_sysinfo2(void) {
  uint32_t free;
  uint16_t max;
  uint8_t frag;
  char tmp[100];
#if defined(DEBUG_SERIAL_HTML)
  Serial.print("Generiere sysinfo2 ... ");
#endif
  uptime::calculateUptime();
#ifdef ESP32
  free = ESP.getFreeHeap();
  frag = 0;
  max = 0;
#else
  ESP.getHeapStats(&free, &max, &frag);
#endif
  snprintf (info_str,INFOSIZE, "{");
  snprintf(tmp,99,"\"MAC\":\"%s\"", WiFi.macAddress().c_str());
  add_2_info_str(tmp,false);
  snprintf(tmp,99,"\"SubNetMask\":\"%s\"",WiFi.subnetMask().toString().c_str());
  add_2_info_str(tmp,true);
#ifdef ESP32
  char tmp1[25];
  snprintf(tmp,99,"\"ResetReason\":\"%s\"", getResetReason(tmp1));
#else
  snprintf(tmp,99,"\"ResetReason\":\"%s\"", ESP.getResetReason().c_str());
#endif
  add_2_info_str(tmp,true);
  snprintf(tmp,99,"\"Heap_max\":\"%0.2fKB\"",(float)max/1024.0);
  add_2_info_str(tmp,true);
  snprintf(tmp,99,"\"Heap_free\":\"%0.2fKB\"",(float)free/1024.0);
  add_2_info_str(tmp,true);
  snprintf(tmp,99,"\"Heap_frag\":\"%0.2f\"",(float)frag/1024.0);
  add_2_info_str(tmp,true);
  snprintf(tmp,99,"\"DnsIP\":\"%s\"",WiFi.dnsIP().toString().c_str());
  add_2_info_str(tmp,true);
  snprintf(tmp,99,"\"BSSID\":\"%s\"",WiFi.BSSIDstr().c_str());
  add_2_info_str(tmp,true);
#ifdef ESP32
  snprintf(tmp,99,"\"CoreVer\":\"%s\"","unknown");
#else
  snprintf(tmp,99,"\"CoreVer\":\"%s\"",ESP.getCoreVersion().c_str());
#endif
  add_2_info_str(tmp,true);
  snprintf(tmp,99,"\"IdeVer\":\"%d\"",ARDUINO);
  add_2_info_str(tmp,true);
  snprintf(tmp,99,"\"SdkVer\":\"%s\"",ESP.getSdkVersion());
  add_2_info_str(tmp,true);
  snprintf(tmp,99,"\"UpTime\":\"%luT%02lu:%02lu:%02lu\"",uptime::getDays(), uptime::getHours(), uptime::getMinutes(), uptime::getSeconds());
  add_2_info_str(tmp,true);
  snprintf(tmp,99,"\"SW\":\"%s / %s\"", SWVERSION, __DATE__);
  add_2_info_str(tmp,true);
  snprintf(tmp,99,"}");
  add_2_info_str(tmp,false);

 add_2_info_str("    ",false);

#if defined(DEBUG_SERIAL_HTML)
  Serial.print(" ok (");
  Serial.print(strlen(info_str));
  Serial.println(" byte)");
#endif
  return info_str;
}

void setup()
{
  // Serial port for debugging purposes
  Serial.begin(115200);
  info_str = (char*)malloc(INFOSIZE);
  tmp_str = (char*)malloc(TMP_STR_SIZE);
  logger.begin();

  // Connect to Wi-Fi
  wifi_con();

  if (!LittleFS.begin()) {
    ESP.restart();
    return;
  } else {
    sprintf(info_str,"%s","++ Begin Startup: LittleFS mounted ++");
    logtopic = log_sys;
    write2log(1, info_str);
  }
  // Fill preference structure
  snprintf(preference.mqttclient,SERVERNAMESIZE,MQTT_CLIENTNAME);
  snprintf(preference.mqttserver,SERVERNAMESIZE,MQTT_SERVER);
  read_preferences();
  initWebSocket();
  logtopic = log_sys;
  write2log(1,"initWebsocket ok");


  httpServer.on("/webcfg", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", mk_webcfg());
  });
  httpServer.on("/cmd", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", mk_cmd(request));
  });
  httpServer.on("/logcfg", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", mk_logcfg());
  });
  httpServer.on("/setcfg", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", mk_setcfg());
  });
  httpServer.on("/sysinfo1", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", mk_sysinfo1());
  });
  httpServer.on("/sysinfo2", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", mk_sysinfo2());
  });
  httpServer.on("/restart", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(304, "message/http", do_restart());
  });
  httpServer.on("/console", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", logger.printBuffer());
  });
  // This serves all static web content
  httpServer.onNotFound(serveFile);
  // Start ElegantOTA
  AsyncElegantOTA.begin(&httpServer);
  // Start server
  httpServer.begin();

#if defined(SW1)
  snprintf(tmp_str,TMP_STR_SIZE-1,SW1_LABEL);
  sw1.begin(tmp_str);
#if defined(SW1_MQTT)
  snprintf(tmp_str,TMP_STR_SIZE-1,SW1_MQTT);
  sw1.set_sw_mqtt(tmp_str);
#endif
#if defined(SW1_SL0_MQTT)
  sw1.set_vol_mqtt(SW1_SL0_MQTT);
#endif
#if defined(SW1_SL1_MQTT)
  sw1.set_R_mqtt(SW1_SL1_MQTT);
#endif
#if defined(SW1_SL2_MQTT)
  sw1.set_G_mqtt(SW1_SL2_MQTT);
#endif
#if defined(SW1_SL3_MQTT)
  sw1.set_B_mqtt(SW1_SL3_MQTT);
#endif
#if defined(SW1_INIT)
  SW1_INIT
#endif
#endif  

  logtopic = log_sys;
  write2log(1,"Setup Ende");
}

void loop()
{
  ws.cleanupClients();
  if (rebootflag) {
    delay(3000);
    ESP.restart();
  }
#if defined(SW1)
  if (sw1.changed()) {
#if defined(DEBUG_SERIAL_SENSOR)
      Serial.println("Switch1 changed");
#endif
    uint8_t swTyp = sw1.get_typ();
    if (sw1.get_state()) {
      SW1_ON_CMD
      if ( swTyp == 2 ) {
        SW1_VOL_CMD
      }
      if ( swTyp == 3 ) {
#if defined(DEBUG_SERIAL_SENSOR)
      Serial.print("Typ 3   ");
      Serial.print(sw1.get_R());
      Serial.print("  ");
      Serial.print(sw1.get_G());
      Serial.print("  ");
      Serial.println(sw1.get_B());
#endif
        SW1_R_CMD
        SW1_G_CMD
        SW1_B_CMD
      }
    } else {
      SW1_OFF_CMD
    }
#if defined(MQTT)
//    send_mqtt_statt=true;
#endif

  }
#endif  

}