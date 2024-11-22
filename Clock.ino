/* 
JOURNAL
Part1.Connect to WiFi and call the API of the "OpenWeather" website to get weather information.
Part2.Display the (1)current date, (2)day of the week, and (3)time on both the screen of ESP32 TTGO and the Serial Monitor.
      NOTE:The date and time display is achieved using the DATE and TIME macros, with an algorithm for accumulation based on this
      (the displayed time may lag about 30 secs behind the real time due to the time taken for compilation)
Part3.Switch the display of (4)weather, (5)temperature, (6)humidity, (7)WiFi username and (8)my name using the GPIO 0 pin 
*/
#include <TFT_eSPI.h>
#include <SPI.h>
TFT_eSPI tft=TFT_eSPI();

#include "uk_icon.h"                             // image in the top left corner
#include "font_sec.h"                            // font file for second digits
#include "animation.h"                           // animation in the bottom
uint8_t frame=0;                                  

//****************WIFI*****************//
#include <WiFi.h>
const char * Ssid="YOIMIYA";              // [Edit] WiFi ssid   YOIMIYA |  Glide_Resident
const char * Password="12345678";         // [Edit] WiFi Password   12345678 | ShadyYieldEnvoy
String str_ssid="YOIMIYA";

//Call API (Provided by "openWeather"website)
#include <HTTPClient.h> 
#include <ArduinoJson.h>
const String apiUrl="http://api.openweathermap.org/data/2.5/weather?q=London,uk&units=metric&APPID=2aa60673dd567e9e38e41ec6eee49921";
StaticJsonDocument<1000> doc;// Create a static JSON document with a capacity of 1000 bytes
String getJson="";
String temperature="";
String humidity="";
String weather="";

void Connect_WiFi()
{  
   WiFi.mode(WIFI_STA);
   WiFi.begin(Ssid,Password);
   while(WiFi.status()!=WL_CONNECTED)
   {
    delay(500);
    Serial.print(".");
   }
   Serial.println("Successfully Connected.");
   Serial.print("IP Address:");
   Serial.println(WiFi.localIP());
   Call_API();
}

/********Call API***********/

void Call_API()
{
   HTTPClient http; // Create an instance of the HTTPClient class to handle HTTP request
   http.begin(apiUrl); // Initialize the HTTP client with the specified API URL
   int httpCode=http.GET(); //Send a GET request to the API
   if(httpCode>0)
   {
      getJson=http.getString();// Retrieve the response as a String
      char data[1000];// Declare a character array to hold the JSON data
      getJson.toCharArray(data,1000); // Convert the JSON to a character array
      deserializeJson(doc,data);// Deserialize the JSON data into the StaticJsonDocument
      temperature=doc["main"]["temp"].as<String>();
      humidity=doc["main"]["humidity"].as<String>();
      weather=doc["weather"][0]["main"].as<String>();
      
   }else{
      Serial.println("HTTP request failed.");
   }
   http.end();// End the HTTP session to free resources

}


//access NTP to find out what day it is today 
#include <NTPClient.h>
WiFiUDP ntpUDP;  // Create a UDP object to handle communication
NTPClient timeClient(ntpUDP,"pool.ntp.org",0,60000);//Initialize NTP client with server, no time offset, update every 60 seconds
const char* daysOfWeek[]={"Sunday","Monday","Tuesday","Wednesday","Thursday","Friday","Saturday"};
String weekday="";



// Control GPIO 0
#include "font.h"
#include "font_weekday.h"
int pinStatus[]={0,1,2,3,4}; // 5 options to choose
String name="Ming";
uint32_t previousTime=0;   
uint8_t indexStatus=0;     // change status
uint8_t guardian=250;      

uint32_t lastTime=0;

//Convert a two-character string to a byte
static byte Convert2d(const char *chr)
{
  uint8_t ten_place = 0;
  if('0' <= *chr  &&  *chr <= '9' )
  { ten_place = *chr - '0'; }
  return 10*ten_place + ( *++chr - '0' );
}

//Display the date and time by using DATE and TIME macros
uint8_t hour=Convert2d(__TIME__);
uint8_t minute=Convert2d(__TIME__+3);
uint8_t second=Convert2d(__TIME__+6);
uint32_t targetTime=0;
uint8_t back_min=61;
String date=__DATE__;

bool flag=1;
bool flagUpdateDay=0; //Refresh the day of the week and the date

String secStr="";
String minStr="";
String hStr="";
String TIME="";

/**************************************
************ S E T _ U P **************
**************************************/

void setup()
 {
   // Initiallize buttons
   pinMode(0,INPUT_PULLUP);
   pinMode(35,INPUT_PULLUP);

   //Initiallization
   tft.init();
   Serial.begin(115200);
   Connect_WiFi();
   tft.fillScreen(TFT_BLACK);

  // Insert Image at top left corner
   tft.setSwapBytes(true);
   tft.pushImage(0,0,48,48,uk_icon);

  // Set the top Font "London"
   tft.setTextColor(TFT_WHITE);
   tft.drawString("London",48,12,4);
   
   targetTime=millis()+1000;
   
   timeClient.begin();
   timeClient.setTimeOffset(0);
   timeClient.update();

}

/**************************************
************** L O O P ****************
**************************************/

void loop() 
{                                                  // Animation
   tft.pushImage(0,195, 135, 65, ani[frame]);
   frame++;
   if(frame>=10)
   frame=0;
                                                   //Calculate Time
  delay(100);
  if(targetTime<millis())
  {
     targetTime=millis()+1000;
     second++;
     if(second>59)
     {
       second=0;
       back_min=minute;
       minute++;
       if(minute>59)
       {
        minute=0;
        hour++;
        if(hour>23)
        {
          hour=0;
          //Refresh the day of the week and the date
          flagUpdateDay=1;  
        }
        else{
          flagUpdateDay=0;
        }
       }
     }
  }
                                              
  if( flag || flagUpdateDay)// Draw once a day
  {                                            // Date
    flag=0;
    tft.setTextColor(TFT_SKYBLUE,TFT_BLACK);
    tft.setCursor(0,48);
    tft.setTextFont(4);
    tft.print(date);
                                              // the day of week
    timeClient.update();
    int days_num=timeClient.getDay();
    weekday=daysOfWeek[days_num];
    tft.setTextColor(TFT_RED,TFT_BLACK);
    tft.setFreeFont(&Orbitron_Medium_16);
    tft.drawString(weekday,0,125);
    
  }
                                     //Manage the displayed content
  if(minute<10){
    minStr="0"+String(minute);
  }else{
    minStr=String(minute);
  }

  if(hour<10){
    hStr="0"+String(hour);
  }else{
    hStr=String(hour);
  }

  if(second<10){
    secStr="0"+String(second);
  }else{
    secStr=String(second);
  }
                                                //hour&minute
  TIME=hStr+":"+minStr;

// The background and the hour-minute digits are drawn once every minute
  if(back_min!=minute)
  {
    tft.setTextColor(0x2124,TFT_BLACK); // Colour of background
    tft.drawString("88:88",0,75,7);
    back_min=minute;
    tft.setTextColor(0x163E); // Colour of time digits
    tft.drawString(TIME,0,75,7);
  }
                                                      //second
  tft.setTextColor(TFT_RED,TFT_BLACK);
  tft.setFreeFont(&DSEG7_Classic_Mini_Regular_18);
  tft.drawString(secStr,105,130);


  //Switch the diaplay content by pressing the button
  if(digitalRead(0)==0 && millis()-previousTime>guardian)
  {
      previousTime=millis();
      indexStatus=(indexStatus+1)%5;
      switch(pinStatus[indexStatus])
      {
        case 0:                                       // Weather
          tft.fillRect(0,145,135,50,TFT_BLACK);
          tft.setFreeFont(&Orbitron_Bold_18);
          tft.setTextColor(TFT_WHITE);
          tft.drawString(weather,30,165);
          break;
       case 1:                                       // Temperature
          tft.fillRect(0,145,135,50,TFT_BLACK);
          tft.setFreeFont(&Orbitron_Bold_18);
          tft.setTextColor(TFT_WHITE);
          tft.drawString("TEMP:",0,150);
          tft.drawString(temperature,75,170);
          break;
       case 2:                                       // Humidity
          tft.fillRect(0,145,135,50,TFT_BLACK);
          tft.setFreeFont(&Orbitron_Bold_18);
          tft.setTextColor(TFT_WHITE);
          tft.drawString("HUM:",10,160);
          tft.drawString(humidity+"%",85,160);
          break; 
       case 3:                                      //WiFi ssid 
          tft.fillRect(0,145,135,50,TFT_BLACK);
          tft.setFreeFont(&Orbitron_Bold_18);
          tft.setTextColor(TFT_WHITE);

          if(str_ssid.length()>=10)
          {
          int gap=str_ssid.length()/2;
          String str1=str_ssid.substring(0,gap-1);
          String str2=str_ssid.substring(gap-1);
          tft.drawString(str1,10,150);
          tft.drawString(str2,10,170);
          }else{
            tft.drawString(str_ssid,15,165);          
          }

          break;
       case 4:                                     
          tft.fillRect(0,145,135,50,TFT_BLACK);
          tft.setFreeFont(&Orbitron_Bold_18);
          tft.setTextColor(TFT_WHITE);
          tft.drawString(name,40,165);
          break;
       default:
          break;
       }
  }

// Display the current time in the Serial Monitor
   if(millis()-lastTime>=1000)
   {
    lastTime=millis();
    String CombinedTime="Date:"+String(__DATE__)+" | "+String(weekday)+" | Time:"+ TIME + ":" + secStr;
    Serial.println(CombinedTime);
   }
   
   
}



//References:
//Time_Clock_Digital.ino
//SampleTime.ino
//(They are built-in example within the Arduino IDE)
