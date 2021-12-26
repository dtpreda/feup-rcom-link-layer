#ifndef FRAME
#define FRAME

// Address Field bit sequences
#define A_CSAR (unsigned char) '\x03'
#define A_CRAS (unsigned char) '\x01'

// Control Field bit sequences
#define SET (unsigned char) '\x03'
#define DISC (unsigned char) '\x0B'
#define UA (unsigned char) '\x07'
#define SEND_I(n) (unsigned char) ((n << 6) | '\x00')
#define RR(n) (unsigned char) ((n << 7) | '\x05')
#define REJ(n) (unsigned char) ((n << 7) | '\x01')

#define MAX_SIZE 2048

void build_su_frame(unsigned char* frame, unsigned char address, unsigned char control, unsigned int is_s);

int build_i_frame(unsigned char *frame, unsigned char address, unsigned char control, unsigned char *data, unsigned int data_size);

#endif FRAME