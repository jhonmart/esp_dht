#include "core.h"
#include <Arduino.h>
#include <ESP8266WiFi.h>

#include "DHTesp.h"
DHTesp dht;
WebSocketsClient webSocket;

String json_db = "";
String last_status = "";

void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
  switch(type) {
    case WStype_CONNECTED: {
      webSocket.sendTXT("Connected");
    }
  }
}

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

System::System(int pin) {
  dht.setup(pin, DHTesp::DHT22);
}
String System::showDHTValue() {
  String json = "{\"temperature\":";
  float humidity = dht.getHumidity();
  float temperature = dht.getTemperature();
  if(isnan(temperature) || isnan(humidity)) {
    delay(100);
    return showDHTValue();
  }
  float heatIndex = dht.computeHeatIndex(temperature, humidity, false);
  float lumens = analogRead(PIN_LDR);

  json += temperature;
  json += ", \"humidity\":";
  json += humidity;
  json += ", \"heat_index\":";
  json += heatIndex;
  json += ", \"lumens\":";
  json += (int)(sqrt(lumens) + 0.5); // Round
  json += "}";

  insertDHTLog(temperature, humidity);
  return json;
};
String System::showLogConfig(){
  return readFile("config");
};
String System::status_chip(){
  uint32_t realSize = ESP.getFlashChipRealSize() / 1e6;
  uint32_t ideSize = ESP.getFlashChipSize() / 1e6;
  FlashMode_t ideMode = ESP.getFlashChipMode();
  uint32_t chipSpeed = ESP.getFlashChipSpeed() / 1e4;
  uint32_t cpuSpeed = ESP.getCpuFreqMHz();
  String configStatus = ideSize == realSize ? "ok" : "wrong";
  uint32_t sketchSize = ESP.getSketchSize();
  uint32_t sketchSizeFree = ESP.getFreeSketchSpace();
  String heapFragmentation = (String)ESP.getHeapFragmentation();
  
  String out = "{\"realSize\":\"";
  out += realSize;
  out += "MB\",\"ideSize\":\"";
  out += ideSize;
  out += "MB\",\"ideMode\":";
  out += (String)ideMode;
  out += ",\"chipSpeed\":\"";
  out += chipSpeed;
  out += "Mhz\",\"cpuSpeed\":\"";
  out += cpuSpeed;
  out += "Mhz\",\"configStatus\":\"";
  out += configStatus;
  out += "\",\"sketchSize\":";
  out += sketchSize;
  out += ",\"sketchSizeFree\":";
  out += sketchSizeFree;
  out += ",\"heapFragmentation\":\"";
  out += heapFragmentation;
  out += "\"}";

  return out;
};
String System::list_wifi(){
  int count_wifi = WiFi.scanNetworks();
  int networkItem = -1;
  String json = "[";

  while (networkItem++ < count_wifi) {
    json += "{\"name\":\"";
    json += WiFi.isHidden(networkItem) ? "<<Hidden>>" : WiFi.SSID(networkItem);
    json += "\",\"encrypt\":\"";
    json += encryptionType(WiFi.encryptionType(networkItem));
    json += "\",\"rssi\":\"";
    json += WiFi.RSSI(networkItem);
    json += "dBm\",\"mac\":\"";
    json += WiFi.BSSIDstr(networkItem);
    json += "\",\"chanel\":";
    json += WiFi.channel(networkItem);
    json += networkItem == count_wifi ? "}" : "},";
  }

  json += "]";

  return json;
};
void System::startConfig(){
  String config_raw = readFile("config");
  String ConnectStatus = "";

  String ws_server_ip;
  String ws_server_path;
  int ws_server_port;
  int ws_reconect_time;
  int ws_reconect_pong;

  createDir("wifi");
  createDir("ws");

  if (config_raw != "fail") {
    String wifi_sta_ssid = getValue(config_raw,'\n', 0);
    String wifi_sta_psw = getValue(config_raw,'\n', 1);
    String wifi_ap_ssid = getValue(config_raw,'\n', 2);
    String wifi_ap_psw = getValue(config_raw,'\n', 3);
    int wifi_type_connect = getValue(config_raw,'\n', 4).toInt();
    int cpu_freq = getValue(config_raw,'\n', 5).toInt();
    
    ws_server_ip = getValue(config_raw,'\n', 6);
    ws_server_path = getValue(config_raw,'\n', 7);
    ws_server_port = getValue(config_raw,'\n', 8).toInt();
    ws_reconect_time = getValue(config_raw,'\n', 9).toInt();
    ws_reconect_pong = getValue(config_raw,'\n', 10).toInt();
    int clientWS = getValue(config_raw,'\n', 11).toInt();

    ConnectStatus = "{\"TryingConnectWith\":\"";
    if(wifi_type_connect == 1) {
      WiFi.begin(wifi_sta_ssid, wifi_sta_psw);
    } else {
      WiFi.softAP(wifi_ap_ssid, wifi_ap_psw);
    }
    
    ConnectStatus += wifi_type_connect == 1 ? "State Mode\"}," : "AP Mode\"},";
    writeFile("wifi/log", ConnectStatus);
    
    int count = 100;
    while(WiFi.status() != WL_CONNECTED && count--) {
      delay(100);
    }

    ConnectStatus = "{\"IP\":\"";
    ConnectStatus += WiFi.localIP() ? WiFi.localIP().toString() : "<<No Connect>>";
    ConnectStatus += "\"}";
    writeFile("wifi/log", ConnectStatus);

    if (clientWS) {
      webSocket.begin(ws_server_ip, ws_server_port, ws_server_path);
      webSocket.onEvent(webSocketEvent);
      webSocket.setAuthorization("user", "Password");
      webSocket.setReconnectInterval(ws_reconect_time);
      webSocket.enableHeartbeat(15000, 3000, ws_reconect_pong);
    }

    String json = "{\"server\":\"";
    json += ws_server_ip;
    json += "\",\"port\":";
    json += ws_server_port;
    json += ",\"reconnect_time\":";
    json += ws_reconect_time;
    json += ",\"pong\":";
    json += ws_reconect_pong;
    json += ",\"status\":\"";
    json += last_status == "Connected" ? "ok" : "fail";
    json += "\"},";
    
    writeFile("ws/log", json);
  } else {
    writeFile("SYSTEN_LOG", "{\"fail\":\"OnReadFile\"}");
  }
};
String System::showDir(){
  return readRoot();
};
void System::logWS(int client, String status, String info) {
  String json = "{\"client\":";
  json += client;
  json += ",\"status\":\"";
  json += status;
  json += "\",\"data\":\"";
  json += info;
  json += "\"},";
  writeFile("ws/log", json);
}
