#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "packet.h"
#include "../../common/common.h"

#define FILESIZE (unsigned char) '\x00'
#define FILENAME (unsigned char) '\x01'

#define MAX_FILENAME_SIZE (unsigned int) MAX_PACKET_SIZE - 9

unsigned int build_control_packet(unsigned char *packet, unsigned char control, unsigned char* file_name, unsigned int file_size) {
    if (file_name == NULL || file_size == 0) {
        return 0;
    }

    packet[0] = control;
    packet[1] = FILESIZE;
    packet[2] = 0x04;

    memcpy(packet + 3, &(file_size), sizeof(unsigned int));

    int offset = 3 + sizeof(unsigned int);

    packet[offset] = FILENAME;
    
    unsigned int file_name_size = strnlen(file_name, MAX_FILENAME_SIZE);
    packet[offset + 1] = file_name_size;

    unsigned int file_name_total = 0;
    for (unsigned int i = 0; i < file_name_size && i < (unsigned int) MAX_PACKET_SIZE - offset - 2 - 1; i++) {
        packet[offset + 2 + i] = file_name[i];
        file_name_total++;
    }

    return CONTROL_HEADER_SIZE(2) + sizeof(unsigned int) + file_name_total;
}

unsigned char process_control_packet(unsigned char *packet, unsigned int size, unsigned char* file_name, unsigned int file_size) {
    if (packet[1] != FILESIZE || packet[2] != 0x04) {
        return U_CHAR_ERROR;
    }

    memcpy(file_size, packet + 3, (size_t)4);

    int offset = 3 + sizeof(unsigned int);

    if (packet[offset] != FILENAME) {
        return U_CHAR_ERROR;
    }

    unsigned int file_name_size = packet[offset + 1];
    for (unsigned int i = 0; i < file_name_size; i++) {
        file_name[i] = packet[offset + 2 + i];
    }

    return packet[0];
}

unsigned int build_data_packet(unsigned char *packet, unsigned char *data, unsigned int *data_size) {
    return 0;
}

unsigned char process_data_packet(unsigned char *packet, unsigned int size, unsigned char *data, unsigned int *data_size) {
    return 0;
}