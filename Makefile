make: dll/dll.c dll/frame/frame.c app/app.c app/packet/packet.c
	gcc -o frame.o dll/frame/frame.c
	gcc -o dll.o dll/dll.c frame.o
	gcc -o packet.o app/packet/packet.c
	gcc -o app.o app/app.c dll.o packet.o
