#include "l3d-cube/l3d-cube.h"
#include <math.h>

TCPServer server = TCPServer(23);
TCPClient client;

Cube cube = Cube();

void setup()
{
  server.begin();

  cube.begin();
  cube.background(black);

  Serial.begin(9600);

  while(!Serial.available()) SPARK_WLAN_Loop();

  Serial.println(WiFi.localIP());
}

void loop()
{
  if (client.connected()) {
    // echo all available bytes back to the client
    while (client.available()) {
      server.write(client.read());
    }
  } else {
    // if no client is yet connected, check for a new connection
    client = server.available();
  }
}
