#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

IPAddress local_ip(192, 168, 1, 202); // IP-адрес
IPAddress gateway(192, 168, 1, 1);    // IP-адрес шлюза  
IPAddress subnet(255, 255, 255, 0);   // Подсеть

WiFiServer server(80);

void setup() 
{
  Serial.begin(9600);

  WiFi.softAPConfig(local_ip, gateway, subnet);
  WiFi.softAP("Hobot 2");
  server.begin();
}

void loop()
{
  if (WiFiClient client = server.accept())
  {
    while (client.connected())
    {
      while (client.available()) Serial.write(client.read());
      while (Serial.available()) client.write(Serial.read());
    }
  }
}