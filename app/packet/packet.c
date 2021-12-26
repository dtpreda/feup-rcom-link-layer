#pragma once

#include <stdlib.h>
#include <stdio.h>

#include "packet.h"

unsigned int build_control_packet(unsigned char *packet, unsigned char control, unsigned char* file_name, unsigned int file_size) {
    return 0;
}

unsigned int process_control_packet(unsigned char *packet, unsigned int size) {
    return 0;
}

unsigned int build_data_packet(unsigned char *packet, unsigned char *data, unsigned int *data_size) {
    return 0;
}

unsigned int process_data_packet(unsigned char *packet, unsigned int size, unsigned char *data, unsigned int *data_size) {
    return 0;
}