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

  Serial.begin(115200);
}

void loop()
{
  // reset on serial data
  while (Serial.available() > 0) {
    Serial.read(); // empty buffer

    Serial.println(WiFi.localIP());

    if (client.connected())
      client.stop();
  }

  // handle network events
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
