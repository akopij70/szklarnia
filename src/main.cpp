
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
//#include <Wire.h>
#include <Adafruit_AHTX0.h>

#define NUMBER_OF_MEASUREMENTS 10

const char* ssid = "TajnaSzklarnia";       
const char* password = "ania12345"; 

ESP8266WebServer server(80);
Adafruit_AHTX0 aht;

bool LEDstatus = LOW;
int SoilPin = A0;

void handle_OnConnect();
void handle_ledon();
void handle_ledoff();
void handle_NotFound();
String updateWebpage(uint8_t LEDstatus);
int GetSoilMoisture();
sensors_event_t temperature, humidity;

void setup() 
{
  Serial.begin(115200);
  if (!aht.begin())
    Serial.println("Problem z czujnikiem");
  pinMode(LED_BUILTIN, OUTPUT); 
  WiFi.softAP(ssid, password);

  IPAddress myIP = WiFi.softAPIP();
  Serial.print("Access Point IP:");
  Serial.println(myIP);
  
  server.on("/", handle_OnConnect);
  server.on("/ledon", handle_ledon);
  server.on("/ledoff", handle_ledoff);
  server.onNotFound(handle_NotFound);
  
  server.begin();
  Serial.println("HTTP Server Started");
  
}

void loop() 
{
  server.handleClient();

  if(!LEDstatus)
    digitalWrite(LED_BUILTIN, HIGH);
  else
    digitalWrite(LED_BUILTIN, LOW);
}

void handle_OnConnect() 
{
  LEDstatus = LOW;
  Serial.println("LED: OFF");
  server.send(200, "text/html", updateWebpage(LEDstatus)); 
}

void handle_ledon() 
{
  LEDstatus = HIGH;
  Serial.println("LED: ON");
  server.send(200, "text/html", updateWebpage(LEDstatus)); 
}

void handle_ledoff() 
{
  LEDstatus = LOW;
  Serial.println("LED: OFF");
  server.send(200, "text/html", updateWebpage(LEDstatus)); 
}

void handle_NotFound()
{
  server.send(404, "text/plain", "Not found");
}

String updateWebpage(uint8_t LEDstatus)
{
  String ptr = "<!DOCTYPE html> <html>\n";
  ptr += "<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  ptr += "<title>LED Control</title>\n";
  ptr += "<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}\n";
  ptr += "body{margin-top: 50px;} h1 {color: #444444;margin: 50px auto 30px;} h3 {color: #444444;margin-bottom: 50px;}\n";
  ptr += ".button {display: block;width: 80px;background-color: #1abc9c;border: none;color: white;padding: 13px 30px;text-decoration: none;font-size: 25px;margin: 0px auto 35px;cursor: pointer;border-radius: 4px;}\n";
  ptr += ".button-on {background-color: #3498db;}\n";
  ptr += ".button-on:active {background-color: #3498db;}\n";
  ptr += ".button-off {background-color: #34495e;}\n";
  ptr += ".button-off:active {background-color: #2c3e50;}\n";
  ptr += "p {font-size: 14px;color: #888;margin-bottom: 10px;}\n";
  ptr += "</style>\n";
  ptr += "</head>\n";
  ptr += "<body>\n";
  ptr += "<h1>ESP8266 Web Server</h1>\n";
  ptr += "<h3>Using Access Point(AP) Mode</h3>\n";
  aht.getEvent(&humidity, &temperature);
  ptr += "Temperatura powietrza: ";
  String temp = String(temperature.temperature);
  ptr += temp;
  ptr += "<br>Wilgotnosc powietrza: ";
  String hum = String(humidity.relative_humidity);
  ptr += hum;

  ptr += "%\n";
  if (LEDstatus)
    ptr +="<p>BLUE LED: ON</p><a class=\"button button-off\" href=\"/ledoff\">OFF</a>\n";
  else
    ptr +="<p>BLUE LED: OFF</p><a class=\"button button-on\" href=\"/ledon\">ON</a>\n";

  ptr +="</body>\n";
  ptr +="</html>\n";
  return ptr;
}

int GetSoilMoisture()
{
  int measurement, percentage_result, average, sum;
  sum = 0;
  for (int i = 0; i <NUMBER_OF_MEASUREMENTS; i++)
  {
    measurement = analogRead(SoilPin);  
    percentage_result = map(measurement, 1024, 480, 0, 100);
    sum += percentage_result;
  }
  average = sum/NUMBER_OF_MEASUREMENTS;
  if (average > 100)
    average = 100;
  return average;
}


/*
#include "ESP8266WiFi.h"

#define NUMBER_OF_MEASUREMENTS 10

int GetSoilMoisture();
void ConnectToWiFi();
void ManageTheWebsite();

void HTTPResponse(WiFiClient &client);
void DisplayWebPage(WiFiClient &client);

int SoilPin = A0;
WiFiServer server(80); //port 80
String header;
const char* ssid = "FunBox2-9501"; //Enter SSID
const char* password = "AA79AFD6541596LED_BUILTIN347DF941C3"; //Enter Password
unsigned long currentTime = millis(); // Zabawa z czasami jest potrzebna do sprawdzania czy mamy polaczenie z klientem
unsigned long previousTime = 0; 
const long timeoutTime = 2000;




void setup(void)
{
  pinMode(SoilPin, OUTPUT); 
  Serial.begin(115200);
  ConnectToWiFi();
}

void loop() 
{
  ManageTheWebsite();
}

void ConnectToWiFi()
{ 
  WiFi.begin(ssid, password);
  Serial.println("");
  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(500);
    Serial.print("*");
  }
  Serial.println("");
  Serial.println("Connected to WiFi");
  Serial.print("IP Address of ESP8266 Module is: ");
  Serial.print(WiFi.localIP());// Print the IP address
  server.begin();
  Serial.println("\nWitamy na pokladzie");
}


void ManageTheWebsite()
{
  WiFiClient client = server.available();
  if (client)
  {
    Serial.println("Mamy goscia");
    String currentLine = ""; //dane od klienta
    currentTime = millis();  //aktualizacja czasow
    previousTime = currentTime;
    while (client.connected() && currentTime - previousTime <= timeoutTime)//dopoki mamy goscia
    {
      currentTime = millis();         
      if (client.available())
      {
        char c = client.read();             // przeczytaj 
        Serial.write(c);                    // wyswietl na serial monitorze
        header += c;
        if (c == '\n') 
        {                    // jak otrzymalismy znak nowej linii
          if (currentLine.length() == 0) // jak currentline jest puste to mamy 2 znaki nowej linii z rzedu
          {
            HTTPResponse(client);       // to jest koniec zapytania klienata czyli pora wyslac odpowiedz
            break; // zakoncz petle while
          }
          else // jak znak nowej linii to czysc currentline
            currentLine = "";
        }
        else if (c != '\r') // cokolwiek innego niz znak powrotu karetki
          currentLine += c; 
      }
    }
    header = ""; // czysc header
    client.stop(); //konczymy polaczenie
    Serial.println("Client disconnected.");
    Serial.println("");
  }
}

void HTTPResponse(WiFiClient &client)
{
  client.println("HTTP/1.1 200 OK");     // naglowek HTTP zaczyna sie od kodu odpowiedzi ( HTTP/1.1 200 OK)      
  client.println("Content-type:text/html"); // dalej content-type zeby klient wiedzial co jest grane
  client.println("Connection: close");
  client.println(); // no i pusta linia
  DisplayWebPage(client);
  client.println(); // odpowiedz HTTP konczy sie pusta linia          
}
void DisplayWebPage(WiFiClient &client)
{
  client.println("<!DOCTYPE html><html>"); //czyli wysylamy html
  client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
  client.println("<link rel=\"icon\" href=\"data:,\">");
  client.println("<body><h1><center>ESP8266 piekna szklarnia</center></h1>"); //tytul
  client.print("Wilgotnosc: ");
  client.print(GetSoilMoisture());
  client.println("%");
  client.println("</body></html>");
}

int GetSoilMoisture()
{
  int measurement, percentage_result, average, sum;
  sum = 0;
  for (int i = 0; i <NUMBER_OF_MEASUREMENTS; i++)
  {
    measurement = analogRead(SoilPin);  
    percentage_result = map(measurement, 1024, 480, 0, 100);
    sum += percentage_result;
  }
  average = sum/NUMBER_OF_MEASUREMENTS;
  if (average > 100)
    average = 100;
  return average;
}
*/

/*
int pin_gleba = A0;
int wynik, pomiar;

void setup() 
{
  pinMode(pin_gleba, OUTPUT);
  Serial.begin(9600);
}

void loop() 
{
  pomiar = analogRead(pin_gleba);  
  wynik = map(pomiar, 1024, 460, 0, 100);
  if (wynik > 100)
    wynik = 100;
  Serial.print(wynik);
  Serial.println("%");
  delay(200);
}
*/