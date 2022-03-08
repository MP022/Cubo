ALLEGRO_VERSION=5.0.10
MINGW_VERSION=4.7.0
FOLDER=C:

FOLDER_NAME=\allegro-$(ALLEGRO_VERSION)-mingw-$(MINGW_VERSION)
PATH_ALLEGRO=$(FOLDER)$(FOLDER_NAME)
LIB_ALLEGRO=\lib\liballegro-$(ALLEGRO_VERSION)-monolith-mt.a
INCLUDE_ALLEGRO=\include

all: cubo.exe
	
cubo.exe: cubo.o
	gcc -o cubo.exe cubo.o $(PATH_ALLEGRO)$(LIB_ALLEGRO)

cubo.o: cubo.c
	gcc -I $(PATH_ALLEGRO)$(INCLUDE_ALLEGRO) -c cubo.c
	
	
clean:
	del cubo.o
	del cubo.exe
