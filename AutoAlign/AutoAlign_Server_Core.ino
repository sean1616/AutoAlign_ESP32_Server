#include <EEPROM.h>
#include <esp_now.h>
#include <WiFi.h>

//------------------------------------------------------------------------------------------------------------------------------------------

#pragma region ESP_Now

uint8_t broadcastAddress_A[] = {0x8C, 0x4B, 0x14, 0x14, 0x57, 0x08};  //Client Mac address
uint8_t broadcastAddress_B[] = {0x8C, 0x4B, 0x14, 0x16, 0x37, 0xF8};  //Client Mac address

String Msg, Msg_Value;

typedef struct struct_message {
    String client_name;
    char msg[50];
    // char value[20];
} struct_message;

// Create a struct_message to hold incoming sensor readings
struct_message incomingReadings;

// struct_receive_msg_UI_Data sendmsg_UI_Data;

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  status == ESP_NOW_SEND_SUCCESS ;
  //
  if(status == 0)
  {
    // Serial.println("OK");
  }
}

String client_Name = "", client_msg = "";

void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  // String ss = "";
  // ss.toCharArray(incomingReadings.msg, 30);
  memcpy(&incomingReadings, incomingData, sizeof(incomingReadings));
  Msg = incomingReadings.msg;

  if( Msg != "")
  {
    client_Name = incomingReadings.client_name;
    client_msg = Msg;

    Serial.println(client_Name + "|||" + client_msg);
  } 
}

//------------------------------------------------------------------------------------------------------------------------------------------

void ESP_Now_Initialize()
{
   // 初始化 ESP-NOW
  WiFi.mode(WIFI_STA);
  if (esp_now_init() != 0) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  else
    Serial.println("Initializing ESP-NOW");

  Serial.print("ESP32 Board MAC Address:  ");
  Serial.println(WiFi.macAddress());
 
  // 绑定數據接收端
  esp_now_peer_info_t peerInfo;
  memset(&peerInfo, 0, sizeof(peerInfo));  //initialize peer if esp32 library version is 2.0.1 (no need in version 1.0.6)
  memcpy(peerInfo.peer_addr, broadcastAddress_B, 6); // Register peer
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  // Add peer 增加一個PEER到名單列
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }
  
  // 設定發送數據時CALLBACK的函式名稱。 通過ESP-NOW傳送數據後，將被呼叫通知是否傳送成功。
  // Once ESPNow is successfully Init, we will register for Send CB to get the status of Trasnmitted packet
  esp_now_register_send_cb(OnDataSent);

  //設定收到數據時CALLBACK的函式名稱。 通過ESP-NOW接收到數據後，將被呼叫以便進一步處理資料。
  // Register for a callback function that will be called when data is received
  esp_now_register_recv_cb(OnDataRecv);  
}

#pragma endregion

//------------------------------------------------------------------------------------------------------------------------------------------
TaskHandle_t Task_1;

void Task_1_sendData(void *pvParameters)
{
  while (true)
  {
    //Call ESP-Now receive data function
    esp_now_register_recv_cb(OnDataRecv);

    //Task1休息，不可省略
    delay(10);
  }
}

//------------------------------------------------------------------------------------------------------------------------------------------

void setup()
{
  Serial.begin(115200);
  Serial.setTimeout(20); //設定序列埠接收資料時的最大等待時間

  //宣告使用EEPROM 512 個位置
  EEPROM.begin(512);

    // 取得本機的MACAddress
  // WiFi.mode(WIFI_MODE_STA);
  // Serial.print("ESP32 Board MAC Address:  ");
  // Serial.println(WiFi.macAddress());

  ESP_Now_Initialize();

  // 在core 0啟動 mision 1
  // xTaskCreatePinnedToCore(
  //     Task_1_sendData, /* 任務實際對應的Function */
  //     "Task_1",        /* 任務名稱 */
  //     10000,           /* 堆疊空間 */
  //     NULL,            /* 無輸入值 */
  //     12,               /* 優先序0(0為最低) */
  //     &Task_1,         /* 對應的任務變數位址 */
  //     0);              /*指定在核心0執行 */
}

unsigned long previousMillis = 0;
const long interval = 300;

void loop()
{
  unsigned long currentMillis = millis();

  // delay(1000);

  if (currentMillis - previousMillis >= interval && false)
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


bool Contains(String text, String search)
{
  if (text.indexOf(search) == -1)
    return false;
  else
    return true;
}