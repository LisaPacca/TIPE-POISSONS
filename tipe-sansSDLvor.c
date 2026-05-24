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
#define NB_POISSONS 200
#define RAYON_ATTRACTION 145
#define RAYON_ORIENTATION 100
#define RAYON_REPULSION 5
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

bool zone_repulsion(Poisson* p, Poisson voisin[NB_POISSONS], int rr, int ro, int ra){
    Vector2* d = malloc(sizeof(Vector2));
    d->x = 0;
    d->y = 0;
    int count = 0;
    for(int j = 0; j<NB_POISSONS; j++){
        Poisson* v = &voisin[j];
        if(v == p) { continue; }
        float dist = distance(p->pos,v->pos);
        Vector2 diff = {v->pos.x - p->pos.x, v->pos.y - p->pos.y};
        if(dist < rr && blind_zone(p->vitesse,diff)){
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
void zone_orientation(Poisson* p, Poisson voisin[NB_POISSONS], int rr, int ro, int ra){
    Vector2* d= malloc(sizeof(Vector2));
    d->x = 0;
    d->y = 0;
    int count = 0;
    for(int j = 0; j<NB_POISSONS; j++){
        Poisson* v = &voisin[j];
        float dist = distance(p->pos,v->pos);
        Vector2 diff = {v->pos.x - p->pos.x, v->pos.y - p->pos.y};
        if(dist < ro && blind_zone(p->vitesse,diff)){
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


void zone_attration(Poisson* p, Poisson voisin[NB_POISSONS], int rr, int ro, int ra){
    Vector2* d= malloc(sizeof(Vector2));
    d->x = 0;
    d->y = 0;
    int count = 0;
    for(int j = 0; j<NB_POISSONS; j++){
        Poisson* v = &voisin[j];
        if(v == p) { continue; }
        float dist = distance(p->pos,v->pos);
        Vector2 diff = {v->pos.x - p->pos.x, v->pos.y - p->pos.y};
        if(dist < ra && dist > ro && blind_zone(p->vitesse, diff)){
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



float rayon_moyen(Poisson poisson[NB_POISSONS], float mx, float my){
    float sum = 0;
    for(int i=0; i<NB_POISSONS; i++){
        sum += sqrtf((poisson[i].pos.x - mx)*(poisson[i].pos.x - mx) + (poisson[i].pos.y - my)*(poisson[i].pos.y - my));
    }
    float rayonmoyen = sum / NB_POISSONS;
    return rayonmoyen;

}

float pgroup(Poisson poisson[NB_POISSONS], float* pgroupe){
    float pgroupx = 0;
    float pgroupy = 0;
    for(int i=0; i<NB_POISSONS; i++){
        float norme = sqrtf(poisson[i].vitesse.x*poisson[i].vitesse.x + poisson[i].vitesse.y*poisson[i].vitesse.y);
        pgroupx += poisson[i].vitesse.x/norme;
        pgroupy += poisson[i].vitesse.y/norme;
    }
    float pgr = sqrtf(pgroupx*pgroupx + pgroupy*pgroupy)/NB_POISSONS;
    *pgroupe = *pgroupe + pgr;
    return pgr;
}

float mgroup(Poisson poisson[NB_POISSONS], float mx, float my, float* mgroupe){
    float mg = 0;
    for(int i=0; i<NB_POISSONS; i++){
        float norme1 = sqrtf(poisson[i].vitesse.x*poisson[i].vitesse.x + poisson[i].vitesse.y*poisson[i].vitesse.y);
        float ricx = poisson[i].pos.x - mx;
        float ricy = poisson[i].pos.y - my;
        float norme2 = sqrtf(ricx*ricx + ricy*ricy);
        mg = mg + ricx*poisson[i].vitesse.y/(norme1*norme2) - ricy*poisson[i].vitesse.x/(norme1*norme2);
    }
    float mgr = sqrtf(mg*mg) / NB_POISSONS;
    *mgroupe = *mgroupe + mgr;
    return mgr;
}


Vector2 moyenne_position(Poisson poisson[NB_POISSONS], int nb_frame, Vector2 moy_prec, Vector2 tab_moy[3], float* tab_rm, bool* tjrscycle, float* pgroupe, float* mgroupe, float* tab_pg, float* tab_mg){
    float mx = 0;
    float my = 0;
    for(int i=0; i<NB_POISSONS; i++){
        mx += poisson[i].pos.x;
        my += poisson[i].pos.y;
    }
    mx = mx / NB_POISSONS;
    my = my / NB_POISSONS;
    
    
    bool dist_moy = distance((Vector2){mx, my}, moy_prec) < 0.5;
    

    
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
    bool cycle =  dist_moy;
    
    
    if(nb_frame >= 9000){
        float pg = pgroup(poisson, pgroupe);
        float mg = mgroup(poisson, mx, my, mgroupe);
    }
   
    if(nb_frame % 100 == 0 && nb_frame >= 9000){
        float pgr = *pgroupe / 100;
        float mgr = *mgroupe / 100;
        tab_pg[(nb_frame-9000)/100] = pgr;
        tab_mg[(nb_frame-9000)/100] = mgr;
        if(cycle){
            float rm = rayon_moyen(poisson, mx, my);
            tab_rm[(nb_frame-9000)/100] = rm;
        }
        else{
            *tjrscycle = false;
        }
        *pgroupe = 0;
        *mgroupe = 0;
    }
    
    return (Vector2){mx, my};
}


//
float simulation(int rr, int ro, int ra){
    srand(time(NULL));
    int n = 11;

    float* tab_rm = malloc(n * sizeof(float));
    float* tab_pg = malloc(n * sizeof(float));
    float* tab_mg = malloc(n * sizeof(float));
    for (int i = 0; i < n; i++) {
        tab_rm[i] = 0;
        tab_pg[i] = 0;
        tab_mg[i] = 0;
    }

    Poisson poissons[NB_POISSONS];

    for(int i=0;i<NB_POISSONS;i++){
        float angle = ((float)rand()/RAND_MAX)*2*M_PI;
        int x = rand()% (LARGEUR-200)+100;
        int y = rand()% (HAUTEUR-200)+100;
        poisson_init(&poissons[i],x,y,angle,i);
    }

    bool* tjrscycle = malloc(sizeof(bool));
    *tjrscycle = true;
    int nb_frame = 0;
    Vector2 moy = {0,0};
    Vector2 tab_moy[3];
    float* pgroup = malloc(sizeof(float));
    float* mgroup = malloc(sizeof(float));
    *pgroup = 0;
    *mgroup = 0;

    while(nb_frame<10000){
        for(int i=0;i<NB_POISSONS;i++) {
            poisson_deplacer(&poissons[i]);
            bool zr = zone_repulsion(&poissons[i],poissons, rr, ro, ra);
            if(zr){
                zone_orientation(&poissons[i],poissons, rr, ro, ra);
                zone_attration(&poissons[i],poissons, rr, ro, ra);
            }
        }
        Vector2 moy = moyenne_position(poissons, nb_frame, moy, tab_moy, tab_rm, tjrscycle, pgroup, mgroup, tab_pg, tab_mg);  
        nb_frame++;
    }

    float rayon_moyen_cycle;
    if(*tjrscycle){
        printf("Cycle !\n");
        float sommerm = 0;
        float sommepg = 0;
        float sommemg = 0;
        for(int i=0; i<n; i++){
            sommerm += tab_rm[i];
            sommepg += tab_pg[i];
            sommemg += tab_mg[i];
        }
        rayon_moyen_cycle = sommerm / n;
        float pgroup = sommepg / n;
        float mgroup = sommemg / n;
        // printf("    ➫ Rayon moyen du cycle : %.3f\n", rayon_moyen_cycle);
        printf("➫ Pgroup : %.3f\n", pgroup);
        printf("➫ Mgroup : %.3f\n", mgroup);
    }
    else{
        // printf("PAS de cycle \n");
        float sommepg = 0;
        float sommemg = 0;
        for(int i=0; i<n; i++){
            sommepg += tab_pg[i];
            sommemg += tab_mg[i];
        }
        float pgroup = sommepg / n;
        float mgroup = sommemg / n;
        printf("➫ Pgroup : %.3f\n", pgroup);
        printf("➫ Mgroup : %.3f\n", mgroup);
        
        rayon_moyen_cycle = -1;
    }
    free(tab_rm);
    free(tjrscycle);
    return rayon_moyen_cycle;
}


int main(){

    FILE* data_file = fopen("rayon_cycle.dat", "w");
    if (data_file == NULL) {
        perror("Erreur lors de l'ouverture du fichier");
        printf("ERREUR");
    }
    else{
        for (int k = 1; k <= 5; k++) {
            printf("# Simulation %d\n", k);
            for(int i=10 ; i<=130; i= i + 10){
                printf("Rayon : %d   ↴\n \n", i);
                float rmc = simulation(RAYON_REPULSION, i, 135);
                int type = 1;
                /*
                type = 1 -> banc
                type = 2 -> cycle
                type = 3 -> essaim
                */
                // printf("comportement collectif : ");
                // scanf("%d", &type);

                printf("\n ------------------------------------------------------------ \n");
                if(rmc != -1){
                    fprintf(data_file, "%d %d %d\n", i, k, type);
                }
                else{
                    fprintf(data_file, "%d %d %d\n", i, k, type);
                }    
            }
        }
        fclose(data_file);

        FILE* gnuplot = popen("gnuplot -persistent", "w");
        if (gnuplot == NULL) {
            perror("Erreur lors de l'ouverture de Gnuplot");
        }
        else{
            fprintf(gnuplot, "set title 'RR= 5 | RA = 155'\n"); // ATTENTION MODIF
            fprintf(gnuplot, "set xlabel 'R_REPULSION'\n"); //ATTENTION
            fprintf(gnuplot, "set ylabel 'Rayon moyen du cycle'\n");
            fprintf(gnuplot, "set xrange [%d:%d]\n", 10, 150); // ATTENTION
            fprintf(gnuplot, "set grid\n");
            fprintf(gnuplot, "plot 'rayon_cycle.dat' using 1:2 with points pointtype 7 pointsize 2 linecolor rgb 'blue' title 'Cycle'\n");
            pclose(gnuplot);
        }
    }
    return 0;
}