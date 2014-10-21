#include <SPI.h>
#include <Ethernet.h>
#include <SD.h>

// size of buffer used to capture HTTP requests
#define REQ_BUF_SZ   40

// MAC address from Ethernet shield sticker under board
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
//IPAddress ip(192, 168, 0, 20); // IP address, may need to change depending on network
EthernetServer server(80);  // create a server at port 80
File webFile;
char HTTP_req[REQ_BUF_SZ] = {0}; // buffered HTTP request stored as null terminated string
char req_index = 0;              // index into HTTP_req buffer
byte therm[] = { 192, 168, 1, 115 };
bool heat, cool, alert, fan;
double Temp_Limit = 1.0, Morning_Temp = 78.0, Mate_Temp, Sleep_Temp;

EthernetClient r_therm;

void setup()
{
  // disable Ethernet chip
  pinMode(10, OUTPUT);
  digitalWrite(10, HIGH);

  Serial.begin(9600);       // for debugging

  // initialize SD card
  Serial.println("Initializing SD card...");
  if (!SD.begin(4)) {
    Serial.println("ERROR - SD card initialization failed!");
    return;    // init failed
  }
  Serial.println("SUCCESS - SD card initialized.");
  // check for index.htm file
  if (!SD.exists("index.htm")) {
    Serial.println("ERROR - Can't find index.htm file!");
    return;  // can't find index file
  }
  Serial.println("SUCCESS - Found index.htm file.");
  pinMode(3, INPUT);        // switch is attached to Arduino pin 3

  Ethernet.begin(mac);  // initialize Ethernet device
  server.begin();    // start to listen for clients
  //Connect to remote unit
  Serial.println(Ethernet.localIP());
  Serial.println("Connecting to remote therm");
  if (r_therm.connect(therm, 2000)) {
    Serial.println("connected");
  }
  else {
    Serial.println("connection failed");
  }
}//(end setup)

void loop()
{
  EthernetClient client = server.available();  // try to get client

  if (client) {  // got client?
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {   // client data available to read
        char c = client.read(); // read 1 byte (character) from client
        // buffer first part of HTTP request in HTTP_req array (string)
        // leave last element in array as 0 to null terminate string (REQ_BUF_SZ - 1)
        if (req_index < (REQ_BUF_SZ - 1)) {
          HTTP_req[req_index] = c;          // save HTTP request character
          req_index++;
        }
        // last line of client request is blank and ends with \n
        // respond to client only after last line received
        if (c == '\n' && currentLineIsBlank) {
          // send a standard http response header
          client.println("HTTP/1.1 200 OK");
          // Ajax request
          if (StrContains(HTTP_req, "input")) {
            client.println("Content-Type: text/xml");
            client.println("Connection: keep-alive");
            client.println();
            Temperature(client);
          }
          else {  // web page request
            // send rest of HTTP header
            client.println("Content-Type: text/html");
            client.println("Connection: keep-alive");
            client.println();
            // send web page
            webFile = SD.open("index.htm");        // open web page file
            if (webFile) {
              while (webFile.available()) {
                client.write(webFile.read()); // send web page to client
              }
              webFile.close();
            }
          }
          // display received HTTP request on serial port
          Serial.println(HTTP_req);
          // reset buffer index and all buffer elements to 0
          req_index = 0;
          StrClear(HTTP_req, REQ_BUF_SZ);
          break;
        }
        // every line of text received from the client ends with \r\n
        if (c == '\n') {
          // last character on line of received text
          // starting new line with next character read
          currentLineIsBlank = true;
        }
        else if (c != '\r') {
          // a text character was received from client
          currentLineIsBlank = false;
        }
      } // end if (client.available())
    } // end while (client.connected())
    delay(1);      // give the web browser time to receive the data
    client.stop(); // close the connection
  } // end if (client)
}

// send the state of the switch to the web browser
void Temperature(EthernetClient cl)
{
  double Temperature;
  cl.print("<?xml version = \"1.0\" ?>");
  cl.print("<inputs>");
  // read the temperature
  r_therm.print('t');
  if (r_therm.available() > 1) {
    byte x = r_therm.read();
    byte y = r_therm.read();
    int Temp = x | (y << 8);
    double Temp_C_Read = (1.1 * Temp * 100.0) / 1024;
    Temperature = Temp_C_Read * 9 / 5 + 32;
    cl.print("<temp>");
    cl.print(Temperature); cl.print(" °F, ");
    cl.print(Temp_C_Read); cl.print(" °C");
    cl.println("</temp>");
  }
  //See which mode is working
  //if (now.hour() >= 8 && now.hour() < 14)
  //{
  cl.print("<sch>");
  cl.print("Morning");
  cl.println("</sch>");
  if (Temperature > (Morning_Temp + Temp_Limit)) {
    cl.print("<mode>");
    cl.print("Cool");
    cl.println("</mode>");
    cool = 1; heat = 0;
  }
  else if (Temperature < (Morning_Temp - Temp_Limit)) {
    cl.print("<mode>");
    cl.print("Heat");
    cl.println("</mode>");
    heat = 1; cool = 0;
  }
  else {
    cl.print("<mode>");
    cl.print("OFF");
    cl.println("</mode>");
    heat = 0; cool = 0;
  }
  //}
  //    else if (now.hour() >= 14 && now.hour() < 20)
  //      {
  //		cl.print("<sch>");
  //		cl.print("Mating");
  //		cl.println("</sch>");
  //         if (Temperature > (Mate_Temp + Temp_Limit)) {
  //            cl.print("<mode>");
  //			cl.print("Cool");
  //			cl.println("</mode>");
  //			cool = 1; heat = 0;
  //         }
  //        else if (Temperature < (Mate_Temp - Temp_Limit)) {
  //            cl.print("<mode>");
  //			cl.print("Heat");
  //			cl.println("</mode>");
  //			heat = 1; cool = 0;
  //        }
  //        else{
  //            cl.print("<mode>");
  //			cl.print("OFF");
  //			cl.println("</mode>");
  //			heat = 0; cool = 0;
  //        }
  //      }

  //else
  //      {
  //        if (Temperature > (Sleep_Temp + Temp_Limit)) {
  //            cl.print("<mode>");
  //			cl.print("Cool");
  //			cl.println("</mode>");
  //			cool = 1; heat = 0;
  //        }
  //        else if (Temperature < (Sleep_Temp - Temp_Limit)) {
  //            cl.print("<mode>");
  //			cl.print("Heat");
  //			cl.println("</mode>");
  //			heat = 1; cool = 0;
  //        }
  //        else {
  //            cl.print("<mode>");
  //			cl.print("OFF");
  //			cl.println("</mode>");
  //			heat = 0; cool = 0;
  //        }
  //      }
  cl.print("</inputs>");
}

void send_data() {
  r_therm.print('c');
  r_therm.print('fan');
  r_therm.print(',');
  r_therm.print('cool');
  r_therm.print(',');
  r_therm.print('heat');
  r_therm.print(',');
  r_therm.print('alert');
  delay(100);
  r_therm.print('s');  
}
// sets every element of str to 0 (clears array)
void StrClear(char *str, char length) {
  for (int i = 0; i < length; i++) {
    str[i] = 0;
  }
}

// searches for the string sfind in the string str
// returns 1 if string found
// returns 0 if string not found
char StrContains(char *str, char *sfind) {
  char found = 0;
  char index = 0;
  char len;

  len = strlen(str);

  if (strlen(sfind) > len) {
    return 0;
  }
  while (index < len) {
    if (str[index] == sfind[found]) {
      found++;
      if (strlen(sfind) == found) {
        return 1;
      }
    }
    else {
      found = 0;
    }
    index++;
  }

  return 0;
}
