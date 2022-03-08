#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_primitives.h>

//----------------------------------------- defines -----------------------------------------

#define NUMONSTROS 20

#define NAO 0
#define SIM 1

//modos de jogo
#define MENU_INICIAL 0
#define TUTORIAL 1
#define EXPLORACAO 2
#define BATALHA 3
#define VITORIA 4
#define DERROTA 5

//atributos usados durante a batalha
#define BARRAVIDA 180

#define VIDA_HEROI 100
#define ATAQUE_HEROI 5

#define VIDA_INIMIGO_COMUM 20
#define ATAQUE_INIMIGO_COMUM 2
#define PONTOS_INIMIGO_COMUM 5

#define VIDA_INIMIGO_INCOMUM 35
#define ATAQUE_INIMIGO_INCOMUM 3
#define PONTOS_INIMIGO_INCOMUM 15

#define VIDA_INIMIGO_RARO 50
#define ATAQUE_INIMIGO_RARO 5
#define PONTOS_INIMIGO_RARO 45

//tipos de opcaomenu
#define JOGAR 0
#define TUTORIAl 1
#define SAIR 2
#define ATAQUE 3
#define ESPECIAL 4
#define FUGIR 5

//tipos de estrutura
#define CENARIO 0
#define CONSTRUCAO 1
#define PROJETIL 2

//estados do heroi
#define PARADO 0
#define ANDANDO 1
#define FUGA 2

//estados do mosntro 
#define ATACANDO 0
#define ATORDOADO 1
#define MORTO 2

//atributos usados no movimento do heroi
#define CIMA 0
#define BAIXO 1
#define DIREITA 2
#define ESQUERDA 3

//raridades
#define COMUM 1
#define INCOMUM 2
#define RARO 3
#define CHEFE 4

//--------------------------------------- variaveis constantes ---------------------------------------

//define o intervalo dos eventos gerados pelo timer
const float FPS = 60;

//tamanho da tela x e y respectivamente
const int SCREEN_W = 960;
const int SCREEN_H = 540;

//--------------------------------------- estruturas de dados ---------------------------------------

//tipo de variavel da coordenada
typedef struct cord{
    float x, y;
} Cord;

//tipo de variavel de estruturas
typedef struct estrutura{
    //tamanho da estrutura
    float tamanho;
    //a cordenada da estrutura
    Cord cordte, cordtd, cordie, cordid, cordc;
    //variavel de inteiro generica
    int tipo;
    //varivael da aparencia da estrutura
    ALLEGRO_BITMAP *aparencia;
} Estrutura;

//tipo de variavel que guarda as coordenadas de um retangulo
typedef struct retangulo{
    //coordenadas do topo esquerdo e canto inferior direito
    Cord TE, IF;
} Retangulo;

//tipo de variavel do personagem
typedef struct heroi{
    //variavel que guarda a pontuacao do heroi
    int pontuacao;
    //variaveis usadas no combate
    float vida, ataque, modificador;
    int inim_enc;
    //as cordenadas do heroi
    Cord cordexp, cordtc, cordie, cordid, cordbat;
    //taman+ho do heroi
    float tamanho, tamanho_batalha;
    //variaveis que indicao direção, velocidade e se o heroi esta em movimento
    unsigned int direcao, direcao_ant, estado, velocidade;
    //variavel que vai guardar a aparencia do heroi
    ALLEGRO_BITMAP *cima, *direita, *esquerda, *baixo, *batalha;
} Heroi;

//tipo de variavel dos monstros
typedef struct monstro{
    //variavel que define qual é o inimigo
    int tipo, pontos;
    //variaveis usadas no combate
    int estado;
    float vida, ataque;
    //cordenada central do monstro
    Cord cordexp, cordbat;
    //tamanho do monstro
    float tamanho;
    //raio de colisao com o monstro
    int raiocol;
    //variavel que vai guardar a aparencia do monstro
    ALLEGRO_BITMAP *sprite;
    //variavel que vai guardar o projetil do monstro
    Estrutura projetil;
} Monstro;

//--------------------------------------- variaveis globais ---------------------------------------
Heroi heroi;
Estrutura grama;
Estrutura castelo;
Estrutura opcaomenu;
Monstro monstros[NUMONSTROS];
int modojogo, jogando;
ALLEGRO_BITMAP *menucenario;

//-------------------------------------------- modulos --------------------------------------------

//modulo que retorna o tipo do monstro de forma aleatoria
int randRaridade(){
    //cria uma variavel e armazena nela um numero aleatorio entre 0 e 99
    int random = 0;
    random = rand()%100;

    //verifica se ele e menor que 60 e retorna 1 se for
    if(random<=60){
        return COMUM;
    //verifica se ele e menor que 90 e retorna 2 se for
    }else if(random>60&&random<90){
        return INCOMUM;
    //verifica se ele e menor que 100 e retorna 3 se for
    }else if(random>=90&&random<100){
        return RARO;
    }
    return 0;
}

//modulo que retorna uma coordenada X aleatoria
float randCoordenadaX(){
    return rand()%SCREEN_W;
}

//modulo que retorna uma coordenada Y aleatoria
float randCoordenadaY(){
    return rand()%SCREEN_H;
}

//modulo que calcula a distancia de dois pontos no plano
float distancia(Cord a, Cord b){
    return sqrt(pow(a.x - b.x, 2) + pow(a.y - b.y, 2));
}

//modulo que verifica a distancia de duas coordenadas
int colisao(Cord a, Cord b, float dist){
    if(distancia(a, b) < dist){
        return 1;
    }
    return 0;
}

//modulo que verifica se a coordenada do monstro é valida
int coordenadaValidaDeMonstro(Cord monstro, float dist){
    if (!colisao(monstro, castelo.cordte, dist + castelo.tamanho/2)&&
        !colisao(monstro, castelo.cordtd, dist + castelo.tamanho/2)&&
        !colisao(monstro, castelo.cordie, dist + castelo.tamanho/2)&&
        !colisao(monstro, castelo.cordid, dist + castelo.tamanho/2)&&
        !colisao(monstro, heroi.cordexp , dist + 2*heroi.tamanho)){
        return 1;
    }
    return 0;
}

//modulo que cria os monstros e os coloca aleatoriamente no display
void criaMonstros(){
    for (int i = 0; i < NUMONSTROS; i++){
        //inicializa o tamanho do monstro
        monstros[i].tamanho = 144;

        //inicializa o raio de colisao com o monstro
        monstros[i].raiocol = 16;

        //inicializa o tipo do monstro com um tipo aleatorio
        monstros[i].tipo=randRaridade();

        //inicializa o estado do monstro 
        monstros[i].estado = ATACANDO;

        //inicializa as coordenadas do monstro na batalha
        monstros[i].cordbat.x = SCREEN_W/6 - monstros[i].tamanho/2;
        monstros[i].cordbat.y = SCREEN_H/2 - monstros[i].tamanho/2;

        //tamanho do projetil do monstro
        monstros[i].projetil.tamanho = 16;

        //a cordenada da estrutura
        monstros[i].projetil.cordte.x = monstros[i].cordbat.x + monstros[i].tamanho - 2*monstros[i].projetil.tamanho;
        monstros[i].projetil.cordte.y = monstros[i].cordbat.y;

        //define o tipo da estrutura projetil
        monstros[i].projetil.tipo = PROJETIL;

        //inicializa a cordenada do monstro com uma aleatoria
        do{
            monstros[i].cordexp.x = randCoordenadaX();
            monstros[i].cordexp.y = randCoordenadaY();
        } while (!coordenadaValidaDeMonstro(monstros[i].cordexp, monstros[i].raiocol)); 

        printf("Monstro: %d / Tipo: %d / Coordenada - X: %0.f - Y: %0.f\n", i, monstros[i].tipo, monstros[i].cordexp.x, monstros[i].cordexp.y);

        //cria a aparencia do monstro e inicializa as variaveis de combate dele
        if (monstros[i].tipo==COMUM){
            monstros[i].vida = VIDA_INIMIGO_COMUM;
            monstros[i].ataque = ATAQUE_INIMIGO_COMUM;
            monstros[i].pontos = PONTOS_INIMIGO_COMUM;
            monstros[i].sprite = al_load_bitmap("inimigo_comum.png");
            monstros[i].projetil.aparencia = al_load_bitmap("projetil.png");
        }else if (monstros[i].tipo==INCOMUM){
            monstros[i].vida = VIDA_INIMIGO_INCOMUM;
            monstros[i].ataque = ATAQUE_INIMIGO_INCOMUM;
            monstros[i].pontos = PONTOS_INIMIGO_INCOMUM;
            monstros[i].sprite = al_load_bitmap("inimigo_incomum.png");
            monstros[i].projetil.aparencia = al_load_bitmap("projetil.png");
        }else if (monstros[i].tipo==RARO){
            monstros[i].vida = VIDA_INIMIGO_RARO;
            monstros[i].ataque = ATAQUE_INIMIGO_RARO;
            monstros[i].pontos = PONTOS_INIMIGO_RARO;
            monstros[i].sprite = al_load_bitmap("inimigo_raro.png");
            monstros[i].projetil.aparencia = al_load_bitmap("projetil.png");
        }
    }
}

//modulo que carrega todos os bitmaps
void carregaBitmap(){

    //avisa o allegro que eu quero modificar as propriedades do heroi
   	al_set_target_bitmap(heroi.cima);
 
	//altera a sprite do heroi
    heroi.cima = al_load_bitmap("heroi_cima.png");
    
    //avisa o allegro que eu quero modificar as propriedades do heroi
   	al_set_target_bitmap(heroi.direita);
 
	//altera a sprite do heroi
    heroi.direita = al_load_bitmap("heroi_direita.png");
    
    //avisa o allegro que eu quero modificar as propriedades do heroi
   	al_set_target_bitmap(heroi.esquerda);
 
	//altera a sprite do heroi
    heroi.esquerda = al_load_bitmap("heroi_esquerda.png");
    
    //avisa o allegro que eu quero modificar as propriedades do heroi
   	al_set_target_bitmap(heroi.baixo);
 
	//altera a sprite do heroi
    heroi.baixo = al_load_bitmap("heroi_baixo.png");
    
    //avisa o allegro que eu quero modificar as propriedades do heroi
   	al_set_target_bitmap(heroi.batalha);

    //carrega a aparencia do heroi em batalha 
    heroi.batalha = al_load_bitmap("heroi_batalha.png");

    //avisa o allegro que eu quero modificar as propriedades do menucenario
    al_set_target_bitmap(menucenario);

    //carrega a aparencia do menucenario
    menucenario = al_load_bitmap("menu_cenario.png");

    //avisa o allegro que eu quero modificar as propriedades da grama
    al_set_target_bitmap(grama.aparencia);

    //carrega a aparencia da grama
    grama.aparencia = al_load_bitmap("grama.png");

    //avisa o allegro que eu quero modificar as propriedades do castelo
   	al_set_target_bitmap(castelo.aparencia);

    //altera a aparencia do castelo
    castelo.aparencia = al_load_bitmap("castelo.png");

    //avisa o allegro que eu quero modificar as propriedades da opcaomenu
   	al_set_target_bitmap(opcaomenu.aparencia);

    //altera a aparencia da opcaomenu
    opcaomenu.aparencia = al_load_bitmap("opcaomenu.png");

}

//modulo que inicia as variaveis globais
void initGlobalVars(){

    //inicializa a variavel que define se o jogador esta jogando
    jogando = SIM;

    //inicializa o modo de jogo
    modojogo = MENU_INICIAL;

    //inicializa o tamanho do heroi
    heroi.tamanho = 32;
    heroi.tamanho_batalha = 128;

    //inicializa a pontuacao do heroi
    heroi.pontuacao = 0;

    //inicializa as coordenadas de exploracao do heroi
    heroi.cordexp.x = SCREEN_W/100;
    heroi.cordexp.y = SCREEN_H - heroi.tamanho - SCREEN_W/100;

    heroi.cordtc.x = heroi.cordexp.x + heroi.tamanho/2;
    heroi.cordtc.y = heroi.cordexp.y;

    heroi.cordie.x = heroi.cordexp.x;
    heroi.cordie.y = heroi.cordexp.y + heroi.tamanho;

    heroi.cordid.x = heroi.cordexp.x + heroi.tamanho;
    heroi.cordid.y = heroi.cordexp.y + heroi.tamanho;

    //inicializa a coordenada de batalha do heroi
    heroi.cordbat.x = 4*SCREEN_W/5 - heroi.tamanho_batalha/2;
    heroi.cordbat.y = SCREEN_H/2 - heroi.tamanho_batalha/2;

    /*inicializa a cor do heroi
    heroi.cor = al_map_rgb(255, 255, 255);
    */

    //inicia a direcao, velocidade e o movimento do heroi
    heroi.direcao = CIMA;
    heroi.direcao_ant = CIMA;
    heroi.estado = PARADO;
    heroi.velocidade = 2;

    //inicializa as variaveis de combate do heroi
    heroi.vida = VIDA_HEROI;
    heroi.ataque = ATAQUE_HEROI;
    heroi.modificador = 1;

    //inicializa o tipo da grama
    grama.tipo = CENARIO;

    //inicializa o tamanho da grama
    grama.tamanho = 1024;

    //inicializa a coordenada da grama
    grama.cordte.x = 0;
    grama.cordte.y = 0;

    //inicializa o tipo do castelo
    castelo.tipo = CONSTRUCAO;

    //inicializa o tamanho do castelo
    castelo.tamanho = 160;

    //inicializa as coordenadas do castelo
    castelo.cordte.x = SCREEN_W - castelo.tamanho - SCREEN_W/100;
    castelo.cordte.y = SCREEN_W/100;

    castelo.cordie.x = castelo.cordte.x;
    castelo.cordie.y = castelo.cordte.y + castelo.tamanho;

    castelo.cordtd.x = castelo.cordte.x + castelo.tamanho;
    castelo.cordtd.y = castelo.cordte.y;

    castelo.cordid.x = castelo.cordte.x + castelo.tamanho;
    castelo.cordid.y = castelo.cordte.y + castelo.tamanho;

    //inicializa a opcaomenu
    opcaomenu.tipo = ATAQUE;

    //inicializa o tamanho da opcaomenu
    opcaomenu.tamanho = 16;

    //inicializa as coordenadas da opcaomenu
    opcaomenu.cordte.x = 0; 
    opcaomenu.cordte.y = 0;

    //inicializa os monstros
    criaMonstros();

    //inicializa os bitmaps
    carregaBitmap();
}

//modulo que desenha a grama
void desenhaGrama(ALLEGRO_DISPLAY *d){

    //avisa que irei mudar as propriedades do display
    al_set_target_bitmap(al_get_backbuffer(d));

    //modifica a cor do display
    al_clear_to_color(al_map_rgb(35,142,35));

    //cria um loop que so acaba quando a coordenada y da grama for maior que SCREEN_H
    while(grama.cordte.y < SCREEN_H ){
        //cria um loop que so acaba quando a coordenada x da grama for maior que SCREEN_W
        while(grama.cordte.x < SCREEN_W){
            //desenha a grama na coordenada dela
            al_draw_bitmap(grama.aparencia, grama.cordte.x, grama.cordte.y, 0);
            //muda a coordenada x da grama
            grama.cordte.x += grama.tamanho;
        }
        //muda a coordenada x da grama
        grama.cordte.x = 0;
        //muda a coordenada y da grama
        grama.cordte.y += grama.tamanho;
    }
    //volta com as coordenadas originais
    grama.cordte.x = 0;
    grama.cordte.y = 0;
}

//modulo que desenha o hub
void desenhaHub(ALLEGRO_DISPLAY *d, ALLEGRO_FONT *fonte_30, ALLEGRO_FONT *fonte_32){
    if(modojogo==TUTORIAL){
        //cria o retangulo que vai ser o menu de opcoes
        Retangulo menu;

        //salva as coordenadas do topo esquerdo do menu
        menu.TE.x = 50;
        menu.TE.y = 50;

        //salva as coordenadas do canto inferior direito do menu
        menu.IF.x = SCREEN_W - 50;
        menu.IF.y = SCREEN_H - 50;

        //desenha o retangulo do menu
        al_draw_filled_rectangle(menu.TE.x, menu.TE.y, menu.IF.x, menu.IF.y, al_map_rgb(100, 65, 165));
        al_draw_rectangle(menu.TE.x, menu.TE.y, menu.IF.x, menu.IF.y, al_map_rgb(135, 206, 235), 5);

        //variavel que guarda os textos do menu
        char tutorial[100];

        //cria um texto de ataque para o menu
        sprintf(tutorial, "Andar:");

        //imprime o texto armazenado em menu
        al_draw_text(fonte_30, al_map_rgb(0, 0, 0), menu.TE.x + 30, menu.TE.y + 30, 0, tutorial);

    }else if(modojogo==MENU_INICIAL){

        //declara que o display sera modificado
        al_set_target_bitmap(al_get_backbuffer(d));

        //desenha a aparencia do menu
        al_draw_bitmap(menucenario, 0, 0, 0);
        
        //variavel que guarda os textos do menu
        char opcoesmenu[20];

        //cria um texto de ataque para o menu
        sprintf(opcoesmenu, "JOGAR");

        //imprime o texto armazenado em menu
        al_draw_text(fonte_30, al_map_rgb(0, 0, 0), SCREEN_W/3 + SCREEN_W/22 + 2, SCREEN_H/2 - SCREEN_H/12, 0, opcoesmenu);

        //imprime o texto armazenado em menu
        al_draw_text(fonte_30, al_map_rgb(0, 0, 0), SCREEN_W/3 + SCREEN_W/22 - 2, SCREEN_H/2 - SCREEN_H/12, 0, opcoesmenu);

        //imprime o texto armazenado em menu
        al_draw_text(fonte_30, al_map_rgb(0, 0, 0), SCREEN_W/3 + SCREEN_W/22, SCREEN_H/2 - SCREEN_H/12 + 2, 0, opcoesmenu);

        //imprime o texto armazenado em menu
        al_draw_text(fonte_30, al_map_rgb(0, 0, 0), SCREEN_W/3 + SCREEN_W/22, SCREEN_H/2 - SCREEN_H/12 - 2, 0, opcoesmenu);

        //imprime o texto armazenado em menu
        al_draw_text(fonte_30, al_map_rgb(255, 255, 255), SCREEN_W/3 + SCREEN_W/22, SCREEN_H/2 - SCREEN_H/12, 0, opcoesmenu);

        //cria um texto com uma das opções do menu
        sprintf(opcoesmenu, "TUTORIAL");

        //imprime o texto armazenado em menu
        al_draw_text(fonte_30, al_map_rgb(0, 0, 0), SCREEN_W/3 + SCREEN_W/22, SCREEN_H/2 - 2, 0, opcoesmenu);

        //imprime o texto armazenado em menu
        al_draw_text(fonte_30, al_map_rgb(0, 0, 0), SCREEN_W/3 + SCREEN_W/22, SCREEN_H/2 + 2, 0, opcoesmenu);

        //imprime o texto armazenado em menu
        al_draw_text(fonte_30, al_map_rgb(0, 0, 0), SCREEN_W/3 + SCREEN_W/22 - 2, SCREEN_H/2, 0, opcoesmenu);

        //imprime o texto armazenado em menu
        al_draw_text(fonte_30, al_map_rgb(0, 0, 0), SCREEN_W/3 + SCREEN_W/22 + 2, SCREEN_H/2, 0, opcoesmenu);

        //imprime o texto armazenado em menu
        al_draw_text(fonte_30, al_map_rgb(255, 255, 255), SCREEN_W/3 + SCREEN_W/22, SCREEN_H/2, 0, opcoesmenu);

        //cria um texto com uma das opções do menu
        sprintf(opcoesmenu, "SAIR");

        //imprime o texto armazenado em menu
        al_draw_text(fonte_30, al_map_rgb(0, 0, 0), SCREEN_W/3 + SCREEN_W/22, SCREEN_H/2 + SCREEN_H/12 - 2, 0, opcoesmenu);

        //imprime o texto armazenado em menu
        al_draw_text(fonte_30, al_map_rgb(0, 0, 0), SCREEN_W/3 + SCREEN_W/22, SCREEN_H/2 + SCREEN_H/12 + 2, 0, opcoesmenu);

        //imprime o texto armazenado em menu
        al_draw_text(fonte_30, al_map_rgb(0, 0, 0), SCREEN_W/3 + SCREEN_W/22 - 2, SCREEN_H/2 + SCREEN_H/12, 0, opcoesmenu);

        //imprime o texto armazenado em menu
        al_draw_text(fonte_30, al_map_rgb(0, 0, 0), SCREEN_W/3 + SCREEN_W/22 + 2, SCREEN_H/2 + SCREEN_H/12, 0, opcoesmenu);

        //imprime o texto armazenado em menu
        al_draw_text(fonte_30, al_map_rgb(255, 255, 255), SCREEN_W/3 + SCREEN_W/22, SCREEN_H/2 + SCREEN_H/12, 0, opcoesmenu);

        //desenha o objeto opcaomenu
        al_draw_bitmap(opcaomenu.aparencia, opcaomenu.cordte.x, opcaomenu.cordte.y, 0);    

    }else if(modojogo==EXPLORACAO){

        //variavel que sera usada pra escrever a pontuacao no display    
        char pontos[20];

        //cria um texto com os pontos do heroi e o coloca na variavel pontos
        sprintf(pontos, "%d", heroi.pontuacao);

        //imprime o texto armazenado em pontos na posicao x=10,y=10 e com a cor rgb(128,200,30)
        al_draw_text(fonte_30, al_map_rgb(148, 0, 211), SCREEN_W/100, SCREEN_W/100, 0, pontos);
    }else if(modojogo==BATALHA){
            
        //cria o retangulo que vai ser o menu de opcoes
        Retangulo menu;
        //salva as coordenadas do topo esquerdo do menu
        menu.TE.x = SCREEN_W - SCREEN_W/3.5;
        menu.TE.y = SCREEN_H - SCREEN_H/4;
        //salva as coordenadas do canto inferior direito do menu
        menu.IF.x = SCREEN_W;
        menu.IF.y = SCREEN_H;

        //desenha o retangulo do menu
        al_draw_filled_rectangle(menu.TE.x, menu.TE.y, menu.IF.x, menu.IF.y, al_map_rgb(100, 65, 165));
        al_draw_rectangle(menu.TE.x, menu.TE.y, menu.IF.x, menu.IF.y, al_map_rgb(135, 206, 235), 5);

        //variavel que guarda os textos do menu
        char opcoesmenu[20];

        //cria um texto de ataque para o menu
        sprintf(opcoesmenu, "Ataque");

        //imprime o texto armazenado em menu
        al_draw_text(fonte_30, al_map_rgb(255, 255, 255), SCREEN_W - SCREEN_W/6 - SCREEN_W/11.5, SCREEN_H - 3*SCREEN_H/13, 0, opcoesmenu);

        //cria um texto com uma das opções do menu
        sprintf(opcoesmenu, "Especial");

        //imprime o texto armazenado em menu
        al_draw_text(fonte_30, al_map_rgb(255, 255, 255), SCREEN_W - SCREEN_W/6 - SCREEN_W/11.5, SCREEN_H - 2*SCREEN_H/13, 0, opcoesmenu);
        
        //cria um texto com uma das opções do menu
        sprintf(opcoesmenu, "Fugir");

        //imprime o texto armazenado em menu
        al_draw_text(fonte_30, al_map_rgb(255, 255, 255), SCREEN_W - SCREEN_W/6 - SCREEN_W/11.5, SCREEN_H - SCREEN_H/13, 0, opcoesmenu);

        //desenha o objeto opcaomenu
        al_draw_bitmap(opcaomenu.aparencia, opcaomenu.cordte.x, opcaomenu.cordte.y, 0);    

        //cria o retangulo que ira ser as barras de vida
        Retangulo barravida;
        //salva as coordenadas do topo esquerdo da vida do monstro
        barravida.TE.x = SCREEN_W/6 - BARRAVIDA/2;
        barravida.TE.y = SCREEN_H/2 - monstros[heroi.inim_enc].tamanho/1.25 - 9;
        //salva as coordenadas do canto inferior direito da vida do monstro
        barravida.IF.x = SCREEN_W/6 + BARRAVIDA/2;
        barravida.IF.y = SCREEN_H/2 - monstros[heroi.inim_enc].tamanho/1.25 + 9;

        //desenha a barra de vida do monstro
        al_draw_filled_rectangle(barravida.TE.x , barravida.TE.y, barravida.IF.x, barravida.IF.y, al_map_rgb(255, 255, 255));
        
        //muda a coordenada x inferior para indentificar o quanto de vida o monstro tem
        if(monstros[heroi.inim_enc].tipo==COMUM){
            barravida.IF.x = SCREEN_W/6 - BARRAVIDA/2 + (BARRAVIDA*(monstros[heroi.inim_enc].vida/VIDA_INIMIGO_COMUM));
        }else if(monstros[heroi.inim_enc].tipo==INCOMUM){
            barravida.IF.x = SCREEN_W/6 - BARRAVIDA/2 + (BARRAVIDA*(monstros[heroi.inim_enc].vida/VIDA_INIMIGO_INCOMUM));
        }else if(monstros[heroi.inim_enc].tipo==RARO){
            barravida.IF.x = SCREEN_W/6 - BARRAVIDA/2 + (BARRAVIDA*(monstros[heroi.inim_enc].vida/VIDA_INIMIGO_RARO));
        }

        //verifica se o monstro tem vida maior que zero e se for o caso desenha indicando qual a vida do monstro
        if(monstros[heroi.inim_enc].vida>0){
            al_draw_filled_rectangle(barravida.TE.x + BARRAVIDA/40, barravida.TE.y + BARRAVIDA/40, barravida.IF.x - BARRAVIDA/40, barravida.IF.y - BARRAVIDA/40, al_map_rgb(255, 0, 0));
        }

        //salva as coordenadas do topo esquerdo da vida do heroi
        barravida.TE.x = heroi.cordbat.x + heroi.tamanho_batalha/2 - BARRAVIDA/2;
        barravida.TE.y = heroi.cordbat.y + heroi.tamanho_batalha/2 - heroi.tamanho_batalha/1.15 - 9;
        //salva as coordenadas do canto inferior direito da vida do heroi
        barravida.IF.x = heroi.cordbat.x + heroi.tamanho_batalha/2 + BARRAVIDA/2;
        barravida.IF.y = heroi.cordbat.y + heroi.tamanho_batalha/2 - heroi.tamanho_batalha/1.15 + 9;

        //desenha a barra de vida do heroi
        al_draw_filled_rectangle(barravida.TE.x , barravida.TE.y, barravida.IF.x, barravida.IF.y, al_map_rgb(255, 255, 255));
        //muda a coordenada x inferior para indentificar o quanto de vida o heroi tem
        barravida.IF.x = heroi.cordbat.x + heroi.tamanho_batalha/2 - BARRAVIDA/2 + (BARRAVIDA*(heroi.vida/VIDA_HEROI));
        //verifica se o heroi tem vida maior que zero e se for o caso desenha indicando qual a vida do heroi
        if(heroi.vida>0){
            al_draw_filled_rectangle(barravida.TE.x + BARRAVIDA/40, barravida.TE.y + BARRAVIDA/40, barravida.IF.x  - BARRAVIDA/40, barravida.IF.y - BARRAVIDA/40, al_map_rgb(255, 0, 0));
        }
    }
    
    //reinicializa o display
    al_flip_display();
}

//modulo que desenha o heroi
void desenhaHeroi(ALLEGRO_DISPLAY *d){

    //declara que o display sera modificado
    al_set_target_bitmap(al_get_backbuffer(d));

    //desenha a aparencia do heroi na tela depois de verificar a direcao
    /*al_draw_filled_triangle(heroi.coordenada.x, heroi.coordenada.y - heroi.tamanho/2,
                            heroi.coordenada.x - heroi.tamanho/2, heroi.coordenada.y + heroi.tamanho/2,
                            heroi.coordenada.x + heroi.tamanho/2, heroi.coordenada.y + heroi.tamanho/2,
                            al_map_rgb(255, 255, 255));
    */

    if (heroi.direcao == CIMA){
        al_draw_bitmap(heroi.cima, heroi.cordexp.x, heroi.cordexp.y, 0);
    }else if (heroi.direcao == BAIXO){
        al_draw_bitmap(heroi.baixo, heroi.cordexp.x, heroi.cordexp.y, 0);
    }else if (heroi.direcao == DIREITA){
        al_draw_bitmap(heroi.direita, heroi.cordexp.x, heroi.cordexp.y, 0);
    }else if (heroi.direcao == ESQUERDA){
        al_draw_bitmap(heroi.esquerda, heroi.cordexp.x, heroi.cordexp.y, 0);
    }

}

//modulo que desenha os monstros
void desenhaCirculoMonstro(){
    int i = 0;
    for(i = 0; i < NUMONSTROS; i++){
        if(monstros[i].estado==ATACANDO){
            al_draw_filled_circle(monstros[i].cordexp.x, monstros[i].cordexp.y, monstros[i].raiocol, al_map_rgb(254, 0, 0));
        }else if(monstros[i].estado==ATORDOADO){
            al_draw_filled_circle(monstros[i].cordexp.x, monstros[i].cordexp.y, monstros[i].raiocol, al_map_rgb(0, 254, 0));
        }else if(monstros[i].estado==MORTO){
            al_draw_filled_circle(monstros[i].cordexp.x, monstros[i].cordexp.y, monstros[i].raiocol, al_map_rgb(0, 0, 0));
        }
    }
}

//modulo que desenha o cenario
void desenhaCenario(ALLEGRO_DISPLAY *d, ALLEGRO_FONT *fonte_30, ALLEGRO_FONT *fonte_32){
    /*declara que o display sera modificado
    al_set_target_bitmap(al_get_backbuffer(d));

    //modifica a cor do display
    al_clear_to_color(al_map_rgb(35,142,35));*/

    //declara que o display sera modificado
    al_set_target_bitmap(al_get_backbuffer(d));

    //desenha a grama
    desenhaGrama(d);

    //coloca o castelo na tela e destroi o bitmap do castelo
    al_draw_bitmap(castelo.aparencia, castelo.cordte.x, castelo.cordte.y, 0);

    //desenha os monstros na tela
    desenhaCirculoMonstro();

    //desenha o heroi
    desenhaHeroi(d);

    //desenha o hub
    desenhaHub(d, fonte_30, fonte_32);

    //reinicializa o display
    //al_flip_display();
}

//modulo que desenha o cenario de batalha
void desenhaBatalha(ALLEGRO_DISPLAY *d, ALLEGRO_FONT *fonte_30, ALLEGRO_FONT *fonte_32){
    //declara que o display sera modificado
    al_set_target_bitmap(al_get_backbuffer(d));

    /*modifica a cor do display
    al_clear_to_color(al_map_rgb(35,142,35));*/

    //desenha a grama
    desenhaGrama(d);

    //desenha o ceu
    al_draw_filled_rectangle(0, 0, SCREEN_W, SCREEN_H/3, al_map_rgb(135, 206, 235));

    //desenha a aparencia do projetil do monstro
    al_draw_bitmap(monstros[heroi.inim_enc].projetil.aparencia, monstros[heroi.inim_enc].projetil.cordte.x + 16, monstros[heroi.inim_enc].projetil.cordte.y, 0);

    //desenha a aparencia do monstro na tela
    al_draw_bitmap(monstros[heroi.inim_enc].sprite, monstros[heroi.inim_enc].cordbat.x, monstros[heroi.inim_enc].cordbat.y, 0);

    //desenha a aparencia do heroi na tela
    al_draw_bitmap(heroi.batalha, heroi.cordbat.x, heroi.cordbat.y, 0);

    //desenha o hub na tela
    desenhaHub(d, fonte_30, fonte_32);

    //reinicializa o display
    //al_flip_display();
}

//modulo que verifica se a pontuacao atual e um novo recorde
int novoRecorde(int pontuacao, int *recorde) {
	FILE *arq = fopen("recorde_CUBO.txt", "r");
	*recorde = -1;
	if(arq != NULL) {
		fscanf(arq, "%d", recorde);
		fclose(arq);
	}
	if(*recorde < pontuacao ) {
		arq = fopen("recorde_CUBO.txt", "w");
		fprintf(arq, "%d", pontuacao);
		fclose(arq);
		return 1;
	}
	return 0;
}

//modulo que desenha o fim do jogo
void desenhaFim(ALLEGRO_DISPLAY *d, ALLEGRO_FONT *fonte_30){

    //declara que o display sera modificado
    al_set_target_bitmap(al_get_backbuffer(d));

    //modifica a cor do display
    al_clear_to_color(al_map_rgb(0,0,0));

    int recorde = 0;
    char final[50];

    if(modojogo == VITORIA){

        //declara que o display sera modificado
        al_set_target_bitmap(al_get_backbuffer(d));

        //cria um texto com os pontos do heroi e o recorde de pontos e coloca na variavel final
        sprintf(final, "Vitoria!");

        //imprime o texto armazenado em pontos na posicao x=10,y=10 e com a cor rgb(128,200,30)
        al_draw_text(fonte_30, al_map_rgb(148, 0, 211), SCREEN_W/3, SCREEN_H/2, 0, final);

        //cria um texto com os pontos do heroi e o recorde de pontos e coloca na variavel final
        sprintf(final, "Pontuacao: %d", heroi.pontuacao);

        //imprime o texto armazenado em pontos na posicao x=10,y=10 e com a cor rgb(128,200,30)
        al_draw_text(fonte_30, al_map_rgb(148, 0, 211), SCREEN_W/3, SCREEN_H/2 + 32, 0, final);
        
        if(novoRecorde(heroi.pontuacao, &recorde)) {
            al_draw_text(fonte_30, al_map_rgb(200, 20, 30), SCREEN_W/3, 100+SCREEN_H/2, 0, "NEW RECORD!");
        }else{
            //cria um texto com os pontos do heroi e o recorde de pontos e coloca na variavel final
            sprintf(final, "Recorde: %d", recorde);

            //imprime o texto armazenado em pontos na posicao x=10,y=10 e com a cor rgb(128,200,30)
            al_draw_text(fonte_30, al_map_rgb(148, 0, 211), SCREEN_W/3, SCREEN_H/2 + 32 + 32, 0, final);
        }

    }else if(modojogo == DERROTA){
        //cria um texto com o recorde de pontos e coloca na variavel final
        sprintf(final, "Derrota!");

        //imprime o texto armazenado em pontos na posicao x=10,y=10 e com a cor rgb(128,200,30)
        al_draw_text(fonte_30, al_map_rgb(148, 0, 211), SCREEN_W/3, SCREEN_H/2, 0, final);

    }
    
    printf("\nFIM DO JOGO!\n");

    //reinicializa o display
    al_flip_display();

    //espera 5 sec
    al_rest(3.0);
}

//modulo que muda a posicao do heroi na tela
void movimentaHeroi(){
    /*coordenadas iniciais
    heroi.cordtc.x = heroi.cordexp.x + heroi.tamanho/2;
    heroi.cordtc.y = heroi.cordexp.y;

    heroi.cordie.x = heroi.cordexp.x;
    heroi.cordie.y = heroi.cordexp.y + heroi.tamanho;

    heroi.cordid.x = heroi.cordexp.x + heroi.tamanho;
    heroi.cordid.y = heroi.cordexp.y + heroi.tamanho;
    */

    //verifica se o heroi esta em movimento e se está no modo exploração
    if (heroi.estado == ANDANDO){
        //verifica qual a direcao do movimento e o faz
        if (heroi.direcao == BAIXO && heroi.cordexp.y < SCREEN_H - heroi.tamanho){
            //muda o heroi de posição
            heroi.cordexp.y += heroi.velocidade;
            
            //muda as coordenadas de colisão
            heroi.cordtc.x = heroi.cordexp.x + heroi.tamanho/2;
            heroi.cordtc.y = heroi.cordexp.y + heroi.tamanho;

            heroi.cordie.x = heroi.cordexp.x + heroi.tamanho;
            heroi.cordie.y = heroi.cordexp.y;

            heroi.cordid.x = heroi.cordexp.x;
            heroi.cordid.y = heroi.cordexp.y;
        }else if (heroi.direcao == CIMA && heroi.cordexp.y > 0){
            //muda o heroi de posição
            heroi.cordexp.y -= heroi.velocidade;

            //muda as coordenadas de colisão
            heroi.cordtc.x = heroi.cordexp.x + heroi.tamanho/2;
            heroi.cordtc.y = heroi.cordexp.y;

            heroi.cordie.x = heroi.cordexp.x;
            heroi.cordie.y = heroi.cordexp.y + heroi.tamanho;

            heroi.cordid.x = heroi.cordexp.x + heroi.tamanho;
            heroi.cordid.y = heroi.cordexp.y + heroi.tamanho;
        }else if (heroi.direcao == DIREITA && heroi.cordexp.x < SCREEN_W - heroi.tamanho){
            //muda o heroi de posição
            heroi.cordexp.x += heroi.velocidade;
            
            //muda as coordenadas de colisão
            heroi.cordtc.x = heroi.cordexp.x + heroi.tamanho;
            heroi.cordtc.y = heroi.cordexp.y + heroi.tamanho/2;

            heroi.cordie.x = heroi.cordexp.x;
            heroi.cordie.y = heroi.cordexp.y;

            heroi.cordid.x = heroi.cordexp.x;
            heroi.cordid.y = heroi.cordexp.y + heroi.tamanho;
        }else if (heroi.direcao == ESQUERDA && heroi.cordexp.x > 0){
            //muda o heroi de posição
            heroi.cordexp.x -= heroi.velocidade;
            
            //muda as coordenadas de colisão
            heroi.cordtc.x = heroi.cordexp.x;
            heroi.cordtc.y = heroi.cordexp.y + heroi.tamanho/2;

            heroi.cordie.x = heroi.cordexp.x + heroi.tamanho;
            heroi.cordie.y = heroi.cordexp.y + heroi.tamanho;

            heroi.cordid.x = heroi.cordexp.x + heroi.tamanho;
            heroi.cordid.y = heroi.cordexp.y;
        }
    //verifica se o heroi esta em movimento e no modo de batalha
    }
}

//modulo que verifica se o heroi se encontrou com algum inimigo e armazena qual inimigo foi
int encontrouInimigo(){
    for(int i = 0; i < NUMONSTROS; i++){
        if ((colisao(heroi.cordtc, monstros[i].cordexp, monstros[i].raiocol)|| 
            colisao(heroi.cordie, monstros[i].cordexp, monstros[i].raiocol)|| 
            colisao(heroi.cordid, monstros[i].cordexp, monstros[i].raiocol))&& 
            monstros[i].estado==ATACANDO){
            
            //verifica o estado do ultimo monstro encontrado e caso esteja atordoado ele muda para atacando
            if(monstros[heroi.inim_enc].estado == ATORDOADO){
                monstros[heroi.inim_enc].estado = ATACANDO;
            }
            
            //o heroi recebi o id do inimigo que ele encontrou
            heroi.inim_enc = i;
            return 1;
        }
    }
    return 0;
}

//modulo que verifica se o heroi chegou ao destino
int chegouDestino(){
    //verifica a coordenada superior direita do heroi
    if((heroi.cordexp.x + heroi.tamanho >= castelo.cordie.x)&&(heroi.cordexp.y <= castelo.cordie.y)){
        return 1;
    }
    return 0;
}

//modulo que realiza o ataque, recebe como parametro o ponteiro de inteiro da vida do alvo, outro do ataque e um inteiro que ira modificar o ataque
void ataque(float *vida_alvo, float *ataque, float modificador){
    printf("ataque: %.0f x %.2f - (%.0f)\n", *ataque, modificador, *vida_alvo);
    *vida_alvo -= (*ataque * modificador);
    printf("vida do alvo: %.0f\n", *vida_alvo);
    if(*vida_alvo<0){
        *vida_alvo = 0;
    }
}

//modulo de animacao de ataque
void ataqueAnimacao(int eheroi, ALLEGRO_DISPLAY *d, ALLEGRO_FONT *fonte_30, ALLEGRO_FONT *fonte_32){
    //variaveis do ataque do heroi
    float ci = 0;
    ci = heroi.cordbat.x;
    int vel = 0;
    vel = heroi.velocidade;
    //verifica se e o heroi atacando
    if(eheroi == SIM){
        for(float i = heroi.cordbat.x; i > SCREEN_W/6 + monstros[heroi.inim_enc].tamanho/2 ; i = heroi.cordbat.x){
            heroi.cordbat.x -= vel;
            vel += vel;
            desenhaBatalha(d, fonte_30, fonte_32);
            al_rest(0.02);
        }
        ataque(&monstros[heroi.inim_enc].vida, &heroi.ataque, heroi.modificador);
        for(float i = heroi.cordbat.x; i < ci; i = heroi.cordbat.x){
            heroi.cordbat.x += (5*heroi.velocidade)*heroi.modificador;
            desenhaBatalha(d, fonte_30, fonte_32);
            al_rest(0.0001);
        }
    }else if(eheroi == NAO){
        for(float i = monstros[heroi.inim_enc].projetil.cordte.x; i < heroi.cordbat.x + 4*monstros[heroi.inim_enc].projetil.tamanho; i = monstros[heroi.inim_enc].projetil.cordte.x){
            desenhaBatalha(d, fonte_30, fonte_32);
            monstros[heroi.inim_enc].projetil.cordte.y -= 2*sin(monstros[heroi.inim_enc].projetil.cordte.x/150.0);
            monstros[heroi.inim_enc].projetil.cordte.x += 6.5;
            al_rest(0.0000000005);
        }
        ataque(&heroi.vida, &monstros[heroi.inim_enc].ataque, 1);
    }
    //a cordenada da estrutura
    monstros[heroi.inim_enc].projetil.cordte.x = monstros[heroi.inim_enc].cordbat.x + monstros[heroi.inim_enc].tamanho - 2*monstros[heroi.inim_enc].projetil.tamanho;
    monstros[heroi.inim_enc].projetil.cordte.y = monstros[heroi.inim_enc].cordbat.y;
    desenhaBatalha(d, fonte_30, fonte_32);
    heroi.cordbat.x = ci;
}

//modulo que realiza a acao do monstro
void acaoMonstro(ALLEGRO_DISPLAY *d, ALLEGRO_FONT *fonte_30, ALLEGRO_FONT *fonte_32){
    printf("acaoMonstro\n");
    ataqueAnimacao(NAO, d, fonte_30, fonte_32);
}

//modulo que realiza a fuga mudando o modo de jogo e a coordenada do heroi
void fuga(){
    modojogo = EXPLORACAO;
    monstros[heroi.inim_enc].estado = ATORDOADO;
    heroi.estado = FUGA;
    //verifica qual a direcao do heroi e retorna o heroi a posicao anterior a dele atualmente
    if (heroi.direcao == BAIXO && heroi.cordexp.y < SCREEN_H - heroi.tamanho){
        //muda o heroi de posição
        heroi.cordexp.y -= heroi.velocidade;
            
        //muda as coordenadas de colisão
        heroi.cordtc.x = heroi.cordexp.x + heroi.tamanho/2;
        heroi.cordtc.y = heroi.cordexp.y + heroi.tamanho;

        heroi.cordie.x = heroi.cordexp.x + heroi.tamanho;
        heroi.cordie.y = heroi.cordexp.y;

        heroi.cordid.x = heroi.cordexp.x;
        heroi.cordid.y = heroi.cordexp.y;
        if(encontrouInimigo()){
            //muda o heroi de posição
            heroi.cordexp.y += heroi.velocidade;

            //muda as coordenadas de colisão
            heroi.cordtc.x = heroi.cordexp.x + heroi.tamanho/2;
            heroi.cordtc.y = heroi.cordexp.y + heroi.tamanho;

            heroi.cordie.x = heroi.cordexp.x + heroi.tamanho;
            heroi.cordie.y = heroi.cordexp.y;

            heroi.cordid.x = heroi.cordexp.x;
            heroi.cordid.y = heroi.cordexp.y;

            //muda a direcao do heroi pra anterior
            heroi.direcao = heroi.direcao_ant;

            fuga();
        }
    }else if (heroi.direcao == CIMA && heroi.cordexp.y > 0){
        //muda o heroi de posição
        heroi.cordexp.y += heroi.velocidade;

        //muda as coordenadas de colisão
        heroi.cordtc.x = heroi.cordexp.x + heroi.tamanho/2;
        heroi.cordtc.y = heroi.cordexp.y;

        heroi.cordie.x = heroi.cordexp.x;
        heroi.cordie.y = heroi.cordexp.y + heroi.tamanho;

        heroi.cordid.x = heroi.cordexp.x + heroi.tamanho;
        heroi.cordid.y = heroi.cordexp.y + heroi.tamanho;
        if(encontrouInimigo()){
            //muda o heroi de posição
            heroi.cordexp.y -= heroi.velocidade;

            //muda as coordenadas de colisão
            heroi.cordtc.x = heroi.cordexp.x + heroi.tamanho/2;
            heroi.cordtc.y = heroi.cordexp.y;

            heroi.cordie.x = heroi.cordexp.x;
            heroi.cordie.y = heroi.cordexp.y + heroi.tamanho;

            heroi.cordid.x = heroi.cordexp.x + heroi.tamanho;
            heroi.cordid.y = heroi.cordexp.y + heroi.tamanho;

            //muda a direcao do heroi pra anterior
            heroi.direcao = heroi.direcao_ant;

            fuga();
        }
    }else if (heroi.direcao == DIREITA && heroi.cordexp.x < SCREEN_W - heroi.tamanho){
        //muda o heroi de posição
        heroi.cordexp.x -= heroi.velocidade;
        
        //muda as coordenadas de colisão
        heroi.cordtc.x = heroi.cordexp.x + heroi.tamanho;
        heroi.cordtc.y = heroi.cordexp.y + heroi.tamanho/2;

        heroi.cordie.x = heroi.cordexp.x;
        heroi.cordie.y = heroi.cordexp.y;

        heroi.cordid.x = heroi.cordexp.x;
        heroi.cordid.y = heroi.cordexp.y + heroi.tamanho;
        if(encontrouInimigo()){
            //muda o heroi de posição
            heroi.cordexp.x += heroi.velocidade;
            
            //muda as coordenadas de colisão
            heroi.cordtc.x = heroi.cordexp.x + heroi.tamanho;
            heroi.cordtc.y = heroi.cordexp.y + heroi.tamanho/2;

            heroi.cordie.x = heroi.cordexp.x;
            heroi.cordie.y = heroi.cordexp.y;

            heroi.cordid.x = heroi.cordexp.x;
            heroi.cordid.y = heroi.cordexp.y + heroi.tamanho;

            //muda a direcao do heroi pra anterior
            heroi.direcao = heroi.direcao_ant;

            fuga();
        }
    }else if (heroi.direcao == ESQUERDA && heroi.cordexp.x > 0){
        //muda o heroi de posição
        heroi.cordexp.x += heroi.velocidade;
        
        //muda as coordenadas de colisão
        heroi.cordtc.x = heroi.cordexp.x;
        heroi.cordtc.y = heroi.cordexp.y + heroi.tamanho/2;

        heroi.cordie.x = heroi.cordexp.x + heroi.tamanho;
        heroi.cordie.y = heroi.cordexp.y + heroi.tamanho;

        heroi.cordid.x = heroi.cordexp.x + heroi.tamanho;
        heroi.cordid.y = heroi.cordexp.y;
        if(encontrouInimigo()){
            //muda o heroi de posição
            heroi.cordexp.x -= heroi.velocidade;
            
            //muda as coordenadas de colisão
            heroi.cordtc.x = heroi.cordexp.x;
            heroi.cordtc.y = heroi.cordexp.y + heroi.tamanho/2;

            heroi.cordie.x = heroi.cordexp.x + heroi.tamanho;
            heroi.cordie.y = heroi.cordexp.y + heroi.tamanho;

            heroi.cordid.x = heroi.cordexp.x + heroi.tamanho;
            heroi.cordid.y = heroi.cordexp.y;

            //muda a direcao do heroi pra anterior
            heroi.direcao = heroi.direcao_ant;

            fuga();
        }
    }
}

//modulo que indica se o heroi conseguiu fugir e realiza a fuga
int tentativaDeFuga(){
    if(randRaridade()==RARO){
        return 0;
    }else{
        fuga();
        return 1;
    }
}

//modulo que verifica o tipo da acao do heroi e a realiza
void acaoHeroi(int tipo, ALLEGRO_DISPLAY *d, ALLEGRO_FONT *fonte_30, ALLEGRO_FONT *fonte_32){
    //verifica qual acao deve acontecer
    if (tipo == ATAQUE){
        //se o heroi for atacar ele ataca e invoca a acao do monstro
        heroi.modificador = 1;
        ataqueAnimacao(SIM, d, fonte_30, fonte_32);
    }else if(tipo == ESPECIAL){
        //cria o modificador e atribui a ele um valor aleatorio de raridade
        float modificador = 0;
        modificador = randRaridade();
        //verifica qual a raridade do modificador e atribuiu um valor ao modificador
        if(modificador==COMUM){
            heroi.modificador = 0.75;
        }else if(modificador==INCOMUM){
            heroi.modificador = 1;
        }else if(modificador==RARO){
            heroi.modificador = 3;
        }
        //se o heroi for atacar com especial ele ataca com um modificador e invoca a acao do monstro
        ataqueAnimacao(SIM, d, fonte_30, fonte_32);
    }else if (tipo == FUGIR){
        //se o heroi quer fugir ent ele tenta e se ele conseguir ele foge caso nao consiga invoca a acao do monstro
        if(tentativaDeFuga()){
            //muda o modo de jogo e muda a posição do heroi pra ele não entrar em batalha
            printf("Fugiu!!");
        }else{
            printf("Nao fugiu!!");
        }
    }
}

//modulo que verifica se a batalha acabou, ou seja, se a vida de algum deles chegou a zero
void verificaFimDaBatalha(){
    if(heroi.vida<1){
        modojogo = DERROTA;
    }else if(monstros[heroi.inim_enc].vida<1.0){
        heroi.pontuacao += monstros[heroi.inim_enc].pontos;
        monstros[heroi.inim_enc].estado = MORTO;
        modojogo = EXPLORACAO;
    }
}

//modulo que efetua o modo de exploração
void modoMenuInicial(ALLEGRO_EVENT_QUEUE *ev_queue, ALLEGRO_DISPLAY *d, ALLEGRO_FONT *fonte_30, ALLEGRO_FONT *fonte_32){
    //enquanto o modo de jogo nao mudar efetua esse modo
    while (modojogo == MENU_INICIAL){
        
        //cria um evento
        ALLEGRO_EVENT ev;

        //aguarda que um evento e o insere na lista de eventos
        al_wait_for_event(ev_queue, &ev);

        //verifica se o evento é de timer e desenha o cenario
        if(ev.type == ALLEGRO_EVENT_KEY_DOWN) {
            //verifica qual tecla foi precionada
            switch(ev.keyboard.keycode) {
                case ALLEGRO_KEY_DOWN:
                    //muda o tipo da estrutura opcaomenu
                    if (opcaomenu.tipo==JOGAR){
                        opcaomenu.cordte.y = SCREEN_H/2 + opcaomenu.tamanho/3;
                        opcaomenu.tipo=TUTORIAl;
                    }else if (opcaomenu.tipo==TUTORIAl){
                        opcaomenu.cordte.y = SCREEN_H/2 + SCREEN_H/12 + opcaomenu.tamanho/3;
                        opcaomenu.tipo=SAIR;
                    }else if (opcaomenu.tipo==SAIR){
                        opcaomenu.cordte.y = SCREEN_H/2 - SCREEN_H/13.5;
                        opcaomenu.tipo=JOGAR;
                    }
                    break;

                case ALLEGRO_KEY_UP:
                    //muda o tipo da estrutura opcaomenu
                    if (opcaomenu.tipo==JOGAR){
                        opcaomenu.cordte.y = SCREEN_H/2 + SCREEN_H/12 + opcaomenu.tamanho/3;
                        opcaomenu.tipo=SAIR;
                    }else if (opcaomenu.tipo==TUTORIAl){
                        opcaomenu.cordte.y = SCREEN_H/2 - SCREEN_H/13.5;
                        opcaomenu.tipo=JOGAR;
                    }else if (opcaomenu.tipo==SAIR){
                        opcaomenu.cordte.y = SCREEN_H/2 + opcaomenu.tamanho/3;
                        opcaomenu.tipo=TUTORIAl;
                    }
                    break;

                case ALLEGRO_KEY_ENTER:

                    //verifica qual foi a opcao escolhida
                    if(opcaomenu.tipo==JOGAR){
                        modojogo=EXPLORACAO;
                    }else if(opcaomenu.tipo==TUTORIAl){
                        modojogo=TUTORIAL;
                    }else if(opcaomenu.tipo==SAIR){
                        //muda o modo de jogo
                        jogando=NAO;
                        modojogo=DERROTA;
                    }                    

                    //limpa a lista de eventos
                    al_flush_event_queue(ev_queue);
                    break;
            }
        }else if(ev.type == ALLEGRO_EVENT_TIMER){
            //desenha o cenario do jogo no display
            desenhaHub(d, fonte_30, fonte_32);
        //verifica se o evento na lista é o comando do display de fecha-lo
        }else if(ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
            //muda o modo de jogo
            jogando = NAO;
            modojogo = DERROTA;
        }
    }
}

//modulo do modo tutorial
void modoTutorial(ALLEGRO_EVENT_QUEUE *ev_queue){
    //enquanto o modo de jogo nao mudar efetua esse modo

    //cria um evento
    ALLEGRO_EVENT ev;

    //aguarda que um evento e o insere na lista de eventos
    al_wait_for_event(ev_queue, &ev);

    //verifica se alguma tecla esta sendo precionada
    if(ev.type == ALLEGRO_EVENT_KEY_DOWN){
        switch(ev.keyboard.keycode) {
            case ALLEGRO_KEY_ENTER:
                modojogo = MENU_INICIAL;
                break;
        }
    }

    //verifica se o evento e de fechar a tela
    if(ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
        //muda o modo de jogo
        jogando = NAO;
        modojogo = DERROTA;
    }
}

//modulo que efetua o modo de exploração
void modoExploracao(ALLEGRO_EVENT_QUEUE *ev_queue, ALLEGRO_DISPLAY *d, ALLEGRO_FONT *fonte_30, ALLEGRO_FONT *fonte_32){
    //enquanto o modo de jogo nao mudar efetua esse modo

    //cria um evento
    ALLEGRO_EVENT ev;

    //aguarda que um evento e o insere na lista de eventos
    al_wait_for_event(ev_queue, &ev);

    //verifica se alguma tecla esta sendo precionada
    if(ev.type == ALLEGRO_EVENT_KEY_DOWN) {
        //verifica qual tecla foi precionada
        switch(ev.keyboard.keycode){
                    
            case ALLEGRO_KEY_DOWN:
                heroi.estado = ANDANDO;
                heroi.direcao_ant = heroi.direcao;
                heroi.direcao = BAIXO;
                break;

            case ALLEGRO_KEY_UP:
                heroi.estado = ANDANDO;
                heroi.direcao_ant = heroi.direcao;
                heroi.direcao = CIMA;
                break;

            case ALLEGRO_KEY_RIGHT:
                heroi.estado = ANDANDO;
                heroi.direcao_ant = heroi.direcao;
                heroi.direcao = DIREITA;
                break;
                    
            case ALLEGRO_KEY_LEFT:
                heroi.estado = ANDANDO;
                heroi.direcao_ant = heroi.direcao;
                heroi.direcao = ESQUERDA;
            break;

            case ALLEGRO_KEY_ESCAPE:
                modojogo=MENU_INICIAL;
            break;
        }
    //verifica se alguma tecla deixou de ser precionada
    }else if(ev.type == ALLEGRO_EVENT_KEY_UP) {
        //verifica qual tecla foi
        switch(ev.keyboard.keycode) {
            case ALLEGRO_KEY_DOWN:

                if (heroi.direcao == BAIXO)                    
                    heroi.estado = PARADO;
                break;
                    
            case ALLEGRO_KEY_UP:
                if (heroi.direcao == CIMA)
                    heroi.estado = PARADO;
                break;

            case ALLEGRO_KEY_RIGHT:
                if (heroi.direcao == DIREITA)
                    heroi.estado = PARADO;
                break;

            case ALLEGRO_KEY_LEFT:
                if (heroi.direcao == ESQUERDA)
                    heroi.estado = PARADO;
                break;
        }
    //verifica se ouve um evento do timer e atualiza a tela
    }else if(ev.type == ALLEGRO_EVENT_TIMER){
        //desenha o cenario do jogo no display
        desenhaCenario(d, fonte_30, fonte_32);
    //verifica se o evento na lista é o comando do display de fecha-lo
    }else if(ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
        //muda o modo de jogo
        jogando = NAO;
        modojogo = DERROTA;
    }

    //movimenta o heroi
    movimentaHeroi();

    //verifica se o heroi chegou no destino
    if(chegouDestino()){
        //muda o modo de jogo
        modojogo = VITORIA;
    }

    //verifica se o heroi se encontrou com um inimigo e muda o modo de jogo para batalha
    if(encontrouInimigo()){
        //define que o heroi nao pode se movimentar
        heroi.estado = PARADO;
        modojogo = BATALHA;
    }
}

//modulo que efetua o modo batalha
void modoBatalha(ALLEGRO_EVENT_QUEUE *ev_queue, ALLEGRO_DISPLAY *d, ALLEGRO_FONT *fonte_30, ALLEGRO_FONT *fonte_32){
    //enquanto o modo de jogo nao mudar efetua esse modo
    while (modojogo == BATALHA){
        
        //cria um evento
        ALLEGRO_EVENT ev;

        //aguarda que um evento e o insere na lista de eventos
        al_wait_for_event(ev_queue, &ev);

        //verifica se alguma tecla esta sendo precionada
        if(ev.type == ALLEGRO_EVENT_KEY_DOWN) {
            //verifica qual tecla foi precionada
            switch(ev.keyboard.keycode) {
                case ALLEGRO_KEY_DOWN:
                    //muda o tipo da estrutura opcaomenu
                    if (opcaomenu.tipo==ATAQUE){
                        opcaomenu.cordte.y = SCREEN_H - 2*SCREEN_H/12 + opcaomenu.tamanho/2;
                        opcaomenu.tipo=ESPECIAL;
                    }else if (opcaomenu.tipo==ESPECIAL){
                        opcaomenu.cordte.y = SCREEN_H - SCREEN_H/12 + opcaomenu.tamanho/2;
                        opcaomenu.tipo=FUGIR;
                    }else if (opcaomenu.tipo==FUGIR){
                        opcaomenu.cordte.y = SCREEN_H - 3*SCREEN_H/12 + opcaomenu.tamanho;
                        opcaomenu.tipo=ATAQUE;
                    }
                    break;

                case ALLEGRO_KEY_UP:
                    //muda o tipo da estrutura opcaomenu
                    if (opcaomenu.tipo==ATAQUE){
                        opcaomenu.cordte.y = SCREEN_H - SCREEN_H/12 + opcaomenu.tamanho/2;
                        opcaomenu.tipo=FUGIR;
                    }else if (opcaomenu.tipo==ESPECIAL){
                        opcaomenu.cordte.y = SCREEN_H - 3*SCREEN_H/12 + opcaomenu.tamanho;
                        opcaomenu.tipo=ATAQUE;
                    }else if (opcaomenu.tipo==FUGIR){
                        opcaomenu.cordte.y = SCREEN_H - 2*SCREEN_H/12 + opcaomenu.tamanho/2;
                        opcaomenu.tipo=ESPECIAL;
                    }
                    break;

                case ALLEGRO_KEY_ENTER:
                    //faz a acao do heroi
                    acaoHeroi(opcaomenu.tipo, d, fonte_30, fonte_32);
                        
                    //verifica se a batalha acabou e muda o modo de jogo
                    verificaFimDaBatalha();

                    //faz a acao do monstro se ele não estiver morto
                    if(monstros[heroi.inim_enc].vida>0 && heroi.estado != FUGA){
                        acaoMonstro(d, fonte_30, fonte_32);
                    }

                    //limpa a lista de eventos
                    al_flush_event_queue(ev_queue);
                    break;
            }  
        //verifica se ouve um evento do timer e atualiza a tela
        }else if (ev.type == ALLEGRO_EVENT_TIMER){
            //desenha o cenario de batalha no display
            desenhaBatalha(d, fonte_30, fonte_32);
        //verifica se o evento na lista é o comando do display de fecha-lo
        }else if(ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
            //muda o modo de jogo
            jogando = NAO;
            modojogo = DERROTA;
        }
    }
}

//no total 27 modulos até agr sem contar o main

//------------------------------------------- main -------------------------------------------

//modulo main do jogo
int main(int argc, char **argv){

    //define a seed do rand
    srand(time(NULL));

    //declaração do display, queue de eventos e do timer
    ALLEGRO_DISPLAY *display = NULL;
    ALLEGRO_EVENT_QUEUE *event_queue = NULL;
    ALLEGRO_TIMER *timer = NULL;

    //----------------------- rotinas de inicializacao ---------------------------------------
    
    //cria e em seguida verifica se o allegro foi inicializado
    al_init();
    if(!al_init()){
        fprintf(stderr, "failed to initialize allegro!\n");
        return -1;
    }

    //inicializa o modulo que permite carregar imagens no jogo
    al_init_image_addon();
    if(!al_init_image_addon()){
        fprintf(stderr, "failed to initialize image_addon!\n");
        return -1;
    }
    
    //cria e em seguida verifica se o timer foi criado
    timer = al_create_timer(1.0 / FPS);
    if(!timer){
        fprintf(stderr, "failed to create timer!\n");
        return -1;
    }

    //cria e em seguida verifica se o display foi criado
    display = al_create_display(SCREEN_W, SCREEN_H);
    if(!display){
        fprintf(stderr, "failed to create display!\n");
        al_destroy_timer(timer);
        return -1;
    }

    //cria e em seguida verifica se a lista de eventos foi criado
    event_queue = al_create_event_queue();
    if(!event_queue){
        fprintf(stderr, "failed to create event_queue!\n");
        al_destroy_display(display);
        al_destroy_timer(timer);
        return -1;
    }

    //declara o teclado para o programa
	al_install_keyboard();
    //inicializa o modulo allegro que cria formas primitivas
    al_init_primitives_addon();
    //inicializa o modulo allegro que carrega as fontes
    al_init_font_addon();
	//inicializa o modulo allegro que entende arquivos tff de fontes
    al_init_ttf_addon();

    //declara o que a lista de eventos vai entender como um evento
    al_register_event_source(event_queue, al_get_display_event_source(display));
    al_register_event_source(event_queue, al_get_timer_event_source(timer));
   	al_register_event_source(event_queue, al_get_keyboard_event_source());

	//carrega o arquivo arial.ttf da fonte Arial e define que sera usado o tamanho 32 (segundo parametro)
    ALLEGRO_FONT *fonte_30 = al_load_font("PressStart2P.ttf", 30, 1);

    //carrega o arquivo arial.ttf da fonte Arial e define que sera usado o tamanho 32 (segundo parametro)
    ALLEGRO_FONT *fonte_32 = al_load_font("PressStart2P.ttf", 15, 1);

    //inicializa todas as variaveis globais
    initGlobalVars();

    //reinicializa o display
    al_flip_display();

    //inicializa o timer
    al_start_timer(timer);
    //----------------------- fim das rotinas de inicializacao ---------------------------------------

    //cria um loop infinito que verificar os eventos e rodar o jogo
    while(jogando){

        //verifica o modo de jogo
        if(modojogo == MENU_INICIAL){
            
            //muda a opcaomenu
            opcaomenu.tipo = JOGAR;
    
            //inicializa as coordenadas da opcaomenu
            opcaomenu.cordte.x = SCREEN_W/3 + SCREEN_W/22 - 1.5*opcaomenu.tamanho; 
            opcaomenu.cordte.y = SCREEN_H/2 - SCREEN_H/13.5;

            //efetua o modo de jogo atual
            modoMenuInicial(event_queue, display, fonte_30, fonte_32);
        }else if(modojogo == TUTORIAL){
            //desenha o tutorial na tela
            desenhaHub(display, fonte_30, fonte_32);
            
            //efetua o modo tutorial
            modoTutorial(event_queue);
        }else if(modojogo == EXPLORACAO){
            //efetua o modo de jogo atual
            modoExploracao(event_queue, display, fonte_30, fonte_32);
        }else if(modojogo == BATALHA){

            //muda a opcaomenu
            opcaomenu.tipo = ATAQUE;

            //inicializa as coordenadas da opcaomenu
            opcaomenu.cordte.y = SCREEN_H - 3*SCREEN_H/12 + opcaomenu.tamanho;
            opcaomenu.cordte.x = SCREEN_W - SCREEN_W/6 - SCREEN_W/13.5 - 2.2*opcaomenu.tamanho;

            //efetua o modo de jogo atual
            modoBatalha(event_queue, display, fonte_30, fonte_32);
        }else if(jogando==SIM && (modojogo == DERROTA || modojogo == VITORIA)){
            //desenha a tela final
            desenhaFim(display, fonte_30);
            
            //define que o jogado nao esta jogando
            jogando = NAO;
        }

    }// fim do while

    //----------------------- rotinas de finalizacao ---------------------------------------
    al_destroy_timer(timer);
    al_destroy_display(display);
    al_destroy_event_queue(event_queue);
    //----------------------- fim das rotinas de finalizacao ---------------------------------------

    return 0;
}
//final do modulo main