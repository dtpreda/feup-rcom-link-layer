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

unsigned char process_control_packet(unsigned char *packet, unsigned int size, unsigned char* file_name, unsigned int* file_size) {
    if (packet[1] != FILESIZE || packet[2] != 0x04) {
        return U_CHAR_ERROR;
    }

    for (int i = 0; i < 4; i++) {
        *file_size = (*file_size) | (unsigned int) (packet[3 + i] << 8*i); 
    }

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

unsigned int build_data_packet(unsigned char *packet, unsigned char sequence_number, unsigned char *data, unsigned int data_size) {
    unsigned char l1 = (unsigned char) (data_size % 256);
    unsigned char l2 = (unsigned char) (data_size / 256);

    packet[0] = DATA;
    packet[1] = sequence_number;
    packet[2] = l2;
    packet[3] = l1;

    memcpy(((void*)packet) + DATA_HEADER_SIZE, data, (size_t)(data_size));
    return data_size + DATA_HEADER_SIZE;
}

unsigned char process_data_packet(unsigned char *packet, unsigned int size, unsigned char *data, unsigned int *data_size) {
    if (packet[0] != DATA || size - DATA_HEADER_SIZE > MAX_DATA_SIZE) {
        return U_CHAR_ERROR;
    }

    *data_size = (unsigned int) packet[3] + (unsigned int) (packet[2] * 256);
    memcpy((void*) data, packet + DATA_HEADER_SIZE, *data_size);
    
    return packet[1];
}