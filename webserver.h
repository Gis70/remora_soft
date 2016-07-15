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

#ifndef ROUTE_H
#define ROUTE_H

// Include main project include file
#include "remora.h"
#include <string.h>

#ifndef stricmp
#define stricmp strcasecmp
#endif

// Exported variables/object instanciated in main sketch
// =====================================================
extern char response[];
extern uint16_t response_idx;

// declared exported function from webserver.cpp
// ===================================================
void handleFormConfig(AsyncWebServerRequest * request);
void handleNotFound(AsyncWebServerRequest * request);
String tinfoJSONTable(AsyncWebServerRequest * request);
String sysJSONTable(AsyncWebServerRequest * request);
void confJSONTable(AsyncWebServerRequest * request);
void spiffsJSONTable(AsyncWebServerRequest * request);
void fpReqJSON(AsyncWebServerRequest * request);
String fpJSON(AsyncWebServerRequest * request, int8_t fp = -1);
String relaisJSON(AsyncWebServerRequest * request);
String delestageJSON(AsyncWebServerRequest * request);
String setParams(AsyncWebServerRequest * request);
String wifiScanJSON(AsyncWebServerRequest * request);
void handleFactoryReset(AsyncWebServerRequest * request);
void handleReset(AsyncWebServerRequest * request);

#endif
