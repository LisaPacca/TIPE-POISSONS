#include </opt/homebrew/Cellar/sdl2/2.32.10/include/SDL2/SDL.h>
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

//taille de l'ecran
#define LARGEUR 1920
#define HAUTEUR 1080

//valeurs aleatoires
#define NB_POISSONS 100
#define RAYON_ATTRACTION 100
#define RAYON_ORIENTATION 40
#define RAYON_REPULSION 30
#define V_INITIALE 2.5
#define TAILLE_POISSON 5

//vecteur dans le plan 
typedef struct {
    float x, y;
} Vector2;//nom à changer

//parametre d'un poisson dans le plan
typedef struct {
    Vector2 pos;
    Vector2 vitesse;
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

void poisson_init(Poisson* p, float x, float y, float angle){
    p->pos = (Vector2){x,y};
    p->vitesse = (Vector2){cosf(angle)*V_INITIALE, sinf(angle)*V_INITIALE}; 
    
}

Vector2 zone_repulsion(Poisson* p, Poisson* v, Vector2 d){
    d.x += (p->pos.x - v->pos.x)/distance(p->pos,v->pos);
    d.y += (p->pos.y - v->pos.y)/distance(p->pos,v->pos);
    return d ; 
}

//pas l'air de faire grand chose si foutre le bordel
Vector2 zone_orientation(Poisson* p, Poisson* v, Vector2 d){
    d.x += v->vitesse.x/V_INITIALE;
    d.y += v->vitesse.y/V_INITIALE;
    return d; 
}


Vector2 zone_attration(Poisson* p, Poisson* v, Vector2 d){
    d.x += (v->pos.x - p->pos.x)/distance(v->pos,p->pos);
    d.y += (v->pos.y - p->pos.y)/distance(v->pos,p->pos);
    return d; 
}


void mouvement(Poisson* p, Poisson voisin[NB_POISSONS]){
    Vector2 d_r={0.0 , 0.0};
    Vector2 d_a={0.0 , 0.0};
    Vector2 d_o={0.0 , 0.0};
    int count_r = 0;
    int count_o = 0;
    int count_a = 0;
    for(int j = 0; j<NB_POISSONS; j++){
        Poisson* v = &voisin[j];
        if(v == p) { continue; }
        float dist = distance(p->pos,v->pos);
        if(dist < RAYON_REPULSION){
            d_r.x += zone_repulsion(p,v,d_r).x;
            d_r.y +=zone_repulsion(p,v,d_r).y;
            count_r ++;
        }
        if(dist < RAYON_ORIENTATION){
            d_o.x += zone_orientation(p,v,d_o).x;
            d_o.y += zone_orientation(p,v,d_o).y;
            count_o ++ ; 
        }
        if(dist < RAYON_ATTRACTION){
            d_a.x += zone_attration(p,v,d_a).x;
            d_a.y += zone_orientation(p,v,d_a).y;
            count_a ++ ; 
        }
        if (count_r > 0){
            p->vitesse.x = d_r.x*V_INITIALE ;
            p->vitesse.y = d_r.y*V_INITIALE ;
        }
        if(count_o == NB_POISSONS){
            p->vitesse.x = d_o.x*V_INITIALE ;
            p->vitesse.y = d_o.y*V_INITIALE ;
        }
        if(count_a == NB_POISSONS){
            p->vitesse.x = d_a.x*V_INITIALE ;
            p->vitesse.y = d_a.y*V_INITIALE ;
        }
        if(count_o + count_a == NB_POISSONS){
            p->vitesse.x = 0.5*(d_o.x + d_a.x)*V_INITIALE ;
            p->vitesse.y = 0.5*(d_o.y + d_a.y)*V_INITIALE ;
        }
    }
}



void poisson_deplacer(Poisson* p){
    p->pos = vec_add(p->pos, p->vitesse);

    if(p->pos.x - TAILLE_POISSON <= 0 || p->pos.x + TAILLE_POISSON >= LARGEUR){
        if(p->pos.x< TAILLE_POISSON) p->pos.x = LARGEUR - TAILLE_POISSON;
        if(p->pos.x> LARGEUR - TAILLE_POISSON) p->pos.x = TAILLE_POISSON;
    }
    if(p->pos.y - TAILLE_POISSON<= 0 || p->pos.y + TAILLE_POISSON >= HAUTEUR){
        if(p->pos.y< TAILLE_POISSON) p->pos.y =  HAUTEUR - TAILLE_POISSON;
        if(p->pos.y> HAUTEUR - TAILLE_POISSON) p->pos.y = TAILLE_POISSON;
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
    draw_circle(renderer,(int)p->pos.x,(int)p->pos.y,TAILLE_POISSON);
}

//
int main(){
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* window = SDL_CreateWindow("Poissons SDL",
        SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED,LARGEUR,HAUTEUR,SDL_WINDOW_FULLSCREEN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window,-1,SDL_RENDERER_ACCELERATED);

    srand(time(NULL));

    Poisson poissons[NB_POISSONS];
    for(int i=0;i<NB_POISSONS;i++){
        float angle = ((float)rand()/RAND_MAX)*2*M_PI;
        int x = rand()% (LARGEUR-200)+100;
        int y = rand()% (HAUTEUR-200)+100;
        poisson_init(&poissons[i],x,y,angle);
    }

    bool running=true;
    SDL_Event e;
    while(running){
        while(SDL_PollEvent(&e)){
            if(e.type==SDL_QUIT) running=false;
            if(e.type==SDL_KEYDOWN && e.key.keysym.sym==SDLK_q) running=false;
        }

        for(int i=0;i<NB_POISSONS;i++) {
            poisson_deplacer(&poissons[i]);
            mouvement(&poissons[i],poissons);
        }

        SDL_SetRenderDrawColor(renderer,0,0,0,255);
        SDL_RenderClear(renderer);

        for(int i=0;i<NB_POISSONS;i++){
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
