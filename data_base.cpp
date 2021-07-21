#include "data_base.h"
#include <SD.h>

String printDirectory(File dir, int numTabs) {
  String data = "[";
  while (true) {
    File entry =  dir.openNextFile();
    if (! entry) {
      // no more files
      break;
    }
    data += "{\"name\":\"";
    data += entry.name();
    data += "\",\"level\":";
    data += numTabs;
    data += ",\"detail\":";
    if (entry.isDirectory()) {
      data += printDirectory(entry, numTabs + 1);
      data += "},";
    } else {
      data += "{";
      data += "\"size\":";
      data += entry.size();
      data += ",\"create_at\":\"";
      data += entry.getCreationTime();
      data += "\",\"last_write\":\"";
      data += entry.getLastWrite();
      data += "\"}},";
    }
    entry.close();
  }
  data.remove(data.length() - 1,1);
  data += "]";
  return data;
}

DataBaseSD::DataBaseSD(int pin) {
  _card = !SD.begin(pin);
}

String DataBaseSD::readFile(String name_file) {
  File fileRead = SD.open(name_file);
  String outData = "";
  if (fileRead) {
    while (fileRead.available()) {
      char character = (char) fileRead.read();
      outData += character;
    }
    fileRead.close();
    return outData;
  } else {
    return "fail";
  }
}

String DataBaseSD::readRoot() {
  File root = SD.open("/");
  return printDirectory(root, 0);
}

String DataBaseSD::createDir(String path) {
  return SD.exists(path) ? "Exists" :  SD.mkdir(path) ? "Created" : "Fail";
}

void DataBaseSD::writeFile(String path, String text) {
  File fileWrite = SD.open(path, FILE_WRITE);
  if (fileWrite) {
    fileWrite.println(text);
    fileWrite.close();
  }
}

void DataBaseSD::insertDHTLog(float temp, float humi) {
  File fileWrite = SD.open("history_dth", FILE_WRITE);
  if (fileWrite) {
    fileWrite.print("[");
    fileWrite.print(temp);
    fileWrite.print(",");
    fileWrite.print(humi);
    fileWrite.print("]");

    fileWrite.close();
  }
}

String DataBaseSD::showDHTDate(int start, int qtd) {
  int sizeItem = 13;
  int lines = 100;
  File fileRead = SD.open("history_dth");
  String outData = "{\"data\": [";
  if (fileRead) {
    int row = 0;
    unsigned long filesize = fileRead.size();
    qtd >= 0 ? fileRead.seek(start * sizeItem) : fileRead.seek(filesize - sizeItem * lines);
    unsigned long limit = (qtd >= 0 ? sizeItem * qtd : sizeItem * lines) + qtd -1;
    while (fileRead.available() && (row++ < limit || !outData.endsWith("]"))) {
      char character = (char) fileRead.read();
      if(character == '[' || outData.indexOf("[[") > 1) {
        outData += character;
        if(fileRead.position() != filesize && (String) character == "]" && row++ < limit)
          outData += ",";
      }
    }
    outData += "], \"size\": ";
    outData += filesize;
    outData += ", \"rows\": ";
    outData += (filesize / sizeItem);
    outData += "}";
    fileRead.close();
  } else {
    outData = "{\"fail\": \"open file\", \"card\": \""; 
    outData += "\", \"file\": \"";
    outData += (fileRead ? "true" : "false");
    outData += "\"}";
  }
  return outData;
}

String DataBaseSD::showInfo() {
  float volumesize;
  float volumeUsed;
  
  volumesize = SD.totalClusters();
  volumesize *= SD.clusterSize();
  volumesize /= 1000; // Kb
  volumesize /= 1024; // Mb
  volumesize /= 1024; // Gb

  String json = "{\"conect\": ";
  json += _card;
  json += ", \"type\": ";
  json += SD.type();
  json += ", \"volume\": ";
  json += volumesize;
  json += ", \"volume_type\": \"FAT";
  json += SD.fatType();
  json += "\"";
  json += "}";

  return json;
}

void DataBaseSD::writeFail(String text) {
  File fileWrite = SD.open("system_info.txt", FILE_WRITE);
  if (fileWrite) {
    fileWrite.println(text);
    fileWrite.close();
  }
}
