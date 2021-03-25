#include <Wire.h>

# define I2C_SLAVE_ADDRESS 11 // 12 pour l'esclave 2 et ainsi de suite

/*---------------------- Display --------------------------*/
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET 7
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

/*--------------------- Clocks ---------------------------*/
// initial Time display is 12:59:45 PM
int h = 00;
int m = 00;
int s = 00;

bool is_loading = true;
byte sync_frame = 0;
#define SYNC_FRAMES 4

// For accurate Time reading, use Arduino Real Time Clock
static uint32_t last_time, now = 0; // RTC

/*--------------------- EEPROM ---------------------------*/
#include <EEPROM.h>
#define H_ADDR (0)
#define M_ADDR (1)
#define S_ADDR (2)

void setup()
{
  Serial.begin(9600);

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // инициализация дисплея по интерфейсу I2C, адрес 0x3C (для OLED 128x32)
  display.clearDisplay(); // очистка дисплея
  display.setTextColor(WHITE); // установка цвета текста

  Wire.begin(I2C_SLAVE_ADDRESS);
  Serial.println("------------------------------------- I am Slave1");
  delay(1000);
  Wire.onRequest(requestEvents);
  Wire.onReceive(receiveEvents);

  now = millis(); // read RTC initial value

  h = EEPROM.read(H_ADDR);
  m = EEPROM.read(M_ADDR);
  s = EEPROM.read(S_ADDR);
}

void loop() {
  display.clearDisplay(); // очистить дисплей
  display.setTextSize(1); // установка размера шрифта
  display.setCursor(7, 2); // установка курсора в позицию X = 0; Y = 0
  display.println("Cyber Culture Motors"); // знак цельсия

  if (is_loading)
  {
    display.setCursor(0, 25); // установка курсора в позицию X = 0; Y = 0
    display.setTextSize(3); // установка размера шрифта
    display.print("Sync");
    sync_frame++;
    if (sync_frame >= SYNC_FRAMES)
    {
      sync_frame = 0;
    }
    for ( int i = 0 ; i < sync_frame; i++) // make 5 time 200ms loop, for faster Button response
    {
      display.print(".");
    }
    delay(200);
  }
  else
  {
    display.setCursor(6, 25); // установка курсора в позицию X = 0; Y = 0
    display.setTextSize(4); // установка размера шрифта
    // Print TIME
    if (h < 10) display.print("0"); // always 2 digits
    display.print(h);
    display.print(":");
    if (m < 10) display.print("0");
    display.print(m);
    //  display.print(":");
    //  if (s < 10) display.print("0");
    //  display.println(s);


    for ( int i = 0 ; i < 5 ; i++) // make 5 time 200ms loop, for faster Button response
    {
      while ((now - last_time) < 200) //delay200ms
      {
        now = millis();
      }
      // inner 200ms loop
      last_time = now; // prepare for next loop
    }

    s = s + 1; //increment sec. counting

    if (s == 60) {
      s = 0;
      m = m + 1;
    }
    if (m == 60)
    {
      m = 0;
      h = h + 1;
    }
    if (h == 24)
    {
      h = 0;
    }
  }

  display.display(); // всё это отображаем на экране
}

byte responce[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

void requestEvents()
{
  Serial.println(F("---> recieved request"));
  Serial.print(F("sending value : "));
  for (int i = 0; i < sizeof(responce); i++) {
    Serial.print(responce[i]);
    Wire.write(responce[i]);
  }
}

void receiveEvents(int numBytes)
{
  Serial.println(F("---> recieved events"));
  int i = 0;
  byte c = Wire.read(); // receive byte as a character
  while (Wire.available()) { // loop through all but the last
    byte c = Wire.read(); // receive byte as a character
    responce[i] = c;
    i++;
    if (i >= sizeof(responce)) break;
  }

  parse_command(responce, numBytes - 1);
}

void parse_command(byte* command, int command_size)
{
  int int_array_size = command_size / 4;
  int int_command[4] = {0, 0, 0, 0};
  convert_bytes_to_ints(command, int_command, command_size);
  int cmd = int_command[0];
  Serial.println(cmd);
  switch (cmd)
  {
    case 1:
      Serial.println("Update time");
      is_loading = false;

      h = int_command[1];
      m = int_command[2];
      s = int_command[3];

      EEPROM.write(H_ADDR, h);
      EEPROM.write(M_ADDR, m);
      EEPROM.write(S_ADDR, s);
      break;
  }
  for (int i = 1; i < 4; i++) {
    Serial.print(int_command[i]);
    Serial.print(" ");
  }
  Serial.println();
}

void convert_bytes_to_ints(byte* byte_array, int* int_array, int byte_array_size)
{
  int int_array_size = byte_array_size / 4;
  for (int i = 0; i < int_array_size; i++) {
    int_array[i] = byte_array[i * int_array_size + 0] + (byte_array[i * int_array_size + 1] << 8) + (byte_array[i * int_array_size + 2] << 16) + (byte_array[i * int_array_size + 3] << 24);
    //    int_array[i] = (byte_array[i * int_array_size + 0] << 24) + (byte_array[i * int_array_size + 1] << 16) + (byte_array[i * int_array_size + 2] << 8) + byte_array[i * int_array_size + 3];
  }
}
