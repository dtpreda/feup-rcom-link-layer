#ifndef DLL
#define DLL

int llopen(int port);

int llwrite(int fd, char* buffer, int length);

int llread(int fd, char* buffer);

int llclose(int fd);

#endif /* DLL */