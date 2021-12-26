#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

#include "dll.h"
#include "frame/frame.h"
#include "../common/common.h"

#define BAUDRATE B38400
#define TIMEOUT 5
#define MAX_ATTEMPTS 5

static int _is_reader;

static struct termios oldtio;

static int open_serial_port(char* port) {
    int fd;
    if ((fd = open(port, O_RDWR | O_NOCTTY)) < 0) {
        perror("open");
        return ERROR;
    }

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

static int close_serial_port(int fd) {
    if (tcsetattr(fd,TCSANOW, &oldtio) == -1) {
        perror("tcsetattr");
        return ERROR;
    }
    close(fd);

    return SUCCESS;
}

static int read_frame(int fd, unsigned char* frame, unsigned int max_size) {
    unsigned int index = 0;
    unsigned int _read_smt = FALSE;
    unsigned char c = '\0';

    while (index < max_size) {
        if (read(fd, &c, 1) > 0) {
            frame[index] = c;
            if (_read_smt && c == FLAG)
                break;
            if (!_read_smt)
                _read_smt = TRUE;
            index++;
        } else {
            return ERROR;
        }
    }

    return index;
}

static int send_su_frame(int fd, unsigned char address, unsigned char control) {
    unsigned char *frame[SU_SIZE];
    build_su_frame(frame, address, control);

    if (write(fd, frame, SU_SIZE) <= 0) {
        return ERROR;
    }

    return SUCCESS;
}

static int send_i_frame(int fd, unsigned char address, unsigned char control, unsigned char* data, unsigned int data_size) {
    unsigned char *frame[MAX_SIZE];
    unsigned int _data_size = 0;

    if ((_data_size = build_i_frame(frame, address, control, data, data_size)) == 0) {
        return ERROR;
    }

    if (write(fd, frame, _data_size) <= 0) {
        return ERROR;
    }

    return SUCCESS;
}

static int connect_reader(int fd) {
    for (int attempt = 0; attempt < MAX_ATTEMPTS; attempt++) {
        unsigned char *frame[SU_SIZE];
        unsigned int frame_size = 0;
        if ((frame_size = read_frame(fd, frame, SU_SIZE)) == ERROR) {
            continue;
        }

        if (process_su_frame(frame, frame_size) != SET) {
            continue;
        }

        if (send_su_frame(fd, A_CRAS, UA) == ERROR) {
            continue;
        }

        return SUCCESS;
    }

    return ERROR;
}

static int connect_writer(int fd) {
    for (int attempt = 0; attempt < MAX_ATTEMPTS; attempt++) {
        if (send_su_frame(fd, A_CSAR, SET) == ERROR) {
            continue;
        }

        unsigned char *frame[SU_SIZE];
        unsigned int frame_size = 0;
        if ((frame_size = read_frame(fd, frame, SU_SIZE)) == ERROR) {
            continue;
        }

        if (process_su_frame(frame, frame_size) != UA) {
            continue;
        }

        return SUCCESS;
    }

    return ERROR;
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
        if (connect_reader(fd) == ERROR) {
            return ERROR;
        }
    } else {
        if (connect_writer(fd) == ERROR) {
            return ERROR;
        }
    }

    _is_reader = is_reader;

    return fd;
}

int llwrite(int fd, unsigned char* buffer, unsigned int length) {
    return 0;
}

int llread(int fd, unsigned char* buffer) {
    return 0;
}

int llclose(int fd) {
    int ret = ERROR;
    if (_is_reader) {
        //ret = disconnect_writer(fd);
        close_serial_port(fd);
    } else {
        //ret = disconnect_reader(fd);
        close_serial_port(fd);
    }
    return ret;
}