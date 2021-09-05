#include<Wire.h>
#include<WiFi.h>
#include "time.h"
#include "RTClib.h"
#include <ThingSpeak.h>
#include <WebServer.h>
#include "DHT.h"

#define LED_BUILTIN 2
const char* ssid       = "GPONWIFI_68E0";
const char* password   = "00000086D5";

const char* ntpServer = "pool.ntp.org";
const long gmtOffsetSec = 0; // UTC
const long dayLightOffsetSec = 0;

const char* thingspeak = "api.thingspeak.com";

WebServer server(80);
WiFiClient client;
#define DHTTYPE DHT22
uint8_t DHTPin = 4;
DHT dht(DHTPin,DHTTYPE);

float Temperature;
float Humidity;

RTC_DS3231 rtc;
const byte rtcTimerIntPin = 2;
volatile byte flag = false;

String apiKey = "8EERZONJ8K9LJT1I";

void printTime()
{
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    abort();
  }
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
}

void settime()
{
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    abort();
  }
  // set RTC using date time from compile time
//  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  // set RTC time using ntp
  rtc.adjust(DateTime(timeinfo.tm_year, timeinfo.tm_mon, timeinfo.tm_mday, timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec));
  DateTime now = rtc.now();
  Serial.println("RTC TIME");
  Serial.print(now.year(), DEC);
  Serial.print('/');
  Serial.print(now.month(), DEC);
  Serial.print('/');
  Serial.print(now.day(), DEC);
  Serial.print(' ');
  Serial.print(now.hour(), DEC);
  Serial.print(':');
  Serial.print(now.minute(), DEC);
  Serial.print(':');
  Serial.print(now.second(), DEC);
  Serial.println();
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  delay(3000);
  //DTH START
  pinMode(DHTPin,INPUT);
  dht.begin();
  Serial.println("Connecting to:");
  Serial.println(ssid);
  WiFi.begin(ssid,password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected..!");
  Serial.print("Got IP: ");  Serial.println(WiFi.localIP());
  server.on("/", handle_OnConnect);
  server.onNotFound(handle_NotFound);
  server.begin();
  Serial.println("HTTP server started");
  //DTH END

  //RTC BEGIN
  configTime(gmtOffsetSec, dayLightOffsetSec, ntpServer);
  printTime();
  if(!rtc.begin()){
      Serial.println("Couldn't find rtc");
      Serial.flush();
      abort();
    }
    if(rtc.lostPower())
    {
      Serial.println("RTC lost power, set time");
      settime();
    }
    //enable 1Hz output
    rtc.writeSqwPinMode(DS3231_SquareWave1Hz);

    //setup to handle interrupt from 1Hz pin
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(rtcTimerIntPin, INPUT_PULLUP);
    attachInterrupt (digitalPinToInterrupt (rtcTimerIntPin), rtc_interrupt, CHANGE);
}

void sendData()
{
  Temperature = dht.readTemperature();
  Humidity = dht.readHumidity();
  String postStr = apiKey;
  postStr +="&field1=";
  postStr += String(Temperature);
  postStr += "&field2=";
  postStr += String(Humidity);
  postStr += "\r\n\r\n";

  if(client.connect(thingspeak,80)){
    client.print("POST /update HTTP/1.1\n");
    client.print("Host: api.thingspeak.com\n");
    client.print("Connection: close\n");
    client.print("X-THINGSPEAKAPIKEY: "+apiKey+"\n");
    client.print("Content-Type: application/x-www-form-urlencoded\n");
    client.print("Content-Length: ");
    client.print(postStr.length());
    client.print("\n\n");
    client.print(postStr);
  }
  Serial.print("Temperature:");
  Serial.print(Temperature);
  Serial.print("Humidity: ");
  Serial.println(Humidity);
}

void rtc_interrupt(void)
{
    flag = true;
}

void loop() {
  // put your main code here, to run repeatedly:
  server.handleClient();
  if(flag){
    //flash led
    digitalWrite(LED_BUILTIN, HIGH);
    sendData();
    delay(100);
    digitalWrite(LED_BUILTIN, LOW);
    flag=false;
  }
}

void handle_OnConnect() {
  Temperature = dht.readTemperature(); // Gets the values of the temperature
  Humidity = dht.readHumidity(); // Gets the values of the humidity 
  server.send(200, "text/html", SendHTML(Temperature,Humidity)); 
}

void handle_NotFound(){
  server.send(404, "text/plain", "Not found");
}

String SendHTML(float Temperaturestat,float Humiditystat){
  String ptr = "<!DOCTYPE html> <html>\n";
  ptr +="<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  ptr +="<title>ESP32 Weather Report</title>\n";
  ptr +="<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}\n";
  ptr +="body{margin-top: 50px;} h1 {color: #444444;margin: 50px auto 30px;}\n";
  ptr +="p {font-size: 24px;color: #444444;margin-bottom: 10px;}\n";
  ptr +="</style>\n";
  ptr +="</head>\n";
  ptr +="<body>\n";
  ptr +="<div id=\"webpage\">\n";
  ptr +="<h1>ESP32 Weather Report</h1>\n";
  
  ptr +="<p>Temperature: ";
  ptr +=(int)Temperaturestat;
  ptr +="°C</p>";
  ptr +="<p>Humidity: ";
  ptr +=(int)Humiditystat;
  ptr +="%</p>";
  
  ptr +="</div>\n";
  ptr +="</body>\n";
  ptr +="</html>\n";
  return ptr;
}
