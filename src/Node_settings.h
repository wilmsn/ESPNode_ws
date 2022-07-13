/******************************************************
The following settings ca be used for the individual node:
(m) = mandatory to enable; (o) = optional

++++++++Debugging:+++++++++++
Enable RF24 debugging
(o) #define DEBUG_SERIAL_RF24
Enable Sensor debugging
(o) #define DEBUG_SERIAL_SENSOR

++++++++RF24:+++++++++++
RF24 Gateway:
(m) #define RF24GW_NO   <Gateway ID>

RF24 Node:

++++++++Actors:+++++++++++

++++++++Sensors:+++++++++++
Bosch BMP180/280 BME280
(m) #define SENSOR_DUMMY

18B20 Dallas Temperature Sensor
(m) #define SENSOR_18B20
 
Analog Input
(m) #define ANALOGINPUT_LABEL      "LDR"






optional:
- This node has a buildin rf24-gateway
// #define RF24GW
- Select a sensor
-- A Dummy for test purposes
// #define SENSOR_DUMMY
-- a Bosch sensor like BMP185/BMP280/BME280
// #define SENSOR_BOSCH
-- A Dallas 18B20 temperature sensor
// #define SENSOR_18B20
-- A Switch / Relais
-- define a pin
// #define RELAIS_1 3
// #define RELAIS_2 3
-- Use Neopixel
// #define NEOPIXEL <Number of Pixel>
-- Use LEDMATRIX
// #define LEDMATRIX <Number of 8x8 Displays>
- the status LED
-- define a pin (default is 3)
// #define STATUSLED       <LED PIN>
-- a pin level for on (default is HIGH)
// #define STATUSLED_ON    <LEVEL>
-- a pin level for off (default is LOW)
// #define STATUSLED_OFF   <LEVEL>
- version of the EEPROM (only a different version stores new values!!)
// #define EEPROM_VERSION  5

******************************************************/
//*****************************************************
//    Individual settings
//-----------------------------------------------------
#if defined(MATRIXNODE)
#define HOSTNAME                   "matrixnode"
#define HOST_DISCRIPTION           "A Matrix Node"
#define MQTT_CLIENTNAME            "matrixnode"
#define MQTT_SERVER                "rpi2.fritz.box"
#define MQTT_STATE_VAR             sw1.get_state()?"on":"off"
#define SW1_TYP                     2
#define SW1_MQTT                   "display"
#define SW1_LABEL                  "Display" 
#define SW1_INIT                   sw1.set_state(1); sw1.set_vol(0);            
#define SW1_ON_CMD                 matrix.on();
#define SW1_OFF_CMD                matrix.off();
#define SW1_VOL_CMD                matrix.setIntensity(sw1.get_vol());
#define SW1_SL0_MQTT               "intensity"
#define LEDMATRIX_DEVICES_X        4
#define LEDMATRIX_DEVICES_Y        1
#define LEDMATRIX_SWITCH_MQTT      "display"
#define LEDMATRIX_TEXT_MQTT        "line"
#define LEDMATRIX_GRAPH_MQTT       "graph"
#define LEDMATRIX_INTENSITY_MQTT   "intensity"
#define FORCEEEPROMUPDATE
#define DEBUG_SERIAL_HTML
#define DEBUG_SERIAL_SENSOR
#define DEBUG_SERIAL_MQTT
#endif
//-----------------------------------------------------
#if defined(WITTYNODE)
#define HOSTNAME               "wittynode"
#define HOST_DISCRIPTION       "A Witty Node"
#define MQTT_CLIENTNAME        "wittynode"
#define MQTT_SERVER            "rpi2.fritz.box"
#define MQTT_STATE_VAR         sw1.get_state()?"on":"off"
//#define RF24NODE_NODEID        0
#define RF24NODE_VAL1_NAME     "LDR"
#define RF24NODE_VAL1_VAR      ldr
#define RF24NODE_VAL1_CHANNEL  70
#define DEBUG_SERIAL_HTML
#define DEBUG_SERIAL_SENSOR
#define DEBUG_SERIAL_MQTT
#define ANALOGINPUT_LABEL      "LDR"
#define SW1_MQTT                "RGB"
#define SW1_LABEL               "RGB"
#define SW1_R_CMD                analogWrite(WITTY_RGB_RT, sw1.get_R()*68);
#define SW1_G_CMD                analogWrite(WITTY_RGB_GN, sw1.get_G()*68);
#define SW1_B_CMD                analogWrite(WITTY_RGB_BL, sw1.get_B()*68);
#define SW1_SL1_MQTT             "R"
#define SW1_SL2_MQTT             "G"
#define SW1_SL3_MQTT             "B"
#define SW1_TYP                 3
#define SW1_LABEL               "RGB"
#define SW1_INIT                pinMode(WITTY_RGB_RT, OUTPUT); pinMode(WITTY_RGB_GN, OUTPUT); pinMode(WITTY_RGB_BL, OUTPUT);
#define SW1_ON_CMD              noop();
#define SW1_OFF_CMD            analogWrite(WITTY_RGB_RT, 0); analogWrite(WITTY_RGB_GN, 0); analogWrite(WITTY_RGB_BL, 0);
#define SENSORINTERVAL         5000
#define MAGICNO                1238
#endif
//-----------------------------------------------------
#if defined(RF24GWTEST)
#define HOSTNAME               "rf24gwtest"
#define HOST_DISCRIPTION       "Ein RF24 Testnode"
#define MQTT_CLIENTNAME        "rf24gwtest"
#define MQTT_SERVER            "rpi2.fritz.box"
#define DEBUG_SERIAL
#define LED                    5
#define SW1_TYP                 1
#define SW1_LABEL               "LED"
#define SW1_MQTT                "sw1"
#define SW1_INIT                pinMode(LED, OUTPUT);
#define SW1_ONCMD               digitalWrite(LED, HIGH);
#define SW1_OFFCMD              digitalWrite(LED, LOW);
#define SI7021
#define SENSORINTERVAL         5000
#define RF24GW_NO              99
#define RF24GW_RADIO_CE_PIN    15
#define RF24GW_RADIO_CSN_PIN   2
//#define RF24NODE_NODEID        1
//#define RF24NODE_VAL1_NAME     "Temperatur"
//#define RF24NODE_VAL1_CHANNEL  39
#define MAGICNO                1233
#endif
//-----------------------------------------------------
#if defined(MICROTESTNODE)
#define HOSTNAME               "micronode"
#define HOST_DISCRIPTION       "Ein RF24 Testnode"
#define MQTT_CLIENTNAME        "micronode"
#define MQTT_SERVER            "rpi2.fritz.box"
//#define DEBUG_SERIAL_SENSOR
#define DEBUG_SERIAL_RF24
#define DEBUG_SERIAL_HTML
#define SENSOR_BOSCH
//#define SENSORINTERVAL         60000
#define RF24GW_NO              99
#define RF24GW_HUB_SERVER      "rpi2.fritz.box"
#define RF24NODE_NODEID        1
#define RF24NODE_VAL1_NAME     "Temperatur"
#define RF24NODE_VAL1_UNIT     "°C"
#define RF24NODE_VAL1_CHANNEL  11
#define RF24NODE_VAL1_VAR      temp
#define RF24NODE_VAL2_NAME     "Luftdruck"
#define RF24NODE_VAL2_UNIT     "hPa"
#define RF24NODE_VAL2_CHANNEL  12
#define RF24NODE_VAL2_VAR      pres
#define RF24NODE_VAL3_NAME     "Luftfeuchtigkeit"
#define RF24NODE_VAL3_UNIT     "%"
#define RF24NODE_VAL3_CHANNEL  13
#define RF24NODE_VAL3_VAR      humi
#define MAGICNO                1231
#define SW1_TYP                 1
#define SW1_LABEL               "LED1"
#define SW1_MQTT                "led1"
#define SW1_INIT                pinMode(D3, OUTPUT); digitalWrite(D3, HIGH);
#define SW1_ON_CMD               digitalWrite(D3, LOW);
#define SW1_OFF_CMD              digitalWrite(D3, HIGH);
#define SW2_TYP                 1
#define SW2_LABEL               "LED2"
#define SW2_MQTT                "led2"
#define SW2_INIT                pinMode(D4, OUTPUT); digitalWrite(D4, HIGH);
#define SW2_ON_CMD               digitalWrite(D4, LOW);
#define SW2_OFF_CMD              digitalWrite(D4, HIGH);
#endif
//-----------------------------------------------------
#if defined(ESPMINI)
#define HOSTNAME               "espmini"
#define HOST_DISCRIPTION       "Ein RF24 Testnode<br>Ausstattung nach Bedarf"
#define DEBUG_SERIAL_HTML
#define DEBUG_SERIAL_MQTT
#define MQTT_CLIENTNAME        "espmini"
#define MQTT_SERVER            "rpi2.fritz.box"
#define SW1_TYP                 1
#define SW1_LABEL              "LED"
#define SW1_MQTT               "led"
#define SW1_INIT                pinMode(2, OUTPUT); digitalWrite(2, HIGH);
#define SW1_ON_CMD              digitalWrite(2, LOW);
#define SW1_OFF_CMD             digitalWrite(2, HIGH);
#define INTERNETRADIO
#endif
//-----------------------------------------------------
/*
 * Einfacher Node zum Test der grundlegenden Funktionen
 */
#if defined(ESPMINIMAL)
#define HOSTNAME               "espminimal"
#define HOST_DISCRIPTION       "Ein Testnode mit minimaler Ausstattung<br>Kein Mqtt<br>Kein RF24GW<br>Kein RF24Node<br>Keine Sensoren<br>Kein Schalter"
#define DEBUG_SERIAL_HTML
#define MAGICNO                1224
#endif
//-----------------------------------------------------

// Generic Settings for all Nodes
#define LOGGER_NUMLINES         25
#define LOGGER_LINESIZE         250
#define INFOSIZE                1000
#define TMP_STR_SIZE            20
#define NODE_DATTYPE            uint8_t
#define SERVERNAMESIZE          25


//define constrains for precompiler 
//no changes below !!!!!!!!!!!!!!!!
#if defined(SENSOR_18B20)
#define MESSAGE1
#undef NOSENSOR
#endif
#if defined(SENSOR_ANALOG)
#define MESSAGE1
#undef NOSENSOR
#endif
#if defined(RF24GW_NO)
#define RF24GW
#define RF24GW_ENABLED  true
#endif
#if defined(RF24NODE_NODEID)
#define RF24NODE
#define RF24NODE_ENABLED     true
#if defined(RF24NODE_VAL1_CHANNEL)
#define RF24NODE_VAL1
#endif
#if defined(RF24NODE_VAL2_CHANNEL)
#define RF24NODE_VAL2
#define RF24NODE_VAL2_ENABLED   true
#endif
#if defined(RF24NODE_VAL3_CHANNEL)
#define RF24NODE_VAL3
#define RF24NODE_VAL3_ENABLED   true
#endif
#endif
#if defined(DEBUG_SERIAL_SENSOR)
#define DEBUG_SERIAL
#endif
#if defined(DEBUG_SERIAL_RF24)
#define DEBUG_SERIAL
#endif
#if defined(DEBUG_SERIAL_HTML)
#define DEBUG_SERIAL
#endif
#if defined(DEBUG_SERIAL_MQTT)
#define DEBUG_SERIAL
#endif
#if defined(MQTT_SERVER) && defined(MQTT_CLIENTNAME)
#define MQTT
#endif
#if defined(SW1_TYP)
#define SW1  1
#endif
#ifndef SW1_VOL_CMD
#define SW1_VOL_CMD         noop();
#endif
#ifndef SW1_R_CMD
#define SW1_R_CMD           noop();
#endif
#ifndef SW1_G_CMD
#define SW1_G_CMD           noop();
#endif
#ifndef SW1_B_CMD
#define SW1_B_CMD           noop();
#endif
#if defined(SW2_TYP)
#define SW2  2
#endif
#if defined(SW3_TYP)
#define SW3  3
#endif
#if defined(SW4_TYP)
#define SW4  4
#endif
#if defined(ANALOGINPUT_LABEL)
#define ANALOGINPUT
#endif
#if defined(LEDMATRIX_DEVICES_X) && defined(LEDMATRIX_DEVICES_Y)
#define LEDMATRIX
#endif
