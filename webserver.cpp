// **********************************************************************************
// Remora WEB Server, route web function
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
// History 2015-06-14 - First release
//         2015-11-31 - Added Remora API
//         2016-01-04 - Added Web Interface part
//
// All text above must be included in any redistribution.
//
// **********************************************************************************

// Include header
#include "webserver.h"

#ifdef ESP8266

/* ======================================================================
Function: getContentType
Purpose : return correct mime content type depending on file extension
Input   : -
Output  : Mime content type
Comments: -
====================================================================== */
String getContentType(String filename) {
  if(filename.endsWith(".htm")) return F("text/html");
  else if(filename.endsWith(".html")) return F("text/html");
  else if(filename.endsWith(".css")) return F("text/css");
  else if(filename.endsWith(".json")) return F("text/json");
  else if(filename.endsWith(".js")) return F("application/javascript");
  else if(filename.endsWith(".png")) return F("image/png");
  else if(filename.endsWith(".gif")) return F("image/gif");
  else if(filename.endsWith(".jpg")) return F("image/jpeg");
  else if(filename.endsWith(".ico")) return F("image/x-icon");
  else if(filename.endsWith(".xml")) return F("text/xml");
  else if(filename.endsWith(".pdf")) return F("application/x-pdf");
  else if(filename.endsWith(".zip")) return F("application/x-zip");
  else if(filename.endsWith(".gz")) return F("application/x-gzip");
  else if(filename.endsWith(".otf")) return F("application/x-font-opentype");
  else if(filename.endsWith(".eot")) return F("application/vnd.ms-fontobject");
  else if(filename.endsWith(".svg")) return F("image/svg+xml");
  else if(filename.endsWith(".woff")) return F("application/x-font-woff");
  else if(filename.endsWith(".woff2")) return F("application/x-font-woff2");
  else if(filename.endsWith(".ttf")) return F("application/x-font-ttf");
  return "text/plain";
}

/* ======================================================================
Function: formatSize
Purpose : format a asize to human readable format
Input   : size
Output  : formated string
Comments: -
====================================================================== */
String formatSize(size_t bytes)
{
  if (bytes < 1024){
    return String(bytes) + F(" Byte");
  } else if(bytes < (1024 * 1024)){
    return String(bytes/1024.0) + F(" KB");
  } else if(bytes < (1024 * 1024 * 1024)){
    return String(bytes/1024.0/1024.0) + F(" MB");
  } else {
    return String(bytes/1024.0/1024.0/1024.0) + F(" GB");
  }
}

/* ======================================================================
Function: handleFileRead
Purpose : return content of a file stored on SPIFFS file system
Input   : file path
Output  : true if file found and sent
Comments: -
====================================================================== */
bool handleFileRead(String path) {
  if ( path.endsWith("/") )
    path += "index.htm";

  String contentType = getContentType(path);
  String pathWithGz = path + ".gz";

  DebugF("handleFileRead ");
  Debug(path);

  if(SPIFFS.exists(pathWithGz) || SPIFFS.exists(path)) {
    if( SPIFFS.exists(pathWithGz) ){
      path += ".gz";
      DebugF(".gz");
    }

    DebuglnF(" found on FS");

    File file = SPIFFS.open(path, "r");
    request->send(file, contentType, file.length);
    file.close();
    return true;
  }

  Debugln("");

  //server.send(404, "text/plain", "File Not Found");
  return false;
}

/* ======================================================================
Function: formatNumberJSON
Purpose : check if data value is full number and send correct JSON format
Input   : String where to add response
          char * value to check
Output  : -
Comments: 00150 => 150
          ADCO  => "ADCO"
          1     => 1
====================================================================== */
void formatNumberJSON( String &response, char * value)
{
  // we have at least something ?
  if (value && strlen(value))
  {
    boolean isNumber = true;
    uint8_t c;
    char * p = value;

    // just to be sure
    if (strlen(p)<=16) {
      // check if value is number
      while (*p && isNumber) {
        if ( *p < '0' || *p > '9' )
          isNumber = false;
        p++;
      }

      // this will add "" on not number values
      if (!isNumber) {
        response += '\"' ;
        response += value ;
        response += '\"' ;
      } else {
        // this will remove leading zero on numbers
        p = value;
        while (*p=='0' && *(p+1) )
          p++;
        response += p ;
      }
    } else {
      response += "\"Error Value too long\"" ;
      Serial.println(F("formatNumberJSON Value too long!"));
    }
  } else {
    response += "\"Error Bad Value\"" ;
    Serial.println(F("formatNumberJSON Bad Value!"));
  }
}

/* ======================================================================
Function: tinfoJSONTable
Purpose : dump all teleinfo values in JSON table format for browser
Input   : request pointer if comming from web request
Output  : JsonStr filled if request is null
Comments: -
====================================================================== */
String tinfoJSONTable(AsyncWebServerRequest * request) {

  // Just to debug where we are
  DebugF("Serving /tinfo page...\r\n");

  #ifdef MOD_TELEINFO

  ValueList * me = tinfo.getList();
  String JsonStr = "";

  // If Web request or just string asking, we'll do JSon stuff
  // in async response,
  AsyncJsonResponse * response = new AsyncJsonResponse(true);
  JsonArray& arr = response->getRoot();

  // Web request ?
  if (request) {
    DebugF("Serving /system page...");
  } else {
    DebugF("Getting system JSON table...");
  }

  // Got at least one ?
  if (me) {

    // Loop thru the node
    while (me->next) {

      // we're there
      ESP.wdtFeed();

      // go to next node
      me = me->next;

      {
        JsonObject& item = arr.createNestedObject();
        item[FPSTR(FP_NA)] = "Uptime";
        item[FPSTR(FP_VA)] = seconds;
        // if (me->checksum == '"' || me->checksum == '\\' || me->checksum == '/')
        //   response += '\\';
        item[FPSTR(FP_CK)] = (char) me->checksum;
        item[FPSTR(FP_FL)] = me->flags;
      }
    }
   // Json end

  } else {
    DebuglnF("sending 404...");
    request->send(404, "text/plain", "No data");
  }
  // Web request send response to client
  size_t jsonlen;
  if (request) {
    DebugF("sending...");
    jsonlen = response->setLength();
    request->send(response);
  } else {
    // Send JSon to our string
    arr.printTo(JsonStr);
    jsonlen =  arr.measureLength();
    // Since it's nor a WEB request, we need to manually delete
    // response object so ArduinJSon object is freed
    delete response;
  }
  Debugf("Json size %lu bytes\r\n", jsonlen);

  #else
    DebuglnF("sending 404...");
    request->send(404, "text/plain", "Teleinfo non activée");
  #endif // MOD_TELEINFO

  return JsonStr;
}

/* ======================================================================
Function: sysJSONTable
Purpose : dump all sysinfo values in JSON table format for browser
Input   : request pointer if comming from web request
Output  : JsonStr filled if request is null
Comments: -
====================================================================== */
String sysJSONTable(AsyncWebServerRequest * request) {
  String JsonStr="";

  // If Web request or just string asking, we'll do JSon stuff
  // in async response,
  AsyncJsonResponse * response = new AsyncJsonResponse(true);
  JsonArray& arr = response->getRoot();

  // Web request ?
  if (request) {
    DebugF("Serving /system page...");
  } else {
    DebugF("Getting system JSON table...");
  }

  { JsonObject& item = arr.createNestedObject();
    item[FPSTR(FP_NA)] = "Uptime";
    item[FPSTR(FP_VA)] = uptime; }

  { JsonObject& item = arr.createNestedObject();
    item[FPSTR(FP_NA)] = "Version Logiciel";
    item[FPSTR(FP_VA)] = REMORA_VERSION; }

  String compiled =  __DATE__ " " __TIME__;
  { JsonObject& item = arr.createNestedObject();
    item[FPSTR(FP_NA)] = "Compilé le";
    item[FPSTR(FP_VA)] = __DATE__ " " __TIME__; }

  String version = "";
  #if defined (REMORA_BOARD_V10)
    version += F("V1.0");
  #elif defined (REMORA_BOARD_V11)
    version += F("V1.1");
  #elif defined (REMORA_BOARD_V12)
    version += F("V1.2 avec MCP23017");
  #elif defined (REMORA_BOARD_V13)
    version += F("V1.3 avec MCP23017");
  #else
    version += F("Non définie");
  #endif
  { JsonObject& item = arr.createNestedObject();
    item[FPSTR(FP_NA)] = "Version Matériel";
    item[FPSTR(FP_VA)] = version; }

  response += "{\"na\":\"Modules activés\",\"va\":\"";
  String modules = "";
  #ifdef MOD_OLED
    modules += F("OLED ");
  #endif
  #ifdef MOD_TELEINFO
    modules += F("TELEINFO ");
  #endif
  #ifdef MOD_RF69
    modules += F("RFM69 ");
  #endif

  { JsonObject& item = arr.createNestedObject();
    item[FPSTR(FP_NA)] = "Modules activés";
    item[FPSTR(FP_VA)] = modules;  }

  // Free mem should be last one but not really readable on bottom table
  { JsonObject& item = arr.createNestedObject();
    item[FPSTR(FP_NA)] = "Free Ram";
    item[FPSTR(FP_VA)] = formatSize(system_get_free_heap_size());  }

  { JsonObject& item = arr.createNestedObject();
    item[FPSTR(FP_NA)] = "SDK Version";
    item[FPSTR(FP_VA)] = system_get_sdk_version(); }

  char analog[8];
  sprintf_P( analog, PSTR("%d mV"), ((1000*analogRead(A0))/1024) );
  { JsonObject& item = arr.createNestedObject();
    item[FPSTR(FP_NA)] = "Analog";
    item[FPSTR(FP_VA)] = analog; }

  { JsonObject& item = arr.createNestedObject();
    item[FPSTR(FP_NA)] = "Reset cause";
    item[FPSTR(FP_VA)] = ESP.getResetReason(); }

  char chipid[9];
  sprintf_P(chipid, PSTR("0x%06X"), system_get_chip_id() );
  { JsonObject& item = arr.createNestedObject();
    item[FPSTR(FP_NA)] = "Chip ID";
    item[FPSTR(FP_VA)] = chipid; }

  char boot_version[7];
  sprintf_P(boot_version, PSTR("0x%0X"), system_get_boot_version() );
  { JsonObject& item = arr.createNestedObject();
    item[FPSTR(FP_NA)] = "Boot Version";
    item[FPSTR(FP_VA)] = boot_version ; }

  // WiFi Informations
  // =================
  const char* modes[] = { "NULL", "STA", "AP", "STA+AP" };
  { JsonObject& item = arr.createNestedObject();
    item[FPSTR(FP_NA)] = "Wifi Mode";
    item[FPSTR(FP_VA)] = modes[wifi_get_opmode()]; }

  const char* phymodes[] = { "", "B", "G", "N" };
  { JsonObject& item = arr.createNestedObject();
    item[FPSTR(FP_NA)] = "Wifi 802.11 Type";
    item[FPSTR(FP_VA)] = phymodes[(int) wifi_get_phy_mode()]; }

  { JsonObject& item = arr.createNestedObject();
    item[FPSTR(FP_NA)] = "Wifi Channel";
    item[FPSTR(FP_VA)] = wifi_get_channel(); }

  { JsonObject& item = arr.createNestedObject();
    item[FPSTR(FP_NA)] = "Wifi AP ID";
    item[FPSTR(FP_VA)] = wifi_station_get_current_ap_id(); }

  { JsonObject& item = arr.createNestedObject();
    item[FPSTR(FP_NA)] = "Wifi Status";
    item[FPSTR(FP_VA)] = (int) wifi_station_get_connect_status(); }

  { JsonObject& item = arr.createNestedObject();
    item[FPSTR(FP_NA)] = "Wifi established in (ms)";
    item[FPSTR(FP_VA)] = wifi_connect_time; }

  { JsonObject& item = arr.createNestedObject();
    item[FPSTR(FP_NA)] = "Wifi Autoconnect";
    item[FPSTR(FP_VA)] = wifi_station_get_auto_connect(); }

  // Flash Stuff
  // ===========
  String FlashChipRealSize = formatSize(ESP.getFlashChipRealSize());
  { JsonObject& item = arr.createNestedObject();
    item[FPSTR(FP_NA)] = "Flash Real Size";
    item[FPSTR(FP_VA)] = FlashChipRealSize ; }

  { JsonObject& item = arr.createNestedObject();
    item[FPSTR(FP_NA)] = "Flash IDE Speed";
    item[FPSTR(FP_VA)] = ESP.getFlashChipSpeed()/1000000 ; }

  char ide_mode[8];
  FlashMode_t im = ESP.getFlashChipMode();
  sprintf_P(ide_mode, PSTR("%s"), im==FM_QIO?"QIO":im==FM_QOUT?"QOUT":im==FM_DIO?"DIO":im==FM_DOUT?"DOUT":"UNKNOWN" );
  { JsonObject& item = arr.createNestedObject();
    item[FPSTR(FP_NA)] = "Flash IDE Mode";
    item[FPSTR(FP_VA)] = ide_mode; }

  { JsonObject& item = arr.createNestedObject();
    item[FPSTR(FP_NA)] = "Firmware Size";
    item[FPSTR(FP_VA)] = formatSize(ESP.getSketchSize()); }

  { JsonObject& item = arr.createNestedObject();
    item[FPSTR(FP_NA)] = "Free Size";
    item[FPSTR(FP_VA)] = formatSize(ESP.getFreeSketchSpace()); }

  // SPIFFS Informations
  // ===================
  FSInfo info;
  SPIFFS.info(info);

  { JsonObject& item = arr.createNestedObject();
    item[FPSTR(FP_NA)] = "SPIFFS Total";
    item[FPSTR(FP_VA)] =  formatSize(info.totalBytes); }

  { JsonObject& item = arr.createNestedObject();
    item[FPSTR(FP_NA)] = "SPIFFS Used";
    item[FPSTR(FP_VA)] = formatSize(info.totalBytes); }

  { JsonObject& item = arr.createNestedObject();
    item[FPSTR(FP_NA)] = "SPIFFS Occupation (%)";
    item[FPSTR(FP_VA)] = 100*info.usedBytes/info.totalBytes; }

  // regarder l'état de tous les fils Pilotes
  String stateFp = "";
  for (uint8_t i=1; i<=NB_FILS_PILOTES; i++)
  {
    fp = etatFP[i-1];
    stateFp = "";
    if      (fp=='E') stateFp += "Eco";
    else if (fp=='A') stateFp += "Arrêt";
    else if (fp=='H') stateFp += "Hors Gel";
    else if (fp=='1') stateFp += "Eco - 1";
    else if (fp=='2') stateFp += "Eco - 2";
    else if (fp=='C') stateFp += "Confort";
    { JsonObject& item = arr.createNestedObject();
    item[FPSTR(FP_NA)] = "Fil Pilote #" + String(i);
    item[FPSTR(FP_VA)] = stateFp; }
  }

  { JsonObject& item = arr.createNestedObject();
    item[FPSTR(FP_NA)] = "Etat Relais";
    item[FPSTR(FP_VA)] = etatrelais ? "Fermé":"Ouvert"; }

  { JsonObject& item = arr.createNestedObject();
    item[FPSTR(FP_NA)] = "Delestage";
    item[FPSTR(FP_VA)] = String(myDelestLimit) + "A"; }

  { JsonObject& item = arr.createNestedObject();
    item[FPSTR(FP_NA)] = "Relestage";
    item[FPSTR(FP_VA)] = String(myRelestLimit) + "A"; }

  { JsonObject& item = arr.createNestedObject();
    item[FPSTR(FP_NA)] = "Etat Delestage";
    item[FPSTR(FP_VA)] = String(nivDelest) + " Zone " + String(plusAncienneZoneDelestee); }

  // Web request send response to client
  size_t jsonlen ;
  if (request) {
    jsonlen = response->setLength();
    request->send(response);
  } else {
    // Send JSon to our string
    arr.printTo(JsonStr);
    jsonlen =  arr.measureLength();
    // Since it's nor a WEB request, we need to manually delete
    // response object so ArduinJSon object is freed
    delete response;
  }
  //Debugf("Json size %lu bytes\r\n", jsonlen);

  // Will be empty for web request
  return JsonStr;
}

/* ======================================================================
Function: confJSONTable
Purpose : dump all config values in JSON table format for browser
Input   : request pointer if comming from web request
Output  : -
Comments: -
====================================================================== */
void confJSONTable(AsyncWebServerRequest * request) {
  IPAddress ip_addr;
  size_t l;
  String str;

  AsyncJsonResponse * response = new AsyncJsonResponse();
  JsonObject& root = response->getRoot();

  DebugF("Serving /config page...");

  root[FPSTR(CFG_SSID)]    = config.ssid;
  root[FPSTR(CFG_PSK)]     = config.psk;
  root[FPSTR(CFG_HOST)]    = config.host ;
  root[FPSTR(CFG_AP_PSK)]  = config.ap_psk ;
  root[FPSTR(CFG_AP_SSID)] = config.ap_ssid ;

  ip_addr=config.ip;   root[FPSTR(CFG_IP)]      = ip_addr.toString();
  ip_addr=config.mask; root[FPSTR(CFG_MASK)]    = ip_addr.toString();
  ip_addr=config.gw;   root[FPSTR(CFG_GATEWAY)] = ip_addr.toString();
  ip_addr=config.dns;  root[FPSTR(CFG_DNS)]     = ip_addr.toString();

  root[FPSTR(CFG_EMON_HOST)] = config.emoncms.host;
  root[FPSTR(CFG_EMON_PORT)] = config.emoncms.port;
  root[FPSTR(CFG_EMON_URL)]  = config.emoncms.url;
  root[FPSTR(CFG_EMON_KEY)]  = config.emoncms.apikey;
  root[FPSTR(CFG_EMON_NODE)] = config.emoncms.node;
  root[FPSTR(CFG_EMON_FREQ)] = config.emoncms.freq;

  root[FPSTR(CFG_JDOM_HOST)] = config.jeedom.host;
  root[FPSTR(CFG_JDOM_PORT)] = config.jeedom.port;
  root[FPSTR(CFG_JDOM_URL)]  = config.jeedom.url;
  root[FPSTR(CFG_JDOM_KEY)]  = config.jeedom.apikey;
  root[FPSTR(CFG_JDOM_ADCO)] = config.jeedom.adco;
  root[FPSTR(CFG_JDOM_FREQ)] = config.jeedom.freq;

  size_t jsonlen ;
  jsonlen = response->setLength();
  request->send(response);
  //Debugf("Json size %lu bytes\r\n", jsonlen);
}

/* ======================================================================
Function: spiffsJSONTable
Purpose : dump all spiffs system in JSON table format for browser
Input   : request pointer if comming from web request
Output  : -
Comments: -
====================================================================== */
void spiffsJSONTable(AsyncWebServerRequest * request) {
  AsyncJsonResponse * response = new AsyncJsonResponse(false);
  JsonObject& root = response->getRoot();

  DebugF("Serving /spiffs page...");

  // Loop trough all files
  JsonArray& a_files = root.createNestedArray("files");
  Dir dir = SPIFFS.openDir("/");
  while (dir.next()) {
    JsonObject& o_item = a_files.createNestedObject();
    o_item[FPSTR(FP_NA)] = dir.fileName();
    o_item[FPSTR(FP_VA)] = dir.fileSize();
  }

  // Get SPIFFS File system informations
  FSInfo info;
  SPIFFS.info(info);
  JsonArray& a_spiffs = root.createNestedArray("spiffs");
  JsonObject& o_item = a_spiffs.createNestedObject();
  o_item["Total"] = info.totalBytes;
  o_item["Used"]  = info.usedBytes ;
  o_item["ram"]   = system_get_free_heap_size();

  size_t jsonlen ;
  jsonlen = response->setLength();
  request->send(response);
  //Debugf("Json size %lu bytes\r\n", jsonlen);

/* ======================================================================
Function: wifiScanJSON
Purpose : scan Wifi Access Point and return JSON code
Input   : request pointer if comming from web request
Output  : -
Comments: -
====================================================================== */
void wifiScanJSON(AsyncWebServerRequest * request) {
  String buff;

  AsyncJsonResponse * response = new AsyncJsonResponse(true);
  JsonArray& arr = response->getRoot();

  // Just to debug where we are
  Debug(F("Serving /wifiscan page..."));

  int n = WiFi.scanNetworks();

  // Just to debug where we are
  Debugf("found %d networks!",n);

  for (int i = 0; i < n; ++i) {

    switch(WiFi.encryptionType(i)) {
      case ENC_TYPE_NONE: buff = "Open";  break;
      case ENC_TYPE_WEP:  buff = "WEP";   break;
      case ENC_TYPE_TKIP: buff = "WPA";   break;
      case ENC_TYPE_CCMP: buff = "WPA2";  break;
      case ENC_TYPE_AUTO: buff = "Auto";  break;
      default:            buff = "????";  break;
    }

    Debugf("[%d] '%s' Encryption=%s Channel=%d\r\n", i, WiFi.SSID(i).c_str(), buff.c_str(), WiFi.channel(i));

    JsonObject& item = arr.createNestedObject();
    item[FPSTR(FP_SSID)]       = WiFi.SSID(i);
    item[FPSTR(FP_RSSI)]       = WiFi.RSSI(i);
    item[FPSTR(FP_ENCRYPTION)] = buff;
    item[FPSTR(FP_CHANNEL)]    = WiFi.channel(i);
  }

  Debugf("%d bytes\r\n", arr.measureLength());
  response->setLength();
  request->send(response);
}

/* ======================================================================
Function: tinfoJSON
Purpose : dump all teleinfo values in JSON
Input   : -
Output  : -
Comments: -
====================================================================== */
void tinfoJSON(AsyncWebServerRequest * request) {
  #ifdef MOD_TELEINFO
    AsyncJsonResponse * response = new AsyncJsonResponse(true);
    JsonArray& arr = response->getRoot();
    ValueList * me = tinfo.getList();

    // Got at least one ?
    if (me) {
      char * p;
      long value;

      // Json start
      // response += FPSTR(FP_JSON_START);
      // response += F("\"_UPTIME\":");
      // response += uptime;
      // response += FPSTR(FP_NL) ;

      // Loop thru the node
      while (me->next) {
        // go to next node
        me = me->next;

        //formatNumberJSON(response, me->value);
        { JsonObject& item = arr.createNestedObject();
          item[FPSTR(FP_NA)] = me->name;
          item[FPSTR(FP_VA)] = me->value;
          item[FPSTR(FP_CK)] = (char) me->checksum;
          item[FPSTR(FP_FL)] = me->flags;
        }
      }
      request->send(response);
    } else {
      request->send(404, "text/plain", "No data");
    }
    request->send(200, "text/json", response);
  #else
    request->send(404, "text/plain", "teleinfo not enabled");
  #endif
}


/* ======================================================================
Function: fpJSON
Purpose : return fp values in JSON
Input   : string
          fp number 1 to NB_FILS_PILOTE (0=ALL)
Output  : -
Comments: -
====================================================================== */
void fpJSON(AsyncJsonResponse & response, uint8_t fp) {
  JsonObject& item = response->getRoot();
  // petite verif
  if (fp >= 0 && fp <= NB_FILS_PILOTES) {
    //response = FPSTR(FP_JSON_START);

    // regarder l'état de tous les fils Pilotes
    for (uint8_t i=1; i<=NB_FILS_PILOTES; i++)
    {
      // Tout les fils pilote ou juste celui demandé
      if (fp==0 || fp==i) {
        item["fp" + String(i)] = etatFP[i-1];
      }
    }
  }
}


/* ======================================================================
Function: relaisJSON
Purpose : return relais value in JSON
Input   : -
Output  : -
Comments: -
====================================================================== */
void relaisJSON(AsyncWebServerRequest * request) {
  AsyncJsonResponse * response = new AsyncJsonResponse(true);
  JsonObject& item = response->getRoot();

  item[FPSTR(FP_RESP)] = String(etatrelais);

  Debugf("%d bytes\r\n", item.measureLength());
  response->setLength();
  request->send(response);
}

/* ======================================================================
Function: delestageJSON
Purpose : return delestage values in JSON
Input   : -
Output  : -
Comments: -
====================================================================== */
void delestageJSON(AsyncWebServerRequest * request) {
  AsyncJsonResponse * response = new AsyncJsonResponse(true);
  JsonObject& item = response->getRoot();

  item["niveau"] = String(nivDelest);
  item["zone"] = String(plusAncienneZoneDelestee);

  Debugf("%d bytes\r\n", item.measureLength());
  response->setLength();
  request->send(response);
}


/* ======================================================================
Function: handleFactoryReset
Purpose : reset the module to factory settingd
Input   : -
Output  : -
Comments: -
====================================================================== */
void handleFactoryReset(AsyncWebServerRequest * request) {
  // Just to debug where we are
  DebugF("Serving /factory_reset page...");
  request->send(200, "text/plain", FPSTR(FP_RESTART));
  ESP.eraseConfig();
  resetConfig();
  delay(1000);
  ESP.restart();
  while (true)
    delay(1);
}

/* ======================================================================
Function: handleReset
Purpose : reset the module
Input   : -
Output  : -
Comments: -
====================================================================== */
void handleReset(AsyncWebServerRequest * request) {
  // Just to debug where we are
  DebugF("Serving /reset page...");
  request->send(200, "text/plain", FPSTR(FP_RESTART));
  delay(1000);
  ESP.restart();
  // This will fire watchdog
  while (true)
    delay(1);
}

/* ======================================================================
Function: handleFormConfig
Purpose : handle main configuration page
Input   : -
Output  : -
Comments: -
====================================================================== */
void handleFormConfig(AsyncWebServerRequest *request) {
  String response="";
  int ret ;
  boolean showconfig = false;

  // We validated config ?
  if (request->hasArg("save")) {
    uint8_t params = request->params();
    int itemp, l;
    char buff[CFG_SERIAL_BUFFER_SIZE+2]; // Assume Max Len buffer for Arg + = + value
    DebuglnF("===== Posted configuration");
    Debugflush();

    // Since Checkbox unchecked are not send (we receive only the one checked as cfg_box=on)
    // We clear all related options of them, if checked we will be receive it
    config.config &= ~(CFG_LCD|CFG_DEBUG|CFG_RGB_LED|CFG_AP|CFG_STATIC|CFG_WIFI);

    // same thing for field that can be empty, we do not receive them and want to
    // be able to clear them
    *config.psk='\0';
    *config.ap_psk='\0';
    *config.ota_auth='\0';
    *config.emoncms.host='\0';
    *config.emoncms.apikey='\0';
    *config.emoncms.url='\0';

    *config.jeedom.host='\0';
    *config.jeedom.apikey='\0';
    *config.jeedom.url='\0';
    *config.jeedom.adco='\0';

    config.ip = 0;
    config.mask = 0;
    config.gw = 0;
    config.dns = 0;


    // Navigate for all args, and simulate as it was typed from command line
    for ( uint8_t i = 0; i < params; i++ ) {
      AsyncWebParameter* param = request->getParam(i);
      // calc total size + ' ' + '\0'
      l = 2 + param->name().length() + param->value().length();
      // fit in buffer and not save command
      if (l < CFG_SERIAL_BUFFER_SIZE && param->name() != "save") {
        snprintf(buff, l, "%s %s", param->name().c_str(), param->value().c_str());
        Debugf("[%02d] %s\r\n", i, buff);
        Debugflush();
        execCmd(buff);
      }
    }

    // now we can save the config
    if ( saveConfig() ) {
      ret = 200;
      response = PSTR("OK");
    } else {
      ret = 412;
      response = PSTR("Error while saving configuration");
    }

  } else  {
    ret = 400;
    response = PSTR("Missing Form Field");
  }

  DebugF("Sending ");
  Debug(ret);
  Debug(":");
  Debugln(response);
  request->send ( ret, "text/plain", response);

  // This is slow, do it after response sent
  if (showconfig) {
    showConfig();
  }
}

/* ======================================================================
Function: handleNotFound
Purpose : default WEB routing when URI is not found
Input   : -
Output  : -
Comments: We search is we have a name that match to this URI, if one we
          return it's pair name/value in json
====================================================================== */
void handleNotFound(AsyncWebServerRequest * request) {
  AsyncJsonResponse * response = new AsyncJsonResponse(true);
  JsonObject& item = response->getRoot();
  //String response = "";

  String sUri = request->url();
  const char * uri;
  bool found = false;
  bool first_elem = true;

  // convert uri to char * for compare
  uri = sUri.c_str();

  DebugF("URI[");
  Debug(strlen(uri));
  DebugF("]='");
  Debug(uri);
  Debugln("'");

  // Got consistent URI, skip fisrt / ?
  // Attention si ? dans l'URL çà ne fait pas partie de l'URI
  // mais de hasArg traité plus bas
  if (uri && *uri=='/' && *++uri ) {
    uint8_t len = strlen(uri);

    #ifdef MOD_TELEINFO
      // We check for an known label
      ValueList * me = tinfo.getList();

      // Got at least one ?
      if (me) {

        // Loop thru the linked list of values
        while (me->next && !found) {
          // go to next node
          me = me->next;

          //Debugf("compare to '%s' ", me->name);
          // Do we have this one ?
          if (!stricmp(me->name, uri)) {
            // no need to continue
            found = true;

            // Add to respone
            item[me->name] = me->value;
            // response += FPSTR("{\r\n\"") ;
            // response += me->name ;
            // response += F("\":") ;
            // formatNumberJSON(response, me->value);
            // response += FPSTR(FP_JSON_END);
          }
        }
      }
    #endif

    // Requêtes d'interrogation
    // ========================

    // http://ip_remora/relais
    if (!found && (len == 2 || len == 3) && (uri[0]=='f' || uri[0]=='F') && (uri[1] == 'p'||uri[1] == 'P')) {
      int8_t fp = -1;

      // http://ip_remora/fp
      if (len == 2) {
        fp = 0;

      // http://ip_remora/fpx
      } else if (len == 3) {
        fp = uri[2];
        if (fp >= '1' && fp <= ('0' + NB_FILS_PILOTES)) {
          fp -= '0';
        }
      }

      if (fp >= 0 && fp <= NB_FILS_PILOTES) {
        fpJSON(response, fp);
        found = true;
      }
    }
  } // valide URI


  // Requêtes modifiantes (cumulable)
  // ================================
  if (!found &&
      (request->hasArg("fp") ||
      request->hasArg("setfp") ||
      request->hasArg("relais"))) {

    int error = 0;
    //response = FPSTR(FP_JSON_START);

    // http://ip_remora/?setfp=CMD
    if (request->hasArg("setfp")) {
      String value = request->arg("setfp");
      error += setfp(value);
    }
    // http://ip_remora/?fp=CMD
    if (request->hasArg("fp")) {
      String value = request->arg("fp");
      error += fp(value);
    }

    // http://ip_remora/?relais=n
    if (request->hasArg("relais")) {
      String value = request->arg("relais");
      // La nouvelle valeur n'est pas celle qu'on vient de positionner ?
      if ( relais(value) != request->arg("relais").toInt() )
        error--;
    }

    item["response"] = String(error);
    found = true;
  }

  // Got it, send json
  if (found) {
    Debugf("%d bytes\r\n", item.measureLength());
    response->setLength();
    request->send(response);
  } else {
    // le fichier demandé existe sur le système SPIFFS ?
    found = handleFileRead(request->url());
  }

  // send error message in plain text
  if (!found) {
    AsyncResponseStream *resHtml = request->beginResponseStream("text/html");
    resHtml->printf("<!DOCTYPE html><html><head><title>Webpage at %s</title></head><body>", request->url().c_str());

    resHtml->print("<h2>Sorry ");
    resHtml->print(request->client()->remoteIP());
    resHtml->print("We're unable to process this page</h2>");

    resHtml->print("<h3>Information</h3>");
    resHtml->print("<ul>");
    resHtml->printf("<li>Version: HTTP/1.%u</li>", request->version());
    resHtml->printf("<li>Method: %s</li>", request->methodToString());
    resHtml->printf("<li>URL: %s</li>", request->url().c_str());
    resHtml->printf("<li>Host: %s</li>", request->host().c_str());
    resHtml->printf("<li>ContentType: %s</li>", request->contentType().c_str());
    resHtml->printf("<li>ContentLength: %u</li>", request->contentLength());
    resHtml->printf("<li>Multipart: %s</li>", request->multipart()?"true":"false");
    resHtml->print("</ul>");

    resHtml->print("<h3>Headers</h3>");
    resHtml->print("<ul>");
    int headers = request->headers();
    for (int i=0;i<headers;i++) {
      AsyncWebHeader* h = request->getHeader(i);
      resHtml->printf("<li>%s: %s</li>", h->name().c_str(), h->value().c_str());
    }
    resHtml->print("</ul>");

    resHtml->print("<h3>Parameters</h3>");
    resHtml->print("<ul>");
    int params = request->params();
    for(int i=0;i<params;i++){
      AsyncWebParameter* p = request->getParam(i);
      if( p->isFile() ) {
        resHtml->printf("<li>FILE[%s]: %s, size: %u</li>", p->name().c_str(), p->value().c_str(), p->size());
      } else if ( p->isPost() ) {
        resHtml->printf("<li>POST[%s]: %s</li>", p->name().c_str(), p->value().c_str());
      } else {
        resHtml->printf("<li>GET[%s]: %s</li>", p->name().c_str(), p->value().c_str());
      }
    }
    resHtml->print("</ul>");

    resHtml->print("</body></html>");
    //send the response last
    request->send(resHtml);
  }

}
#endif