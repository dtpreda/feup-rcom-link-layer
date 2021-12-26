#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "frame.h"
#include "../../common/common.h"

#define FLAG (unsigned char) '\x7e'

#define ESCAPE_FIRST (unsigned char) '\x7d'
#define ESCAPE_SECOND (unsigned char) '\x5e'
#define ESCAPEX2_FIRST (unsigned char) '\x7d'
#define ESCAPEX2_SECOND (unsigned char) '\x5d'

#define MAX_DATA_SIZE 1024

static unsigned char xor (unsigned char *data, unsigned int size) {
    if (size < 2)
    {
        return 0;
    }
    unsigned char _xor = data[0];
    for (unsigned int i = 1; i < size; i++)
    {
        _xor = _xor ^ data[i];
    }
    return _xor;
}

static int byte_stuffing(unsigned char* data, unsigned int* size) {
    for (unsigned int i = 0; i < (*size) - 1; i++) {
        if (data[i] == (unsigned char)FLAG) {
            if (*size == MAX_DATA_SIZE) {
                return ERROR;
            }
            *size = (*size) + 1;

            data[i] = ESCAPE_FIRST;
            for (int j = (*size) - 1; j >= i +1; j--) {
                data[j + 1] = data[j];
            }
            data[i + 1] = ESCAPE_SECOND;
        } 
        else if (data[i] == (unsigned char) ESCAPE_FIRST) {
            if (*size == MAX_DATA_SIZE) {
                return ERROR;
            }
            *size = (*size) + 1;
            
            data[i] = ESCAPEX2_FIRST;
            for (int j = (*size) - 1; j >= i +1; j--) {
                data[j + 1] = data[j];
            }
            data[i + 1] = ESCAPEX2_SECOND;
        }
    }
    return SUCCESS;
}

static void byte_destuffing(unsigned char* data, unsigned int* size) {
    unsigned int less = 0;

    for (int i = 0; i < (*size) - 1; i++) {
        if (data[i] == ESCAPE_FIRST && data[i+1] == ESCAPE_SECOND) {
            data[i] = (unsigned char) FLAG;

            for(int j = i + 1; j < (*size) - 1; j++) {
                data[j] = data[j+1];
            }

            less++;
        }
        else if (data[i] == ESCAPEX2_FIRST && data[i+1] == ESCAPEX2_SECOND) {
            data[i] = (unsigned char) ESCAPE_FIRST;

            for(int j = i + 1; j < (*size) - 1; j++) {
                data[j] = data[j+1];
            }

            less++;
        }
    }
    
    *size = (*size) - less * sizeof(unsigned char);
}

void build_su_frame(unsigned char *frame, unsigned char address, unsigned char control, unsigned int is_s) {
    frame[0] = FLAG;
    frame[1] = address;
    frame[2] = control;

    unsigned char _to_bcc[2] = {address, control};
    frame[3] = xor(_to_bcc, 2);
    frame[4] = FLAG;

    return 5;
}

int build_i_frame(unsigned char *frame, unsigned char address, unsigned char control, unsigned char *data, unsigned int data_size) {
    unsigned int _data_size = data_size;
    unsigned char dbcc = xor(data, data_size);
    if (byte_stuffing(data, &_data_size) == ERROR)
    {
        return ERROR;
    }

    if (_data_size > MAX_DATA_SIZE) {
        return ERROR;
    }

    frame[0] = FLAG;
    frame[1] = address;
    frame[2] = control;

    unsigned char _to_bcc[2] = {address, control};
    frame[3] = xor(_to_bcc, 2);

    memcpy(((void *)frame) + 4, data, (size_t)_data_size);
    frame[4 + data_size] = dbcc;
    frame[4 + data_size + 1] = FLAG;

    return SUCCESS;
}
