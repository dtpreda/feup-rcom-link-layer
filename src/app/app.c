#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <unistd.h>

#include "../common/common.h"
#include "../dll/dll.h"
#include "packet/packet.h"

#define MAX_SEQ_NUMBER 256
#define NEXT_SEQ_NUMBER(n) (n + 1) % MAX_SEQ_NUMBER
#define APP_FILENAME_MAX FILENAME_MAX / 8

enum input_sm {
    FLAG,  
    FILE_NAME,
    PORT,  
    PATH, 
    NEW_FILENAME 
};

static unsigned char _seq = 0x00;

static struct {
    unsigned int file_name;
    unsigned int port;
    unsigned int path;
    unsigned int new_file_name;
} input_flags = {0, 0, 0, 0};

static unsigned int file_size = 0;
static unsigned int port = 0;
static char file_name[APP_FILENAME_MAX] = {};
static char new_file_name[APP_FILENAME_MAX] = {};
static char path[APP_FILENAME_MAX] = {};
static char mode = '\0';

static int get_control_packet(int fd, unsigned char control, unsigned char* file_name, unsigned int file_size) {
    unsigned char packet[MAX_PACKET_SIZE];
    unsigned int size = 0;
    
    if ((size = llread(fd, packet)) == ERROR) {
        return ERROR;
    }

    if (process_control_packet(packet, size, file_name, file_size) != control) {
        return ERROR;
    }

    return SUCCESS;
}

static int parse_input(int argc, char* argv[]) {
    enum input_sm ism = FLAG;

    for (int i = 1; i < argc; i++) {
        if (ism == FLAG) {
            if (strncmp(argv[i], "-s", 2) == 0) {
                ism = FILE_NAME;
                continue;
            } else if (strncmp(argv[i], "-p", 2) == 0) {
                ism = PORT;
                continue;
            } else if (strncmp(argv[i], "-r", 2) == 0) {
                ism = PATH;
                continue;
            } else if (strncmp(argv[i], "-n", 2) == 0) {
                ism = NEW_FILENAME;
                continue;
            }
            return -1;

        } else if (ism == FILE_NAME) {
            if (input_flags.path || input_flags.file_name) {
                return -1;
            }
            input_flags.file_name = 1;

            mode = 'w';
            
            strncpy(file_name, argv[i], APP_FILENAME_MAX);
            ism = FLAG;

        } else if (ism == PORT) {
            if (input_flags.port) {
                return -1;
            }
            input_flags.port = 1;
            port = strtol(argv[i], NULL, 10);
            if (port == LONG_MIN || port == LONG_MAX) {
                return -1;
            }
            
            ism = FLAG;

        } else if (ism == PATH) {
            if (input_flags.file_name || input_flags.path) {
                return -1;
            }
            input_flags.path = 1;

            mode = 'r';

            strncpy(path, argv[i], APP_FILENAME_MAX);
            ism = FLAG;

        } else if (ism == NEW_FILENAME) {
            if (input_flags.new_file_name) {
                return -1;
            }

            input_flags.new_file_name = 1;
            strncpy(new_file_name, argv[i], APP_FILENAME_MAX);
            ism = FLAG;
        }
    }

    if (input_flags.port == 0 || (input_flags.file_name == 0 && input_flags.path == 0)) {
        return -1;
    }

    return 0;
}

static void print_progress(double cur_size, double total_size, int is_reader) {
    double progress = (cur_size / total_size) * 100;
    if (is_reader) {
        printf("\rReceived %.0f bytes out of a total of %.0f bytes (%.0f%%)", cur_size, total_size, progress);
    } else {
        printf("\rSent %.0f bytes out of a total of %.0f bytes (%.0f%%)", cur_size, total_size, progress);
    }
    printf("|");
    for (int i = 0; i < ((unsigned int) progress) / 4; i++) {
        printf("\033[102m \033[0m");
    }
    for (int i = ((unsigned int) progress) / 4; i < 100 / 4; i++) {
        printf(" ");
    }
    printf("|");
    fflush(stdout);
}

int main(int argc, char* argv[]) {
    if (parse_input(argc, argv) != 0) {
        printf("Correct usage: ./app -p <port number> -s <file to send> -r <where to store received file> [-n <new file name>]\n");
        return ERROR;
    }

    int fd;
    if (mode == 'w') {
        if (access(file_name, F_OK) != 0) {
            printf("There is no such file\n");
            return ERROR;
        }

        printf("Establishing a connection...\n");

        if ((fd = llopen(port, FALSE)) == ERROR) {
            printf("Unable to establish a connection\n");
            return ERROR;
        }

        printf("Connection established!\n");

        unsigned char start_packet[MAX_PACKET_SIZE];
        unsigned int size = build_control_packet(start_packet, START, file_name, file_size);

        if (llwrite(fd, start_packet, size) == ERROR) {
            printf("Unable to send start control packet\n");
            return ERROR;
        }
    }
    else if (mode == 'r')
    {
    }

    return ERROR;
}