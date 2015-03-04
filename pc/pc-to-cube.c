#include <stdio.h>
#include <stdlib.h>

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <sys/socket.h>
#include <termios.h>
#include <unistd.h>

#define forever for(;;)

#define IP_LEN 15 // 255.255.255.255 is 15 characters

#define ARG_COUNT   1
#define ARG_SIZE    2

// cube
char* cubeInfo(char* port, unsigned int packetSize);

// serial
int set_interface_attribs(int fd, int speed, int parity);
void set_blocking(int fd, int should_block);

int main(int argc, char *argv[]) {
    unsigned int packetSize;
    unsigned int packetCount;

    // validate arguments

    if (argc != 3) {
        fprintf(stderr, "must give # of packets, packet size (bytes)\n");
        exit(1);
    }

    packetCount = atoi(argv[ARG_COUNT]);
    packetSize = atoi(argv[ARG_SIZE]);

    if (packetCount == 0) {
        fprintf(stderr, "%s is an invalid # of packets", argv[ARG_COUNT]);
        exit(1);
    }

    if (packetSize == 0) {
        fprintf(stderr, "%s is an invalid packet size", argv[ARG_SIZE]);
        exit(1);
    }

    // get the IP & set packet size
    char* address = cubeInfo("/dev/ttyACM0", packetSize);
    printf("Cube IP is %s.\n", address);

    // network test
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        perror("Could not create socket.");
        exit(1);
    } else {
        printf("Socket created.");
    }

    struct sockaddr_in server;
    server.sin_addr.s_addr = inet_addr(address);
    server.sin_family = AF_INET;
    server.sin_port = htons(23); // cube listens on telnet port

    if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
        fprintf(stderr, "Connect failed.\n");
        exit(1);
    }

    printf("Connected to server.\n");

    char packet[512];
    if (send(sock, packet, sizeof(packet), 0) < 0) {
        fprintf(stderr, "Send failed.\n");
        exit(1);
    } else {
        printf("Sent packet.\n");
    }

    char reply[1];
    if (recv(sock, reply, 1, 0) < 0) {
        fprintf(stderr, "Recv failed.\n");
    } else {
        printf("Got reply: %c.\n", reply[0]);
    }

    close(sock);

    return 0;
}

// set packet size over serial
// returns ip address
char* cubeInfo(char* port, unsigned int packetSize) {
    int fd = open(port, O_RDWR | O_NOCTTY | O_SYNC);
    if (fd < 0) {
        fprintf(stderr, "error %d opening %s: %s", errno, port, strerror(errno));
        exit(1);
    }

    set_interface_attribs(fd, B115200, 0);
    set_blocking(fd, 0);

    char sizeBuffer[16];
    int numChars = sprintf(sizeBuffer, "%d", packetSize);
    write(fd, sizeBuffer, numChars);
    write(fd, "\r", 1);

    char c;
    char* addressBuffer = calloc(IP_LEN, sizeof(char));
    int rxCount = 0;

    forever {
        int len = read(fd, &c, 1);
        
        if (c == '\n' || c == '\r')
            break;

        if (len > 0) {
            if (rxCount < IP_LEN) {
                addressBuffer[rxCount] = c;
            } else {
                if (rxCount >= sizeof addressBuffer + 1) {
                    fprintf(stderr, "Corrupted serial message? \"%s\"", addressBuffer);
                    exit(1);
                }
            }

            rxCount++;
        }
    }

    close(fd);

    return addressBuffer;
}


int set_interface_attribs(int fd, int speed, int parity) {
    struct termios tty;
    memset(&tty, 0, sizeof tty);
    if (tcgetattr(fd, &tty) != 0) {
        fprintf(stderr, "error %d from tcgetattr", errno);
        return -1;
    }

    cfsetospeed (&tty, speed);
    cfsetispeed (&tty, speed);

    tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;     // 8-bit chars
    // disable IGNBRK for mismatched speed tests; otherwise receive break
    // as \000 chars
    tty.c_iflag &= ~IGNBRK;         // disable break processing
    tty.c_lflag = 0;                // no signaling chars, no echo,
    // no canonical processing
    tty.c_oflag = 0;                // no remapping, no delays
    tty.c_cc[VMIN]  = 0;            // read doesn't block
    tty.c_cc[VTIME] = 5;            // 0.5 seconds read timeout

    tty.c_iflag &= ~(IXON | IXOFF | IXANY); // shut off xon/xoff ctrl

    tty.c_cflag |= (CLOCAL | CREAD);// ignore modem controls,
    // enable reading
    tty.c_cflag &= ~(PARENB | PARODD);      // shut off parity
    tty.c_cflag |= parity;
    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag &= ~CRTSCTS;

    if (tcsetattr(fd, TCSANOW, &tty) != 0) {
        fprintf(stderr, "error %d from tcsetattr", errno);
        return -1;
    }
    return 0;
}

void set_blocking(int fd, int should_block) {
    struct termios tty;
    memset(&tty, 0, sizeof tty);
    if (tcgetattr(fd, &tty) != 0) {
        fprintf(stderr, "error %d from tggetattr", errno);
        return;
    }

    tty.c_cc[VMIN]  = should_block ? 1 : 0;
    tty.c_cc[VTIME] = 5;            // 0.5 seconds read timeout

    if (tcsetattr(fd, TCSANOW, &tty) != 0)
        fprintf(stderr, "error %d setting term attributes", errno);
}

