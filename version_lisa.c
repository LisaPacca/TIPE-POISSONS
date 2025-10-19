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
#define NB_POISSONS 200
#define RAYON_ATTRACTION 120
#define RAYON_ORIENTATION 90
#define RAYON_REPULSION 40
#define V_INITIALE 2
#define TAILLE_POISSON 5
#define BLIND M_PI

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

 
Vector2 vec_add(Vector2 a, Vector2 b)
    { return (Vector2){a.x+b.x,a.y+b.y}; }

float distance(Vector2 a, Vector2 b){
    return sqrtf((a.x-b.x)*(a.x-b.x) + (a.y-b.y)*(a.y-b.y));
}

float norme(Vector2* v){
    return sqrtf(v->x*v->x + v->y*v->y);
}

void poisson_init(Poisson* p, float x, float y, float angle){
    p->pos = (Vector2){x,y};
    p->vitesse = (Vector2){cosf(angle)*V_INITIALE, sinf(angle)*V_INITIALE}; 
}

Vector2* zone_repulsion(Poisson* p, Poisson* v, Vector2* d){
    if (distance(p->pos,v->pos)!= 0) {
        d->x += (p->pos.x - v->pos.x)/distance(p->pos,v->pos);
        d->y += (p->pos.y - v->pos.y)/distance(p->pos,v->pos);
    }
    return d ; 
}

Vector2* zone_orientation(Poisson* p, Poisson* v, Vector2* d){
    d->x += v->vitesse.x/V_INITIALE;
    d->y += v->vitesse.y/V_INITIALE;
    return d; 
}

Vector2* zone_attraction(Poisson* p, Poisson* v, Vector2* d){
    if (distance(v->pos,p->pos)!= 0) {
        d->x += (v->pos.x - p->pos.x)/distance(v->pos,p->pos);
        d->y += (v->pos.y - p->pos.y)/distance(v->pos,p->pos);
    }
    return d ; 
}

Vector2 changement_direction (Poisson* p, Vector2 *d, float alpha){
    float n = norme(d); 
    Vector2 cible = { (d->x/n)*V_INITIALE , (d->y/n)*V_INITIALE };
    p->vitesse.x = (1 - alpha)*p->vitesse.x + alpha*cible.x;
    p->vitesse.y = (1 - alpha)*p->vitesse.y + alpha*cible.y;
    float n_p = sqrtf(p->vitesse.x*p->vitesse.x + p->vitesse.y*p->vitesse.y);
    if (n_p > 0) {
        p->vitesse.x = (p->vitesse.x / n_p) * V_INITIALE;
        p->vitesse.y = (p->vitesse.y / n_p) * V_INITIALE;
    }
    return p->vitesse;
}

void mouvement(Poisson* p, Poisson voisin[NB_POISSONS],float alpha){
    Vector2* d_r= malloc(sizeof(Vector2)); 
    Vector2* d_a= malloc(sizeof(Vector2)); 
    Vector2* d_o= malloc(sizeof(Vector2)); 
    d_r->x = 0.0; 
    d_r->y = 0.0;
    d_a->x = 0.0; 
    d_a->y = 0.0;
    d_o->x = 0.0; 
    d_o->y = 0.0;
    int count_r = 0;
    int count_o = 0;
    int count_a = 0;
    for(int j = 0; j<NB_POISSONS; j++){
        Poisson* v = &voisin[j];
        if(v == p) { continue; }
        float dist = distance(p->pos,v->pos);
        if(dist < RAYON_REPULSION){
            d_r->x += zone_repulsion(p,v,d_r)->x;
            d_r->y += zone_repulsion(p,v,d_r)->y;
            count_r ++;
        }
        else if(dist < RAYON_ORIENTATION && dist > RAYON_REPULSION){
            d_o->x += zone_orientation(p,v,d_o)->x;
            d_o->y += zone_orientation(p,v,d_o)->y;
            count_o ++ ; 
        }
        else if(dist < RAYON_ATTRACTION && dist > RAYON_ORIENTATION){
            d_a->x += zone_attraction(p,v,d_a)->x;
            d_a->y += zone_attraction(p,v,d_a)->y;
            count_a ++ ; 
        }
    }
    float n_r = norme(d_r);
    float n_a = norme(d_a);
    float n_o = norme(d_o);
    if (count_r > 0 && n_r > 0) {
        changement_direction(p,d_r,alpha);
    }
    else if(count_o > 0 && count_a > 0 && n_o > 0 && n_a > 0){
        Vector2* d_a_bis = malloc(sizeof(Vector2));
        Vector2* d_o_bis = malloc(sizeof(Vector2));
        d_a_bis -> x = 0.3*d_a->x;
        d_a_bis -> y = 0.3*d_a->y;
        d_o_bis -> x = 0.7*d_o->x;
        d_o_bis -> y = 0.7*d_o->y;
        p->vitesse = changement_direction(p, d_a_bis,alpha);
        p->vitesse = changement_direction(p, d_o_bis,alpha);
        free(d_a_bis);
        free(d_o_bis);
    }
    else if(count_o > 0 && n_o > 0 ){
        p->vitesse = changement_direction(p, d_o,alpha);
    }
    else if(count_a > 0 && n_a > 0){
        p->vitesse = changement_direction(p,d_a,alpha);
    }
    free(d_r);
    free(d_a);
    free(d_o);
}


void poisson_deplacer(Poisson* p, float bruit){
    float r = (((float)rand() / RAND_MAX)*2*bruit) - bruit;
    p->vitesse.x = cosf(r)*p->vitesse.x - sinf(r)*p->vitesse.y;
    p->vitesse.y = sinf(r)*p->vitesse.x + cosf(r)*p->vitesse.y;
    p->pos = vec_add(p->pos, p->vitesse);

    if(p->pos.x - TAILLE_POISSON <= 0 || p->pos.x + TAILLE_POISSON >= LARGEUR){
        if(p->pos.x< TAILLE_POISSON){ 
            p->pos.x = TAILLE_POISSON;
            p->vitesse.x *= -1;
        }        
        if(p->pos.x> LARGEUR - TAILLE_POISSON){
            p->pos.x = LARGEUR - TAILLE_POISSON;
            p->vitesse.x *= -1;
        }
    }
    if(p->pos.y - TAILLE_POISSON<= 0 || p->pos.y + TAILLE_POISSON >= HAUTEUR){
        if(p->pos.y< TAILLE_POISSON){
            p->pos.y = TAILLE_POISSON;
            p->vitesse.y *= -1;
        }
        if(p->pos.y> HAUTEUR - TAILLE_POISSON){
            p->pos.y = HAUTEUR - TAILLE_POISSON;
            p->vitesse.y *= -1;
        }
    }
}

//dessine un cercle
void draw_circle(SDL_Renderer* renderer, int cx, int cy, int r) {
    for(int w = -r; w <= r; w++) {
        for(int h = -r; h <= r; h++) {
            if(w*w + h*h <= r*r) {
                SDL_RenderDrawPoint(renderer, cx + w, cy + h);
            }
        }
    }
}

//dessine un poisson 
void poisson_dessiner(Poisson* p, SDL_Renderer* renderer, SDL_Color c) {
    SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, 255);

    // Dessine le corps (cercle)
    draw_circle(renderer, (int)p->pos.x, (int)p->pos.y, TAILLE_POISSON);

    // Normalisation de la vitesse pour direction
    float n = sqrtf(p->vitesse.x * p->vitesse.x + p->vitesse.y * p->vitesse.y);
    if (n > 0.001f) {
        // Coordonnées de la queue
        float dx = 2*(p->vitesse.x / n) * TAILLE_POISSON * 2.0f;
        float dy = 2*(p->vitesse.y / n) * TAILLE_POISSON * 2.0f;

        // On trace une ligne partant du centre vers l'arrière (queue)
        SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255); // gris clair pour contraster
        SDL_RenderDrawLine(renderer,
            (int)p->pos.x, (int)p->pos.y,
            (int)(p->pos.x - dx), (int)(p->pos.y - dy)
        );
    }
}

//
int main(){
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* window = SDL_CreateWindow("Poissons SDL",
        SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED,LARGEUR,HAUTEUR,SDL_WINDOW_FULLSCREEN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window,-1,SDL_RENDERER_ACCELERATED);
    srand(time(NULL));
    float alpha = 0.2; 
    float bruit = 0.1; 
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
        for (int i = 0; i < NB_POISSONS; i++){
            mouvement(&poissons[i], poissons,alpha);
        }
        for (int i = 0; i < NB_POISSONS; i++){
            poisson_deplacer(&poissons[i],bruit);
        }
        SDL_SetRenderDrawColor(renderer,0,0,0,255);
        SDL_RenderClear(renderer);

        for(int i=0;i<NB_POISSONS;i++){
            poisson_dessiner(&poissons[i],renderer,COULEURS[6]); // rouge
        }

        SDL_RenderPresent(renderer);
        SDL_Delay(15);
        while(SDL_PollEvent(&e)){
            if(e.type==SDL_QUIT) {
                running=false;
            }
            if(e.type==SDL_KEYDOWN){
                switch(e.key.keysym.sym){
                case SDLK_q: running=false; 
                break;
                case SDLK_UP: alpha += 0.01f; 
                if(alpha>1.0f){
                    alpha=1.0f; 
                    break;
                }
                case SDLK_DOWN: alpha -= 0.01f; 
                if(alpha<0.0f) {
                    alpha=0.0f; 
                    break;
                }
                case SDLK_RIGHT: bruit += 0.01f; 
                if(bruit>1.0f) {
                    bruit=1.0f; break;
                }
                case SDLK_LEFT: bruit -= 0.01f; 
                if(bruit<0.0f) {
                        bruit=0.0f; break;
                }
                }
            }
        }
    }
    char titre[100];
    sprintf(titre, "Poissons SDL - alpha=%.2f, bruit=%.2f", alpha, bruit);
    SDL_SetWindowTitle(window, titre);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
