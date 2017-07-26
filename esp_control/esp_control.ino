//#define useESP32      // or disable with // for ESP8266
//#define useAP         // or disable with // for STA

#if defined useESP32
#include <WiFi.h>
const int DoutPin = 5; // P5
const int DinPin = 17; // P17 Digital pin to be read
#else
#include <ESP8266WiFi.h>
const int DoutPin = 13; // GPIO13
const int DinPin = 12; // Digital pin to be read
#endif


#if defined useAP       //setting the addresses                    // AP MODE

IPAddress APip(192,168,210,1);
IPAddress APgateway(192, 168, 210, 1);
IPAddress APsubnet(255, 255, 255, 0);
#define APssid "myESP"                                // set for AP MODE login
#define APpassword "password"

#else                                               // set for STA MODE login

IPAddress STAip(192,168,1,210);
IPAddress STAgateway(192, 168, 1, 1);
IPAddress STAsubnet(255, 255, 255, 0);
#define STAssid "mySSID"
#define STApassword  "password"

#endif
 
// AinPin A0 is fix in code ( 10bit ADC so read 1024 as 1V MAX )
// ESP32 AoutPin is fix in code DAC1 ADC18 P25
// Setting 8bit DAC channel 1 (GPIO 25) voltage to approx 0.78 of VDD_A voltage (VDD * 200 / 255). 
// For VDD_A 3.3V, this is 2.59V:


const long serdb = 115200;



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
  Serial.println("___v1.1____");


#if defined useAP       // AP MODE
  Serial.print("Configuring access point: SSID: ");
  Serial.print(APssid);
  Serial.print(" password: ");  
  Serial.println(APpassword); 
  WiFi.softAP(APssid, APpassword);
  delay(10);  
  WiFi.softAPConfig(APip,APgateway,APsubnet);  

#else                                               // set for STA MODE login

  Serial.print("Connecting to ");
  Serial.println(STAssid); 

  WiFi.begin(STAssid, STApassword);
  WiFi.config(STAip,STAgateway,STAsubnet);

 
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("WiFi connected");
#endif

  Serial.print("Server started at http://");

  // Start the server
  server.begin();
  
#if defined useAP       // AP MODE
  Serial.print(WiFi.softAPIP());                  // Print the IP address
#else                                               // for STA MODE
  Serial.print(WiFi.localIP());                  // Print the IP address
#endif
 
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
  String s = ("HTTP/1.1 200 OK\r\n");
  s += ("Content-Type: text/html\r\n\r\n");
  s += ("<!DOCTYPE HTML>\r\n<html>\r\n");
  s += ("<head> <title>KLL engineering</title> </head>\r\n");
  s += ("<body>\r\n");
  s += ("i am a ESP SoC development board running as webserver / WIFI remote IO / <br />\r\n");

#if defined useESP32 
  s += (" <img src=\"http://kll.engineering-news.org/kllfusion01/downloads/ESP32_myPINOUT.jpg\" alt=\"ESP32\" > ");  
#else
  s += (" <img src=\"http://kll.engineering-news.org/kllfusion01/downloads/ESP8266_myPINOUT.jpg\" alt=\"ESP8266\" > ");
#endif
  s += ("<br />\r\n");
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
  s += ("<br>\r\n");  
  s += ("DinPin ");
  s += (DinPin);
  s += (" : ");
  if(digitalRead(DinPin)) {   s += ("On");  } else {    s += ("Off");  }    s += (" <br>\r\n");
  s += ("AinPin ");  s += ("A0");  s += (" : ");  s += (analogRead(A0));  s += (" <br>\r\n");

#if defined useESP32
  s += ("AoutPin ");  s += ("DAC1 : ");  //DAC1 ADC18 P25
  s += (dac1v);
  s += (" _<a href=\"/DACUP\"\"><button>  UP </button></a>");  
  s += (" _<a href=\"/DACDN\"\"><button> DOWN </button></a>");  
    s += (" <br>\r\n");
#endif
  
  s += ("<br>\r\n");
  s += ("</body>");
  s += ("</html>");

  client.print(s);
 
  delay(1);
  client.stop();
  Serial.println("Client disonnected");
  Serial.println("");
 
}

