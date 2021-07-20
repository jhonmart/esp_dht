#include "core.h"
#include <Arduino.h>
#include <ESP8266WiFi.h>

#include "DHTesp.h"
DHTesp dht;

String getValue(String data, char separator, int index) {
    int found = 0;
    int strIndex[] = { 0, -1 };
    int maxIndex = data.length() - 1;

    for (int i = 0; i <= maxIndex && found <= index; i++) {
        if (data.charAt(i) == separator || i == maxIndex) {
            found++;
            strIndex[0] = strIndex[1] + 1;
            strIndex[1] = (i == maxIndex) ? i+1 : i;
        }
    }
    return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}

String getValue(String data, char separator, int index);
String wifiStringState(int code);

System::System(int pin) {
  dht.setup(pin, DHTesp::DHT22);
}


void System::pushDHTDate(float temp, float humi){
  insertDHTLog(temp, humi);
};
DTH_Struct System::showDHTValue() {
  float humidity = dht.getHumidity();
  float temperature = dht.getTemperature();
  float heatIndex = dht.computeHeatIndex(temperature, humidity, false);

  return { temperature, temperature, heatIndex };
};
String System::showDHTHistory(int start, int qtd){
  return showDHTDate(start, qtd);
};
String System::showInfo(){
  return showInfo();
};
String System::showLogConfig(){
  return readFile("config");
};
void System::writeFile(String path, String text){
  writeFile(path, text);
};
SYS_Status_Struct System::status_chip(){
  return {};
  // uint32_t realSize;
  // uint32_t ideSize;
  // FlashMode_t ideMode;
  // uint32_t chipId;
  // uint32_t chipSpeed;
  // String ideModeName;
  // String configStatus;
  // uint32_t sketchSize;
  // uint32_t sketchSizeFree;
  // String heapFragmentation;
};
String System::list_wifi(bool write_data){
  return "";
};
void System::startConfig(){
  String config_raw = readFile("config");

  if (config_raw != "fail") {
    String wifi_sta_ssid = getValue(config_raw,'\n', 0);
    String wifi_sta_psw = getValue(config_raw,'\n', 1);
    String wifi_ap_ssid = getValue(config_raw,'\n', 2);
    String wifi_ap_psw = getValue(config_raw,'\n', 3);
    int wifi_type_connect = getValue(config_raw,'\n', 4).toInt();
    int cpu_freq = getValue(config_raw,'\n', 5).toInt();

    if(wifi_type_connect == 1) {
      // log connection
      WiFi.begin(wifi_sta_ssid, wifi_sta_psw);
    } else {
      WiFi.softAP(wifi_ap_ssid, wifi_ap_psw);
    }
//    WiFi.localIP() // log this
  } else {
    // You have a problem!
  }
};
String System::readFile(String path){
  return readFile(path);
};
String System::showDir(){
  return readRoot();
};
