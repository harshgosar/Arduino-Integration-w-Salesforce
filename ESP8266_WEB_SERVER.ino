/*******************************
   Author:          Harsh Gosar (harsh2astrotech@gmail.com)
   Description:     To test ESP8266MOD by sending commant from Serial Interface
   Created Date:    9-MAR-2018
   Version:         1.0
   Supported board: Arduino Mega 2560 or board that supports multiple hardware serial ports
*/
#include <ArduinoJson.h>
StaticJsonBuffer<200> jsonBuffer;
//Serial port  -----> Tx=> 1, Rx=> 0
//Serial1 port -----> Tx=> 18, Rx=> 19  ----> connect ESP8266 Tx to Mega's Rx and Rx to Mega's Tx

void setup() {
  // initialize both serial ports:
  Serial.begin(115200);
  while (!Serial) continue;
  Serial1.begin(115200);
  while (!Serial1) continue;

  Initialize();
}

void Initialize() {
  Serial.print("Initializing.");
  sendData("AT+RST\r\n", 2000, false);
  Serial.print(".");
  sendData("AT+CWMODE=1\r\n", 1000, false);
  Serial.print(".");
  sendData("AT+CIPMUX=1\r\n", 1000, false);
  Serial.print(".");
  sendData("AT+CWLAP\r\n", 2000, false);
  Serial.print(".");
  sendData("AT+CWJAP=\"Astro_Freak\",\"Harshgosar1994\"\r\n", 2000, false);
  Serial.print(".");
  sendData("AT+CIFSR\r\n", 1000, false);
  Serial.print(".");
  sendData("AT+CIPSERVER=1,1025\r\n", 1000, false);
  Serial.print(". Done");
  Serial.println();
}
void loop() {
  //Check if ESP8266 is transmitting anything
  if (Serial1.available()) {
    if (Serial1.find("+IPD,")) {
      delay(10);        // Waiting for 1 sec
      int connectionId = Serial1.read() - 48; // Subtracting 48 from the character to get the number.
      Serial.print("connectionId:");
      Serial.println(connectionId);

      Serial1.find("pin=");                   // Advancing the cursor to the "pin="
      int pinNumber = (Serial1.read() - 48) * 10; // Getting the first number which is pin 13
      pinNumber += (Serial1.read() - 48);    // This will get the second number. For example, if the pin number is 13 then the 2nd number will be 3 and then add it to the first number

      Serial.print("pinNumber:");
      Serial.println(pinNumber);

      Serial1.find("state=");                   // Advancing the cursor to the "pin="
      int state = Serial1.read() -48; // Getting the first number which is pin 13
      Serial.print("state:");
      Serial.println(state);

      digitalWrite(pinNumber, boolean(state)); // This will toggle the pin

      /*JsonObject& root      = jsonBuffer.createObject();
      JsonArray& data       = root.createNestedArray("status");
      JsonObject& pin       = jsonBuffer.createObject();
      JsonObject& pinState  = jsonBuffer.createObject();
      pin["pin"]            = pin;
      pinState["pinState"]  = state;

      data.add(pin);
      data.add(pinState);*/

      String webpage = "{\"pin\":";
      webpage += pinNumber;
      webpage += ",\"state\":";
      webpage += boolean(state);
      webpage += "}";
      //Serial.println(webpage);
      String cipSend = "AT+CIPSEND=";
      cipSend += connectionId;
      cipSend += ",";
      cipSend += webpage.length();
      cipSend += "\r\n";

      sendData(cipSend, 1000, false);
      sendData(webpage, 1000, false); 

      // The following commands will close the connection
      String closeCommand = "AT+CIPCLOSE=";
      closeCommand += connectionId;
      closeCommand += "\r\n";
      sendData(closeCommand, 1000, true);   // Sending the data to the ESP8266 to close the command
      Serial1.flush();
    } else {
      String response = "";
      while (Serial1.available()) {
        char inByte = Serial1.read();
        response += inByte;
      }
      //Print message recieved from ESP8266 to Arduino's Serial port
      Serial.print(response);
    }
  }

  //Check if Arduino is transmitting anything, i.e. user is tying something in serial interface command line and sending it.
  if (Serial.available()) {
    String request = "";
    while (Serial.available()) {
      char inByte = Serial.read();
      request += inByte;
    }
    //Print message typed in Ardunio serial interface command line to ESP8266's serial port
    sendData(request, 1000, true);
  }
}

void sendData(String command, const int timeout, boolean debug) {
  String response = "";
  Serial1.print(command);
  long int time = millis();
  while ( (time + timeout) > millis()) {
    while (Serial1.available()) {
      char c = Serial1.read();      // Read the next character.
      response += c;                // Storing the response from the Serial1
    }
  }
  if (response.indexOf("ERROR") > 0) {
    Serial.println("An error occured while executing command:" + command);
    //delay(100);
    //abort();
  }

  if (debug) {
    Serial.println(response);
  }
}
