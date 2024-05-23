#include "SoftwareSerial.h"

SoftwareSerial esp8266(0, 1); // RX, TX

void setup()
{
  Serial.begin(9600); // serial port used for debugging
  esp8266.begin(115200);  // your ESP's baud rate might be different
}

void loop()
{
  if(esp8266.available())  // check if the ESP is sending a message
  {
    while(esp8266.available())
    {
      char c = esp8266.read();  // read the next character.

      Serial.write(c);  // writes data to the serial monitor
    }
  }

  if(Serial.available())
  {
    delay(10);  // wait to let all the input command in the serial buffer

    // read the input command in a string
    String cmd = "";
    while(Serial.available())
    {
      cmd += (char)Serial.read();
    }
    // send to the esp8266
    esp8266.println(cmd); 
  }
}