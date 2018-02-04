/*
    Copyright (C) 2018  Florian Völker

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

    This software is based on the ESP8266 mDNS responder sample.

    The software is tested with the following Boards:
      - WeMos D1 mini [Pin 2 = D4 (build-in LED), Pin 4 = D2 (PWM-Pin), Pin 5 = D1, Pin 14 = D5]
 */

#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiClient.h>
#include <Servo.h>

//WiFi-settings
const char* ssid = "SSID_HERE";
const char* password = "PASSWORD_HERE";

//Pin-settings
static constexpr int switchPin = 14;  //a additional button to trigger something (use external pullup or software-pullup!)
static constexpr int detectPin = 5;   //Detects the switch-state (use external pullup or software-pullup!)
static constexpr int ledPin = 2;      //let's light something up

//Servo-settings
static constexpr int servoPosMid = 90;
static constexpr int servoPosOff = 150;
static constexpr int servoPosOn = 33;

//initiate some libraries
Servo myservo;
// TCP server at port 80 will respond to HTTP requests
WiFiServer server(80);

//initiate some states. They should be false at the beginning.
bool state = false;
bool button_state = false;


void setup(void){
  //Initiate everything necessary
  pinMode(detectPin, INPUT_PULLUP);
  pinMode(switchPin, INPUT_PULLUP);
  pinMode(ledPin, OUTPUT);

  digitalWrite(ledPin, HIGH);
  
  myservo.attach(servoPin);
  myservo.write(servoPosMid);
  delay(200);
  myservo.detach();
  
  Serial.begin(115200);
  
  // Connect to WiFi network
  WiFi.begin(ssid, password);
  Serial.println("");
  
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // Set up mDNS responder:
  //   the fully-qualified domain name is "esp8266.local"
  if (!MDNS.begin("esp8266")) {
    Serial.println("Error setting up MDNS responder!");
    while(1) { 
      delay(1000);
    }
  }
  Serial.println("mDNS responder started");
  
  // Start TCP (HTTP) server
  server.begin();
  Serial.println("TCP server started");
  
  // Add service to MDNS-SD
  MDNS.addService("http", "tcp", 80);
}

void loop(void)
{ 
  //Check the inputs
  state = digitalRead(detectPin);
  if (!digitalRead(switchPin)){
    button_state = true;
    Serial.print("Pressed!!");
    digitalWrite(ledPin, LOW);
  }
  Serial.print(button_state);
  
  // Check if a client has connected
  WiFiClient client = server.available();
  if (!client) {
    Serial.println("loop return");
    return;
  }
  Serial.println("");
  Serial.println("New client");

  // Wait for data from client to become available
  while(client.connected() && !client.available()){
    delay(1);
  }
  
  // Read the first line of HTTP request
  String req = client.readStringUntil('\r');
  
  // First line of HTTP request looks like "GET /path HTTP/1.1"
  // Retrieve the "/path" part by finding the spaces
  int addr_start = req.indexOf(' ');
  int addr_end = req.indexOf(' ', addr_start + 1);
  if (addr_start == -1 || addr_end == -1) {
    Serial.print("Invalid request: ");
    Serial.println(req);
    return;
  }
  req = req.substring(addr_start + 1, addr_end);
  Serial.print("Request: ");
  Serial.println(req);
  client.flush();
  
  String s;
  if (req == "/")
  {
    IPAddress ip = WiFi.localIP();
    String ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);
    s = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE HTML>\r\n<html>Hello from ESP8266 at ";
    s += ipStr;
    s += "</html>\r\n\r\n";
    Serial.println("Sending 200");
  } else if (req == "/1")
  {
    //prevent switching on, if sensor detects the switch is on
    if(!state){
      s = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE HTML>\r\n<html>";
      s += "Set: On";
      s += "</html>\r\n\r\n";
      myservo.attach(servoPin);
      myservo.write(servoPosOn);
      delay(1000);
      myservo.write(servoPosMid);
      delay(200);
      myservo.detach(); //I'd problems with a noisy switch, this should solve it.
    }
  } else if (req == "/0")
  {
    //prevent switching off, if sensor detects the switch is off
    if(state){
      s = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE HTML>\r\n<html>";
      s += "Set: Off";
      s += "</html>\r\n\r\n";
      myservo.attach(servoPin);
      myservo.write(servoPosOff);
      delay(1000);
      myservo.write(servoPosMid);
      delay(200);
      myservo.detach(); //I'd problems with a noisy switch, this should solve it.
    }
  } else if (req == "/stat")
  {
    if(state){
      s = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE HTML>\r\n<html>";
      s += "Status: On";
      s += "</html>\r\n\r\n";
    }else{
      s = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE HTML>\r\n<html>";
      s += "Status: Off";
      s += "</html>\r\n\r\n";
    }
  }
  else if (req == "/button")
  {
    if(button_state){
      s = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE HTML>\r\n<html>";
      s += "1";
      s += "</html>\r\n\r\n";
    }else{
      s = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE HTML>\r\n<html>";
      s += "0";
      s += "</html>\r\n\r\n";
    }
  }
  else if (req == "/b_reset")
  {
    button_state = false;
    digitalWrite(ledPin, HIGH);
    
    s = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE HTML>\r\n<html>";
      s += "reset";
      s += "</html>\r\n\r\n";
  }
  else
  {
    s = "HTTP/1.1 404 Not Found\r\n\r\n";
    Serial.println("Sending 404");
  }
  client.print(s);
  
  Serial.println("Done with client");
}

