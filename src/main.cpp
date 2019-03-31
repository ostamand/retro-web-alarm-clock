#include <Arduino.h>

// wifi
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>

#include <Wire.h>
#include "config.h"

// ota
#include <ArduinoOTA.h>
#include <ESP8266mDNS.h>

// common
#include "O1SA_Arduino_Common.h"

// ds1307 alarm clock
#include "DS1307_AlarmClock.h"
#include "AlarmClockInfo.h"
#include "ClockTime.h"
#include "ClockDateTime.h"

// vs1053
#include "SPI.h"
#include "SD.h"
#include "Adafruit_VS1053.h"

// led backpack
#include "Adafruit_GFX.h"
#include "Adafruit_LEDBackpack.h"

// adafruit io
#include "AdafruitIO_WiFi.h"

// esp8266 sdk
extern "C"{
  #include <user_interface.h>
}

AdafruitIO_WiFi io(ADAFRUIT_IO_USERNAME, ADAFRUIT_IO_KEY, WIFI_SSID,
                   WIFI_PASSWORD);

AdafruitIO_Feed * alarm1Feed = io.feed(ALARM1_FEED);

AdafruitIO_Feed * infoFeed = io.feed(INFO_FEED);

DS1307_AlarmClock alarmClock (DS1307_ADDRESS, SNOOZE_TIME_MIN, NUM_SNOOZE_MAX);

Adafruit_VS1053_FilePlayer musicPlayer =
  Adafruit_VS1053_FilePlayer(VS1053_RESET, VS1053_CS,
                             VS1053_DCS, VS1053_DREQ, CARDCS);

Adafruit_AlphaNum4 display = Adafruit_AlphaNum4();

// local variables
ClockDateTime   currentDisplayTime;
bool            playAlarm           =             false;
bool            isDisplayOn         =             false;
os_timer_t      clockTimer;
bool            isLongClick         =             false;
bool            isSingleClick       =             false;
bool            isClickPending		  =             false;
uint32_t 		    timeClick			      =				      0;			// microseconds
uint8_t         lastPushButtonState =             0;

// check if night for LCD display brightness setting
bool isNight(){
	bool isNight = false;
	uint8_t t = alarmClock.getCurrentTime() -> hour();
	// e.g. 19 -> 7
	if( t - CLOCK_NIGHT_START >= 0 || CLOCK_NIGHT_END - t >= 0 ){isNight = true;}
	return isNight;
}

// alarm#1 adafruit io callback
// 1. OFF							    Set alarm #1 off
// 2. HH:MM:(W/WKD/WWKD)	Set alarm #1 time (format thru MQTT feed)
// 3. HH h MM						  Set alarm #1 time (format thru google assistant)
// 4. DISPLAY_ON					Set time display on
// 5. DISPLAY_OFF					Set time display off
// 6. SHOW_CONFIG					Show config
// 7. UPD_TIME            Update time thru api
void handleAlarm1Message(AdafruitIO_Data *data){

  MAIN_DEBUG_PRINTF("Alarm#1 Adafruit.IO feed: %s\n", data -> value());

  uint8_t numSubStrings;
  int hh;
  int mm;
  char str[41];

  char * value = Common::removeWhiteSpaces((const char *)data -> value());
  char ** split = Common::splitBy(data -> value(), ":", &numSubStrings);

  // format: hh:mm:dd
  if(numSubStrings == 3){
    hh = atoi(split[0]);
    mm = atoi(split[1]);
    bool week = false;
    bool wkdn = false;

	  char * lbl = Common::removeWhiteSpaces((const char *)split[2]);

    if( strcmp(lbl,"W") == 0){week = true;}
    if( strcmp(lbl, "WKD") == 0){ wkdn = true; }
    if( strcmp(lbl, "WWKD") == 0){
      week = true;
      wkdn = true;
    }
    alarmClock.addOrUpdateAlarm(1,hh,mm,0,week,wkdn);

	  // free memory allocated by remove white spaces
	  delete lbl;

    // send feedback to adafruit io
    sprintf(str, "Alarm clock / Alarm #1 set to: %s",
                 alarmClock.getAlarmWithID(1) -> time().getPrintableString() );
    infoFeed->save(str);
  }
  else if(strcmp(value, "OFF" )==0){
	  // set alarm #1 to off
    alarmClock.addOrUpdateAlarm(1,0,0,0,false,false);

	  // send feedback to adafruit io
	  infoFeed->save((char *)"Alarm clock / Alarm #1 OFF");
  }
  else if(strcmp(value, "DISPLAY_ON")==0){
    // set time display on
	  isDisplayOn = true;

	  // send feedback to adafruit io
    infoFeed->save((char *)"Alarm clock / Time display ON");
  }
  else if(strcmp(value, "DISPLAY_OFF")==0){
    display.clear();
    display.writeDisplay();
    delay(50);

	  // set time display off
	  isDisplayOn = false;

	  // send feedback to adafruit io
	  infoFeed->save((char *)"Alarm clock / Time display OFF");
  }
  else if(strcmp(value, "SHOW_CONFIG")==0){
    // force display alarm summary
	  isLongClick = true;

	  // send feedback to adafruit io
	  infoFeed->save((char *)"Alarm clock / Show alarm summary");
  }
  else if(strcmp(value, "UPD_TIME") == 0 ){

    // need wifi, add check?
    MAIN_DEBUG_PRINTLN("UPD_TIME");

    char route[35];
    strcpy(route, OMIOT_ROOT);
    strcat(route, "time");

    MAIN_DEBUG_PRINTF("route: %s\n", route)
    HTTPClient http;
    http.begin(route);
    int httpCode = http.GET();
    MAIN_DEBUG_PRINTF("recieved http code: %d\n", httpCode);

    if(httpCode < 0){return;}

    // process answer

    String s = http.getString();

    MAIN_DEBUG_PRINTF("response: %s\n", s.c_str());

    const size_t bufferSize = JSON_OBJECT_SIZE(7) + 50;
    DynamicJsonBuffer jsonBuffer(bufferSize);
    JsonObject& root = jsonBuffer.parseObject(s);
    DayOfTheWeek dd =  ClockDateTime::convertIntToDay(root["dd"]);
    ClockDateTime c (root["y"],root["m"],root["d"],root["hh"], root["mm"],root["ss"], dd);

    MAIN_DEBUG_PRINTF("Date: %s\n", c.getPrintableString());

    alarmClock.setCurrentTime(c);

    infoFeed->save((char *)"New time set.");
    http.end();
  }
  else{
	  // free memory allocated by split for re-use
	  int i;
	  for(i = 0; i < numSubStrings; i++){
      delete split[i];
    }
    delete[] split;

	  // split by h, no white spaces
	  split = Common::splitBy(value, "h", &numSubStrings);

  	// format: xxhyy
  	if(numSubStrings == 2){
  		hh = atoi(split[0]);
  		mm = atoi(split[1]);

  		// set alarm #1 week & weekend
  		alarmClock.addOrUpdateAlarm(1,hh,mm,0,true,true);

  		// send feedback to adafruit io
  		sprintf(str, "Alarm clock / Alarm #1 set to: %s",
                   alarmClock.getAlarmWithID(1) -> time().getPrintableString() );
  		infoFeed->save(str);
  	}
    else if(numSubStrings == 1){
      hh = atoi(split[0]);

      // set alarm #1 week & weekend
  		alarmClock.addOrUpdateAlarm(1,hh,0,0,true,true);

      // send feedback to adafruit io
      sprintf(str, "Alarm clock / Alarm #1 set to: %s",
                   alarmClock.getAlarmWithID(1) -> time().getPrintableString() );
      infoFeed->save(str);
    }
  }

  // free memory allocated by split by
  int i;
  for(i = 0; i < numSubStrings; i++){
    delete split[i];
  }
  delete[] split;

  // free memory allocated by remove white spaces
  delete value;
}

void manageDisplay(){
  // first single click = display on, second single click = display off
  if(isSingleClick){
    isDisplayOn = !isDisplayOn;
    if(!isDisplayOn){
      MAIN_DEBUG_PRINTLN("clear");
      currentDisplayTime.setClockTime(0,0,0);
      display.clear();
      display.writeDisplay();
    }
    isSingleClick = false;
  }

  // when long click, display alarm summary
  if(isLongClick){
    display.clear();
    display.writeDisplay();
    delay(50);

    int i;
    for(i=0; i<alarmClock.getNumOfAlarms(); i++){

      AlarmClockInfo a = alarmClock.getAlarms()[i];
      display.writeDigitAscii(0, 'A');
      display.writeDigitAscii(1, 'L');
      display.writeDigitAscii(2,  ' ');
      display.writeDigitAscii(3, '0' + a.id() );
      display.writeDisplay();
      delay(1000);

      if(a.time().isSet()){
        char str[5];
        sprintf(str, "%02d%02d", a.time().hour(), a.time().minute() );
        display.writeDigitAscii(0, str[0]);
        display.writeDigitAscii(1, str[1]);
        display.writeDigitAscii(2, str[2]);
        display.writeDigitAscii(3, str[3]);
      }
      else{
        display.writeDigitAscii(0, 'O');
        display.writeDigitAscii(1, 'F');
        display.writeDigitAscii(2, 'F');
        display.writeDigitAscii(3, ' ');
      }
      display.writeDisplay();
      delay(2000);
    }
    if(!isDisplayOn){
      display.clear();
      display.writeDisplay();
      delay(50);
    }
    else{
      // to force display update
      ClockDateTime c = ClockDateTime(0,0,0,0,0,0);
      c.copyTo(&currentDisplayTime);
    }
    isLongClick = false;
  }

  // update display
  if(isDisplayOn){
    ClockDateTime * t = alarmClock.getCurrentTime();
    if(currentDisplayTime.hour() != t -> hour() || currentDisplayTime.minute() != t -> minute() ){
      t -> copyTo(&currentDisplayTime);
      char str[5];
      sprintf(str, "%02d%02d", t -> hour(), t -> minute() );
      if(isNight()){display.setBrightness(BRIGHTNESS_LCD_NIGHT);}
      if(!isNight()){display.setBrightness(BRIGHTNESS_LCD_DAY);}
      display.writeDigitAscii(0, str[0]);
      display.writeDigitAscii(1, str[1]);
      display.writeDigitAscii(2, str[2]);
      display.writeDigitAscii(3, str[3]);
      MAIN_DEBUG_PRINTLN(str);
      display.writeDisplay();
    }
  }
}

void managePlayer(){
  // single click, alarm off
  // long click, snoozed
  if(playAlarm && (isSingleClick || isLongClick) ){

    if( isSingleClick && alarmClock.getAlarmWithID(1) -> isOn() ){
      MAIN_DEBUG_PRINTLN("Alarm off");
      alarmClock.getAlarmWithID(1) -> setOff(); // off
    }
    else{
      MAIN_DEBUG_PRINTLN("Alarm snoozed");
    }
    isSingleClick = false;
    isLongClick = false;
    playAlarm = false;
    musicPlayer.stopPlaying();
  }

  if(playAlarm){
    if (musicPlayer.stopped()){
      if(alarmClock.getAlarmWithID(1) -> isOn()){
        musicPlayer.startPlayingFile("alarm002.mp3");
      }
    }
  }
}

void clockHandler(void * pArg){
  alarmClock.run();       // run alarm clock
}

void managePushButton(){
  // logic to determine if single click or long click (with filtering)
  // current time in microseconds
  uint32_t currentTime = system_get_time();
  // check value vs threshold
  uint8_t val = 0;
  if(analogRead(A0) > PUSHBUTTON_THRESH_HIGH){val = 1;}

  if(timeClick == 0){
    // button being pushed for the first time
    if(lastPushButtonState != val && val == 1) { timeClick = currentTime; };
    // button released (long click
    if(lastPushButtonState != val && val == 0) { lastPushButtonState = 0;};
  }
  else if( currentTime > timeClick + PUSHBUTTON_FILTERING * 1000 ){
	// check if button is still being pushed (filtering)
  	if(!isClickPending){
  		if(val == 1 ){
  			// button still being pushed
  			isClickPending = true;
  			lastPushButtonState = 1;
  		}
		  // button released before PUSHBUTTON_FILTERING
  		if(val == 0){ timeClick = 0;}
  	}
  	else{
  		if(val == 0){
  			if( currentTime < timeClick + (PUSHBUTTON_FILTERING + PUSHBUTTON_LONG_CLICK) * 1000 ){
          // button released before long click
  				isSingleClick = true;
  				lastPushButtonState = 0;
  				// reset stuff
  				isClickPending = false;
  				timeClick = 0;
				  MAIN_DEBUG_PRINTLN("single click");
  			}
  		}
  		else{
  			if(currentTime > timeClick + (PUSHBUTTON_FILTERING + PUSHBUTTON_LONG_CLICK) * 1000 ){
  				// button is still being pushed after threshold
  				// lastPushButtonState = 1
  				isLongClick = true;
  				// reset stuff
  				isClickPending = false;
  				timeClick = 0;
				  MAIN_DEBUG_PRINTLN("long click");
  			}
  		}
  	}
  }
}

void onAlarm(AlarmClockInfo * data ){

  MAIN_DEBUG_PRINTF("Alarm ID: %d\n", data -> id() );
  playAlarm = true;
  if(data -> numSnooze() == 0){
    MAIN_DEBUG_PRINTLN("First alarm");
  }
  else{
    // alarm will be set off after the max number of snooze is reached
    MAIN_DEBUG_PRINTF("Snooze #%d\n", data -> numSnooze());
  }
}

void setup(){
  Serial.begin(115200);

  // get alarms from RAM
  MAIN_DEBUG_PRINTLN("Getting alarms config from RAM.");
  alarmClock.readFromRAM();
  int count = alarmClock.getNumOfAlarms() ;
  MAIN_DEBUG_PRINTF("Number of alarms defined in RAM: %d\n", count);

  // print alarms info to serial
  int i;
  if(count > 0){
    for(i = 0; i< alarmClock.getNumOfAlarms(); i++){
	  AlarmClockInfo a = alarmClock.getAlarms()[i];
    }
  }

  // assign alarm callback
  alarmClock.onAlarm(onAlarm);

  // vs1053
  musicPlayer.begin();
  SD.begin(CARDCS);
  musicPlayer.setVolume(0,0);
  musicPlayer.useInterrupt(VS1053_FILEPLAYER_PIN_INT);  // DREQ int

  // led backpack
  display.begin(0x70);
  display.clear();
  display.writeDisplay();
  delay(50);

  // pushbutton
  pinMode(A0,INPUT);

  // timers
  os_timer_setfn(&clockTimer, clockHandler, NULL);
  os_timer_arm(&clockTimer, 500, 1 ); // run clock each 0.5 sec

  // adafruit io
  io.connect();
  alarm1Feed->onMessage(handleAlarm1Message);

  while(io.status() < AIO_CONNECTED){
   MAIN_DEBUG_PRINT(".");
   delay(500);
  }
  MAIN_DEBUG_PRINT(" ");
  MAIN_DEBUG_PRINTLN(io.statusText());

  // send feedback to adafruit io
  infoFeed->save((char *)"Alarm clock / Init");

  // ota
  ArduinoOTA.setHostname(OTA_HOST_NAME);
  ArduinoOTA.setPassword(OTA_PASSWORD);
  ArduinoOTA.onStart([]() { Serial.println("Start"); });
  ArduinoOTA.onEnd([]() { Serial.println("\nEnd"); });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total){
	   Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error){
     Serial.printf("Error[%u]: ", error);
     if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
     else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
     else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
     else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
     else if (error == OTA_END_ERROR) Serial.println("End Failed");
   });
   ArduinoOTA.begin();
   MAIN_DEBUG_PRINTLN("OTA ready");
}

void loop(){
    if(!playAlarm){
      ArduinoOTA.handle();      // run ota
      io.run();                 // run adafruit.io
    }
    managePushButton();         // push button
    managePlayer();             // speaker
    manageDisplay();            // display
    yield();                    // esp stuff
}
