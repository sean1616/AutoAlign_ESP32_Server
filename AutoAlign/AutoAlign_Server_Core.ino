#include <EEPROM.h>
#include <esp_now.h>
#include <WiFi.h>
// #include <ESPAsyncWebServer.h>

//------------------------------------------------------------------------------------------------------------------------------------------

#pragma region ESP_Now

// REPLACE WITH THE MAC Address of your receiver
uint8_t broadcastAddress_A[] = {0x8C, 0x4B, 0x14, 0x14, 0x57, 0x08};  //Client Mac address
uint8_t broadcastAddress_B[] = {0x8C, 0x4B, 0x14, 0x16, 0x37, 0xF8};  //Client Mac address

String Msg, Msg_Value;

typedef struct struct_message {
    String client_name;
    char msg[30];
    // char value[20];
} struct_message;

// typedef struct struct_receive_msg_UI_Data {
//     String msg;
//     double _Target_IL;
//     int _Q_Z_offset;
//     double _ref_Dac;
//     int _speed_x;
//     int _speed_y;
//     int _speed_z;
//     int _QT;
// } struct_receive_msg_UI_Data;

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

// void DataSent_Controller(String MSG)
// {
//   sendmsg_UI_Data.msg = "Core:" + MSG;
 
//   esp_err_t result = esp_now_send(broadcastAddress_A, (uint8_t *) &sendmsg_UI_Data, sizeof(sendmsg_UI_Data));
// }

String client_A_Name = "", client_A_msg = "";

void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  String ss = "";
  ss.toCharArray(incomingReadings.msg, 30);
  memcpy(&incomingReadings, incomingData, sizeof(incomingReadings));
  Msg = incomingReadings.msg;

  if( Msg != "")
  {
    client_A_Name = incomingReadings.client_name;
    client_A_msg = Msg;

    Serial.println(client_A_Name + "|||" + client_A_msg);
  } 
}

// void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
//   String ss = "";
//   ss.toCharArray(incomingReadings.cmd, 30);
//   ss.toCharArray(incomingReadings.value, 20);
//   memcpy(&incomingReadings, incomingData, sizeof(incomingReadings));
//   Msg = incomingReadings.cmd;
//   Msg_Value = incomingReadings.value;


//   if( Msg != "")
//   {
//     name_from_contr = incomingReadings.contr_name;
//     cmd_from_contr = Msg;

//     if( Msg_Value != "")
//     {
//       cmd_value_from_contr = Msg_Value;
//     }
//     else
//       Serial.println(incomingReadings.contr_name + Msg);
//   } 
// }

//------------------------------------------------------------------------------------------------------------------------------------------

void ESP_Now_Initialize()
{
   // ????????? ESP-NOW
  WiFi.mode(WIFI_STA);
  if (esp_now_init() != 0) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  else
    Serial.println("Initializing ESP-NOW");
 
  // ?????????????????????
  esp_now_peer_info_t peerInfo;
  memcpy(peerInfo.peer_addr, broadcastAddress_B, 6); // Register peer
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  // // Add peer ????????????PEER????????????
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }
  
  // ?????????????????????CALLBACK?????????????????? ??????ESP-NOW?????????????????????????????????????????????????????????
  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_register_send_cb(OnDataSent);

  //?????????????????????CALLBACK?????????????????? ??????ESP-NOW???????????????????????????????????????????????????????????????
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

    //Task1?????????????????????
    delay(10);
  }
}

//------------------------------------------------------------------------------------------------------------------------------------------

void setup()
{
  Serial.begin(115200);
  Serial.setTimeout(20); //???????????????????????????????????????????????????

  //????????????EEPROM 512 ?????????
  EEPROM.begin(512);

    // ???????????????MACAddress
  // WiFi.mode(WIFI_MODE_STA);
  // Serial.print("ESP32 Board MAC Address:  ");
  // Serial.println(WiFi.macAddress());

  ESP_Now_Initialize();

  // ???core 0?????? mision 1
  // xTaskCreatePinnedToCore(
  //     Task_1_sendData, /* ?????????????????????Function */
  //     "Task_1",        /* ???????????? */
  //     10000,           /* ???????????? */
  //     NULL,            /* ???????????? */
  //     12,               /* ?????????0(0?????????) */
  //     &Task_1,         /* ??????????????????????????? */
  //     0);              /*???????????????0?????? */

  return;
}

unsigned long previousMillis = 0;
const long interval = 300;

void loop()
{
  unsigned long currentMillis = millis();

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