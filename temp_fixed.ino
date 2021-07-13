#include <WebSocketsServer.h>
#include "core.h"

WiFiServer server(80);

System sys;

int SOCKET_PORT  = 8080;

WebSocketsServer _socket = WebSocketsServer(SOCKET_PORT);

void socketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  switch (type) {
  case WStype_DISCONNECTED:
    //USE_SERIAL.printf("[%u] Disconnected!\n", num);
    break;
  case WStype_CONNECTED: {
    _socket.sendTXT(num, "Connected: ok?");
  }
  break;
  case WStype_TEXT:
    if (memcmp(payload, "temp_humi", 9) == 0) {
      String json = "{\"temperature\": ";
      DTH_Struct state = DHTData();
      float humidity = state.humidity;
      float temperature = state.temperature;
      float heatIndex = state.heatIndex;

      json += temperature;
      json += ", \"humidity\": ";
      json += humidity;
      json += ", \"heat_index\": ";
      json += heatIndex;
      json += ", \"valid\": true}";

      _socket.sendTXT(num, json);
      sys.pushDHTDate(temperature, humidity);
    } else if (memcmp(payload, "temp_card", 9) == 0) {
      if(memcmp(payload, "temp_card:", 10) == 0) {
        String str = String(((char *) payload)); // Convertion char* to String
        int start = getValue(str,':', 1).toInt();
        int qtd = getValue(str,':', 2).toInt();
        String json = sys.showDHTDate(start, qtd);
        _socket.sendTXT(num, json);
      } else {
        String json = sys.showDHTDate();
        _socket.sendTXT(num, json);
      }
    } else if (memcmp(payload, "card_info", 9) == 0) { 
      String json = sys.showInfo();
      _socket.sendTXT(num, json);
    } else if (memcmp(payload, "conf_info", 9) == 0) { 
      String json = sys.showLogConfig();
      _socket.sendTXT(num, json);
    } else if (memcmp(payload, "show_root", 9) == 0) { 
      String json = sys.showDir();
      _socket.sendTXT(num, json);
    } else if (memcmp(payload, "show_wifi", 9) == 0) { 
      String json = sys.list_wifi();
      _socket.sendTXT(num, json);
    } else if (memcmp(payload, "read_file", 9) == 0) { 
      String str = String(((char *) payload)); // Convertion char* to String
      String path = getValue(str,':', 1);
      String json = sys.readFile(path);
      _socket.sendTXT(num, json);
    } else 
      _socket.broadcastTXT(payload);
    break;
  case WStype_BIN:
    hexdump(payload, length);
    break;
  }
}

void setup() {
  sys.startConfig();
  ESP.wdtDisable(); // Desabilita o SW WDT. 
  ESP.wdtEnable(5000); 

  sys.status_chip();
  
  server.begin();

  _socket.begin();
  _socket.onEvent(socketEvent);
}

void loop() {
  _socket.loop();
  ESP.wdtFeed(); // Alimenta o Watchdog.
}
