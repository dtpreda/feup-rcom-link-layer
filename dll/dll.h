#ifndef DLL
#define DLL

int llopen(int port, int is_reader);

int llwrite(int fd, unsigned char* data, unsigned int data_size);

int llread(int fd, unsigned char* buffer);

int llclose(int fd);

#endif /* DLL */