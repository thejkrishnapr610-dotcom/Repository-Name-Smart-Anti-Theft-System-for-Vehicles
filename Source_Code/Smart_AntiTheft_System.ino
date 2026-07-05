#include <Wire.h>
#include <Keypad.h>
#include <SPI.h>
#include <U8g2lib.h>
#include <MPU6050.h>

MPU6050 mpu;

// ---------- OLED ----------
U8G2_SSD1306_128X64_NONAME_F_4W_HW_SPI display(
U8G2_R0,
18,   // CLK
19,   // MOSI
21,   // CS
20,   // DC
22);  // RESET

// ---------- Keypad ----------
const byte ROWS = 4;
const byte COLS = 3;

char keys[ROWS][COLS] = {
{'1','2','3'},
{'4','5','6'},
{'7','8','9'},
{'*','0','#'}
};

byte rowPins[ROWS] = {10,11,12,13};
byte colPins[COLS] = {6,7,8};

Keypad keypad = Keypad(makeKeymap(keys),rowPins,colPins,ROWS,COLS);

// ---------- Pins ----------
#define RELAY_PIN 16
#define BUZZER_PIN 14
#define SYSTEM_LED 15
#define IGNITION_LED 18
#define SWITCH_PIN 9

String password="1234";
String entered="";

bool vehicleOn=false;
bool unlocked=false;

void setup()
{
Serial.begin(115200);

Wire.begin();
mpu.initialize();

pinMode(RELAY_PIN,OUTPUT);
pinMode(BUZZER_PIN,OUTPUT);
pinMode(SYSTEM_LED,OUTPUT);
pinMode(IGNITION_LED,OUTPUT);
pinMode(SWITCH_PIN,INPUT_PULLUP);

digitalWrite(RELAY_PIN,LOW);
digitalWrite(BUZZER_PIN,LOW);
digitalWrite(SYSTEM_LED,LOW);
digitalWrite(IGNITION_LED,LOW);

display.begin();
display.clearBuffer();
display.setFont(u8g2_font_ncenB08_tr);
display.drawStr(5,25,"SMART ANTI-THEFT");
display.sendBuffer();

delay(2000);
}

void loop()
{
vehicleOn=!digitalRead(SWITCH_PIN);

if(vehicleOn)
{
digitalWrite(SYSTEM_LED,HIGH);
passwordScreen();
}
else
{
digitalWrite(SYSTEM_LED,LOW);
digitalWrite(RELAY_PIN,LOW);
digitalWrite(IGNITION_LED,LOW);

motionDetection();
}
}

void passwordScreen()
{
display.clearBuffer();
display.drawStr(0,15,"Enter Password");
display.drawStr(0,35,entered.c_str());
display.sendBuffer();

char key=keypad.getKey();

if(key)
{
if(key=='#')
{
if(entered==password)
{
unlockVehicle();
}
else
{
wrongPassword();
}
entered="";
}
else if(key=='*')
{
entered="";
}
else
{
entered+=key;
}
}
}

void unlockVehicle()
{
unlocked=true;

digitalWrite(RELAY_PIN,HIGH);
digitalWrite(IGNITION_LED,HIGH);

display.clearBuffer();
display.drawStr(15,30,"ACCESS GRANTED");
display.sendBuffer();

Serial.println("Vehicle Started");

delay(2000);
}

void wrongPassword()
{
digitalWrite(RELAY_PIN,LOW);
digitalWrite(IGNITION_LED,LOW);

display.clearBuffer();
display.drawStr(10,30,"WRONG PASSWORD");
display.sendBuffer();

for(int i=0;i<5;i++)
{
digitalWrite(BUZZER_PIN,HIGH);
delay(200);
digitalWrite(BUZZER_PIN,LOW);
delay(200);
}

sendSMS();
}

void motionDetection()
{
int16_t ax,ay,az;
int16_t gx,gy,gz;

mpu.getMotion6(&ax,&ay,&az,&gx,&gy,&gz);

if(abs(ax)>15000 || abs(ay)>15000)
{
display.clearBuffer();
display.drawStr(10,30,"THEFT ALERT");
display.sendBuffer();

digitalWrite(BUZZER_PIN,HIGH);

sendSMS();

delay(3000);

digitalWrite(BUZZER_PIN,LOW);
}
}

void sendSMS()
{
Serial.println("AT");
delay(1000);

Serial.println("AT+CMGF=1");
delay(1000);

Serial.println("AT+CMGS=\"+91XXXXXXXXXX\"");
delay(1000);

Serial.println("Alert! Vehicle Theft Detected.");
delay(1000);

Serial.write(26);

Serial.println("SMS Sent");
}
