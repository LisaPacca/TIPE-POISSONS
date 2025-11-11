#include <SDL2/SDL.h>
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

//taille de l'ecran
#define LARGEUR 1000
#define HAUTEUR 1000

//valeurs aleatoires
#define NB_POISSONS 150
#define RAYON_ATTRACTION 200
#define RAYON_ORIENTATION 100
#define RAYON_REPULSION 17
#define V_INITIALE 3.5
#define TAILLE_POISSON 5
#define ALPHA 0.1
#define BRUIT 0.05
#define BLIND M_PI

typedef struct {
    float x, y;
} Vector2;//nom à changer

struct Poisson{
    Vector2 pos;
    Vector2 vitesse;
    struct Poisson* tab_orientation[NB_POISSONS];
    struct Poisson* tab_attraction[NB_POISSONS];
    int id;
    bool infecte;
};
typedef struct Poisson Poisson;

//affichage
SDL_Color COULEURS[] = {
    {255,255,255,255}, // blanc 0
    {0,0,0,255},       // noir 1
    {255,0,0,255},     // rouge 2
    {255,165,0,255},   // orange 3
    {0,255,0,255},     // vert 4
    {255,255,0,255},   // jaune 5
    {0,0,255,255},     // bleu 6
    {0,255,255,255},   // cyan 7
    {128,0,128,255},   // violet 8
    {128,128,128,255}, // gris 9
    {0,255,0,255},     // vert clair 10
    {255,192,203,255}, // rose 11
    {165,42,42,255},   // marron 12
    {75,0,130,255},    // indigo 13
    {240,230,140,255}  // kaki 14
};

//fonctions a refaire 
Vector2 vec_add(Vector2 a, Vector2 b)
    { return (Vector2){a.x+b.x,a.y+b.y}; }

float distance(Vector2 a, Vector2 b){
    return sqrtf((a.x-b.x)*(a.x-b.x) + (a.y-b.y)*(a.y-b.y));
}

float normer(Vector2 v){
    return sqrtf(v.x*v.x + v.y*v.y);
}

void poisson_init(Poisson* p, float x, float y, float angle, int i){
    p->pos = (Vector2){x,y};
    p->vitesse = (Vector2){cosf(angle)*V_INITIALE, sinf(angle)*V_INITIALE}; 
    for(int i=0; i<NB_POISSONS; i++){
        p->tab_orientation[i] = NULL;
        p->tab_attraction[i] = NULL;
    }
    p->infecte = false;
    if(i==0){p->infecte = true;}
    p->id = i;   
}

float valabs(float x){return sqrtf(x*x);}

bool appartient(Poisson* p, Poisson* tab[NB_POISSONS]){
    for(int i=0; i<NB_POISSONS; i++){
        if(p == tab[i]) { return true; }
    }
    return false;
}


bool blind_zone(Vector2 a, Vector2 b){
    float costheta = (a.x*b.x + a.y*b.y)/(normer(a)*normer(b));
    float theta = acosf(costheta);
    if(theta > (M_PI-(BLIND/2))){return false;}
    return true;
    
}

bool zone_repulsion(Poisson* p, Poisson voisin[NB_POISSONS]){
    Vector2* d = malloc(sizeof(Vector2));
    d->x = 0;
    d->y = 0;
    int count = 0;
    for(int j = 0; j<NB_POISSONS; j++){
        Poisson* v = &voisin[j];
        if(v == p) { continue; }
        float dist = distance(p->pos,v->pos);
        Vector2 diff = {v->pos.x - p->pos.x, v->pos.y - p->pos.y};
        if(dist < RAYON_REPULSION && blind_zone(p->vitesse,diff)){
            count++;
            d->x += (p->pos.x - v->pos.x)/distance(p->pos,v->pos);
            d->y += (p->pos.y - v->pos.y)/distance(p->pos,v->pos);  
        }
    }
    if(count != 0){
        float norme = sqrtf((d->x)*(d->x)+(d->y)*(d->y));
        if(norme>0){
            d->x = d->x/norme;
            d->y = d->y/norme;
            p->vitesse.x = ((1-ALPHA)*p->vitesse.x + ALPHA*d->x)*V_INITIALE;
            p->vitesse.y = ((1-ALPHA)*p->vitesse.y + ALPHA*d->y)*V_INITIALE;

            float v = normer(p->vitesse);
            if(v>V_INITIALE){
                p->vitesse.x = ((p->vitesse.x)/v)*V_INITIALE;
                p->vitesse.y = ((p->vitesse.y)/v)*V_INITIALE;
            }
        }
    }
    free(d);
    return (count == 0);
}

//pas l'air de faire grand chose si foutre le bordel
void zone_orientation(Poisson* p, Poisson voisin[NB_POISSONS]){
    Vector2* d= malloc(sizeof(Vector2));
    d->x = 0;
    d->y = 0;
    int count = 0;
    for(int j = 0; j<NB_POISSONS; j++){
        Poisson* v = &voisin[j];
        float dist = distance(p->pos,v->pos);
        Vector2 diff = {v->pos.x - p->pos.x, v->pos.y - p->pos.y};
        if(dist < RAYON_ORIENTATION && blind_zone(p->vitesse,diff)){
            count++;
            d->x += v->vitesse.x/sqrtf((v->vitesse.x)*(v->vitesse.x)+(v->vitesse.y)*(v->vitesse.y));
            d->y += v->vitesse.y/sqrtf((v->vitesse.x)*(v->vitesse.x)+(v->vitesse.y)*(v->vitesse.y));
            p->tab_orientation[j] = v;
            v->infecte = true;
        }
        else{
            p->tab_orientation[j] = NULL;
        }
    }
    if(count != 0 ){
        float norme = sqrtf((d->x)*(d->x)+(d->y)*(d->y));
        if(norme>0){
            d->x = d->x/norme;
            d->y= d->y/norme;
            p->vitesse.x = ((1-ALPHA)*p->vitesse.x + ALPHA*d->x)*V_INITIALE;
            p->vitesse.y = ((1-ALPHA)*p->vitesse.y + ALPHA*d->y)*V_INITIALE;

            float v = normer(p->vitesse);
            if(v>V_INITIALE){
                p->vitesse.x = ((p->vitesse.x)/v)*V_INITIALE;
                p->vitesse.y = ((p->vitesse.y)/v)*V_INITIALE;
            }
        }
    }       
    free(d);
}


void zone_attration(Poisson* p, Poisson voisin[NB_POISSONS]){
    Vector2* d= malloc(sizeof(Vector2));
    d->x = 0;
    d->y = 0;
    int count = 0;
    for(int j = 0; j<NB_POISSONS; j++){
        Poisson* v = &voisin[j];
        if(v == p) { continue; }
        float dist = distance(p->pos,v->pos);
        Vector2 diff = {v->pos.x - p->pos.x, v->pos.y - p->pos.y};
        if(dist < RAYON_ATTRACTION && dist > RAYON_ORIENTATION && blind_zone(p->vitesse, diff)){
            count++;
            d->x += (v->pos.x - p->pos.x)/distance(v->pos,p->pos)*V_INITIALE;
            d->y += (v->pos.y - p->pos.y)/distance(v->pos,p->pos)*V_INITIALE;
            p->tab_attraction[j] = v;
        }
        else{
            p->tab_attraction[j] = NULL;
        }
    }
    if(count != 0){
       float norme = sqrtf((d->x)*(d->x)+(d->y)*(d->y));
        if(norme>0){
            d->x = d->x/norme;
            d->y= d->y/norme;
            p->vitesse.x = ((1-ALPHA)*p->vitesse.x + ALPHA*d->x);
            p->vitesse.y = ((1-ALPHA)*p->vitesse.y + ALPHA*d->y);

            float v = normer(p->vitesse);
            if(v>V_INITIALE){
                p->vitesse.x = ((p->vitesse.x)/v)*V_INITIALE;
                p->vitesse.y = ((p->vitesse.y)/v)*V_INITIALE;
            }
        }
    }
    free(d);
}

void poisson_deplacer(Poisson* p){
    float r = (((float)rand() / RAND_MAX)*2*BRUIT) - BRUIT;
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
void draw_circle(SDL_Renderer* renderer, int cx, int cy, int r) {
    int x = 0;
    int y = r;
    int d = 3 - 2 * r; // Décision initiale pour le tracé

    // Dessiner les points des 8 octants
    while (y >= x) {
        SDL_RenderDrawPoint(renderer, cx + x, cy + y);
        SDL_RenderDrawPoint(renderer, cx - x, cy + y);
        SDL_RenderDrawPoint(renderer, cx + x, cy - y);
        SDL_RenderDrawPoint(renderer, cx - x, cy - y);
        SDL_RenderDrawPoint(renderer, cx + y, cy + x);
        SDL_RenderDrawPoint(renderer, cx - y, cy + x);
        SDL_RenderDrawPoint(renderer, cx + y, cy - x);
        SDL_RenderDrawPoint(renderer, cx - y, cy - x);

        x++;

        // Mise à jour de la décision
        if (d > 0) {
            y--;
            d = d + 4 * (x - y) + 10;
        } else {
            d = d + 4 * x + 6;
        }
    }
}

//dessine un poisson 
void poisson_dessiner(Poisson* p, SDL_Renderer* renderer, SDL_Color c1, SDL_Color c2, SDL_Color c3, SDL_Color c4,SDL_Color c5,SDL_Color c6, Poisson voisin[NB_POISSONS]){
    SDL_SetRenderDrawColor(renderer, c1.r,c1.g,c1.b,255);
    draw_circle(renderer,(int)p->pos.x,(int)p->pos.y,TAILLE_POISSON);
    if(p == &voisin[0]){SDL_SetRenderDrawColor(renderer, c3.r,c3.g,c3.b,255);
    }
    else if(p->infecte && !appartient(p, voisin[0].tab_orientation) && !appartient(p, voisin[0].tab_attraction)) {
        SDL_SetRenderDrawColor(renderer, c5.r,c5.g,c5.b,255);
    }
    else if (appartient(p, voisin[0].tab_orientation)) {
        SDL_SetRenderDrawColor(renderer, c4.r,c4.g,c4.b,255);
    }
    else if (appartient(p, voisin[0].tab_attraction)) {
        SDL_SetRenderDrawColor(renderer, c6.r,c6.g,c6.b,255);
    }
    else {SDL_SetRenderDrawColor(renderer, c2.r,c2.g,c2.b,255);}
    float dx = (p->vitesse.x / normer(p->vitesse)) * TAILLE_POISSON * 2.0f;
    float dy = (p->vitesse.y / normer(p->vitesse)) * TAILLE_POISSON * 2.0f;
    draw_circle(renderer,(int)(p->pos.x + 3.5/5 * dx),(int)(p->pos.y + 3.5/5 * dy),TAILLE_POISSON);
    float n = sqrtf(p->vitesse.x * p->vitesse.x + p->vitesse.y * p->vitesse.y);
    if (n > 0.001f) {
        float dx = (p->vitesse.x / n) * TAILLE_POISSON * 2.0f;
        float dy = (p->vitesse.y / n) * TAILLE_POISSON * 2.0f;
        SDL_SetRenderDrawColor(renderer, 0, 255, 255, 255);
        SDL_RenderDrawLine(renderer,(int)p->pos.x, (int)p->pos.y,(int)(p->pos.x - dx), (int)(p->pos.y - dy));
    }
}

float rayon_moyen(Poisson poisson[NB_POISSONS], float mx, float my,SDL_Renderer* renderer){
    float sum = 0;
    for(int i=0; i<NB_POISSONS; i++){
        sum += sqrtf((poisson[i].pos.x - mx)*(poisson[i].pos.x - mx) + (poisson[i].pos.y - my)*(poisson[i].pos.y - my));
    }
    float rayonmoyen = sum / NB_POISSONS;
    draw_circle(renderer, (int)mx, (int)my, (int)rayonmoyen);

}


Vector2 moyenne_position(Poisson poisson[NB_POISSONS], SDL_Renderer* renderer, int nb_frame, Vector2 moy_prec){
    float mx = 0;
    float my = 0;
    for(int i=0; i<NB_POISSONS; i++){
        mx += poisson[i].pos.x;
        my += poisson[i].pos.y;
    }
    mx = mx / NB_POISSONS;
    my = my / NB_POISSONS;
    draw_circle(renderer, (int)mx, (int)my, TAILLE_POISSON);
    bool dist_moy = distance((Vector2){mx, my}, moy_prec) < 30;
    

    
    float max = 0;
    float min = 0;
    for(int i=0; i<NB_POISSONS; i++){
        float dist = sqrtf((poisson[i].pos.x - mx)*(poisson[i].pos.x - mx) + (poisson[i].pos.y - my)*(poisson[i].pos.y - my));
        if(i == 0){
            max = dist;
            min = dist;
        }
        else {
            if(dist > max){
                max = dist;
            }
            if(dist < min){
                min = dist;
            }
        }
    };
    bool cycle = ((max-min)<600) && dist_moy;
    if(cycle){
        rayon_moyen(poisson, mx, my,renderer);
        draw_circle(renderer, (int)mx, (int)my, min);
        draw_circle(renderer, (int)mx, (int)my, max);
    }

    if(nb_frame % 100 == 0){
        printf("Moyenne position : (%.2f, %.2f)\n", mx, my);
        printf("Cycle ? %s\n", cycle ? "Oui" : "Non");
        if(nb_frame % 100 == 0 && !dist_moy){
            printf("    -> probleme distance à ancienne moyenne : %.2f \n", distance((Vector2){mx, my}, moy_prec));
        }
        else if(nb_frame % 100 == 0 && (max-min)>=600){
            printf("    -> probleme dispersion des poissons : %d \n", (int)(max-min));
        }
        printf("\n");
    }
    
    return (Vector2){mx, my};
}

void cycle_infecte(Poisson poisson[NB_POISSONS], int nb_frame){
    bool all_infecte = true;
    for(int i=0; i<NB_POISSONS; i++){
        if(!poisson[i].infecte){
            if(nb_frame % 200 == 0){
            printf("Poisson %d n'est pas infecté.\n", poisson[i].id);
            }

            all_infecte = false;
        }
    }
    if(all_infecte){
        printf("Tous les poissons sont infectés !\n");
    }
}



//
int main(){
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* window = SDL_CreateWindow("MES POISSONS D'AMOUR",SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED,LARGEUR,HAUTEUR,SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window,-1,SDL_RENDERER_ACCELERATED);

    srand(time(NULL));

    Poisson poissons[NB_POISSONS];

    for(int i=0;i<NB_POISSONS;i++){
        float angle = ((float)rand()/RAND_MAX)*2*M_PI;
        int x = rand()% (LARGEUR-200)+100;
        int y = rand()% (HAUTEUR-200)+100;
        poisson_init(&poissons[i],x,y,angle,i);
    }

    bool running=true;
    SDL_Event e;
    int nb_frame = 0;
    Vector2 moy = {0,0};

    while(running){
        for(int i=0;i<NB_POISSONS;i++) {
            poisson_deplacer(&poissons[i]);
            bool zr = zone_repulsion(&poissons[i],poissons);
            if(zr){
                zone_orientation(&poissons[i],poissons);
                zone_attration(&poissons[i],poissons);
            }
           /* if(poissons[i].id != 0){
                (&poissons[i])->infecte = false;
            }
            cycle_infecte(poissons, nb_frame); */
        }

        SDL_SetRenderDrawColor(renderer,0,0,0,255);
        SDL_RenderClear(renderer);

        for(int i=NB_POISSONS-1 ;i>=0;i--){
            poisson_dessiner(&poissons[i],renderer,COULEURS[7] /*cyan*/, COULEURS[0]/*blanc*/,  COULEURS[4] /*vert*/, COULEURS[2] /*rouge*/,COULEURS[8] /*violet*/ ,COULEURS[5] /*jaune*/,poissons); 
        }
        SDL_SetRenderDrawColor(renderer,255,165,0,255);
        Vector2 moy = moyenne_position(poissons, renderer, nb_frame, moy);   

        while(SDL_PollEvent(&e)){
            if(e.type==SDL_QUIT) running=false;
            if(e.type==SDL_KEYDOWN && e.key.keysym.sym==SDLK_q) running=false;
            if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_d) {
                //
            }
        }


        SDL_RenderPresent(renderer);
        SDL_Delay(15);
        nb_frame++;
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
