#include "l3d-cube/l3d-cube.h"
#include <math.h>

#define MAX_PACKET_SIZE 4096
int packetSize = 512;

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
  static String cmdBuffer = "";

  while (Serial.available() > 0) {
    char c = Serial.read();

    if (c == '\n' || c == '\r') {
        long maybeSize = cmdBuffer.toInt();

        if (maybeSize > 0 && maybeSize <= MAX_PACKET_SIZE) {
          packetSize = maybeSize;

          Serial.println(WiFi.localIP());

          if (client.connected())
            client.stop();

          byteCount = 0;
        }

        cmdBuffer = "";
    } else {
        cmdBuffer += c;
    }
  }

  // handle network events
  if (client.connected()) {
    while (client.available() > 0) {
      client.read();
      byteCount++;

      if (byteCount % packetSize == 0) {
        server.write('y');
      }
    }
  } else {
    // if no client is yet connected, check for a new connection
    client = server.available();
  }
}
