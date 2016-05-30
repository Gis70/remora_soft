// **********************************************************************************
// Remora Configuration source file
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

#include "./config.h" 

#ifdef ESP8266

// Configuration structure for whole program
_Config config;

uint8_t ser_idx = 0;
char    ser_buf[CFG_SERIAL_BUFFER_SIZE + 1];

uint16_t crc16Update(uint16_t crc, uint8_t a)
{
  int i;
  crc ^= a;
  for (i = 0; i < 8; ++i)  {
    if (crc & 1)
      crc = (crc >> 1) ^ 0xA001;
    else
      crc = (crc >> 1);
  }
  return crc;
}

/* ======================================================================
Function: outputBuffer
Purpose : dump buffer to Debug/Serial or on websocket
Input   : buffer
          websocket client ID, 0 if no websoket
Output  : -
Comments: -
====================================================================== */
void outputBuffer(char * buff, uint32_t clientid) 
{
  if (clientid) 
    // Send via WebSocket
    ws.text(clientid, buff, strlen(buff));
  else
    // Send to Serial
    Debug(buff);
}

/* ======================================================================
Function: eeprom_dump
Purpose : dump eeprom value to serial 
Input 	: -
Output	: -
Comments: -
====================================================================== */
void eepromDump(uint8_t bytesPerRow) 
{
  uint16_t i,b;
  char buf[10];
  uint16_t j=0 ;
  
  // default to 16 bytes per row
  if (bytesPerRow==0)
    bytesPerRow=16;

  Debugln();
    
  // loop thru EEP address
  for (i = 0; i < sizeof(_Config); i++) {
    // First byte of the row ?
    if (j==0) {
			// Display Address
      Debugf("%04X : ", i);
    }

    // write byte in hex form
    Debugf("%02X ", EEPROM.read(i));

		// Last byte of the row ?
    // start a new line
    if (++j >= bytesPerRow) {
			j=0;
      Debugln();
		}
  }
}

/* ======================================================================
Function: readConfig
Purpose : fill config structure with data located into eeprom
Input 	: true if we need to clear actual struc in case of error
Output	: true if config found and crc ok, false otherwise
Comments: -
====================================================================== */
bool readConfig (bool clear_on_error) 
{
	uint16_t crc = ~0;
	uint8_t * pconfig = (uint8_t *) &config ;
	uint8_t data ;

	// For whole size of config structure
	for (uint16_t i = 0; i < sizeof(_Config); ++i) {
		// read data
		data = EEPROM.read(i);
		
		// save into struct
		*pconfig++ = data ;
		
		// calc CRC
		crc = crc16Update(crc, data);
	}
	
	// CRC Error ?
	if (crc != 0) {
		// Clear config if wanted
    if (clear_on_error)
		  memset(&config, 0, sizeof( _Config ));
		return false;
	}
	
	return true ;
}

/* ======================================================================
Function: saveConfig
Purpose : save config structure values into eeprom
Input 	: -
Output	: true if saved and readback ok
Comments: once saved, config is read again to check the CRC
====================================================================== */
bool saveConfig (void) 
{
  uint8_t * pconfig ;
  bool ret_code;

  //eepromDump(32);

  // Init pointer 
  pconfig = (uint8_t *) &config ;
	
	// Init CRC
  config.crc = ~0;

	// For whole size of config structure, pre-calculate CRC
  for (uint16_t i = 0; i < sizeof (_Config) - 2; ++i)
    config.crc = crc16Update(config.crc, *pconfig++);

	// Re init pointer 
  pconfig = (uint8_t *) &config ;

  // For whole size of config structure, write to EEP
  for (uint16_t i = 0; i < sizeof(_Config); ++i) 
    EEPROM.write(i, *pconfig++);

  // Physically save
  EEPROM.commit();
  
  // Read Again to see if saved ok, but do 
  // not clear if error this avoid clearing
  // default config and breaks OTA
  ret_code = readConfig(false);
  
  DebugF("Write config ");
  
  if (ret_code) {
    Debugln(F("OK!"));
  } else {
    Debugln(F("Error!"));
  }

  //eepromDump(32);
  
  // return result
  return (ret_code);
}

/* ======================================================================
Function: showConfig
Purpose : display configuration
Input 	: -
Output	: -
Comments: -
====================================================================== */
void showConfig(uint16_t section = CFG_HLP_ALL, uint32_t clientid = 0 ) 
{
  char buff[256]="";
  #define OUT(x,y) { sprintf_P(buff+strlen(buff),PSTR(x),y); }

  if (section & CFG_HLP_SYS) {
    strcat_P(buff, PSTR("Config   : ")); 
    if(config.config&CFG_AP)      strcat_P(buff, PSTR("ACCESS_POINT "));
    if(config.config&CFG_WIFI)    strcat_P(buff, PSTR("WIFI "));
    if(config.config&CFG_STATIC)  strcat_P(buff, PSTR("STATIC "));
    if(config.config&CFG_RGB_LED) strcat_P(buff, PSTR("RGBLED "));
    if(config.config&CFG_DEBUG)   strcat_P(buff, PSTR("DEBUG "));
    if(config.config&CFG_LCD)     strcat_P(buff, PSTR("OLED "));
    if(config.config&CFG_SI7021)  strcat_P(buff, PSTR("SI7021 "));
    if(config.config&CFG_SHT10)   strcat_P(buff, PSTR("SHT10 "));

    const char* ledtype[] = { "None", "RGB", "GRB", "RGBW", "GRBW" };
    sprintf_P(buff+strlen(buff), PSTR("\nRGB LED  : #%d GPIO%d %s  Brigth %d %%  Heartbeat %d sec\n"), 
                    config.led_num, config.led_gpio,
                    ledtype[(uint8_t)config.led_type],
                    config.led_bright, config.led_hb/10); 

    // Send to correct client output 
    outputBuffer(buff, clientid);
  }

  if (section & CFG_HLP_WIFI) {
    strcpy_P(buff, PSTR("\r\n===== Wifi\r\n")); 
    OUT("SSID     : %s\n", config.ssid);
    OUT("psk      : %s\n", config.psk);
    OUT("host     : %s\n", config.host);
    OUT("ap SSID  : %s\n", config.ap_ssid);
    OUT("ap psk   : %s\n", config.ap_psk);
    OUT("Static IP: %s",  ((IPAddress) config.ip).toString().c_str());
    OUT("/%s  ",          ((IPAddress) config.mask).toString().c_str());
    OUT("GW:%s  ",        ((IPAddress) config.gw).toString().c_str());
    OUT("DNS:%s\n",       ((IPAddress) config.dns).toString().c_str());

    // Send to correct client output 
    outputBuffer(buff, clientid);
 }

  if (section & CFG_HLP_DATA) {
    strcpy_P(buff, PSTR("\r\n===== Data Server\n")); 
    OUT("host : %s\n", config.emoncms.host); 
    OUT("port : %d\n", config.emoncms.port); 
    OUT("url  : %s\n", config.emoncms.url); 
    OUT("key  : %s\n", config.emoncms.apikey); 
    OUT("node : %d\n", config.emoncms.node); 
    OUT("freq : %d\n", config.emoncms.freq); 

    // Send to correct client output 
    outputBuffer(buff, clientid);
  }

  if (section & CFG_HLP_JEEDOM) {
    strcpy_P(buff, PSTR("\r\n===== Jeedom Server\n")); 
    OUT("host : %s\n", config.jeedom.host); 
    OUT("port : %d\n", config.jeedom.port); 
    OUT("url  : %s\n", config.jeedom.url); 
    OUT("key  : %s\n", config.jeedom.apikey); 
    OUT("adco : %d\n", config.jeedom.adco); 
    OUT("freq : %d\n", config.jeedom.freq); 

    // Send to correct client output 
    outputBuffer(buff, clientid);
  }
}

/* ======================================================================
Function: showHelp
Purpose : display help
Input   : bits flag of section to show
          WebSocket client ID if want to send to WS
Output  : -
Comments: -
====================================================================== */
void showHelp(uint16_t section, uint32_t clientid) 
{
  if (clientid) {
    ws.text(clientid, HELP_HELP);
    if (section & CFG_HLP_SYS)      ws.text(clientid, HELP_SYS);
    if (section & CFG_HLP_WIFI)     ws.text(clientid, HELP_WIFI);
    if (section & CFG_HLP_DATA)     ws.text(clientid, HELP_DATA);
    if (section & CFG_HLP_JEEDOM)   ws.text(clientid, HELP_JEEDOM);
  } else {
    Debug(FPSTR(HELP_HELP));
    if (section & CFG_HLP_SYS)      Debug(FPSTR(HELP_SYS));
    if (section & CFG_HLP_WIFI)     Debug(FPSTR(HELP_WIFI));
    if (section & CFG_HLP_DATA)     Debug(FPSTR(HELP_DATA));
    if (section & CFG_HLP_JEEDOM)   Debug(FPSTR(HELP_JEEDOM));
  }
}

void catParam(char * dest, uint8_t maxsize, char * par1, char * par2 , char * par3 )
{
  if (par1) {
    strncpy(dest, par1, maxsize);

    if (par2) {
      maxsize = maxsize - strlen(dest) - 1;
      strcat(dest, "_");
      strncat(dest, par2, maxsize);

      if (par3) {
        maxsize = maxsize - strlen(par2) - 1;
        strcat(dest, "_");
        strncat(dest, par3, maxsize);
      }
    }
  // No param, clear field
  } else {
    strcpy(dest, "");
  }

}

/* ======================================================================
Function: ResetConfig
Purpose : Set configuration to default values
Input   : -
Output  : -
Comments: -
====================================================================== */
void resetConfig(void) 
{
  // enable default configuration
  memset(&config, 0, sizeof(_Config));

  // Set default Hostname
  sprintf_P(config.host, PSTR("Remora_%06X"), ESP.getChipId());
  strcpy_P(config.ota_auth, PSTR(DEFAULT_OTA_AUTH));
  config.ota_port = DEFAULT_OTA_PORT ;

  // Add other init default config here

  // Emoncms
  strcpy_P(config.emoncms.host, CFG_EMON_DEFAULT_HOST);
  config.emoncms.port = CFG_EMON_DEFAULT_PORT;
  strcpy_P(config.emoncms.url, CFG_EMON_DEFAULT_URL);
  config.emoncms.apikey[0] = '\0';
  config.emoncms.node = 0;
  config.emoncms.freq = 0;

  // Jeedom
  strcpy_P(config.jeedom.host, CFG_JDOM_DEFAULT_HOST);
  config.jeedom.port = CFG_JDOM_DEFAULT_PORT;
  strcpy_P(config.jeedom.url, CFG_JDOM_DEFAULT_URL);
  strcpy_P(config.jeedom.adco, CFG_JDOM_DEFAULT_ADCO);
  config.jeedom.apikey[0] = '\0';
  config.jeedom.freq = 0;


  config.config |= CFG_RGB_LED;

  // save back
  saveConfig();
}

/* ======================================================================
Function: resetBoard
Purpose : do a board reset
Input   : -
Output  : -
Comments: -
====================================================================== */
void resetBoard(uint32_t clientid) {
  //webSocket.disconnect();
  //delay(100);
  //server.close();
  //DNS.stop();

  // Default boot pin mode, be sure to avoid potential lockup at boot
  pinMode(0, INPUT);
  pinMode(2, INPUT);
  pinMode(15, INPUT);
  ESP.reset();

  // Should never arrive there
  while (true);
}

/* ======================================================================
Function: execCmd
Purpose : execute a configuration command
Input   : command line
Output  : -
Comments: command can be sent by serial, websocket or by form post 
====================================================================== */
void execCmd(char *line, uint32_t clientid)
{
  char sep[4] = " _,"; // Parse with space, underscore and coma
  char * cmd = NULL;
  char * par1= NULL;
  char * par2= NULL;
  char * par3= NULL;
  char * par4= NULL;

  // get the command and params
  if ( (cmd=strtok(line,sep)) != NULL ) {
    if ( (par1=strtok(NULL,sep)) != NULL ) {
      if ( (par2=strtok(NULL,sep)) != NULL ) {
        if ( (par3=strtok(NULL,sep)) != NULL ) {
          par4=strtok(NULL,sep);
        }
      }
    }
  }

  Debugf(">'%s','%s','%s','%s','%s'\r\n", cmd, par1, par2, par3, par4);

  // Error ? show help
  if (!cmd || !par1) {
    showHelp(CFG_HLP_HELP, clientid);
  } else {
    IPAddress ip_addr;

    // Show command
    if (!strcasecmp_P(cmd, PSTR("show"))) {

      // Show config or show help
      if ( !strcasecmp_P(par1, PSTR("config")) || !strcasecmp_P(par1, PSTR("help")) ) {
        uint16_t cfg_hlp = CFG_HLP_ALL;
        if (par2) {
          if (!strcasecmp_P(par2, PSTR("sys"))) {
            cfg_hlp = CFG_HLP_SYS ;
          } else if (!strcasecmp_P(par2, PSTR("wifi"))) {
            cfg_hlp = CFG_HLP_WIFI;
          } else if (!strcasecmp_P(par2, PSTR("data"))) {
            cfg_hlp = CFG_HLP_DATA;
          } else if (!strcasecmp_P(par2, PSTR("sens"))) {
            cfg_hlp = CFG_HLP_SENSOR;
          } else if (!strcasecmp_P(par2, PSTR("jdom"))) {
            cfg_hlp = CFG_HLP_JEEDOM;
          } else if (!strcasecmp_P(par2, PSTR("domz"))) {
            cfg_hlp = CFG_HLP_DOMZ;
          } else if (!strcasecmp_P(par2, PSTR("cnt"))) {
            cfg_hlp = CFG_HLP_COUNTER;
          }
        } else {

        }
        
        if (!strcasecmp_P(par1, PSTR("config"))) {
          showConfig(cfg_hlp, clientid);
        } else {
          showHelp(cfg_hlp, clientid);
        }
      }

    // Save command
    } else if(!strcasecmp_P(cmd, CFG_SAVE) ) {
      saveConfig();

    // Reset command
    } else if (!strcasecmp_P(cmd, PSTR("reset")) ) {

      if (!strcasecmp_P(par1, PSTR("sdk"))) {
        ESP.eraseConfig(); // Delete SDK Config (Wifi Credentials)
      } else if (!strcasecmp_P(par1, PSTR("config"))) {
        resetConfig();
      } else if (!strcasecmp_P(par1, PSTR("board"))) {
        resetBoard();
      }

    // Wifi command
    } else if (!strcasecmp_P(cmd, CFG_SSID)) {
      catParam(config.ssid, CFG_SSID_SIZE, par1, par2, par3);
    } else if (!strcasecmp_P(cmd, CFG_PSK )) {
      catParam(config.psk, CFG_PSK_SIZE, par1, par2, par3);
    } else if (!strcasecmp_P(cmd, CFG_HOST)) {
      catParam(config.host, CFG_HOSTNAME_SIZE, par1, par2, par3);
    } else if (!strcasecmp_P(cmd, CFG_AP_PSK)) {
      catParam(config.ap_psk, CFG_PSK_SIZE, par1, par2, par3);
  
    } else if (!strcasecmp_P(cmd, CFG_AP_SSID)) {
      catParam(config.ap_ssid, CFG_SSID_SIZE, par1, par2, par3);
    } else if (!strcasecmp_P(cmd, CFG_IP) && ip_addr.fromString(par1) ) {
      config.ip = ip_addr;
    } else if (!strcasecmp_P(cmd, CFG_MASK) && ip_addr.fromString(par1) ) {
      config.mask = ip_addr;
    } else if (!strcasecmp_P(cmd, CFG_GATEWAY) && ip_addr.fromString(par1) ) {
      config.gw = ip_addr;
    } else if (!strcasecmp_P(cmd, CFG_DNS) && ip_addr.fromString(par1) ) {
      config.dns = ip_addr;
    }

    // We had a 2nd parameters?
    if ( par2 ) {
      unsigned long v=atol(par2);

      // ota command
      if (!strcasecmp_P(cmd, PSTR("ota")) ) {
        Debugf("Cmd='ota_','%s','%s'\r\n", par1, par2);
        if (!strcasecmp_P(par1, &CFG_OTA_AUTH[4])) {
          catParam(config.ota_auth, CFG_PSK_SIZE, par2, par3, par4);
        } else if (!strcasecmp_P(par1, &CFG_OTA_PORT[4] )) {
          config.ota_port = (v>=0 && v<=65535) ? v : DEFAULT_OTA_PORT;
        } 

      // emon command
      } else if (!strcasecmp_P(cmd, PSTR("emon")) ) {
        Debugf("Cmd='emon_','%s','%s'\r\n", par1, par2);
        if (!strcasecmp_P(par1, &CFG_EMON_HOST[5])) {
          catParam(config.emoncms.host, CFG_EMON_HOST_SIZE, par2, par3, par4);
        } else if (!strcasecmp_P(par1, &CFG_EMON_PORT[5] )) {
          config.emoncms.port = (v>=0 && v<=65535) ? v : CFG_EMON_DEFAULT_PORT ;
        } else if (!strcasecmp_P(par1, &CFG_EMON_URL[5])) {
          catParam(config.emoncms.url, CFG_EMON_URL_SIZE, par2, par3, par4);
        } else if (!strcasecmp_P(par1, &CFG_EMON_KEY[5])) {
          catParam(config.emoncms.apikey, CFG_EMON_APIKEY_SIZE, par2, par3, par4);
        } else if (!strcasecmp_P(par1, &CFG_EMON_NODE[5])) {
          config.emoncms.node = (v>=0 && v<=255) ? v : 0 ;
        } else if (!strcasecmp_P(par1, &CFG_EMON_FREQ[5])) {
          config.emoncms.freq = (v>0 && v<=86400) ? v : 0;
        }

      // jeedom command
      } else if (!strcasecmp_P(cmd, PSTR("jdom")) ) {
        Debugf("Cmd='jdom_','%s','%s'\r\n", par1, par2);
        if (!strcasecmp_P(par1, &CFG_JDOM_HOST[5])) {
          catParam(config.jeedom.host, CFG_JDOM_HOST_SIZE, par2, par3, par4);
        } else if (!strcasecmp_P(par1, &CFG_JDOM_PORT[5] )) {
          config.jeedom.port = (v>=0 && v<=65535) ? v : CFG_JDOM_DEFAULT_PORT ;
        } else if (!strcasecmp_P(par1, &CFG_JDOM_URL[5])) {
          catParam(config.jeedom.url, CFG_JDOM_URL_SIZE, par2, par3, par4);
        } else if (!strcasecmp_P(par1, &CFG_JDOM_KEY[5])) {
          catParam(config.jeedom.apikey, CFG_JDOM_APIKEY_SIZE, par2, par3, par4);
        } else if (!strcasecmp_P(par1, &CFG_JDOM_ADCO[5])) {
          catParam(config.jeedom.adco, CFG_ADCO_SIZE, par2, par3, par4);
        } else if (!strcasecmp_P(par1, &CFG_JDOM_FREQ[5])) {
          config.jeedom.freq = (v>0 && v<=86400) ? v : 0;
        }

      // cfg command
      } else  if (!strcasecmp_P(cmd, PSTR("cfg")) ) {
        uint32_t o_msk = 0x0000; // Future bits to set
        uint32_t a_msk = 0xFFFF; // Future bits to clear

        Debugf("Cmd='cfg_','%s','%s','%s'\r\n", par1, par2, par3);
      
        if (par2) {
          uint8_t val = !strcasecmp(par2,"on") ? 1:0;

          if (!strcasecmp_P(par1, &CFG_CFG_AP[4])) {
            if (val) o_msk |= CFG_AP; else a_msk &= ~CFG_AP;
          } else if (!strcasecmp_P(par1, &CFG_CFG_WIFI[4] )) {
            if (val) o_msk |= CFG_WIFI; else a_msk &= ~CFG_WIFI;
          } else if (!strcasecmp_P(par1, &CFG_CFG_RGBLED[4] )) {
            if (val) o_msk |= CFG_RGB_LED; else a_msk &= ~CFG_RGB_LED;
          } else if (!strcasecmp_P(par1, &CFG_CFG_DEBUG[4] )) {
            if (val) o_msk |= CFG_DEBUG; else a_msk &= ~CFG_DEBUG;
          } else if (!strcasecmp_P(par1, &CFG_CFG_OLED[4] )) {
            if (val) o_msk |= CFG_LCD; else a_msk &= ~CFG_LCD;
          } else if (!strcasecmp_P(par1, &CFG_CFG_STATIC[4] )) {
            if (val) o_msk |= CFG_STATIC; else a_msk &= ~CFG_STATIC;
          }

          config.config |= o_msk; // Set needed bits
          config.config &= a_msk; // clear needed bits
        } // par2

      }

    } // par2
  } // Cmd && Par1
}

/* ======================================================================
Function: handle_serial
Purpose : manage serial command typed on keyboard
Input   : command line if coming from web socket else NULL
          client ID if coming from web socket else 0
Output  : -
Comments: -
====================================================================== */
void handle_serial(char * line, uint32_t clientid)
{
  // received line from Web Sockets
  if (clientid) {
    execCmd(line, clientid);
  } else {
    // Real Serial
    char c;
    uint8_t nb_char=0;
    boolean reset_buf = false;

    // We take only 8 char per loop, the other 
    // will be taken next loop call, this avoid 
    // long blocking while for other tasks
    if (Serial.available()) {
      c = Serial.read();
      nb_char++;
      // Space in Buffer
      if ( ser_idx < CFG_SERIAL_BUFFER_SIZE && c!='\r') {
        if ( c=='\n') {  // End of command
          ser_buf[ser_idx] = '\0'; // end of buffer
          execCmd(ser_buf);
          reset_buf = true;
        } else { // Discard unknown char
          ser_buf[ser_idx++] = c ;
        }
      } else {
        // Too long discard and reset buffer
        reset_buf = true;
      }

      // discard and reset buffer ?
      if (reset_buf) {
        ser_idx = 0;
        memset(ser_buf, 0, sizeof(ser_buf)); // clear buffer
      }
    } // While char
  }
}
#endif // ESP8266
