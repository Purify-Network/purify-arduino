/*  ============================================================================================================

    Sample code to create a setup web portal using a soft access point on an ESP8266.

    For more info, please watch my video at https://youtu.be/1VW-z4ibMXI
    MrDIY.ca

     An example showing how to use a 4051 multiplexer with an esp8266
 to connect up to 8 analog sensors.

 Wiring:
 Wemos ->  4051
 ---------------
 D4    ->   S0 (A)
 D3    ->   S1 (B)
 D2    ->   S2 (C)
 A0    ->   Common
 3.3v  ->   VCC
 G     ->   GND
 G     ->   Inhibit
 G     ->   VEE  
 
 4051 Option pins are then wired to whatever Analog sensors required

 One thing to note: say for example if you only require 2 analog sensors,
 You can just wire up S0(A) and connect S1 & S2 to GND and you will be 
 able to switch between option 1 and option 2 pins.
 Same goes for up to 4 pins (just use S0 & S1)

  ============================================================================================================== */

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>
// #include "GravityTDS.h"

#define MUX_A D4
#define MUX_B D3
#define MUX_C D2
#define ANALOG_INPUT A0

ESP8266WebServer    server(80);

// const int adcPin = A0;

// GravityTDS gravityTds;
// float temperature = 25, tdsValue = 0;



#define VREF 5.0      // analog reference voltage(Volt) of the ADC
#define SCOUNT  30           // sum of sample point
int analogBuffer[SCOUNT];    // store the analog value in the array, read from ADC
int analogBufferTemp[SCOUNT];
int analogBufferIndex = 0,copyIndex = 0;
float averageVoltage = 0,tdsValue = 0,temperature = 25;

  
// calculate your own m using ph_calibrate.ino
// When using the buffer solution of pH4 for calibration, m can be derived as:
 // m = (pH7 - pH4) / (Vph7 - Vph4)
const float m = -5.436; 
float phGlobal = 0;


float calibration_value = 30.5;
int phval = 0; 
unsigned long int avgval; 
int buffer_arr[10],temp;


struct settings {
  char ssid[30];
  char password[30];
} user_wifi = {};

void setup() {

  //Deifne output pins for Mux
  pinMode(MUX_A, OUTPUT);
  pinMode(MUX_B, OUTPUT);     
  pinMode(MUX_C, OUTPUT);   

  Serial.begin(115200);

    // gravityTds.setPin(ANALOG_INPUT);
    // gravityTds.setAref(5.0);  //reference voltage on ADC, default 5.0V on Arduino UNO
    // gravityTds.setAdcRange(1024);  //1024 for 10bit ADC;4096 for 12bit ADC
    // gravityTds.begin();  //initialization

  EEPROM.begin(sizeof(struct settings) );
  EEPROM.get( 0, user_wifi );
   
  WiFi.mode(WIFI_STA);
  WiFi.begin(user_wifi.ssid, user_wifi.password);
  
  byte tries = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    if (tries++ > 30) {
      WiFi.mode(WIFI_AP);
      WiFi.softAP("Purify_01", "1234test");
      break;
    }
  }
  server.on("/",  handlePortal);
  server.begin();

  
}


void loop() {

      //  float Po = analogRead(adcPin) * 3.0 / 1024;
  //  Serial.print("po value: ");
  //  Serial.println(Po); 
  //  float phValue = 7 - (2.5 - Po) * m;

  float yy;
    changeMux(LOW, LOW, HIGH);
    yy = analogRead(ANALOG_INPUT);
    Serial.println(yy);
    for(int i = 0; i < 15; i++){
      calculateTDS();
    }
    float ii = calculateTDS();
    // gravityTds.setTemperature(temperature);  // set the temperature and execute temperature compensation
    // gravityTds.update();  //sample and calculate
    // tdsValue = gravityTds.getTdsValue();  // then get the value
    // Serial.print(tdsValue,0);
    // Serial.println("ppm");
    delay(1000);



 for(int i=0;i<10;i++) 
 { 

  float value;
  changeMux(LOW, LOW, LOW);
  value = analogRead(ANALOG_INPUT); //Value of the sensor connected Option 0 pin of Mux
  if(i==1)
    Serial.println(value);
  buffer_arr[i]=value;
  delay(30);
 }
 for(int i=0;i<9;i++)
 {
 for(int j=i+1;j<10;j++)
 {
 if(buffer_arr[i]>buffer_arr[j])
 {
 temp=buffer_arr[i];
 buffer_arr[i]=buffer_arr[j];
 buffer_arr[j]=temp;
 }
 }
 }
 avgval=0;
 for(int i=2;i<8;i++)
 avgval+=buffer_arr[i];
 float volt=(float)avgval*5.0/1024/6;
 float phValue = -5.70 * volt + calibration_value;


  
   Serial.print("p h value = "); 
   Serial.println(phValue);
   phGlobal = phValue;
   delay(1000);

  server.handleClient();
}

void handlePortal() {

  if (server.method() == HTTP_POST) {

    strncpy(user_wifi.ssid,     server.arg("ssid").c_str(),     sizeof(user_wifi.ssid) );
    strncpy(user_wifi.password, server.arg("password").c_str(), sizeof(user_wifi.password) );
    user_wifi.ssid[server.arg("ssid").length()] = user_wifi.password[server.arg("password").length()] = '\0';
    EEPROM.put(0, user_wifi);
    EEPROM.commit();

    server.send(200, "application/json", "{'hello': 'world'}");

    // server.send(200,   "text/html",  "<!doctype html><html lang='en'><head><meta charset='utf-8'><meta name='viewport' content='width=device-width, initial-scale=1'><title>Wifi Setup</title><style>*,::after,::before{box-sizing:border-box;}body{margin:0;font-family:'Segoe UI',Roboto,'Helvetica Neue',Arial,'Noto Sans','Liberation Sans';font-size:1rem;font-weight:400;line-height:1.5;color:#212529;background-color:#f5f5f5;}.form-control{display:block;width:100%;height:calc(1.5em + .75rem + 2px);border:1px solid #ced4da;}button{border:1px solid transparent;color:#fff;background-color:#007bff;border-color:#007bff;padding:.5rem 1rem;font-size:1.25rem;line-height:1.5;border-radius:.3rem;width:100%}.form-signin{width:100%;max-width:400px;padding:15px;margin:auto;}h1,p{text-align: center}</style> </head> <body><main class='form-signin'> <h1>Wifi Setup</h1> <br/> <p>Your settings have been saved successfully!<br />Please restart the device.</p></main></body></html>" );
  } else {
    server.send(200, "application/json", "{'ph': " + String(phGlobal) + "}");
    // server.send(200,   "text/html", "<!doctype html><html lang='en'><head><meta charset='utf-8'><meta name='viewport' content='width=device-width, initial-scale=1'><title>Wifi Setup</title> <style>*,::after,::before{box-sizing:border-box;}body{margin:0;font-family:'Segoe UI',Roboto,'Helvetica Neue',Arial,'Noto Sans','Liberation Sans';font-size:1rem;font-weight:400;line-height:1.5;color:#212529;background-color:#f5f5f5;}.form-control{display:block;width:100%;height:calc(1.5em + .75rem + 2px);border:1px solid #ced4da;}button{cursor: pointer;border:1px solid transparent;color:#fff;background-color:#007bff;border-color:#007bff;padding:.5rem 1rem;font-size:1.25rem;line-height:1.5;border-radius:.3rem;width:100%}.form-signin{width:100%;max-width:400px;padding:15px;margin:auto;}h1{text-align: center}</style> </head> <body><main class='form-signin'> <form action='/' method='post'> <h1 class=''>Wifi Setup</h1><br/><div class='form-floating'><label>SSID</label><input type='text' class='form-control' name='ssid'> </div><div class='form-floating'><br/><label>Password</label><input type='password' class='form-control' name='password'></div><br/><br/><button type='submit'>Save</button><p style='text-align: right'><a href='https://www.mrdiy.ca' style='color: #32C5FF'>mrdiy.ca</a></p></form></main> </body></html>" );
  }
}



void changeMux(int c, int b, int a) {
  digitalWrite(MUX_A, a);
  digitalWrite(MUX_B, b);
  digitalWrite(MUX_C, c);
}




int calculateTDS(){
     static unsigned long analogSampleTimepoint = millis();
   if(millis()-analogSampleTimepoint > 40U)     //every 40 milliseconds,read the analog value from the ADC
   {
     analogSampleTimepoint = millis();
     analogBuffer[analogBufferIndex] = analogRead(ANALOG_INPUT);    //read the analog value and store into the buffer
     analogBufferIndex++;
     if(analogBufferIndex == SCOUNT) 
         analogBufferIndex = 0;
   }   
   static unsigned long printTimepoint = millis();
   if(millis()-printTimepoint > 800U)
   {
      printTimepoint = millis();
      for(copyIndex=0;copyIndex<SCOUNT;copyIndex++)
        analogBufferTemp[copyIndex]= analogBuffer[copyIndex];
      averageVoltage = getMedianNum(analogBufferTemp,SCOUNT) * (float)VREF / 1024.0; // read the analog value more stable by the median filtering algorithm, and convert to voltage value
      float compensationCoefficient=1.0+0.02*(temperature-25.0);    //temperature compensation formula: fFinalResult(25^C) = fFinalResult(current)/(1.0+0.02*(fTP-25.0));
      float compensationVolatge=averageVoltage/compensationCoefficient;  //temperature compensation
      tdsValue=(133.42*compensationVolatge*compensationVolatge*compensationVolatge - 255.86*compensationVolatge*compensationVolatge + 857.39*compensationVolatge)*0.5; //convert voltage value to tds value
      //Serial.print("voltage:");
      //Serial.print(averageVoltage,2);
      //Serial.print("V   ");
      Serial.print("TDS Value:");
      Serial.print(tdsValue,0);
      Serial.println("ppm");
      return tdsValue;
}
return 0;
}




int getMedianNum(int bArray[], int iFilterLen) 
{
      int bTab[iFilterLen];
      for (byte i = 0; i<iFilterLen; i++)
      bTab[i] = bArray[i];
      int i, j, bTemp;
      for (j = 0; j < iFilterLen - 1; j++)   
      {
      for (i = 0; i < iFilterLen - j - 1; i++) 
          {
        if (bTab[i] > bTab[i + 1]) 
            {
        bTemp = bTab[i];
            bTab[i] = bTab[i + 1];
        bTab[i + 1] = bTemp;
         }
      }
      }
      if ((iFilterLen & 1) > 0)
    bTemp = bTab[(iFilterLen - 1) / 2];
      else
    bTemp = (bTab[iFilterLen / 2] + bTab[iFilterLen / 2 - 1]) / 2;
      return bTemp;
}