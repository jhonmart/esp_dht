#include <Arduino.h>
#include <WebSocketsClient.h>
#include "data_base.h"
#define PIN_DHT D2


String getValue(String data, char separator, int index);

class System:public DataBaseSD {
   public:
    System(int pin=PIN_DHT);
    String status_chip();
    String showDHTHistory(int start = 0, int qtd = 0);
    String list_wifi();
    String showLogConfig();
    String showDHTValue();
    String showDir();
    void logWS(int client, String status, String info);
    void firebaseSetTemp(String json_data);
    void startConfig();
};
