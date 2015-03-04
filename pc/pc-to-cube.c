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

// timing
double elapsedMicroseconds(struct timeval x, struct timeval y);
float average(long* arr, int len);

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
    fflush(stdout);

    // setup connection 

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        fprintf(stderr, "Could not create socket.\n");
        exit(1);
    } else {
        printf("Socket created.\n");
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
    fflush(stdout);

    // do test

    printf("Sending %d packets. Packet size is %d bytes.\n", packetCount, packetSize);
    fflush(stdout);

    char packet[packetSize];
    char reply[1];

    long time[packetCount];
    struct timeval timeStart, timeEnd;

    int packetIndex;
    for(packetIndex = 0; packetIndex < packetCount; packetIndex++) {
        gettimeofday(&timeStart, NULL);

        if (send(sock, packet, sizeof(packet), 0) < 0) {
            fprintf(stderr, "Send failed on packet #%d.\n", packetIndex);
            exit(1);
        }

        if (recv(sock, reply, 1, 0) < 0) {
            fprintf(stderr, "Recv failed on packet #%d.\n", packetIndex);
        }

        if (reply[0] != 'y') {
            fprintf(stderr, "Weird reply on packet #%d: %c (%x).\n", reply[0], reply[0]);
        } else {
            gettimeofday(&timeEnd, NULL);
            time[packetIndex] = elapsedMicroseconds(timeStart, timeEnd);
        }
    }

    float averageMillis = average(time, packetCount) / 1000.f;
    printf("Test completed.\nOn average, %f milliseconds per packet.\n", averageMillis);
    fflush(stdout);

    close(sock);

    return 0;
}

// get average of array of longs
float average(long* arr, int len) {
    long total = 0;

    int i;
    for(i = 0; i < len; i++)
        total += arr[i];

    return (float)total / len;
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

// get time difference in microseconds
double elapsedMicroseconds(struct timeval x, struct timeval y) {
    double x_ms, y_ms, diff;

    x_ms = (double)x.tv_sec * 1000000 + (double)x.tv_usec;
    y_ms = (double)y.tv_sec * 1000000 + (double)y.tv_usec;

    diff = (double)y_ms - (double)x_ms;

    return diff;
}
