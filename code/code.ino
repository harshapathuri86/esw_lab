
#include<Wire.h>
#include<WiFi.h>
#include "time.h"
#include "RTClib.h"
#include <ThingSpeak.h>
#include <WebServer.h>
#include "DHT.h"

#define LED_BUILTIN 2

const int BUZZER = 5;
bool onWiFi = false;
bool onAlarm;
const int touchPin = 12;
const int touchThreshold = 25;
const int touchReadings = 100;
const int timeout = 10;
const char* ssid       = "GPONWIFI_68E0";
const char* password   = "00000086D5";
const char* ntpServer = "pool.ntp.org";
const long gmtOffsetSec = 19800; // UTC +5:30
const long dayLightOffsetSec = 0;
const char* thingspeak = "api.thingspeak.com";
String apiKey = "8EERZONJ8K9LJT1I";

WebServer server(80);
WiFiClient client;

#define DHTTYPE DHT22
uint8_t DHTPin = 4;
DHT dht(DHTPin, DHTTYPE);

float Temperature;
float Humidity;

RTC_DS3231 rtc;
const byte rtcTimerIntPin = 2;
volatile byte flag = false;

void printTime()
{
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    abort();
  }
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
}

void settime()
{
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    abort();
  }
  // set RTC using date time from compile time
  // rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
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

bool wifiConnect() {
  onWiFi = false;
  int retries = 0;
  Serial.printf("Connecting to %s ", ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    retries++;
    if (retries == (timeout * 2)) {
      Serial.println(" TIMEOUT");
      break;
    }
  }
  if (WiFi.status() == WL_CONNECTED) {
    onWiFi = true;
    Serial.println(" CONNECTED");
  }
  return onWiFi;
}

bool getNtpTime() {
  if (onWiFi == true) {
    configTime(gmtOffsetSec, dayLightOffsetSec, ntpServer);
    return true;
  } else {
    Serial.println("getNtpTime: Not connected to wifi!");
  }
  return false;
}

void setupServer()
{
  server.enableCORS();
  server.on("/live", handle_OnConnect);
  server.on("/", handle_OnConnect_page);
  server.onNotFound(handle_NotFound);
  server.begin();
  Serial.println("HTTP server started");
}

void setExternalRTC()
{
  if (!rtc.begin()) {
    Serial.println("Couldn't find rtc");
    Serial.flush();
    abort();
  }
  if (rtc.lostPower())
  {
    Serial.println("RTC lost power, set time");
    settime();
  }

  // Disable and clear both alarms
  rtc.disableAlarm(1);
  rtc.disableAlarm(2);
  rtc.clearAlarm(1);
  rtc.clearAlarm(2);

  // Set alarm time
  //https://github.com/adafruit/RTClib/blob/master/src/RTClib.h
  rtc.setAlarm1(rtc.now() + TimeSpan(0, 0, 0, 30), DS3231_A1_Minute);
  rtc.setAlarm2(rtc.now() + TimeSpan(0, 0, 1, 0), DS3231_A2_Minute);

}

void readTouch()
{
  int avgTouch = 0;
  for (int i = 0; i < touchReadings; i++)
  {
    avgTouch += touchRead(touchPin);
  }
  yield();
  avgTouch /= touchReadings;
  if (avgTouch < touchThreshold) {
    Serial.println("Detected touch");
    onAlarm = false;
  }
}

void playBuzzer(int times) {
  onAlarm = true;
  for (int t = 0; t < times; t++) {
    if (onAlarm == false) {
      break;
    }
    digitalWrite(BUZZER, HIGH);
    delay(200);
    digitalWrite(BUZZER, LOW);
    delay(1);
    //Read touch sensor to check if alarm is stopped manually
    readTouch();
    delay(5);
  }
  onAlarm = false;
}

void setup() {
  Serial.begin(115200);
  delay(3000);

  // Alarm
  pinMode(BUZZER, OUTPUT);

  // Internal LED
  pinMode(LED_BUILTIN, OUTPUT);

  //DTH
  pinMode(DHTPin, INPUT);
  dht.begin();

  // WiFi
  wifiConnect();
  if (onWiFi == true)
  {
    Serial.println("WiFi connected..!");
    Serial.print("Got IP: ");  Serial.println(WiFi.localIP());
  }
  else {
    Serial.println("Timed out connecting WiFi: aborting");
    abort();
  }

  // Web Server to query live data
  setupServer();

  //Set internal RTC
  getNtpTime();
  delay(100);
  printTime();

  //Set external RTC
  setExternalRTC();
}

void sendData()
{
  Temperature = dht.readTemperature();
  Humidity = dht.readHumidity();
  String postStr = apiKey;
  postStr += "&field1=";
  postStr += String(Temperature);
  postStr += "&field2=";
  postStr += String(Humidity);
  postStr += "\r\n\r\n";

  if (client.connect(thingspeak, 80)) {
    client.print("POST /update HTTP/1.1\n");
    client.print("Host: api.thingspeak.com\n");
    client.print("Connection: close\n");
    client.print("X-THINGSPEAKAPIKEY: " + apiKey + "\n");
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


void loop() {

  server.handleClient();

  if (rtc.alarmFired(2) == true) {
    DateTime now = rtc.now();
    rtc.clearAlarm(2);
    rtc.setAlarm2(now + TimeSpan(0, 0, 1, 0), DS3231_A2_Minute);
    sendData();
    Serial.println(WiFi.localIP());
    char buff[] = "Alarm triggered at hh:mm:ss DDD, DD MMM YYYY";
    Serial.println(now.toString(buff));

    //flash led
    digitalWrite(LED_BUILTIN, HIGH);
    delay(100);
    digitalWrite(LED_BUILTIN, LOW);

  }
  if (rtc.alarmFired(1) == true)
  {
    DateTime now = rtc.now();
    rtc.disableAlarm(1);
    rtc.clearAlarm(1);
    rtc.setAlarm1(now + TimeSpan(0, 0, 0, 30), DS3231_A1_Second);
    char buff[] = "Alarm 1:Buzzer triggered at hh:mm:ss DDD, DD MMM YYYY";
    Serial.println(rtc.now().toString(buff));
    playBuzzer(10);
  }
}

void handle_OnConnect() {
  Temperature = dht.readTemperature(); // Gets the values of the temperature
  Humidity = dht.readHumidity(); // Gets the values of the humidity
  server.send(200, "application/json", sendString(Temperature, Humidity));
  //  server.send(200, "text/html", SendHTML(Temperature, Humidity));
}

void handle_OnConnect_page() {
  Temperature = dht.readTemperature(); // Gets the values of the temperature
  Humidity = dht.readHumidity(); // Gets the values of the humidity
  //  server.send(200,"application/json",sendString(Temperature,Humidity));
  server.send(200, "text/html", SendHTML(Temperature, Humidity));
}

String sendString(float Temperature, float Humidity)
{
  String ptr = "{";
  ptr += "\"temperature\": \"" + String(Temperature) + "\"";
  ptr += ",\"humidity\": \"" + String(Humidity) + "\"";
  ptr += "}";
  return ptr;
}

void handle_NotFound() {
  server.send(404, "text/plain", "Not found");
}

String SendHTML(float Temperaturestat, float Humiditystat) {
  String ptr = "<!DOCTYPE html> <html>\n";
  ptr += "<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  ptr += "<title>ESP32 Weather Report</title>\n";
  ptr += "<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}\n";
  ptr += "body{margin-top: 50px;} h1 {color: #444444;margin: 50px auto 30px;}\n";
  ptr += "p {font-size: 24px;color: #444444;margin-bottom: 10px;}\n";
  ptr += "</style>\n";
  ptr += "</head>\n";
  ptr += "<body>\n";
  ptr += "<div id=\"webpage\">\n";
  ptr += "<h1>ESP32 Weather Report</h1>\n";

  ptr += "<p>Temperature: ";
  ptr += String(Temperaturestat, 2);
  ptr += " °C</p>";
  ptr += "<p>Humidity: ";
  ptr += String(Humiditystat, 2);
  ptr += "%</p>";

  ptr += "</div>\n";
  ptr += "</body>\n";
  ptr += "</html>\n";
  return ptr;
}
