#ifndef DLL
#define DLL

int llopen(int port, int is_reader);

int llwrite(int fd, unsigned char* buffer, unsigned int length);

int llread(int fd, unsigned char* buffer);

int llclose(int fd);

#endif /* DLL */