#include "core.h"
#include <Arduino.h>
#include <ESP8266WiFi.h>

#include "DHTesp.h"
DHTesp dht;
WebSocketsClient webSocket;

String json_db = "";
String last_status = "";

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


void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
  System sys;
  switch(type) {
    case WStype_DISCONNECTED: 
      sys.logWS(255, "Server", "[WSc] Disconnected!\n");
      break;
    case WStype_CONNECTED: {
      sys.logWS(255, "Server", String((char *)payload));
      webSocket.sendTXT("Connected");
      last_status = "Connected";
    }
      break;
    case WStype_TEXT:
      sys.logWS(255, "Server", String((char *)payload));
      break;
    case WStype_BIN:
      sys.logWS(255, "Server",  (String)length);
      hexdump(payload, length);
      break;
        case WStype_PING:
            // pong will be send automatically
            sys.logWS(255, "Server", "[WSc] get ping\n");
            break;
        case WStype_PONG:
            // answer to a ping we send
            sys.logWS(255, "Server", "[WSc] get pong\n");
            break;
    }

}

System::System(int pin) {
  dht.setup(pin, DHTesp::DHT22);
}
void System::firebaseSetTemp(String json_data) {
  if(json_db != json_data) {
    webSocket.sendTXT(json_data);
    json_db = json_data;
  }
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

  json += temperature;
  json += ", \"humidity\":";
  json += humidity;
  json += ", \"heat_index\":";
  json += heatIndex;
  json += "}";
  firebaseSetTemp(json);

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

  createDir("wifi");
  createDir("ws");

  if (config_raw != "fail") {
    String wifi_sta_ssid = getValue(config_raw,'\n', 0);
    String wifi_sta_psw = getValue(config_raw,'\n', 1);
    String wifi_ap_ssid = getValue(config_raw,'\n', 2);
    String wifi_ap_psw = getValue(config_raw,'\n', 3);
    int wifi_type_connect = getValue(config_raw,'\n', 4).toInt();
    int cpu_freq = getValue(config_raw,'\n', 5).toInt();
    String ws_server_ip = getValue(config_raw,'\n', 6);
    String ws_server_path = getValue(config_raw,'\n', 7);
    int ws_server_port = getValue(config_raw,'\n', 8).toInt();
    int ws_reconect_time = getValue(config_raw,'\n', 9).toInt();
    int ws_reconect_pong = getValue(config_raw,'\n', 10).toInt();

    ConnectStatus = "{\"TryingConnectWith\":\"";
    if(wifi_type_connect == 1) {
      WiFi.begin(wifi_sta_ssid, wifi_sta_psw);
    } else {
      WiFi.softAP(wifi_ap_ssid, wifi_ap_psw);
    }
    
    ConnectStatus += wifi_type_connect == 1 ? "State Mode\"}," : "AP Mode\"},";
    writeFile("wifi/log", ConnectStatus);
    
    int count = 10;
    while(WiFi.status() != WL_CONNECTED && count--) {
      delay(100);
    }
    ConnectStatus = "{\"IP\":\"";
    ConnectStatus += WiFi.localIP() ? WiFi.localIP().toString() : "<<No Connect>>";
    ConnectStatus += "\"}";
    writeFile("wifi/log", ConnectStatus);
    webSocket.begin(ws_server_ip, ws_server_port, ws_server_path);
    webSocket.onEvent(webSocketEvent);
    webSocket.setAuthorization("user", "Password");
    webSocket.setReconnectInterval(ws_reconect_time);
    webSocket.enableHeartbeat(15000, 3000, ws_reconect_pong);
    delay(100);
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
    
    firebaseSetTemp(json);
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
