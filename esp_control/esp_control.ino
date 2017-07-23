#define useESP32      // or disable with // for ESP8266

#if defined useESP32
#include <WiFi.h>
const int DoutPin = 5; // P5
const int DinPin = 12; // Digital pin to be read
#else
#include <ESP8266WiFi.h>
const int DoutPin = 13; // GPIO13
const int DinPin = 12; // Digital pin to be read
#endif

// AinPin A0 is fix in code ( 10bit ADC so read 1024 as 1V MAX )
// ESP32 AoutPin is fix in code DAC1 ADC18 P25
// Setting 8bit DAC channel 1 (GPIO 25) voltage to approx 0.78 of VDD_A voltage (VDD * 200 / 255). 
// For VDD_A 3.3V, this is 2.59V:


const long serdb = 115200;

// set for STA MODE login ( IP from router )
#define ssid  "kll-wlan"
#define password  "WELOVEKOH-SAMUI"


WiFiServer server(80);

int dac1v = 127;  // 0 .. 255
int dac1d = 3;    // add button

void setup() {
  pinMode(DoutPin, OUTPUT);
  digitalWrite(DoutPin, LOW);
 
  Serial.begin(serdb);
  delay(7);
  Serial.println();
  Serial.println("___________");
  Serial.println(" KLL myESP ");
  Serial.println("___________");
 
  // Connect to WiFi network
  Serial.print("Connecting to ");
  Serial.println(ssid); 
  WiFi.begin(ssid, password);
 
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
 
  // Start the server
  server.begin();
  Serial.println("Server started at http://");
  Serial.print(WiFi.localIP());                  // Print the IP address
  Serial.println("/");
}
 
void loop() {
  // Check if a client has connected
  WiFiClient client = server.available();
  if (!client) {
    return;
  }
 
  // Wait until the client sends some data
  Serial.println("new client");
  while(!client.available()){
    delay(1);
  }
 
  // Read the first line of the request
  String request = client.readStringUntil('\r');
  Serial.println(request);
  client.flush();
 
  // Match the request
 
  int value = LOW;
  if (request.indexOf("/LED=ON") != -1)  {
    digitalWrite(DoutPin, HIGH);
    value = HIGH;
  }
  if (request.indexOf("/LED=OFF") != -1)  {
    digitalWrite(DoutPin, LOW);
    value = LOW;
  }
#if defined useESP32
  if (request.indexOf("/DACUP") != -1)  {
    dac1v = dac1v+dac1d;
    if ( dac1v > 255) { dac1v = 255; }
    dacWrite(25,dac1v);
  }
  if (request.indexOf("/DACDN") != -1)  {
    dac1v = dac1v-dac1d;
    if ( dac1v < 0 ) { dac1v = 0; }
    dacWrite(25,dac1v);
  }
#endif


  
  // Return the response
  String s = "HTTP/1.1 200 OK\r\n";
  s += "Content-Type: text/html\r\n\r\n";
  s += "<!DOCTYPE HTML>\r\n<html>\r\n";
  s += ("i am a ESP SoC development board running as webserver / WIFI remote IO / <br />");

#if defined useESP32 
  s += (" <img src=\"http://kll.engineering-news.org/kllfusion01/downloads/ESP32_myPINOUT.jpg\" alt=\"ESP32\" > ");  
#else
  s += (" <img src=\"http://kll.engineering-news.org/kllfusion01/downloads/ESP8266_myPINOUT.jpg\" alt=\"ESP8266\" > ");
#endif
  s += ("<br />");
  s += ("DoutPin ");
  s += (DoutPin);
  s += (" : ");
 
  if(value == HIGH) {
  s += ("On");
  s += (" _<a href=\"/LED=OFF\"\"><button> Off </button></a>");  
  } else {
  s += ("Off");
  s += (" _<a href=\"/LED=ON\"\"><button> On </button></a>");
  }
  s += ("<br>");  
  s += ("DinPin ");
  s += (DinPin);
  s += (" : ");
  if(digitalRead(DinPin)) {   s += ("On");  } else {    s += ("Off");  }    s += (" <br>");
  s += ("AinPin ");  s += ("A0");  s += (" : ");  s += (analogRead(A0));  s += (" <br>");

#if defined useESP32
  s += ("AoutPin ");  s += ("DAC1 : ");  //DAC1 ADC18 P25
  s += (dac1v);
  s += (" _<a href=\"/DACUP\"\"><button>  UP </button></a>");  
  s += (" _<a href=\"/DACDN\"\"><button> DOWN </button></a>");  
  
//  s += ("<span id=\"range\">0</span>");
//  s += (" _<input type=\"range\" value=\"127\" min=\"0\" max=\"255\" onchange=\"showValue(this.value)\"/>");
//  s += (" _<a href=\"/DAC\"\"><button> send </button></a>");
//  s += ("<script type=\"text/javascript\"> function showValue(newValue) { document.getElementById(\"range\").innerHTML=newValue;} </script> "); 
  s += (" <br>");
#endif
  
  s += ("<br>");
  s += ("</html>");

  client.print(s);
 
  delay(1);
  client.stop();
  Serial.println("Client disonnected");
  Serial.println("");
 
}

