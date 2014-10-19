#include <SoftwareSerial.h>

//Pin outputs
const int Fan_Pin  = 4;
const int AC_Pin   = 5;
const int Heat_Pin = 6;
const int Alert_Pin= 7;
const int R_Temp = A0;

int numReading   = 100; //Number of readings to average

//bits for various system
const byte Off   = 0b1000;
const byte Cool  = 0b0001;
const byte Heat  = 0b0010;
const byte Alert = 0b0100;

SoftwareSerial mySerial(2, 3);

void setup() {
  //Turn on serial port
  Serial.begin(9600);
  mySerial.begin(9600);
  analogReference(INTERNAL); // Change internal reference to 1.1v for more accuracy
  //Setup pins directions
  pinMode(Fan_Pin, OUTPUT); 
  pinMode(AC_Pin, OUTPUT); 
  pinMode(Heat_Pin, OUTPUT);
  pinMode(Alert_Pin, OUTPUT);
  digitalWrite(Fan_Pin, LOW);
  digitalWrite(AC_Pin, LOW);
  digitalWrite(Heat_Pin, LOW);
  digitalWrite(Alert_Pin, LOW);
}

void loop() {
  //If there's any serial available, read it:
  while (mySerial.available() > 0) {
    if (mySerial.read() == 't')
    {
     byte lbyte, hbyte; // To hold the low and high byte of 10bit reading
     int tvar; // Temporary variable for holding stuffs
     for(int x = 0; x < numReading; x++) //Read the analog pin for certain amount
     {
      tvar += analogRead(A0);
      delay(1);
     }
     R_Temp = (tvar/numReading); // average the temperature reading
     lbyte = lowByte(R_Temp);    //get the low byte
     hbyte = highByte(R_Temp);   // get the high byte
 
     mySerial.write(lbyte);      //Send the low byte
     mySerial.write(hbyte);      //Send the high byte
 
     mySerial.flush();           //Clear the serial port
    }
    else if (mySerial.read() == 's')
    {
      Fan_State   = mySerial.parseInt();
      AC_State    = mySerial.parseInt();
      Heat_State  = mySerial.parseInt();
      Alert_State = mySerial.parseInt();
    }
    
    if (mySerial.read() == '\n')
    {
      digitalWrite(Fan, Fan_State);
      digitalWrite(AC, AC_State);
      digitalWrite(Heat, Heat_State);
      digitalWrite(Alert, Alert_State);
    }
  }
}
