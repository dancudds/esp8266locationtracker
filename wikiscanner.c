#include <ESP8266WiFi.h>
#include "FS.h"
#include <Ticker.h>
//#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>

Ticker tickaway;

boolean timetoscan = false;
const char* hosts3 = "newlocation.s3.amazonaws.com";
const int httpPort = 80;

void upload(){

Serial.println("scan start for upload");
  int n = WiFi.scanNetworks();
  Serial.println("scan done");
  if (n > 0) {
    for (int i = 0; i < n; i++) {
      if (WiFi.encryptionType(i) == ENC_TYPE_NONE) {
        Serial.println("Found open SSID " + WiFi.SSID(i));
        const char * c = WiFi.SSID(i).c_str();
         WiFi.begin(c);
   int tries = 0;
   while (WiFi.status() != WL_CONNECTED  && tries < 20) {
    delay(500);
    Serial.print(".");
    tries++;
  }
  WiFiClient client;
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  if (WiFi.localIP().toString() != "0.0.0.0") {
  Serial.print("Trying to connect ");
  Serial.println(hosts3);
  String urls3upload = "/location?Signature=yoursigD&Expires=1769298015&AWSAccessKeyId=yourid";
  Serial.print("Requesting AWS URL: ");
  Serial.println(urls3upload);
    if (!client.connect(hosts3, httpPort)) {
    Serial.println("connection failed");
  }else //upload all spiff content
  {
  Serial.println("Made connection");
  File fr = SPIFFS.open("/networks.txt", "r");
  Serial.print(fr.size());
  int buffersize = fr.size() / 80;
client.print(String("PUT ") + urls3upload + " HTTP/1.1\r\n" +
               "Host: " + hosts3 + "\r\n" + 
               "Accept: */*\r\nUser-Agent: curl/7.43.0\r\nContent-Length: " + (fr.size()+40) + "\r\nExpect: 100-continue\r\n\r\n");
  delay(2000); // wait for 100 continue
  String line = client.readStringUntil('\r');
  Serial.print(line);
  //client.print("hh");

  
   
  if (!fr) {
    Serial.println("File doesn't exist yet. ");
 
  } else {
    // we could open the file
    client.println(ESP.getChipId()); //print the chipID

    while(fr.available()) {
      //Lets read line by line from the file
      String line = fr.readStringUntil('\n');
      client.print(line);
      client.print('\n');
      Serial.print('.');
    }
    Serial.println("Printing this many buffers ");
    Serial.print(buffersize);
  for (int i = 0; i < buffersize; i++) {
    client.println("buffer");// need to buffer out difference between file size and printed length into S3 or it will fail ungracefully
    Serial.println("buffer line");
  }
    
  }
  Serial.println("Finished upload");
   while(client.available()){
    
    String line = client.readStringUntil('\r');
    Serial.println(line);

    if (line.indexOf("HTTP/1.1 200") > 0) {
      //format file system as upload was good
      Serial.println("Upload went well. Trying to format SPIFFS");
      bool result = SPIFFS.format(); 
      Serial.println("SPIFFS formated: " + result);
    t_httpUpdate_return ret = ESPhttpUpdate.update("www.yourdomain.com", 80, "/update.bin", "optional current version string here");
        

        switch(ret) {
            case HTTP_UPDATE_FAILED:
                Serial.printf("HTTP_UPDATE_FAILD Error (%d): %s", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
                 // Serial.println("HTTP_UPDATE_FAILD Error");
            
                break;

            case HTTP_UPDATE_NO_UPDATES:
                Serial.println(("HTTP_UPDATE_NO_UPDATES"));
                break;

            case HTTP_UPDATE_OK:
                Serial.println(("HTTP_UPDATE_OK"));
                break;
        }
      return;
    }
  }
    
  }

  }
  else
  {Serial.println("No valid IP connection, moving next");}





  
      }
     }
  } 
  
}

void wifiScanandSave(){
bool result = SPIFFS.begin();
Serial.println("SPIFFS opened: " + result);
FSInfo fs_info;
SPIFFS.info(fs_info);
Serial.println(fs_info.totalBytes);
Serial.println(fs_info.usedBytes);
File f = SPIFFS.open("/networks.txt", "a");
if (!f) {
      Serial.println("file append failed");
      }
  Serial.println("scan start");
  int n = WiFi.scanNetworks();
  Serial.println("scan done");
  f.println("----New Scan----");
  if (n > 0) {
    for (int i = 0; i < n; i++) {
      f.println(String() + WiFi.BSSIDstr(i)  + " (" + WiFi.RSSI(i) + ")");
      Serial.println(String() + "SSID " + WiFi.SSID(i) + " " + WiFi.BSSIDstr(i) + String((WiFi.encryptionType(i) == ENC_TYPE_NONE) ? " " : " *") + " (" + WiFi.RSSI(i) + ")");
    }
  } else {
    Serial.println(String() + "No WLAN found");
  }
  f.println("----End Scan----");
  f.close();

 
  // this opens the file "f.txt" in read-mode
  File fr = SPIFFS.open("/networks.txt", "r");
   
  if (!fr) {
    Serial.println("File doesn't exist yet. ");
 
  } else {
    // we could open the file
    while(fr.available()) {
      //Lets read line by line from the file
      String line = fr.readStringUntil('\n');
      Serial.println(line);
    }
  }

  



  upload();


}

void changetimetoscan(){
  timetoscan = true;
}




void setup() {
  // put your setup code here, to run once:
delay(1000);
Serial.begin(9600);
Serial.println("Setup complete");
Serial.println(ESP.getChipId());
Serial.println(ESP.getFreeSketchSpace());

wifiScanandSave();

tickaway.attach(3600, changetimetoscan); //cannot use IO calls during ticker. Need to set flag instead



}

void loop() {

  if (timetoscan){
    timetoscan = false;
    wifiScanandSave();
  }
  // put your main code here, to run repeatedly:
  

}
