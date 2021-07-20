#include <Arduino.h>
#define PIN_SD D8

class DataBaseSD {
   public:
    DataBaseSD(int pin=PIN_SD);
    void insertDHTLog(float temp, float humi);
    String showDHTDate(int start = 0, int qtd = 0);
    String showInfo();
    void writeFail(String text);
    String readFile(String name_file);
    void writeFile(String path, String text);
    String readRoot();
    String createDir(String path);
  private:
    bool _card;
};
