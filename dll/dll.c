#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <termios.h>
#include <string.h>
#include <fcntl.h>

#include "dll.h"
#include "../common/common.h"

#define BAUDRATE B38400
#define TIMEOUT 5
#define MAX_ATTEMPTS 5

static int _is_reader;

static int open_serial_port(char* port) {
    int fd;
    if ((fd = open(port, O_RDWR | O_NOCTTY)) < 0) {
        perror("open");
        return ERROR;
    }

    struct termios oldtio;

    if (tcgetattr(fd, &oldtio) == -1) {
        perror("tcgetattr");
        return ERROR;
    }

    struct termios newtio;

    bzero(&newtio, sizeof(newtio));
    newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
    newtio.c_cflag &= ~CRTSCTS;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;
    newtio.c_lflag = ~(ICANON | ECHO | ISIG);

    newtio.c_cc[VTIME] = TIMEOUT;
    newtio.c_cc[VMIN] = 0;

    tcflush(fd, TCIOFLUSH);

    if (tcsetattr(fd, TCSANOW, &newtio) == -1) {
      perror("tcsetattr");
      return ERROR;
    }

    return fd;
}

int llopen(int port, int is_reader) {
    if (port >= 100) {
        return -1;
    }

    int fd;
    char tty_port[12];
    snprintf(tty_port, sizeof(tty_port), "%s%i", "/dev/ttyS", port);
    
    if ((fd = open_serial_port(tty_port)) == ERROR) {
        return ERROR;
    }
    
    if (is_reader) {
        /*
        if (connect_reader(fd) == ERROR) {
            return ERROR;
        }
        */
    } else {
        /*
        if (connect_writer(fd) == ERROR) {
            return ERROR;
        }
        */
    }

    _is_reader = is_reader;

    return fd;
}

int llwrite(int fd, char* buffer, int length) {
    return 0;
}

int llread(int fd, char* buffer) {
    return 0;
}

int llclose(int fd) {
    return 0;
}