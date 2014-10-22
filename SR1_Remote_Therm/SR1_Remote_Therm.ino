#include <SoftwareSerial.h>

//Pin outputs
const int Fan_Pin   = 4;
const int AC_Pin    = 5;
const int Heat_Pin  = 6;
const int Alert_Pin = 7;
const int R_Temp    = A0;
const int L_Temp    = A1;
const bool Print    = 1;//For debug output

int numReading   = 100; //Number of readings to average

//States of the pin
bool Fan_State, AC_State, Heat_State, Alert_State;

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
    char temp;
    temp = mySerial.read();
    if (temp == 't')
    {
      byte lbyte, hbyte; // To hold the low and high byte of 10bit reading
      int rvar, lvar, tvar; // Temporary variable for holding stuffs
      for (int x = 0; x < numReading; x++) //Read the analog pin for certain amount
      {
        lvar += analogRead(L_Temp);
        delay(1);
        rvar += analogRead(R_Temp);
        delay(1);
      }
      rvar = (rvar / numReading); // average right temperature reading
      lvar = (lvar / numReading); // average left temperature reading
      tvar = ((lvar + rvar)/2);  // add the two readings to get more better values
      lbyte = lowByte(tvar);    //get the low byte
      hbyte = highByte(tvar);   // get the high byte

      mySerial.write(lbyte);      //Send the low byte
      mySerial.write(hbyte);      //Send the high byte

      if (Print)
      {
        double Temperature = (1.1 * tvar * 100.0) / 1024;
        Serial.print("tvar: ");  Serial.println(tvar);
        Serial.print("lbyte: "); Serial.println(lbyte);
        Serial.print("hbyte: "); Serial.println(hbyte);
        Serial.print("Temperature in C: "); Serial.println(Temperature);
      }
    }

    else if (temp == 'c')
    {
      Fan_State   = mySerial.parseInt();
      AC_State    = mySerial.parseInt();
      Heat_State  = mySerial.parseInt();
      Alert_State = mySerial.parseInt();

      if (Print)
      {
        Serial.print("Fan_State: ");  Serial.println(Fan_State);
        Serial.print("AC_State: "); Serial.println(AC_State);
        Serial.print("Heat_State: "); Serial.println(Heat_State);
        Serial.print("Alert_State: "); Serial.println(Alert_State);
      }
    }

    else if (temp == 's')
    {
      digitalWrite(Fan_Pin, Fan_State);
      digitalWrite(AC_Pin, AC_State);
      digitalWrite(Heat_Pin, Heat_State);
      digitalWrite(Alert_Pin, Alert_State);

      if (Print)
      {
        Serial.print("Fan_Pin: ");  Serial.println(Fan_State);
        Serial.print("AC_Pin: "); Serial.println(AC_State);
        Serial.print("Heat_Pin: "); Serial.println(Heat_State);
        Serial.print("Alert_Pin: "); Serial.println(Alert_State);
      }
    }
    Serial.flush();
  }
}

