#include <EEPROM.h>

#include <WiFi.h>
#include <ESPAsyncWebServer.h>
// #include <esp_now.h>

TaskHandle_t Task_1;
#define WDT_TIMEOUT 3

// Set your access point network credentials
const char *ssid = "GFI-ESP32-Access-Point";
const char *password = "22101782";

String server_ID = "GFI-ESP32-Access-Point";
String server_Password = "22101782";

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);


void Task_1_sendData(void *pvParameters)
{
}

String readData()
{
  return String(550 / 100.0F);
}

String Data;

// REPLACE WITH THE MAC Address of your receiver
uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

int tttt = 0;
String WIFIreadData()
{
  tttt += 1;
  if (tttt > 500)
    tttt = 0;
  return String(tttt);
}

void CleanEEPROM(int startPosition, int datalength)
{
  for (size_t i = startPosition; i < (startPosition + datalength); i++)
  {
    EEPROM.write(i, ' ');
  }
  Serial.println("Clean EEPROM");
}

String ReadInfoEEPROM(int start_position, int data_length)
{
  String EEPROM_String = "";
  for (int i = 0; i < data_length; i++)
  {
    uint8_t a = EEPROM.read(i + start_position);
    if (a != 255)
      EEPROM_String += char(EEPROM.read(i + start_position));
  }
  EEPROM_String.trim();
  return EEPROM_String;
}

void WriteInfoEEPROM(String data, int start_position)
{
  for (int i = 0; i < data.length(); ++i)
  {
    EEPROM.write(i + start_position, data[i]);
  }
}

String WR_EEPROM(int start_position, String data)
{
  CleanEEPROM(start_position, 32); //Clean EEPROM(int startPosition, int datalength)

  WriteInfoEEPROM(String(data), start_position); //Write Data to EEPROM (data, start_position)
  EEPROM.commit();

  String s = ReadInfoEEPROM(start_position, 32);
  return s;
}

void setup()
{
  Serial.begin(115200);
  Serial.setTimeout(20); //設定序列埠接收資料時的最大等待時間

  //宣告使用EEPROM 512 個位置
  EEPROM.begin(512);

  // WiFi.mode(WIFI_STA);
  // WiFi.setSleep(false);
  // WiFi.softAP(ssid, password);

  // while (WiFi.status()!=WL_CONNECTED)
  // {
  //   delay(200);
  //   Serial.println(".");
  // }
  // Serial.println("ID Address:");
  // Serial.println(WiFi.localIP());

  // server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
  //           { request->send_P(200, "text/plain", "Hello World!"); });
  // // server.on("/", HTTP_GET, handleRoot);

  // server.begin();

  // Serial.println("Server Started");

  // Setting the ESP as an access point
  Serial.println("Setting AP (Access Point)...");
  // Remove the password parameter, if you want the AP (Access Point) to be open
  // WiFi.softAP(ssid, password);
  String ID = ReadInfoEEPROM(0, 32);
  if (Contains(ID, "??") || ID == "")
  {
    server_ID = "GFI-ESP32-Access-Point";
    WiFi.softAP(server_ID.c_str(), password);
  }
  else
  {
    server_ID = ID;
  }

  String PW = ReadInfoEEPROM(32, 32);
  if (Contains(PW, "??"))
  {
    server_Password = "22101782";
  }
  else
  {
    server_Password = PW;
  }

  Serial.println("Server ID: " + String(server_ID));
  Serial.println("Server Password: " + String(server_Password));
  WiFi.softAP(server_ID.c_str(), server_Password.c_str());

  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);

  server.on("/Data", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send_P(200, "text/plain", WIFIreadData().c_str()); });

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            {
              if (request->hasHeader("Data"))
              {
                AsyncWebHeader *h = request->getHeader("Data");

                if (request->hasHeader("Station"))
                {
                  AsyncWebHeader *hh = request->getHeader("Station");
                  Serial.print(hh->value().c_str());
                  Serial.print("|||");
                  Serial.println(h->value().c_str());
                }
                else
                  Serial.println(h->value().c_str());
              }

              // int headers = request->headers();
              // for (int j = 0; j<headers; j++)
              // {
              //   AsyncWebHeader* h=request->getHeader(j);

              //   Serial.printf("HEADER[%s]: %s\n", h->name().c_str(), h->value().c_str());
              // }

              request->send(200, "server", "msg received");
            });

  // server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){

  //   int paramsNr = request->params();

  //   Serial.println("Version:" + request->version());
  //   Serial.println("method:" + request->method());
  //   Serial.println("url:" + request->url());
  //   Serial.println("host:" + request->host());
  //   Serial.println("contentType:" + request->contentType());
  //   Serial.println("contentLength:" + request->contentLength());
  //   Serial.println("multipart:" + request->multipart());

  //   int headers = request->headers();
  //   for (int j = 0; j<headers; j++)
  //   {
  //     AsyncWebHeader* h=request->getHeader(j);
  //     Serial.printf("HEADER[%s]: %s\n", h->name().c_str(), h->value().c_str());
  //   }

  //   for(int i=0;i<paramsNr;i++){

  //       AsyncWebParameter* p = request->getParam(i);
  //       // Serial.print("Stage: ");
  //       // Serial.println(p->name());
  //       // Serial.print("Data: ");
  //       Serial.println(p->value());
  //       // Serial.println("------");
  //   }

  //   request->send(200, "text/plain", "get msg received");
  // });

  // Start server
  server.begin();
}

unsigned long previousMillis = 0;
const long interval = 300;

void loop()
{
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval)
  {
    String cmd = "";
    if (Serial.available())
    {
      cmd = Serial.readString();

      if (cmd != "")
      {
        cmd.trim();

        Serial.println("Get cmd: " + cmd);

        if (Contains(cmd, "ID#"))
        {
          cmd.remove(0, 3);
          Serial.println("Set Server ID: " + WR_EEPROM(0, cmd));
        }
        else if (Contains(cmd, "PW#"))
        {
          cmd.remove(0, 3);
          Serial.println("Set Server Password: " + WR_EEPROM(32, cmd));
        }
        else if (cmd == "ID?")
        {
          Serial.println(ReadInfoEEPROM(0, 32));
        }
        else if (cmd == "PW?")
        {
          Serial.println(ReadInfoEEPROM(32, 32));
        }
      }
    }

    previousMillis = currentMillis;
  }
}

//------------------------------------------------------------------------------------------------------------------------------------------

bool Contains(String text, String search)
{
  if (text.indexOf(search) == -1)
    return false;
  else
    return true;
}