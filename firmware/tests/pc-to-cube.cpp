#include "l3d-cube/l3d-cube.h"
#include <math.h>

#define PACKET_SIZE 512

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
  static int byteCount = 0;

  // reset on serial data
  while (Serial.available() > 0) {
    Serial.read(); // empty buffer

    Serial.println(WiFi.localIP());

    if (client.connected())
      client.stop();

    byteCount = 0;
  }

  // handle network events
  if (client.connected()) {
    while (client.available() > 0) {
      client.read();
      byteCount++;

      if (byteCount % PACKET_SIZE == 0) {
        server.write('y');
      }
    }
  } else {
    // if no client is yet connected, check for a new connection
    client = server.available();
  }
}
