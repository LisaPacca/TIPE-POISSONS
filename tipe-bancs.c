#include <SDL2/SDL.h>
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

//taille de l'ecran
#define LARGEUR 1920
#define HAUTEUR 1080

//valeurs aleatoires
#define NB_POISSONS 50
#define RAYON_ATTRACTION 150
#define RAYON_ALIGNEMENT 40
#define RAYON_REPULSION 10
#define VITESSE_INITIALE 2.5
#define TAILLE_POISSON 5

//vecteur dans le plan 
typedef struct {
    float x, y;
} Vector2;//nom à changer

//Poisson tous ou un seul ? 
typedef struct {
    int nb_poisson;
    float r1, r2, r3;
    float v_norm, v_init;
    int taille_poisson;
} Setting; //nom à changer

//parametre d'un poisson dans le plan
typedef struct {
    Vector2 pos;
    Vector2 vitesse;
    Setting* setting;
} Poisson;

//affichage
SDL_Color COULEURS[] = {
    {255,255,255,255}, // blanc
    {0,0,0,255},       // noir
    {255,0,0,255},     // rouge
    {255,165,0,255},   // orange
    {0,128,0,255},     // vert
    {0,0,255,255},     // bleu
    {0,255,255,255}    // cyan
};

//fonctions a refaire 
Vector2 vec_add(Vector2 a, Vector2 b)
    { return (Vector2){a.x+b.x,a.y+b.y}; }

float distance(Vector2 a, Vector2 b){
    return sqrtf((a.x-b.x)*(a.x-b.x) + (a.y-b.y)*(a.y-b.y));
}

float norme(Vector2 v){
    return sqrtf(v.x*v.x + v.y*v.y);
}

void poisson_init(Poisson* p, float x, float y, float angle, Setting* s){
    p->pos = (Vector2){x,y};
    p->setting = s;
    p->vitesse = (Vector2){cosf(angle)*s->v_norm, sinf(angle)*s->v_norm}; 
    
}

void zone_repulsion(Poisson* p, Poisson voisin[NB_POISSONS]){
    Vector2 d={0.0 , 0.0};
    int count = 0;
    for(int j = 0; j<p->setting->nb_poisson; j++){
        Poisson* v = &voisin[j];
        if(v == p) { continue; }
        float dist = distance(p->pos,v->pos);
        if(dist < RAYON_REPULSION){
            count++;
            printf("repulsion \n");
            d.x += (p->pos.x - v->pos.x)/distance(p->pos,v->pos);
            d.y += (p->pos.y - v->pos.y)/distance(p->pos,v->pos);
          
        }
    }
    if(count!=0){
        p->vitesse.x = d.x*p->setting->v_norm ;
        p->vitesse.y = d.y*p->setting->v_norm ;
    }
}

//pas l'air de faire grand chose si foutre le bordel
void zone_orientation(Poisson* p, Poisson voisin[NB_POISSONS]){
    Vector2 d={0.0 , 0.0};
    int count = 0;
    for(int j = 0; j<p->setting->nb_poisson; j++){
        Poisson* v = &voisin[j];
        float dist = distance(p->pos,v->pos);
        if(dist < RAYON_ALIGNEMENT && dist > RAYON_REPULSION){
            count++;
            printf("orientation \n");
            d.x += v->vitesse.x/v->setting->v_norm;
            d.y += v->vitesse.y/v->setting->v_norm;
        }
    }
    if(count != 0 ){
        p->vitesse.x = d.x*p->setting->v_norm;
        p->vitesse.y = d.y*p->setting->v_norm;
    }
}


void zone_attration(Poisson* p, Poisson voisin[NB_POISSONS]){
    Vector2 d={0.0 , 0.0};
    int count = 0;
    for(int j = 0; j<p->setting->nb_poisson; j++){
        Poisson* v = &voisin[j];
        if(v == p) { continue; }
        float dist = distance(p->pos,v->pos);
        if(dist < RAYON_ATTRACTION && dist > RAYON_ALIGNEMENT){
            count++;
            printf("alignement \n");
            d.x += (v->pos.x - p->pos.x)/distance(v->pos,p->pos);
            d.y += (v->pos.y - p->pos.y)/distance(v->pos,p->pos);
        }
    }
    if(count != 0){
        p->vitesse.x = d.x*p->setting->v_norm;
        p->vitesse.y = d.y*p->setting->v_norm;
    }
}




void poisson_deplacer(Poisson* p){
    p->pos = vec_add(p->pos, p->vitesse);

    if(p->pos.x - p->setting->taille_poisson <= 0 || p->pos.x + p->setting->taille_poisson >= LARGEUR){
        if(p->pos.x< p->setting->taille_poisson) p->pos.x = LARGEUR - p->setting->taille_poisson;
        if(p->pos.x> LARGEUR - p->setting->taille_poisson) p->pos.x = p->setting->taille_poisson;
    }
    if(p->pos.y - p->setting->taille_poisson <= 0 || p->pos.y + p->setting->taille_poisson >= HAUTEUR){
        if(p->pos.y< p->setting->taille_poisson) p->pos.y =  HAUTEUR - p->setting->taille_poisson;
        if(p->pos.y> HAUTEUR - p->setting->taille_poisson) p->pos.y = p->setting->taille_poisson;
    }
}

//dessine un cercle
void draw_circle(SDL_Renderer* renderer, int cx, int cy, int r) {
    // Dessiner le cercle (boule)
    for(int w = -r; w <= r; w++) {
        for(int h = -r; h <= r; h++) {
            if(w*w + h*h <= r*r) {
                SDL_RenderDrawPoint(renderer, cx + w, cy + h);
            }
        }
    }
}

//dessine un poisson 
void poisson_dessiner(Poisson* p, SDL_Renderer* renderer, SDL_Color c){
    SDL_SetRenderDrawColor(renderer, c.r,c.g,c.b,255);
    draw_circle(renderer,(int)p->pos.x,(int)p->pos.y,p->setting->taille_poisson);
}

//
int main(){
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* window = SDL_CreateWindow("Poissons SDL",
        SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED,LARGEUR,HAUTEUR,SDL_WINDOW_FULLSCREEN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window,-1,SDL_RENDERER_ACCELERATED);

    srand(time(NULL));

    Setting settings = {NB_POISSONS, RAYON_REPULSION, RAYON_ALIGNEMENT, RAYON_ATTRACTION,
                        VITESSE_INITIALE,VITESSE_INITIALE,TAILLE_POISSON};

    Poisson poissons[NB_POISSONS];
    for(int i=0;i<settings.nb_poisson;i++){
        float angle = ((float)rand()/RAND_MAX)*2*M_PI;
        int x = rand()% (LARGEUR-200)+100;
        int y = rand()% (HAUTEUR-200)+100;
        poisson_init(&poissons[i],x,y,angle,&settings);
    }

    bool running=true;
    SDL_Event e;
    while(running){
        while(SDL_PollEvent(&e)){
            if(e.type==SDL_QUIT) running=false;
            if(e.type==SDL_KEYDOWN && e.key.keysym.sym==SDLK_q) running=false;
        }

        for(int i=0;i<settings.nb_poisson;i++) {
            poisson_deplacer(&poissons[i]);
            zone_repulsion(&poissons[i],poissons);
            //zone_orientation(&poissons[i],poissons);
            //zone_attration(&poissons[i],poissons);
        }

        SDL_SetRenderDrawColor(renderer,0,0,0,255);
        SDL_RenderClear(renderer);

        for(int i=0;i<settings.nb_poisson;i++){
            poisson_dessiner(&poissons[i],renderer,COULEURS[2]); // rouge
        }

        SDL_RenderPresent(renderer);
        SDL_Delay(15);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
