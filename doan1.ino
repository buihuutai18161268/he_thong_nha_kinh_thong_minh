#include <SimpleDHT.h>
#include "MQ135.h"  
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd (0x27, 16,2); // Tạo đối tượng LCD và chỉ định các chân


#define pin_analog 33 // độ ẩm đất
#define pin_relay 19 // máy bơm
#define pin_relay1 18 // quạt

#define  cambien  35 // khai báo chân digital  cho cảm biến ánh sáng

//2 chan set chieu quay, hoat dong dung hay quay cua dong co ( trang 342 sach arduino), chan co xung PWM(chan dau ~)
#define IN1 27
#define IN2 26

// cảm biến mưa
#define rainSensor  34
int TT_maiche ;
int TT_tudong ;
int TT_quat;
int TT_maybom ;

// DHT11
int pinDHT11 = 4;
SimpleDHT11 dht11;
byte temperature = 0;
byte humidity = 0;

// MQ-135
#define PIN_MQ135 36   //Khai báo pin nối với chân AO
MQ135 mq135_sensor = MQ135(PIN_MQ135);   //Khai báo đối tượng thư viện
float ppm;
int do_am_dat;

//app blynk
#define BLYNK_PRINT Serial
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

char auth[] = "R4txN3R4EID5h4MWHI0rz8DFMbCOkeeU";
char ssid[] = "iPhone";
char pass[] = "taodeptraikhong?";
WidgetLED led1(V1);

BlynkTimer timer;
// V1 LED Widget is blinking
// day du lieu len app blynk
void blinkLedWidget() // kiem tra xem co ket noi voi blynk chua
{
  if (led1.getValue()) {
    led1.off();
    Serial.println("LED on V1: off");
  } else {
    led1.on();
    Serial.println("LED on V1: on");
  }
}

// máy bơm
BLYNK_WRITE(V0)
{
    int pinvalue0 = param.asInt();
    // String i = param.asStr();
    // double d = param.asDouble();
    if(pinvalue0 == 1)
    {
      TT_maybom = 1;
    }
    else
    {
      TT_maybom = 0;
    }
}

// quạt
BLYNK_WRITE(V2)
{
    int pinvalue1 = param.asInt();
    // String i = param.asStr();
    // double d = param.asDouble();
    if( pinvalue1 == 1)
    {
      TT_quat = 1;
    }
    else
    {
      TT_quat = 0;
    }
    // gui ve app blynk
    // Blynk.virtualWrite(V2,pinvalue);
}

// mái che
BLYNK_WRITE(V3)
{
    int pinvalue2 = param.asInt();
    Serial.println(pinvalue2);
    // String i = param.asStr();
    // double d = param.asDouble();
    if ( pinvalue2 == 0)
    {
      TT_maiche = 0;
    }
    else
    {
       TT_maiche = 1;
    }
    // gui ve app blynk
    // Blynk.virtualWrite(V2,pinvalue);
}

// chế độ tự động
BLYNK_WRITE(V7)
{
    int pinvalue7 = param.asInt();
    // String i = param.asStr();
    // double d = param.asDouble();
    if ( pinvalue7 == 1)
    {
       TT_tudong = 1;
    }
    else
    {
       TT_tudong = 0;
    }
}

// DHT- nhiet do do am
void sendSensor()
{
  // You can send any value at any time.
  // Please don't send more that 10 values per second.
   Blynk.virtualWrite(V5, humidity);
   Blynk.virtualWrite(V4, temperature);
}


void sendSensor1()
{
   float ppm = mq135_sensor.getPPM()/100;
   Blynk.virtualWrite(V6, ppm);
}

void setup() 
{
  Serial.begin(9600);//Mở cổng Serial ở mức 960
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
  delay(500);
  Serial.print("dang ket noi....");
  }

  Serial.println("WiFi da ket noi");  

  Blynk.begin(auth, ssid, pass);
  timer.setInterval(1000L, blinkLedWidget);
  timer.setInterval(1000L, sendSensor);
  timer.setInterval(1000L, sendSensor1);
  
  // MQ135
  pinMode (PIN_MQ135, INPUT);
  // máy bơm
  pinMode (pin_analog, INPUT);
  pinMode (pin_relay, OUTPUT);
  // quạt 
  pinMode (pin_relay1, OUTPUT);
  // cảm biến ánh sáng
  pinMode(cambien,INPUT);//pinMode nhận tín hiệu đầu vào cho cảm biên ánh sáng
  // cảm biến mưa
  pinMode(rainSensor,INPUT);
  // mái che
  pinMode(IN1,OUTPUT);
  pinMode(IN2,OUTPUT);
  // LCD_I2C
  lcd.init(); // LCD 16x2
  lcd.backlight();
}

void loop() 
{
  // DHT11
  readDHT11();
  //MQ-135
  readMQ135();
  // quạt
  if (TT_tudong == 1)
  {
    float ppm = mq135_sensor.getPPM()/100;
    if(int(ppm) > 20)
    {
        digitalWrite(pin_relay1, LOW); // kích hoạt relay
    }
    else
    {
      digitalWrite(pin_relay1, HIGH); 
    }
  }
  else
  {
    if(TT_quat == 1)
    {
      digitalWrite(pin_relay1, LOW); // kích hoạt relay
    }
    else
    {
      digitalWrite(pin_relay1, HIGH);
    }
  }
  
  // cảm biên ánh sáng
  int value1 = analogRead(cambien);//lưu giá trị cảm biến vào biến value1
  Serial.println(value1);
  
  // cảm biên mưa
   int value = analogRead(rainSensor);//Đọc tín hiệu cảm biến mưa
   Serial.println(value);
   if( TT_tudong == 1)
   {
     if(value > 1800)
     {
         Serial.println("Dang khong mua"); // mở
         if(temperature > 32)
         {
             digitalWrite(IN1, HIGH);
             digitalWrite(IN2, LOW);
         }
         else
         {
            if(value1 > 500)
            {
               digitalWrite(IN1, HIGH);
               digitalWrite(IN2, LOW);
            }
            else
            {
               digitalWrite(IN1, LOW);
               digitalWrite(IN2, HIGH);;
            }
         }
     }
   else
   {
       Serial.println("Dang mua");
       digitalWrite(IN1, LOW);
       digitalWrite(IN2, HIGH);
   }
   }
   else 
   {
      if(TT_maiche == 1)
      {
           digitalWrite(IN1, HIGH);
           digitalWrite(IN2, LOW);
      }
      else
      {
           digitalWrite(IN1, LOW);
           digitalWrite(IN2, HIGH);
      }
   }
  // máy bơm
  int do_am_dat = analogRead(pin_analog); 
  Serial.println(do_am_dat);
  lcd.setCursor(10, 0);
  lcd.print("d=");
  lcd.setCursor(12,0);
  lcd.print(do_am_dat);
  if (TT_tudong == 1)
  {
    if(do_am_dat > 3500)    // Đất khô
    {
      digitalWrite(pin_relay, LOW);  // kích hoạt rờ le
    }
    else // Đất đủ dộ ẩm
    {
      digitalWrite(pin_relay, HIGH);  // tắt rờ le
    }
  }
  else
  {
    if(TT_maybom == 1)
    {
      digitalWrite(pin_relay, LOW); // kích hoạt rờ le
    }
    else
    {
       digitalWrite(pin_relay, HIGH);
    }
  }

  // BLYNK
  Blynk.run();
  timer.run();
  delay(1000);
}

void readDHT11()
{
  // start working...
  Serial.println("=================================");
  Serial.println("Sample DHT11...");
  lcd.setCursor(0, 1);
  lcd.print("T=");
  lcd.setCursor(2, 1);
  lcd.print(temperature);
  lcd.setCursor(4, 1);
  lcd.print("o");
  lcd.setCursor(5, 1);
  lcd.print("C");
  lcd.setCursor(6, 1);
  lcd.print("    ");
  lcd.setCursor(10, 1);
  lcd.print("H=");
  lcd.setCursor(12, 1);
  lcd.print(humidity);
  lcd.setCursor(14, 1);
  lcd.print("%");
  lcd.setCursor(15, 1);
  lcd.print(" ");

  // read without samples.
  if (dht11.read(pinDHT11, &temperature, &humidity, NULL)) {
    Serial.print("Read DHT11 failed.");
  }  
}
void readMQ135() {
  float ppm = mq135_sensor.getPPM()/100;  //Đọc giá trị ppm
  Serial.print("PPM: ");
  Serial.print(ppm);
  Serial.println("ppm");
  lcd.setCursor(0, 0);
  lcd.print("p=");
  lcd.setCursor(2, 0);
  lcd.print(ppm);
  lcd.setCursor(6, 0);
  lcd.print("    ");
}
