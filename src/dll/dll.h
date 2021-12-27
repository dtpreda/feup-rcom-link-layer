#ifndef DLL
#define DLL

int llopen(int port, int is_reader);

int llwrite(unsigned char* data, unsigned int data_size);

int llread(unsigned char* data);

int llclose();

#endif /* DLL */