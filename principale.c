#include <stdio.h>
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include "simulation.h"

int main() {
    FILE* f = fopen("courbe.csv", "w");
    if(!f) { 
        printf("Erreur ouverture CSV\n"); 
        return 1; 
    }

    fprintf(f, "r_repulsion,rayon_moyen\n");

    float rmin = 100;
    float rmax = 110;
    int N = 10;

    for(int i=0; i<N; i++) {
        float r = rmin + i;
        bool cycle = trouve_cycle();
        float rayon = simulation_f(r, 100, 200);

        printf("rayon_repulsion=%.2f  ""Cycle ? %s\n", cycle ? "Oui" : "Non"" rayon_moyen_cycle=%.3f\n", r, cycle, rayon);
        fprintf(f, "%.2f,%.3f\n", r, rayon);
    }

    fclose(f);
    return 0;
}
