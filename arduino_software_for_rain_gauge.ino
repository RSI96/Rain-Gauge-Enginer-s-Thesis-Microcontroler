#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

#define LWIP_HAVE_LOOPIF   LWIP_NETIF_LOOPBACK

const long utcOffsetInSeconds = 3600;

int hallSensor = 5;
int prevState = HIGH;
int id = -2;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);

void setup() {
  
  Serial.begin(115200);
  
  pinMode(hallSensor, INPUT);
  digitalWrite(hallSensor, HIGH);
  
  WiFi.begin("NAZWA_SIECI", "HASŁO_DO_SIECI");
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Czekam na połączenie");
  }
  timeClient.begin();
}

void loop() {
  
  if (WiFi.status() == WL_CONNECTED) {
    
    int currState = digitalRead(hallSensor);
    
    if(currState != prevState)
    {
    id++;
    
    timeClient.update();
    unsigned long epoch = timeClient.getEpochTime();
    String year = String((epoch/31556926)+1970);
    String month = String(((epoch%31556926)/2629743)+1);
    String day = String(((epoch%2629743)/86400)+1);
    String date = year + "-" + month + "-" + day;
    String time_ = timeClient.getFormattedTime();
    String date_time = date + " " + time_;
    
    StaticJsonDocument<300> JSONdocument;
    
    JSONdocument["id"] = id;
    JSONdocument["measurement"] = 0.115;
    JSONdocument["date"] = date;
    JSONdocument["time"] = time_;
    JSONdocument["date_time"] = date_time;
    
    char JSONmessageBuffer[300];
    serializeJsonPretty(JSONdocument, JSONmessageBuffer);
    Serial.println(JSONmessageBuffer);
    
    HTTPClient http;
    
    http.begin("http://ADRES_SERWERA:8080/all");
    http.addHeader("Content-Type", "application/json");
    
    int httpCode = http.POST(JSONmessageBuffer);
    String payload = http.getString();
    
    Serial.println(httpCode);
    Serial.println(payload);
    
    http.end();
    }
    prevState = currState;
  } else { Serial.println("Błąd w połączeniu WiFi");}
}
