LIBS=`pkg-config --cflags --libs allegro-5 allegro_acodec-5 allegro_audio-5 allegro_color-5 allegro_dialog-5 allegro_font-5 allegro_image-5 allegro_main-5 allegro_memfile-5 allegro_physfs-5 allegro_primitives-5 allegro_ttf-5` -lm

INCLUDES=-I/usr/include/allegro5/

all: cubo.exe
	
cubo.exe: cubo.o
	gcc $(INCLUDES) -o cubo.exe cubo.o $(LIBS)

cubo.o: cubo.c
	gcc $(INCLUDES) -c cubo.c	
	
	
clean:
	rm cubo.o
	rm cubo.exe
