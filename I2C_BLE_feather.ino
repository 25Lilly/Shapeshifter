#include <bluefruit.h>
#include <Wire.h>

#define I2C_ADDRS_AMT 15
#define PRESET_AMT 4


int I2C_ADDRS[I2C_ADDRS_AMT] {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F};
int PRESET_LINE[I2C_ADDRS_AMT] {90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90};
//Square angles and order
int PRESET_SQUARE[I2C_ADDRS_AMT] {270, 270, 270, 270, 270, 270, 270, 270, -1, -1, -1, -1, -1, -1, -1};
int SQUARE_I2C_ADDRS[I2C_ADDRS_AMT] {0x02, 0x05, 0x0F, 0x0C, 0x08, 0x06, 0x09, 0x0B, 0x01, 0x03, 0x04, 0x07, 0x0A, 0x0D, 0x0E};
//Bird angles and order
int PRESET_BIRD[I2C_ADDRS_AMT] {270, 270, 0, 0, 0, 0, 0, 0, 270, 270, 180, 0, 0, 180, -1}; 
int BIRD_I2C_ADDRS[I2C_ADDRS_AMT] {0x01, 0x0F, 0x02, 0x0E, 0x03, 0x0D, 0x04, 0x0C, 0x05, 0x0B, 0x06, 0x07, 0x09, 0x0A, 0x08};
//Cobra angles and order
int PRESET_COBRA[I2C_ADDRS_AMT]{180, 0, 0, 180, 0, 180, 180, 0, 180, 0, 0, 0, 270, -1, -1};
int COBRA_I2C_ADDRS[I2C_ADDRS_AMT]{0x01, 0x02, 0x03, 0x04, 0x05, 0x07, 0x06, 0x08, 0x09, 0x0B, 0x0A, 0x0C, 0x0D, 0x0E, 0x0F};
//Giraffe angles and order- IF THE UPLOAD DOESN'T WORK 
int PRESET_GIRAFFE[I2C_ADDRS_AMT] {270, 270, 270, 270, 270, 270, 270, 270, -1, -1, -1, -1, -1, -1, -1};
int GIRAFFE_I2C_ADDRS[I2C_ADDRS_AMT] {0x02, 0x05, 0x0F, 0x0C, 0x08, 0x06, 0x09, 0x0B, 0x01, 0x03, 0x04, 0x07, 0x0A, 0x0D, 0x0E};

//int PRESET_HEART[I2C_ADDRS_AMT] {270, 270, 90, 90, 270, 90, 90, 270, 270}; // For 10 servos
//int PRESET_ZRO[I2C_ADDRS_AMT] {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}; // For 10 servos
//int PRESET_MAX[I2C_ADDRS_AMT] {270, 270, 270, 270, 270, 270, 270, 270, 270, 270}; // For 10 servos
int* PRESETS[PRESET_AMT] {PRESET_LINE, PRESET_SQUARE, PRESET_COBRA, PRESET_BIRD};
int* PRESETS_ADDRS[PRESET_AMT] {I2C_ADDRS, SQUARE_I2C_ADDRS, COBRA_I2C_ADDRS, BIRD_I2C_ADDRS};
int BTN_ANGLES[4] {0, 180, 270, 90}; //5 -> 0, 6 -> 180, 7 -> 270, 8 -> 90

int last_preset = 0;
String inString = "";  // string to hold input
int current_addr = 0;

// OTA DFU service
BLEDfu bledfu;

// Uart over BLE service
BLEUart bleuart;

// Function prototypes for packetparser.cpp
uint8_t readPacket (BLEUart *ble_uart, uint16_t timeout);
float   parsefloat (uint8_t *buffer);
void    printHex   (const uint8_t * data, const uint32_t numBytes);

// Packet buffer
extern uint8_t packetbuffer[];

void setup(void)
{
  Serial.begin(115200);
  while ( !Serial ) delay(10);   // for nrf52840 with native usb

//  Serial.println(F("Using Adafruit Bluefruit52 Controller App"));
//  Serial.println(F("-------------------------------------------"));

  Bluefruit.begin();
  Bluefruit.setTxPower(4);    // Check bluefruit.h for supported values

  // To be consistent OTA DFU should be added first if it exists
  bledfu.begin();

  // Configure and start the BLE Uart service
  bleuart.begin();

  // Set up and start advertising
  startAdv();

  Serial.println(F("Please use Adafruit Bluefruit LE app to connect in Controller mode"));
  Serial.println(F("Then use the game controller to control individual servos"));
  Serial.println();

  Wire.begin(); // join i2c bus (address optional for master)
  Serial.print("Transmitting to: 0X");
  Serial.println(I2C_ADDRS[current_addr], HEX);
}

void startAdv(void)
{
  // Advertising packet
  Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
  Bluefruit.Advertising.addTxPower();

  // Include the BLE UART (AKA 'NUS') 128-bit UUID
  Bluefruit.Advertising.addService(bleuart);

  // Secondary Scan Response packet (optional)
  // Since there is no room for 'Name' in Advertising packet
  Bluefruit.ScanResponse.addName();

  /* Start Advertising
     - Enable auto advertising if disconnected
     - Interval:  fast mode = 20 ms, slow mode = 152.5 ms
     - Timeout for fast mode is 30 seconds
     - Start(timeout) with timeout = 0 will advertise forever (until connected)

     For recommended advertising interval
     https://developer.apple.com/library/content/qa/qa1931/_index.html
  */
  Bluefruit.Advertising.restartOnDisconnect(true);
  Bluefruit.Advertising.setInterval(32, 244);    // in unit of 0.625 ms
  Bluefruit.Advertising.setFastTimeout(30);      // number of seconds in fast mode
  Bluefruit.Advertising.start(0);                // 0 = Don't stop advertising after n seconds
}

/**************************************************************************/
/*!
    @brief  Constantly poll for new command or response data
*/
/**************************************************************************/
void loop(void)
{
  // Wait for new data to arrive
  uint8_t len = readPacket(&bleuart, 500);
  if (len == 0) return;

  // Got a packet!
  // printHex(packetbuffer, len);

  // Buttons
  if (packetbuffer[1] == 'B') {
    uint8_t buttnum = packetbuffer[2] - '0';
    boolean pressed = packetbuffer[3] - '0';
    if (pressed) {
      Serial.print ("Button "); Serial.print(buttnum);
      Serial.println(" pressed");
      button_parse(buttnum);
    } else {
      //Serial.println(" released");
    }
  }

  if (packetbuffer[1] == 'S') {
    char servo = packetbuffer[2];
    Serial.print("recieved address ");
    Serial.println(servo);
    if (hex_val(servo) - 1 < I2C_ADDRS[I2C_ADDRS_AMT - 1]) {
      current_addr = hex_val(servo) - 1;
      Serial.print("Address set to: 0x");
      Serial.println(I2C_ADDRS[current_addr], HEX);
    }

  }
}

byte sendAngle(int angle, int i2c_addr) {
  if (angle < 0) {
    return 5;
  }
  byte send_angle = angle / 2;
  Wire.beginTransmission(i2c_addr); // transmit to device #I2C_S1_ADDR
  Wire.write(send_angle);              // sends one byte
  Serial.println("Sent");
  byte test = Wire.endTransmission();    // stop transmitting
  if (!test) {
    Serial.print("Sent ");
    Serial.print(send_angle);
    Serial.print(" to 0x");
    Serial.println(i2c_addr, HEX);
  } else {
    Serial.print("Unabe to send ");
    Serial.print(send_angle * 2);
    Serial.print(" to 0x");
    Serial.println(i2c_addr, HEX);
  }
  return test;
}

void button_parse(uint8_t button_num) {
  if (button_num > 4 && button_num < 9) {
    button_num -= 5;
    int angle = BTN_ANGLES[button_num];
    Serial.print("Attempting send to 0x");
    Serial.print(I2C_ADDRS[current_addr], HEX);
    Serial.print(" angle: ");
    Serial.println(angle);
    byte send_result = sendAngle(angle, I2C_ADDRS[current_addr]);
    Serial.println(send_result);
  } else if (button_num == 1) {
    current_addr ++;
    if (current_addr > I2C_ADDRS_AMT - 1) { //if the current address is greater than the amount of i2c addresses
      current_addr = 0;
    }
    Serial.print("Address set to: 0x");
    Serial.println(I2C_ADDRS[current_addr]);
  } else if ( (button_num > 1 && button_num < 5) || button_num > 8) {
    if (button_num > 8){
      button_num = button_num - 4; 
    }
    int* preset = PRESETS[button_num - 2];
    int* preset_addrs = PRESETS_ADDRS[button_num - 2];
    go_to_preset(preset, preset_addrs);
    last_preset = button_num - 2;
    //Print the preset:
    //    Serial.print("[");
    //    for (int i = 0; i < I2C_ADDRS_AMT; i ++) {
    //      Serial.print(preset[i]);
    //      Serial.print(" ");
    //    }
    //    Serial.println("] ");

  }
}

void go_to_preset(int* preset, int* preset_addrs) {
  Serial.println("Going to preset");
  //going back to line from the last preset
  int* last_addrs = PRESETS_ADDRS[last_preset];
  for (int i = I2C_ADDRS_AMT - 1; i > -1; i--) {
    sendAngle(90, last_addrs[i]);
    delay(100); //delay to send twice (in case of unsuccessful send)
    if (!sendAngle(90, last_addrs[i])) {
      delay(1000);
    }
  }

  for (int i = 0; i < I2C_ADDRS_AMT; i++) {
    sendAngle(preset[i], preset_addrs[i]);
    delay(100); //delay to send twice (in case of unsuccessful send)
    if (!sendAngle(preset[i], preset_addrs[i])) {
      delay(3000);
    }
  }
}

byte hex_val (char inchar) {
  byte outval = 0;
  if (inchar >= '0' && inchar <= '9') {
    outval = (inchar - '0');
  }
  else if (inchar >= 'A' && inchar <= 'F') {
    outval = (inchar - 'A' + 10);
  }
  return outval;
}
