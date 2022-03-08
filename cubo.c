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

//estados do monstro 
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
    int tipo;
    float tamanho;
    Cord cordte, cordtd, cordie, cordid, cordc;
    ALLEGRO_BITMAP *aparencia;
} Estrutura;

//tipo de variavel que guarda as coordenadas de um retangulo
typedef struct retangulo{
    //coordenadas do topo esquerdo e canto inferior direito
    Cord TE, IF;
} Retangulo;

//tipo de variavel do personagem
typedef struct heroi{
    int pontuacao;
    float vida, ataque, modificador;
    int inim_enc;
    Cord cordexp, cordtc, cordie, cordid, cordbat;
    float tamanho, tamanho_batalha;
    unsigned int direcao, direcao_ant, estado, velocidade;
    ALLEGRO_BITMAP *cima, *direita, *esquerda, *baixo, *batalha;
} Heroi;

//tipo de variavel dos monstros
typedef struct monstro{
    int tipo, pontos;
    int estado;
    float vida, ataque;
    Cord cordexp, cordbat;
    float tamanho;
    int raiocol;
    ALLEGRO_BITMAP *sprite;
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
    int random = 0;
    random = rand()%100;

    if(random<=60){
        return COMUM;
    }else if(random>60&&random<90){
        return INCOMUM;
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

//modulo que verifica se distancia de duas coordenadas e menor que a desejada 
int colisao(Cord a, Cord b, float dist){
    if(distancia(a, b) < dist){
        return 1;
    }
    return 0;
}

//modulo que verifica se a coordenada do monstro e valida
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

//modulo que cria os monstros e os distribui aleatoriamente no mapa
void criaMonstros(){
    for (int i = 0; i < NUMONSTROS; i++){
        //inicializa as variaveis baisicas do monstro
        monstros[i].tamanho = 144;
        monstros[i].raiocol = 16;
        monstros[i].tipo = randRaridade();
        monstros[i].estado = ATACANDO;
        monstros[i].cordbat.x = SCREEN_W/6 - monstros[i].tamanho/2;
        monstros[i].cordbat.y = SCREEN_H/2 - monstros[i].tamanho/2;
        
        //inicializa as variaveis do projetil do monstro
        monstros[i].projetil.tamanho = 16;
        monstros[i].projetil.cordte.x = monstros[i].cordbat.x + monstros[i].tamanho - 2*monstros[i].projetil.tamanho;
        monstros[i].projetil.cordte.y = monstros[i].cordbat.y;
        monstros[i].projetil.tipo = PROJETIL;

        //inicializa o monstro com uma cordenada aleatoria
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
            monstros[i].sprite = al_load_bitmap("Personagens/inimigo_comum.png");
            monstros[i].projetil.aparencia = al_load_bitmap("Personagens/projetil.png");
        }else if (monstros[i].tipo==INCOMUM){
            monstros[i].vida = VIDA_INIMIGO_INCOMUM;
            monstros[i].ataque = ATAQUE_INIMIGO_INCOMUM;
            monstros[i].pontos = PONTOS_INIMIGO_INCOMUM;
            monstros[i].sprite = al_load_bitmap("Personagens/inimigo_incomum.png");
            monstros[i].projetil.aparencia = al_load_bitmap("Personagens/projetil.png");
        }else if (monstros[i].tipo==RARO){
            monstros[i].vida = VIDA_INIMIGO_RARO;
            monstros[i].ataque = ATAQUE_INIMIGO_RARO;
            monstros[i].pontos = PONTOS_INIMIGO_RARO;
            monstros[i].sprite = al_load_bitmap("Personagens/inimigo_raro.png");
            monstros[i].projetil.aparencia = al_load_bitmap("Personagens/projetil.png");
        }
    }
}

//modulo que carrega todos os bitmaps
void carregaBitmap(){
   	al_set_target_bitmap(heroi.cima);
    heroi.cima = al_load_bitmap("Personagens/heroi_cima.png");

   	al_set_target_bitmap(heroi.direita);
    heroi.direita = al_load_bitmap("Personagens/heroi_direita.png");

   	al_set_target_bitmap(heroi.esquerda);
    heroi.esquerda = al_load_bitmap("Personagens/heroi_esquerda.png");

   	al_set_target_bitmap(heroi.baixo);
    heroi.baixo = al_load_bitmap("Personagens/heroi_baixo.png");

   	al_set_target_bitmap(heroi.batalha);
    heroi.batalha = al_load_bitmap("Personagens/heroi_batalha.png");

    al_set_target_bitmap(menucenario);
    menucenario = al_load_bitmap("Cenarios/menu_cenario.png");

    al_set_target_bitmap(grama.aparencia);
    grama.aparencia = al_load_bitmap("Cenarios/grama.png");

   	al_set_target_bitmap(castelo.aparencia);
    castelo.aparencia = al_load_bitmap("Cenarios/castelo.png");

   	al_set_target_bitmap(opcaomenu.aparencia);
    opcaomenu.aparencia = al_load_bitmap("Personagens/opcaomenu.png");
}

//modulo que inicia as variaveis globais
void initGlobalVars(){
    jogando = SIM;
    modojogo = MENU_INICIAL;

    //inicializa as variaveis do heroi
    heroi.tamanho = 32;
    heroi.tamanho_batalha = 128;
    heroi.pontuacao = 0;

    heroi.cordexp.x = SCREEN_W/100;
    heroi.cordexp.y = SCREEN_H - heroi.tamanho - SCREEN_W/100;

    heroi.cordtc.x = heroi.cordexp.x + heroi.tamanho/2;
    heroi.cordtc.y = heroi.cordexp.y;

    heroi.cordie.x = heroi.cordexp.x;
    heroi.cordie.y = heroi.cordexp.y + heroi.tamanho;

    heroi.cordid.x = heroi.cordexp.x + heroi.tamanho;
    heroi.cordid.y = heroi.cordexp.y + heroi.tamanho;

    heroi.cordbat.x = 4*SCREEN_W/5 - heroi.tamanho_batalha/2;
    heroi.cordbat.y = SCREEN_H/2 - heroi.tamanho_batalha/2;

    heroi.direcao = CIMA;
    heroi.direcao_ant = CIMA;
    heroi.estado = PARADO;
    heroi.velocidade = 2;

    heroi.vida = VIDA_HEROI;
    heroi.ataque = ATAQUE_HEROI;
    heroi.modificador = 1;

    //inicializa as variaveis da grama
    grama.tipo = CENARIO;
    grama.tamanho = 1024;

    grama.cordte.x = 0;
    grama.cordte.y = 0;

    //inicializa as variaveis do castelo
    castelo.tipo = CONSTRUCAO;
    castelo.tamanho = 160;

    castelo.cordte.x = SCREEN_W - castelo.tamanho - SCREEN_W/100;
    castelo.cordte.y = SCREEN_W/100;

    castelo.cordie.x = castelo.cordte.x;
    castelo.cordie.y = castelo.cordte.y + castelo.tamanho;

    castelo.cordtd.x = castelo.cordte.x + castelo.tamanho;
    castelo.cordtd.y = castelo.cordte.y;

    castelo.cordid.x = castelo.cordte.x + castelo.tamanho;
    castelo.cordid.y = castelo.cordte.y + castelo.tamanho;

    //inicializa  as variaveis do opcaomenu
    opcaomenu.tipo = ATAQUE;
    opcaomenu.tamanho = 16;

    opcaomenu.cordte.x = 0; 
    opcaomenu.cordte.y = 0;

    //inicializa os monstros
    criaMonstros();

    //inicializa os bitmaps
    carregaBitmap();
}

//modulo que desenha o heroi
void desenhaHeroi(ALLEGRO_DISPLAY *d){
    al_set_target_bitmap(al_get_backbuffer(d));

    //desenha o heroi na direcao que ele esta
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
void desenhaCirculoMonstro(ALLEGRO_DISPLAY *d){
    al_set_target_bitmap(al_get_backbuffer(d));
    //muda a cor do monstro dependendo do seu estado
    for(int i = 0; i < NUMONSTROS; i++){
        if(monstros[i].estado==ATACANDO){
            al_draw_filled_circle(monstros[i].cordexp.x, monstros[i].cordexp.y, monstros[i].raiocol, al_map_rgb(254, 0, 0));
        }else if(monstros[i].estado==ATORDOADO){
            al_draw_filled_circle(monstros[i].cordexp.x, monstros[i].cordexp.y, monstros[i].raiocol, al_map_rgb(0, 254, 0));
        }else if(monstros[i].estado==MORTO){
            al_draw_filled_circle(monstros[i].cordexp.x, monstros[i].cordexp.y, monstros[i].raiocol, al_map_rgb(0, 0, 0));
        }
    }
}

//modulo que desenha o hub
void desenhaHub(ALLEGRO_DISPLAY *d, ALLEGRO_FONT *fonte_30, ALLEGRO_FONT *fonte_15){
    if(modojogo==TUTORIAL){
        //inicializa e desenha o retangulo do tutorial
        Retangulo menu;
        menu.TE.x = 50;
        menu.TE.y = 50;
        menu.IF.x = SCREEN_W - 50;
        menu.IF.y = SCREEN_H - 50;
        al_draw_filled_rectangle(menu.TE.x, menu.TE.y, menu.IF.x, menu.IF.y, al_map_rgb(100, 65, 165));
        al_draw_rectangle(menu.TE.x, menu.TE.y, menu.IF.x, menu.IF.y, al_map_rgb(135, 206, 235), 5);

        //imprime os textos do tutorial
        char tutorial[100];
        sprintf(tutorial, "Andar:");
        al_draw_text(fonte_30, al_map_rgb(0, 0, 0), menu.TE.x + 30, menu.TE.y + 30, 0, tutorial);

        //modo de jogo incompleto
    }else if(modojogo==MENU_INICIAL){
        //imprime os textos do menu
        char opcoesmenu[20];
        sprintf(opcoesmenu, "JOGAR");
        al_draw_text(fonte_30, al_map_rgb(0, 0, 0), SCREEN_W/3 + SCREEN_W/22 + 2, SCREEN_H/2 - SCREEN_H/12, 0, opcoesmenu);
        al_draw_text(fonte_30, al_map_rgb(0, 0, 0), SCREEN_W/3 + SCREEN_W/22 - 2, SCREEN_H/2 - SCREEN_H/12, 0, opcoesmenu);
        al_draw_text(fonte_30, al_map_rgb(0, 0, 0), SCREEN_W/3 + SCREEN_W/22, SCREEN_H/2 - SCREEN_H/12 + 2, 0, opcoesmenu);
        al_draw_text(fonte_30, al_map_rgb(0, 0, 0), SCREEN_W/3 + SCREEN_W/22, SCREEN_H/2 - SCREEN_H/12 - 2, 0, opcoesmenu);

        al_draw_text(fonte_30, al_map_rgb(255, 255, 255), SCREEN_W/3 + SCREEN_W/22, SCREEN_H/2 - SCREEN_H/12, 0, opcoesmenu);

        sprintf(opcoesmenu, "TUTORIAL");
        al_draw_text(fonte_30, al_map_rgb(0, 0, 0), SCREEN_W/3 + SCREEN_W/22, SCREEN_H/2 - 2, 0, opcoesmenu);
        al_draw_text(fonte_30, al_map_rgb(0, 0, 0), SCREEN_W/3 + SCREEN_W/22, SCREEN_H/2 + 2, 0, opcoesmenu);
        al_draw_text(fonte_30, al_map_rgb(0, 0, 0), SCREEN_W/3 + SCREEN_W/22 - 2, SCREEN_H/2, 0, opcoesmenu);
        al_draw_text(fonte_30, al_map_rgb(0, 0, 0), SCREEN_W/3 + SCREEN_W/22 + 2, SCREEN_H/2, 0, opcoesmenu);

        al_draw_text(fonte_30, al_map_rgb(255, 255, 255), SCREEN_W/3 + SCREEN_W/22, SCREEN_H/2, 0, opcoesmenu);

        sprintf(opcoesmenu, "SAIR");
        al_draw_text(fonte_30, al_map_rgb(0, 0, 0), SCREEN_W/3 + SCREEN_W/22, SCREEN_H/2 + SCREEN_H/12 - 2, 0, opcoesmenu);
        al_draw_text(fonte_30, al_map_rgb(0, 0, 0), SCREEN_W/3 + SCREEN_W/22, SCREEN_H/2 + SCREEN_H/12 + 2, 0, opcoesmenu);
        al_draw_text(fonte_30, al_map_rgb(0, 0, 0), SCREEN_W/3 + SCREEN_W/22 - 2, SCREEN_H/2 + SCREEN_H/12, 0, opcoesmenu);
        al_draw_text(fonte_30, al_map_rgb(0, 0, 0), SCREEN_W/3 + SCREEN_W/22 + 2, SCREEN_H/2 + SCREEN_H/12, 0, opcoesmenu);

        al_draw_text(fonte_30, al_map_rgb(255, 255, 255), SCREEN_W/3 + SCREEN_W/22, SCREEN_H/2 + SCREEN_H/12, 0, opcoesmenu);

        //desenha o objeto opcaomenu
        al_draw_bitmap(opcaomenu.aparencia, opcaomenu.cordte.x, opcaomenu.cordte.y, 0);    
    }else if(modojogo==EXPLORACAO){
        //imprime a puontução do heroi
        char pontos[20];
        sprintf(pontos, "%d", heroi.pontuacao);
        al_draw_text(fonte_30, al_map_rgb(148, 0, 211), SCREEN_W/100, SCREEN_W/100, 0, pontos);
    }else if(modojogo==BATALHA){
        //desenha o menu 
        Retangulo menu;
        menu.TE.x = SCREEN_W - SCREEN_W/3.5;
        menu.TE.y = SCREEN_H - SCREEN_H/4;

        menu.IF.x = SCREEN_W;
        menu.IF.y = SCREEN_H;

        al_draw_filled_rectangle(menu.TE.x, menu.TE.y, menu.IF.x, menu.IF.y, al_map_rgb(100, 65, 165));
        al_draw_rectangle(menu.TE.x, menu.TE.y, menu.IF.x, menu.IF.y, al_map_rgb(135, 206, 235), 5);

        //imprime os textos do menu
        char opcoesmenu[20];
        sprintf(opcoesmenu, "Ataque");
        al_draw_text(fonte_30, al_map_rgb(255, 255, 255), SCREEN_W - SCREEN_W/6 - SCREEN_W/11.5, SCREEN_H - 3*SCREEN_H/13, 0, opcoesmenu);

        sprintf(opcoesmenu, "Especial");
        al_draw_text(fonte_30, al_map_rgb(255, 255, 255), SCREEN_W - SCREEN_W/6 - SCREEN_W/11.5, SCREEN_H - 2*SCREEN_H/13, 0, opcoesmenu);

        sprintf(opcoesmenu, "Fugir");
        al_draw_text(fonte_30, al_map_rgb(255, 255, 255), SCREEN_W - SCREEN_W/6 - SCREEN_W/11.5, SCREEN_H - SCREEN_H/13, 0, opcoesmenu);

        //desenha o objeto opcaomenu
        al_draw_bitmap(opcaomenu.aparencia, opcaomenu.cordte.x, opcaomenu.cordte.y, 0);    

        //desenha as barras de vida
        Retangulo barravida;
        //---------monstro---------//
        barravida.TE.x = SCREEN_W/6 - BARRAVIDA/2;
        barravida.TE.y = SCREEN_H/2 - monstros[heroi.inim_enc].tamanho/1.25 - 9;

        barravida.IF.x = SCREEN_W/6 + BARRAVIDA/2;
        barravida.IF.y = SCREEN_H/2 - monstros[heroi.inim_enc].tamanho/1.25 + 9;

        al_draw_filled_rectangle(barravida.TE.x , barravida.TE.y, barravida.IF.x, barravida.IF.y, al_map_rgb(255, 255, 255));
        
        //muda a coordenada x inferior para indentificar o quanto de vida o monstro tem
        if(monstros[heroi.inim_enc].tipo==COMUM){
            barravida.IF.x = SCREEN_W/6 - BARRAVIDA/2 + (BARRAVIDA*(monstros[heroi.inim_enc].vida/VIDA_INIMIGO_COMUM));
        }else if(monstros[heroi.inim_enc].tipo==INCOMUM){
            barravida.IF.x = SCREEN_W/6 - BARRAVIDA/2 + (BARRAVIDA*(monstros[heroi.inim_enc].vida/VIDA_INIMIGO_INCOMUM));
        }else if(monstros[heroi.inim_enc].tipo==RARO){
            barravida.IF.x = SCREEN_W/6 - BARRAVIDA/2 + (BARRAVIDA*(monstros[heroi.inim_enc].vida/VIDA_INIMIGO_RARO));
        }

        //indica a vida do monstro caso seja maior que 0
        if(monstros[heroi.inim_enc].vida>0){
            al_draw_filled_rectangle(barravida.TE.x + BARRAVIDA/40, barravida.TE.y + BARRAVIDA/40, barravida.IF.x - BARRAVIDA/40, barravida.IF.y - BARRAVIDA/40, al_map_rgb(255, 0, 0));
        }

        //---------heroi---------//
        barravida.TE.x = heroi.cordbat.x + heroi.tamanho_batalha/2 - BARRAVIDA/2;
        barravida.TE.y = heroi.cordbat.y + heroi.tamanho_batalha/2 - heroi.tamanho_batalha/1.15 - 9;

        barravida.IF.x = heroi.cordbat.x + heroi.tamanho_batalha/2 + BARRAVIDA/2;
        barravida.IF.y = heroi.cordbat.y + heroi.tamanho_batalha/2 - heroi.tamanho_batalha/1.15 + 9;

        al_draw_filled_rectangle(barravida.TE.x , barravida.TE.y, barravida.IF.x, barravida.IF.y, al_map_rgb(255, 255, 255));
        //muda a coordenada x inferior para indentificar o quanto de vida o heroi tem
        barravida.IF.x = heroi.cordbat.x + heroi.tamanho_batalha/2 - BARRAVIDA/2 + (BARRAVIDA*(heroi.vida/VIDA_HEROI));
        //indica a vida do heroi caso seja maior que 0
        if(heroi.vida>0){
            al_draw_filled_rectangle(barravida.TE.x + BARRAVIDA/40, barravida.TE.y + BARRAVIDA/40, barravida.IF.x  - BARRAVIDA/40, barravida.IF.y - BARRAVIDA/40, al_map_rgb(255, 0, 0));
        }
    }
    //reinicializa o display
    al_flip_display();
}

//modulo que desenha o cenario
void desenhaCenario(ALLEGRO_DISPLAY *d, ALLEGRO_FONT *fonte_30, ALLEGRO_FONT *fonte_15){
    if(modojogo==MENU_INICIAL || modojogo==TUTORIAL){
        al_set_target_bitmap(al_get_backbuffer(d));
        al_draw_bitmap(menucenario, 0, 0, 0);
    }else if(modojogo==EXPLORACAO){
        al_set_target_bitmap(al_get_backbuffer(d));
        al_draw_bitmap(grama.aparencia, grama.cordte.x, grama.cordte.y, 0);
        al_draw_bitmap(castelo.aparencia, castelo.cordte.x, castelo.cordte.y, 0);
        desenhaCirculoMonstro(d);
        desenhaHeroi(d);
    }else if(modojogo==BATALHA){
        al_set_target_bitmap(al_get_backbuffer(d));
        al_draw_bitmap(grama.aparencia, grama.cordte.x, grama.cordte.y, 0);
        al_draw_filled_rectangle(0, 0, SCREEN_W, SCREEN_H/3, al_map_rgb(135, 206, 235));
        al_draw_bitmap(monstros[heroi.inim_enc].projetil.aparencia, monstros[heroi.inim_enc].projetil.cordte.x + 16, monstros[heroi.inim_enc].projetil.cordte.y, 0);
        al_draw_bitmap(monstros[heroi.inim_enc].sprite, monstros[heroi.inim_enc].cordbat.x, monstros[heroi.inim_enc].cordbat.y, 0);
        al_draw_bitmap(heroi.batalha, heroi.cordbat.x, heroi.cordbat.y, 0);
    }
    desenhaHub(d, fonte_30, fonte_15);
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
    al_set_target_bitmap(al_get_backbuffer(d));
    al_clear_to_color(al_map_rgb(0,0,0));

    int recorde = 0;
    char final[50];

    if(modojogo == VITORIA){
        al_set_target_bitmap(al_get_backbuffer(d));
        sprintf(final, "Vitoria!");
        al_draw_text(fonte_30, al_map_rgb(148, 0, 211), SCREEN_W/3, SCREEN_H/2, 0, final);

        sprintf(final, "Pontuacao: %d", heroi.pontuacao);
        al_draw_text(fonte_30, al_map_rgb(148, 0, 211), SCREEN_W/3, SCREEN_H/2 + 32, 0, final);
        
        if(novoRecorde(heroi.pontuacao, &recorde)) {
            al_draw_text(fonte_30, al_map_rgb(200, 20, 30), SCREEN_W/3, 100+SCREEN_H/2, 0, "NEW RECORD!");
        }else{
            sprintf(final, "Recorde: %d", recorde);
            al_draw_text(fonte_30, al_map_rgb(148, 0, 211), SCREEN_W/3, SCREEN_H/2 + 32 + 32, 0, final);
        }
    }else if(modojogo == DERROTA){
        sprintf(final, "Derrota!");
        al_draw_text(fonte_30, al_map_rgb(148, 0, 211), SCREEN_W/3, SCREEN_H/2, 0, final);
    }
    printf("\nFIM DO JOGO!\n");

    //reinicializa o display
    al_flip_display();

    //espera 5 sec
    al_rest(3.0);
}

//modulo que movimenta o heroi na tela
void movimentaHeroi(){
    if (heroi.estado == ANDANDO){
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
    }
}

//modulo que verifica se o heroi se encontrou com algum inimigo e armazena qual inimigo foi
int encontrouInimigo(){
    for(int id = 0; id < NUMONSTROS; id++){
        if ((colisao(heroi.cordtc, monstros[id].cordexp, monstros[id].raiocol)|| 
            colisao(heroi.cordie, monstros[id].cordexp, monstros[id].raiocol)|| 
            colisao(heroi.cordid, monstros[id].cordexp, monstros[id].raiocol))&& 
            monstros[id].estado==ATACANDO){
            
            //verifica o estado do ultimo monstro encontrado e caso esteja atordoado ele muda para atacando
            if(monstros[heroi.inim_enc].estado == ATORDOADO){
                monstros[heroi.inim_enc].estado = ATACANDO;
            }
            
            //o heroi recebi o id do inimigo que ele encontrou
            heroi.inim_enc = id;
            return 1;
        }
    }
    return 0;
}

//modulo que verifica se o heroi chegou ao destino
int chegouDestino(){
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

//modulo de animacao de ataque (DEVE SER MODIFICADO)
void ataqueAnimacao(int eheroi, ALLEGRO_DISPLAY *d, ALLEGRO_FONT *fonte_30, ALLEGRO_FONT *fonte_15){
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
            desenhaCenario(d, fonte_30, fonte_15);
            al_rest(0.02);
        }
        ataque(&monstros[heroi.inim_enc].vida, &heroi.ataque, heroi.modificador);
        for(float i = heroi.cordbat.x; i < ci; i = heroi.cordbat.x){
            heroi.cordbat.x += (5*heroi.velocidade)*heroi.modificador;
            desenhaCenario(d, fonte_30, fonte_15);
            al_rest(0.0001);
        }
    }else if(eheroi == NAO){
        for(float i = monstros[heroi.inim_enc].projetil.cordte.x; i < heroi.cordbat.x + 4*monstros[heroi.inim_enc].projetil.tamanho; i = monstros[heroi.inim_enc].projetil.cordte.x){
            desenhaCenario(d, fonte_30, fonte_15);
            monstros[heroi.inim_enc].projetil.cordte.y -= 2*sin(monstros[heroi.inim_enc].projetil.cordte.x/150.0);
            monstros[heroi.inim_enc].projetil.cordte.x += 6.5;
            al_rest(0.0000000005);
        }
        ataque(&heroi.vida, &monstros[heroi.inim_enc].ataque, 1);
    }
    //a cordenada da estrutura
    monstros[heroi.inim_enc].projetil.cordte.x = monstros[heroi.inim_enc].cordbat.x + monstros[heroi.inim_enc].tamanho - 2*monstros[heroi.inim_enc].projetil.tamanho;
    monstros[heroi.inim_enc].projetil.cordte.y = monstros[heroi.inim_enc].cordbat.y;
    desenhaCenario(d, fonte_30, fonte_15);
    heroi.cordbat.x = ci;
}

//modulo que realiza a acao do monstro
void acaoMonstro(ALLEGRO_DISPLAY *d, ALLEGRO_FONT *fonte_30, ALLEGRO_FONT *fonte_15){
    printf("acaoMonstro\n");
    ataqueAnimacao(NAO, d, fonte_30, fonte_15);
}

//modulo que realiza a fuga mudando o modo de jogo e a coordenada do heroi
void fuga(){
    modojogo = EXPLORACAO;
    monstros[heroi.inim_enc].estado = ATORDOADO;
    heroi.estado = FUGA;

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

//modulo que indica se o heroi conseguiu fugir 
int tentativaDeFuga(){
    if(randRaridade()==RARO){
        return 0;
    }else{
        return 1;
    }
}

//modulo que verifica o tipo da acao do heroi e a realiza
void acaoHeroi(int tipo, ALLEGRO_DISPLAY *d, ALLEGRO_FONT *fonte_30, ALLEGRO_FONT *fonte_15){
    if (tipo == ATAQUE){
        heroi.modificador = 1;
        ataqueAnimacao(SIM, d, fonte_30, fonte_15);
    }else if(tipo == ESPECIAL){
        //cria o modificador e atribui a ele um valor aleatorio deacordo com a raridade
        float modificador = 0;
        modificador = randRaridade();
        if(modificador==COMUM){
            heroi.modificador = 0.75;
        }else if(modificador==INCOMUM){
            heroi.modificador = 1;
        }else if(modificador==RARO){
            heroi.modificador = 3;
        }
        ataqueAnimacao(SIM, d, fonte_30, fonte_15);
    }else if (tipo == FUGIR){
        if(tentativaDeFuga()){
            fuga();
            printf("Fugiu!!");
        }else{
            printf("Nao fugiu!!");
        }
    }
}

//modulo que verifica e efetua o da batalha
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
void modoMenuInicial(ALLEGRO_EVENT_QUEUE *ev_queue, ALLEGRO_DISPLAY *d, ALLEGRO_FONT *fonte_30, ALLEGRO_FONT *fonte_15){
    while (modojogo == MENU_INICIAL){
        ALLEGRO_EVENT ev;
        al_wait_for_event(ev_queue, &ev);

        //verifica qual o evento e o faz
        if(ev.type == ALLEGRO_EVENT_KEY_DOWN) {
            switch(ev.keyboard.keycode) {
                case ALLEGRO_KEY_DOWN:
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
                        jogando=NAO;
                        modojogo=DERROTA;
                    }                    

                    //limpa a lista de eventos
                    al_flush_event_queue(ev_queue);
                    break;
            }
        }else if(ev.type == ALLEGRO_EVENT_TIMER){
            //desenha o cenario do jogo no display
            desenhaCenario(d, fonte_30, fonte_15);
        }else if(ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
            jogando = NAO;
            modojogo = DERROTA;
        }
    }
}

//modulo do modo tutorial (INCOMPLETO)
void modoTutorial(ALLEGRO_EVENT_QUEUE *ev_queue){
    ALLEGRO_EVENT ev;
    al_wait_for_event(ev_queue, &ev);

    //verifica se o enter foi selecionado
    if(ev.type == ALLEGRO_EVENT_KEY_DOWN){
        switch(ev.keyboard.keycode) {
            case ALLEGRO_KEY_ENTER:
                modojogo = MENU_INICIAL;
                break;
        }
    }

    //verifica se o evento e de fechar a tela
    if(ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
        jogando = NAO;
        modojogo = DERROTA;
    }
}

//modulo que efetua o modo de exploração
void modoExploracao(ALLEGRO_EVENT_QUEUE *ev_queue, ALLEGRO_DISPLAY *d, ALLEGRO_FONT *fonte_30, ALLEGRO_FONT *fonte_15){
    ALLEGRO_EVENT ev;
    al_wait_for_event(ev_queue, &ev);

    //verifica qual o evento e o faz
    if(ev.type == ALLEGRO_EVENT_KEY_DOWN) {
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
    }else if(ev.type == ALLEGRO_EVENT_KEY_UP) {
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
    }else if(ev.type == ALLEGRO_EVENT_TIMER){
        desenhaCenario(d, fonte_30, fonte_15);
    }else if(ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
        jogando = NAO;
        modojogo = DERROTA;
    }
    //movimenta o heroi
    movimentaHeroi();

    //verifica se o heroi chegou no destino
    if(chegouDestino()){
        modojogo = VITORIA;
    }

    //verifica se o heroi se encontrou com um inimigo e muda o modo de jogo para batalha
    if(encontrouInimigo()){
        heroi.estado = PARADO;
        modojogo = BATALHA;
    }
}

//modulo que efetua o modo batalha
void modoBatalha(ALLEGRO_EVENT_QUEUE *ev_queue, ALLEGRO_DISPLAY *d, ALLEGRO_FONT *fonte_30, ALLEGRO_FONT *fonte_15){
    while (modojogo == BATALHA){
        ALLEGRO_EVENT ev;
        al_wait_for_event(ev_queue, &ev);

        //verifica qual o evento e o faz
        if(ev.type == ALLEGRO_EVENT_KEY_DOWN) {
            switch(ev.keyboard.keycode) {
                case ALLEGRO_KEY_DOWN:
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
                    acaoHeroi(opcaomenu.tipo, d, fonte_30, fonte_15);
                    verificaFimDaBatalha();

                    if(monstros[heroi.inim_enc].vida>0 && heroi.estado != FUGA){
                        acaoMonstro(d, fonte_30, fonte_15);
                    }
                    verificaFimDaBatalha();

                    //limpa a lista de eventos
                    al_flush_event_queue(ev_queue);
                    break;
            }  
        }else if (ev.type == ALLEGRO_EVENT_TIMER){
            desenhaCenario(d, fonte_30, fonte_15);
        }else if(ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
            jogando = NAO;
            modojogo = DERROTA;
        }
    }
}

//no total 29 modulos sem contar o main

//------------------------------------------- main -------------------------------------------
int main(int argc, char **argv){
    //define a seed do rand
    srand(time(NULL));

    ALLEGRO_DISPLAY *display = NULL;
    ALLEGRO_EVENT_QUEUE *event_queue = NULL;
    ALLEGRO_TIMER *timer = NULL;

    //----------------------- rotinas de inicializacao ---------------------------------------
    
    //inicializa e verifica se os itens necessarios foram inicializados
    al_init();
    if(!al_init()){
        fprintf(stderr, "failed to initialize allegro!\n");
        return -1;
    }

    al_init_image_addon();
    if(!al_init_image_addon()){
        fprintf(stderr, "failed to initialize image_addon!\n");
        return -1;
    }

    timer = al_create_timer(1.0 / FPS);
    if(!timer){
        fprintf(stderr, "failed to create timer!\n");
        return -1;
    }

    display = al_create_display(SCREEN_W, SCREEN_H);
    if(!display){
        fprintf(stderr, "failed to create display!\n");
        al_destroy_timer(timer);
        return -1;
    }

    event_queue = al_create_event_queue();
    if(!event_queue){
        fprintf(stderr, "failed to create event_queue!\n");
        al_destroy_display(display);
        al_destroy_timer(timer);
        return -1;
    }

	al_install_keyboard();
    al_init_primitives_addon();
    al_init_font_addon();
    al_init_ttf_addon();

    //declara o que a lista de eventos vai entender como um evento
    al_register_event_source(event_queue, al_get_display_event_source(display));
    al_register_event_source(event_queue, al_get_timer_event_source(timer));
   	al_register_event_source(event_queue, al_get_keyboard_event_source());

	//carrega a fonte PressStart2P e define que sera usado o tamanho 30
    ALLEGRO_FONT *fonte_30 = al_load_font("Fonte/PressStart2P.ttf", 30, 1);

    //carrega a fonte PressStart2P e define que sera usado o tamanho 15
    ALLEGRO_FONT *fonte_15 = al_load_font("Fonte/PressStart2P.ttf", 15, 1);

    initGlobalVars();
    al_flip_display();
    al_start_timer(timer);
    //----------------------- fim das rotinas de inicializacao ---------------------------------------

    while(jogando){
        //verifica o modo de jogo
        if(modojogo == MENU_INICIAL){
            opcaomenu.tipo = JOGAR;
    
            //inicializa as coordenadas da opcaomenu
            opcaomenu.cordte.x = SCREEN_W/3 + SCREEN_W/22 - 1.5*opcaomenu.tamanho; 
            opcaomenu.cordte.y = SCREEN_H/2 - SCREEN_H/13.5;

            //efetua o modo de jogo atual
            modoMenuInicial(event_queue, display, fonte_30, fonte_15);
        }else if(modojogo == TUTORIAL){
            desenhaHub(display, fonte_30, fonte_15);
            
            //efetua o modo de jogo atual
            modoTutorial(event_queue);
        }else if(modojogo == EXPLORACAO){
            //efetua o modo de jogo atual
            modoExploracao(event_queue, display, fonte_30, fonte_15);
        }else if(modojogo == BATALHA){
            opcaomenu.tipo = ATAQUE;

            //inicializa as coordenadas da opcaomenu
            opcaomenu.cordte.y = SCREEN_H - 3*SCREEN_H/12 + opcaomenu.tamanho;
            opcaomenu.cordte.x = SCREEN_W - SCREEN_W/6 - SCREEN_W/13.5 - 2.2*opcaomenu.tamanho;

            //efetua o modo de jogo atual
            modoBatalha(event_queue, display, fonte_30, fonte_15);
        }else if(jogando==SIM && (modojogo == DERROTA || modojogo == VITORIA)){
            desenhaFim(display, fonte_30);
            jogando = NAO;
        }
    }

    //----------------------- rotinas de finalizacao ---------------------------------------
    al_destroy_timer(timer);
    al_destroy_display(display);
    al_destroy_event_queue(event_queue);
    return 0;
}