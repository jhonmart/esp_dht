#include "core.h"
#include <Arduino.h>
#include <ESP8266WiFi.h>

#include "DHTesp.h"
DHTesp dht;


String encryptionType(int type) {
  switch (type) {
    case 1:
      return "WEP";
    break;
    case 2:
      return "WPA/PSK";
    break;
    case 4:
      return "WPA2/PSK";
    break;
    case 7:
      return "<<OPEN>>";
    break;
    case 8:
      return "WPA/WPA2/PSK";
    break;
    default: {
      String out = "Type not found: ";
      out += type;
      return out;
    }
    break;
  }
}


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
  if(isnan(temperature) || isnan(humidity)) {
    delay(100);
    return showDHTValue();
  }
  float heatIndex = dht.computeHeatIndex(temperature, humidity, false);
  insertDHTLog(temperature, humidity);
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
  uint32_t realSize = ESP.getFlashChipRealSize();
  uint32_t ideSize = ESP.getFlashChipSize();
  FlashMode_t ideMode = ESP.getFlashChipMode();
  uint32_t chipId = ESP.getFlashChipSpeed();
  uint32_t chipSpeed = ESP.getCpuFreqMHz();
  String ideModeName = "";
  String configStatus = ideSize == realSize ? "ok" : "wrong";
  uint32_t sketchSize = ESP.getSketchSize();
  uint32_t sketchSizeFree = ESP.getFreeSketchSpace();
  String heapFragmentation = (String)ESP.getHeapFragmentation();
  
  return { realSize, ideSize, ideMode, chipId, chipSpeed, ideModeName, configStatus, sketchSize, sketchSizeFree, heapFragmentation };
};
String System::list_wifi(bool write_data){
  int count_wifi = WiFi.scanNetworks();
  int networkItem = -1;
  String json = "[";

  while (networkItem++ < count_wifi) {
    json += "{\"name\":\"";
    json += WiFi.SSID(networkItem);
    json += "\",\"encrypt\":\"";
    json += encryptionType(WiFi.encryptionType(networkItem));
    json += "\",\"rssi\":";
    json += WiFi.RSSI(networkItem);
    json += "dBm\",\"mac\":";
    json += WiFi.BSSIDstr(networkItem);
    json += "\",\"chanel\":";
    json += WiFi.channel(networkItem);
    json += "\",\"visibility\":";
    json += WiFi.isHidden(networkItem) ? "<<Hidden>>" : "visible";
    json += networkItem == count_wifi ? "\"}" : "\"},";
  }

  json += "]";

  return json;
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
