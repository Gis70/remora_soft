// **********************************************************************************
// Remora Configuration include file
// **********************************************************************************
// Creative Commons Attrib Share-Alike License
// You are free to use/extend this library but please abide with the CC-BY-SA license:
// Attribution-NonCommercial-ShareAlike 4.0 International License
// http://creativecommons.org/licenses/by-nc-sa/4.0/
//
// This program works with the Remora board
// see schematic here https://github.com/thibdct/programmateur-fil-pilote-wifi
//
// Written by Charles-Henri Hallard (http://hallard.me)
//
// History 2016-02-04 - First release
//
// All text above must be included in any redistribution.
//
// **********************************************************************************

#ifndef __CONFIG_H__
#define __CONFIG_H__

// Include main project include file
#include "remora.h"

#ifdef ESP8266

#define CFG_SSID_SIZE 		32
#define CFG_PSK_SIZE  		64
#define CFG_HOSTNAME_SIZE 16

#define CFG_EMON_HOST_SIZE 		32
#define CFG_EMON_APIKEY_SIZE 	32
#define CFG_EMON_URL_SIZE 		32
#define CFG_EMON_DEFAULT_PORT 80
#define CFG_EMON_DEFAULT_HOST "emoncms.org"
#define CFG_EMON_DEFAULT_URL  "/input/post.json"

#define CFG_JDOM_HOST_SIZE    32
#define CFG_JDOM_APIKEY_SIZE  32
#define CFG_JDOM_URL_SIZE     64
#define CFG_JDOM_ADCO_SIZE    12
#define CFG_JDOM_DEFAULT_PORT 80
#define CFG_JDOM_DEFAULT_HOST "jeedom.local"
#define CFG_JDOM_DEFAULT_URL  "/jeedom/plugins/teleinfo/core/php/jeeTeleinfo.php"
#define CFG_JDOM_DEFAULT_ADCO "0000111122223333"

// Port pour l'OTA
#define DEFAULT_OTA_PORT     8266
#define DEFAULT_OTA_AUTH     "OTA_Remora"
//#define DEFAULT_OTA_AUTH     ""
#define CFG_ADCO_SIZE    12

// Bit definition for different configuration modes
#define CFG_LCD				  0x0001	// Enable display
#define CFG_DEBUG			  0x0002	// Enable serial debug
#define CFG_RGB_LED     0x0004  // Enable RGB LED
#define CFG_BAD_CRC     0x8000  // Bad CRC when reading configuration

// Web Interface Configuration Form field names
#define CFG_FORM_SSID     FPSTR("ssid")
#define CFG_FORM_PSK      FPSTR("psk")
#define CFG_FORM_HOST     FPSTR("host")
#define CFG_FORM_AP_PSK   FPSTR("ap_psk")
#define CFG_FORM_OTA_AUTH FPSTR("ota_auth")
#define CFG_FORM_OTA_PORT FPSTR("ota_port")

#define CFG_FORM_EMON_HOST  FPSTR("emon_host")
#define CFG_FORM_EMON_PORT  FPSTR("emon_port")
#define CFG_FORM_EMON_URL   FPSTR("emon_url")
#define CFG_FORM_EMON_KEY   FPSTR("emon_apikey")
#define CFG_FORM_EMON_NODE  FPSTR("emon_node")
#define CFG_FORM_EMON_FREQ  FPSTR("emon_freq")

#define CFG_FORM_JDOM_HOST  FPSTR("jdom_host")
#define CFG_FORM_JDOM_PORT  FPSTR("jdom_port")
#define CFG_FORM_JDOM_URL   FPSTR("jdom_url")
#define CFG_FORM_JDOM_KEY   FPSTR("jdom_apikey")
#define CFG_FORM_JDOM_ADCO  FPSTR("jdom_adco")
#define CFG_FORM_JDOM_FREQ  FPSTR("jdom_freq")

#define CFG_FORM_IP  FPSTR("wifi_ip");
#define CFG_FORM_GW  FPSTR("wifi_gw");
#define CFG_FORM_MSK FPSTR("wifi_msk");

// RGB Led 
#define DEFAULT_LED_BRIGHTNESS  50                // 50%
#define DEFAULT_LED_HEARTBEAT   10                // 1s (1/10s)
#define DEFAULT_LED_TYPE        NeoPixelType_Rgb  // RGB
#define DEFAULT_LED_GPIO        0                 // GPIO 0
#define DEFAULT_LED_NUM         1                 // 1 RGB LED

// Bit definition for different configuration modes
#define CFG_LCD         0x0001  // Enable display
#define CFG_DEBUG       0x0002  // Enable serial debug
#define CFG_RGB_LED     0x0004  // Enable RGB LED
#define CFG_AP          0x0008  // Enable Wifi Access Point
#define CFG_SI7021      0x0010  // SI7021 seen
#define CFG_SHT10       0x0020  // SHT10 seen
#define CFG_STATIC      0x2000  // Enable Static IP
#define CFG_WIFI        0x4000  // Enable Wifi
#define CFG_BAD_CRC     0x8000  // Bad CRC when reading configuration

// Show config and help sections
#define CFG_HLP_ALL     0xFFFF
#define CFG_HLP_HELP    0x0000
#define CFG_HLP_SYS     0x0001
#define CFG_HLP_WIFI    0x0002
#define CFG_HLP_DATA    0x0004
#define CFG_HLP_SENSOR  0x0008
#define CFG_HLP_JEEDOM  0x0010
#define CFG_HLP_DOMZ    0x0020
#define CFG_HLP_COUNTER 0x0040

#define CFG_SERIAL_BUFFER_SIZE 128

#pragma pack(push)  // push current alignment to stack
#pragma pack(1)     // set alignment to 1 byte boundary

// Config for emoncms
// 128 Bytes
typedef struct 
{
  char  host[CFG_EMON_HOST_SIZE+1]; 		// FQDN 
  char  apikey[CFG_EMON_APIKEY_SIZE+1]; // Secret
  char  url[CFG_EMON_URL_SIZE+1];  			// Post URL
  uint16_t port;    								    // Protocol port (HTTP/HTTPS)
  uint16_t node;     									  // optional node
  uint32_t freq;                        // refresh rate
  uint8_t filler[21];									  // in case adding data in config avoiding loosing current conf by bad crc*/
} _emoncms;

// Config for jeedom
// 160 Bytes
typedef struct 
{
  char  host[CFG_JDOM_HOST_SIZE+1];     // FQDN 
  char  apikey[CFG_JDOM_APIKEY_SIZE+1]; // Secret
  char  url[CFG_JDOM_URL_SIZE+1];       // Post URL
  char  adco[CFG_JDOM_ADCO_SIZE+1];     // Identifiant compteur
  uint16_t port;                        // Protocol port (HTTP/HTTPS)
  uint32_t freq;                        // refresh rate
  uint8_t filler[10];                   // in case adding data in config avoiding loosing current conf by bad crc*/
} _jeedom;

// Config saved into eeprom
// 1024 bytes total including CRC
typedef struct 
{
  char  ssid[CFG_SSID_SIZE+1]; 		 // SSID     
  char  psk[CFG_PSK_SIZE+1]; 		   // Pre shared key
  char  host[CFG_HOSTNAME_SIZE+1]; // Hostname 
  char  ap_psk[CFG_PSK_SIZE+1];    // Access Point Pre shared key
  char  ap_ssid[CFG_SSID_SIZE+1];  // Access Point SSID name
  char  ota_auth[CFG_PSK_SIZE+1];  // OTA Authentication password
  uint32_t config;           		   // Bit field register 
  uint16_t ota_port;         		   // OTA port
  uint8_t led_bright;              // RGB Led brigthness
  uint8_t led_hb;                  // RGB Led HeartBeat
  uint8_t led_type;                // RGB Led type
  uint8_t led_num;                 // # of RGB LED
  uint8_t led_gpio;                // GPIO driving RGBLED
  uint32_t ip;                     // Static Wifi IP Address
  uint32_t mask;                   // Static Wifi NetMask
  uint32_t gw;                     // Static Wifi Gateway Address
  uint32_t dns;                    // Static Wifi DNS server Address
  uint8_t  filler[77];      		   // in case adding data in config avoiding loosing current conf by bad crc
  _emoncms emoncms;                // Emoncms configuration
  _jeedom  jeedom;                 // jeedom configuration
  uint8_t  filler1[352];           // Another filler in case we need more
  uint16_t crc;
} _Config;


// Exported variables/object instancied in main sketch
// ===================================================
extern _Config config;

#pragma pack(pop)
 
// Declared exported function from route.cpp
// ===================================================
bool readConfig(bool clear_on_error=true);
bool saveConfig(void);
void showConfig(void);
void showHelp(uint16_t section = CFG_HLP_ALL, uint32_t clientid = 0);
void resetConfig(void);
void resetBoard(uint32_t clientid = 0);
void execCmd(char * line, uint32_t clientid = 0);
void handle_serial(char * line = NULL, uint32_t clientid = 0 );

#endif // ESP8266 
#endif // CONFIG_h

