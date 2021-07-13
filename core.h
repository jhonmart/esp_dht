#include <Arduino.h>

struct DTH_Struct {
  float temperature;
  float humidity;
  float heatIndex;
};

struct SYS_Struct {
  String wifi_sta_ssid;
  String wifi_sta_psw;
  String wifi_ap_ssid;
  String wifi_ap_psw;
  int wifi_type_connect;
  int cpu_freq;
};

struct SYS_Status_Struct {
  uint32_t realSize;
  uint32_t ideSize;
  FlashMode_t ideMode;
  uint32_t chipId;
  uint32_t chipSpeed;
  String ideModeName;
  String configStatus;
  uint32_t sketchSize;
  uint32_t sketchSizeFree;
  String heapFragmentation;
};

String getValue(String data, char separator, int index);
String wifiStringState(int code);

DTH_Struct DHTData();
SYS_Struct SYSData();

class System {
   public:
    void pushDHTDate(float temp, float humi);
    String showDHTDate(int start = 0, int qtd = 0);
    String showInfo();
    String showLogConfig();
    void writeFile(String path, String text);
    SYS_Status_Struct status_chip();
    String list_wifi(bool write_data = false);
    void startConfig();
    String readFile(String path);
    String showDir();
};
