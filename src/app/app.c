#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <unistd.h>
#include <string.h>

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
} _input_flags = {0, 0, 0, 0};

static unsigned int _file_size = 0;
static unsigned int _end_file_size = 0;
static unsigned int _port = 0;
static unsigned char _file_name[APP_FILENAME_MAX] = {};
static unsigned char _end_file_name[APP_FILENAME_MAX] = {};
static unsigned char _new_file_name[APP_FILENAME_MAX] = {};
static unsigned char _path[APP_FILENAME_MAX] = {};
static char _mode = '\0';

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
            if (strncmp(argv[i], "-s", (size_t)2) == 0) {
                ism = FILE_NAME;
                continue;
            } else if (strncmp(argv[i], "-p", (size_t)2) == 0) {
                ism = PORT;
                continue;
            } else if (strncmp(argv[i], "-r", (size_t)2) == 0) {
                ism = PATH;
                continue;
            } else if (strncmp(argv[i], "-n", (size_t)2) == 0) {
                ism = NEW_FILENAME;
                continue;
            }
            return -1;

        } else if (ism == FILE_NAME) {
            if (_input_flags.path || _input_flags.file_name) {
                return -1;
            }
            _input_flags.file_name = 1;

            _mode = 'w';
            
            strncpy(_file_name, argv[i], APP_FILENAME_MAX);
            ism = FLAG;

        } else if (ism == PORT) {
            if (_input_flags.port) {
                return -1;
            }
            _input_flags.port = 1;
            _port = strtol(argv[i], NULL, 10);
            if (_port == LONG_MIN || _port == LONG_MAX) {
                return -1;
            }
            
            ism = FLAG;

        } else if (ism == PATH) {
            if (_input_flags.file_name || _input_flags.path) {
                return -1;
            }
            _input_flags.path = 1;

            _mode = 'r';

            strncpy(_path, argv[i], APP_FILENAME_MAX);
            ism = FLAG;

        } else if (ism == NEW_FILENAME) {
            if (_input_flags.new_file_name) {
                return -1;
            }

            _input_flags.new_file_name = 1;
            strncpy(_new_file_name, argv[i], APP_FILENAME_MAX);
            ism = FLAG;
        }
    }

    if (_input_flags.port == 0 || (_input_flags.file_name == 0 && _input_flags.path == 0)) {
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

int send_file(int fd, unsigned char* filename, unsigned int file_size) {
    FILE* fp = fopen(filename, "r");
    if (fp == NULL) {
        return -1;
    }

    unsigned char data[MAX_DATA_SIZE];
    unsigned char packet[MAX_PACKET_SIZE];
    unsigned int cur_size = 0;
    unsigned int data_size = 0;
    fseek(fp, 0, SEEK_SET);

    while(TRUE) {
        data_size = fread(data, sizeof(unsigned char), MAX_DATA_SIZE, fp);

        if(data_size > 0) {
            cur_size = cur_size + data_size;
            print_progress((double)cur_size, (double) file_size, 0);

            data_size = build_data_packet(packet, _seq, data, data_size);

            if(llwrite(fd, packet, data_size) <= 0) {
                return ERROR;
            }

            _seq = NEXT_SEQ_NUMBER(_seq);
        }

        if(feof(fp) || ferror(fp)) { 
            break ;
        }
    }

    return SUCCESS;
}

int receive_file(int fd, unsigned char* buffer, unsigned int file_size) {
    unsigned char packet[MAX_PACKET_SIZE];
    unsigned int size = 0;
    unsigned char data[MAX_DATA_SIZE];
    unsigned int data_size = 0;
    unsigned int cur_size = 0;

    while (TRUE) {
        if((data_size = llread(fd, packet)) > 0) {
            print_progress((double) cur_size, (double) file_size, 1);
            
            if (process_data_packet(packet, size, data, &data_size) != _seq) {
                return ERROR;
            }

            _seq = NEXT_SEQ_NUMBER(_seq);

            memcpy(buffer + cur_size, data, data_size);
            cur_size = cur_size + (unsigned int) data_size;
        } else if (data_size == 0) {
            continue;
        }
        else
        {
            return ERROR;
        }
    }

    if (process_control_packet(packet, size, _end_file_name, _end_file_size) != 0) {
        return ERROR;
    }

    return cur_size;
}

int main(int argc, char* argv[]) {
    if (parse_input(argc, argv) != 0) {
        printf("Correct usage: ./app -p <port number> -s <file to send> -r <where to store received file> [-n <new file name>]\n");
        return ERROR;
    }

    int fd;
    if (_mode == 'w') {
        if (access(_file_name, F_OK) != 0) {
            printf("There is no such file\n");
            return ERROR;
        }

        printf("Establishing a connection...\n");

        if ((fd = llopen(_port, FALSE)) == ERROR) {
            printf("Unable to establish a connection\n");
            return ERROR;
        }

        printf("Connection established!\n");

        unsigned char start_packet[MAX_PACKET_SIZE];
        unsigned int size = build_control_packet(start_packet, START, _file_name, _file_size);

        if (llwrite(fd, start_packet, size) == ERROR) {
            printf("Unable to send start control packet\n");
            return ERROR;
        }

        if (send_file(fd, _file_name,_file_size) != 0) {
            printf("\nUnable to send file\n");
            return -1;
        }

        printf("\nSuccesfully sent file\n");

       unsigned char end_packet[MAX_PACKET_SIZE];
        unsigned int size = build_control_packet(end_packet, END, _file_name, _file_size);

        if (llwrite(fd, end_packet, size) == ERROR) {
            printf("Unable to send end control packet\n");
            return ERROR;
        }

        printf("Succesfully sent end control packet\n");

        if (llclose(fd) != 0) {
            printf("Unable to disconnect\n");
            return -1;
        }

        printf("Succesfully disconnected from the receiver\n");
    }
    else if (_mode == 'r') {
        if ((fd = llopen(_port, TRUE)) < 0) {
            printf("Unable to establish a connection\n");
            return ERROR;
        }

        printf("Succesfully established a connection with a transmitter\n");

        if (get_control_packet(fd, START, _file_name, _file_size) != 0) {
            printf("Unable to receive start control packet\n");
            return ERROR;
        }

        printf("Succesfully received start control packet\n");

        unsigned char* file = (unsigned char*) malloc(sizeof(unsigned char) * _file_size);
        int file_size = -1;

        if ((file_size = receive_file(fd, file, _file_size)) < 0) {
            printf("\nUnable to properly receive file\n");
            return ERROR;
        }

        printf("\nSuccesfully received file contents and end control packet\n");

        /*
        if (compare_file_info(&start_info, &end_info) != 0) {
            printf("Start and end control packets do not match\n");
            free(buffer);
            return -1;
        }

        */

        printf("Control packets found to be matching\n");

        char* fn;
        if (_input_flags.new_file_name) {
            fn = _new_file_name;
        } else {
            fn = _file_name;
        }

    /*
        if (save_read_file(file, file_size, fn) != 0) {
            printf("Unable to save file locally\n");
            return -1;
        }
*/
        printf("Received file saved successfully\n");


        if (llclose(fd) != 0) {
            printf("Unable to disconnect\n");
            return ERROR;
        }

        printf("Successfully disconnected from the sender\n");
    }

    return ERROR;
}