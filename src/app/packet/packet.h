#ifndef PACKET
#define PACKET

#define DATA (unsigned char) '\x01'

#define START (unsigned char) '\x02'
#define END (unsigned char) '\x03'

#define DATA_HEADER_SIZE (unsigned int) 4
#define CONTROL_HEADER_SIZE(n) (unsigned int) n*2 + 1
#define MAX_PACKET_SIZE (unsigned int) 512
#define MAX_DATA_SIZE (unsigned int) MAX_PACKET_SIZE - DATA_HEADER_SIZE

unsigned int build_control_packet(unsigned char *packet, unsigned char control, unsigned char* file_name, unsigned int file_size);

unsigned char process_control_packet(unsigned char *packet, unsigned int size, unsigned char* file_name, unsigned int file_size);

unsigned int build_data_packet(unsigned char *packet, unsigned char sequence_number, unsigned char *data, unsigned int data_size);

unsigned char process_data_packet(unsigned char *packet, unsigned int size, unsigned char *data, unsigned int *data_size);

#endif /* PACKET */