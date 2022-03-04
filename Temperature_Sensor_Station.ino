#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WiFiClient.h>
#include <Arduino_JSON.h>
#include <ESP8266HTTPClient.h>
#include "SinricPro.h"
#include "DHT.h"

#define DHTTYPE DHT11   // DHT 11
#define APP_KEY "APP_KEY"
#define APP_SECRET "SECRET_KEY"
#define device_ID_1 "DEVICE_ID"


const char* ssid = "WIFI_SSID";  
const char* password = "WIFI_PWD";  
String openWeatherMapApiKey = "API_KEY"; 
String city = "CITY; //OpenWeather City
String countryCode = "COUNTRY"; 

unsigned long lastTime = 0;
unsigned long timerDelay = 10000;
String jsonBuffer;

ESP8266WebServer server(80);

// DHT Sensor
uint8_t DHTPin = D8; 
               
// Initialize DHT sensor.
DHT dht(DHTPin, DHTTYPE);                

float Temperature;
float TempKelvin;
float TempFar;
float Humidity;
 
void setup() {
  Serial.begin(115200);
  delay(100);
  
  pinMode(DHTPin, INPUT);

  dht.begin(); 


  Serial.println("Connecting to ");
  Serial.println(ssid);

  //connect to your local wi-fi network
  WiFi.begin(ssid, password);

  //check wi-fi is connected to wi-fi network
  while (WiFi.status() != WL_CONNECTED) {
  delay(1000);
  Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected..!");
  Serial.print("Got IP: ");  Serial.println(WiFi.localIP());
  Serial.println(jsonBuffer);

  server.on("/", handle_OnConnect);
  server.onNotFound(handle_NotFound);

  server.begin();
  Serial.println("HTTP server started");

}
void loop() {


  
  server.handleClient();

}


void handle_OnConnect() {

 Temperature = dht.readTemperature(true); // Gets the values of the temperature
  Humidity = dht.readHumidity(); // Gets the values of the humidity 
  String serverPath = "http://api.openweathermap.org/data/2.5/weather?q=" + city + "," + countryCode + "&APPID=" + openWeatherMapApiKey;
  jsonBuffer = httpGETRequest(serverPath.c_str());
  Serial.println(jsonBuffer);
  
  
 

  server.send(200, "text/html", SendHTML(Temperature,Humidity)); 
  
  Serial.println(jsonBuffer);

  
}
void handle_NotFound(){
  server.send(404, "text/plain", "Not found");
}


String SendHTML(float Temperaturestat,float Humiditystat){
 JSONVar myObject = JSON.parse(jsonBuffer);
  int TempKelvin = (myObject["main"]["temp"]);
  int OutsideTemp = (((TempKelvin - 273.15)*9)/5)+32;
  int OutsideHumidity = (myObject["main"]["humidity"]);
  String WindowAnswer = WinLogic(OutsideTemp, Temperaturestat); 
  String ptr = "<!DOCTYPE html> <html>\n";
  ptr +="<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  ptr +="<title>ESP8266 Weather Report</title>\n";
  ptr +="<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}\n";
  ptr +="body{margin-top: 50px;} h1 {color: #444444;margin: 50px auto 30px;}\n";
  ptr +="p {font-size: 24px;color: #444444;margin-bottom: 10px;}\n";
  ptr +="</style>\n";
  ptr +="</head>\n";
  ptr +="<body>\n";
  ptr +="<div id=\"webpage\">\n";
  ptr +="<h1>ESP8266 NodeMCU Weather Report</h1>\n";
  
  ptr +="<p>Temperature: ";
  ptr +=(int)Temperaturestat;
  ptr +=" \xB0""F</p>";
  ptr +="<p>Humidity: ";
  ptr +=(int)Humiditystat;
 // ptr +="<p>Json: ";
 // ptr += jsonBuffer;
  ptr +="%</p>";
    ptr +="<p>Temperature Outside Kelvin: ";
  ptr += TempKelvin;
  ptr +="<p>Temperature Outside Fahrenheit: ";
  ptr += OutsideTemp;
  ptr +="<p>Outside Humidity: ";
  ptr += OutsideHumidity;
  ptr +="<p>Can I open the Window?: ";
  ptr += WindowAnswer;
  
  ptr +="</div>\n";
  ptr +="</body>\n";
  ptr +="</html>\n";
  return ptr;
}

String httpGETRequest(const char* serverName) {
  WiFiClient client;
  HTTPClient http;
    
  // Your IP address with path or Domain name with URL path 
  http.begin(client, serverName);
  
  // Send HTTP POST request
  int httpResponseCode = http.GET();
  
  String payload = "{}"; 
  
  if (httpResponseCode>0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    payload = http.getString();
  }
  else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }
  // Free resources
  http.end();

  return payload;
}

// Window Logic
String WinLogic(int OutsideTemp, int Temperaturestat){
  String result;

  if (Temperaturestat - OutsideTemp <= 15 && Temperaturestat - OutsideTemp > 0 ){
  result = "Open the Window";
  
  }else if(Temperaturestat < OutsideTemp){
  result = "Keep the Window Closed";

  }else if (Temperaturestat - OutsideTemp >= 15){
  result = "Too cold, close the window";  
  }
  return result;
}
