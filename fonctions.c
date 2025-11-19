#include "fonctions.h"

Grille *chargerGrille(Grille * grille_solution) {

    Grille *grille_jeu = malloc(sizeof(Grille));

    grille_jeu->hauteur = grille_solution->hauteur;
    grille_jeu->largeur = grille_solution->largeur;
    grille_jeu->nombre_cases_noires=0;

    if(grille_solution->hauteur != grille_jeu->hauteur) {
        printf("Erreur de hauteur de la grille de jeu\n");
    }
    if (grille_solution->largeur != grille_jeu->largeur) {
        printf("Erreur de largeur de la grille de jeu\n");
    }

    grille_jeu->cases = malloc(grille_jeu->hauteur * sizeof(int *));

    for (int i = 0; i < grille_jeu->hauteur; i++) {
        grille_jeu->cases[i] = malloc(grille_jeu->largeur * sizeof(int));
    }
    return grille_jeu;
}

Grille *chargerGrillesolution(FILE *fichier) {

    if (!fichier) {
        printf("Erreur : impossible d’ouvrir le fichier.\n");
        return NULL;
    }

    int lignes = 0;
    int colonnes = 0;
    int caractere, colonnesActuelles = 0;

    while ((caractere = fgetc(fichier)) != EOF) {
        if (caractere == '0' || caractere == '1') {
            colonnesActuelles++;
        } else if (caractere == '\n') {
            if (colonnes == 0) {
                colonnes = colonnesActuelles;
            }
            lignes++;
            colonnesActuelles = 0;
        }
    }


    if (colonnesActuelles > 0) {
        lignes++;
        if (colonnes == 0) {
            colonnes = colonnesActuelles;
        }
    }

    rewind(fichier);

    Grille *grille_solution = malloc(sizeof(Grille));
    grille_solution->hauteur = lignes;
    grille_solution->largeur = colonnes;
    grille_solution->nombre_cases_noires=0;
    grille_solution->cases = malloc(grille_solution->hauteur * sizeof(int *));
    for (int i = 0; i < grille_solution->hauteur; i++) {
        grille_solution->cases[i] = malloc(grille_solution->largeur * sizeof(int));
    }

    for (int i = 0; i < grille_solution->hauteur; i++) {
        for (int j = 0; j < grille_solution->largeur; j++) {
            int c;
            do {
                c = fgetc(fichier);
            } while (c != '0' && c != '1' && c != EOF);
            if (c == EOF) {
                break;
            }
            grille_solution->cases[i][j] = c - '0';

            if(c==1) {
                grille_solution->nombre_cases_noires++;
            }
        }
    }

    fclose(fichier);
    return grille_solution;
}

void Verification(Grille * grille_solution, Grille * grille_jeu){

    int compteur=0;

    for(int i=0;i<grille_jeu->hauteur;i++) {
        if( grille_jeu->cases[i] == grille_solution->cases[i]) {
            compteur ++;
        }
    }
    if (compteur == grille_solution->nombre_cases_noires){
        printf("yay\n");
    }
}

void libererGrille(Grille *grille) {
    for (int i = 0; i < grille->hauteur; i++) {
        free(grille->cases[i]);
    }
    free(grille->cases);
    free(grille);
}

void dessinerGrille(SDL_Renderer *rendu, Grille *grille) {
    for (int i = 0; i < grille->hauteur; i++) {
        for (int j = 0; j < grille->largeur; j++) {
            SDL_Rect cellule = {
                .x = j * TAILLE_CASE,
                .y = i * TAILLE_CASE,
                .w = TAILLE_CASE,
                .h = TAILLE_CASE
            };
            SDL_RenderFillRect(rendu, &cellule);
            SDL_SetRenderDrawColor(rendu, 200, 200, 200, 255);
            SDL_RenderDrawRect(rendu, &cellule);
        }
    }
}

void gererClic(Grille *grille, int x, int y) {
    if (x >= 0 && y >= 0 && x < grille->largeur * TAILLE_CASE && y < grille->hauteur * TAILLE_CASE) {
        int col = x / TAILLE_CASE;
        int ligne = y / TAILLE_CASE;
        grille->cases[ligne][col] = !grille->cases[ligne][col];
    }
}

hauteur largeur
CASE CASE CASE ...
...
void sauvegarderGrille(const char *nomFichier, Grille *grille) {
    FILE *f = fopen(nomFichier, "w");
    if (!f) {
        printf("Erreur : impossible d’ouvrir %s pour sauvegarde.\n", nomFichier);
        return;
    }

    fprintf(f, "%d %d\n", grille->hauteur, grille->largeur);

    for (int i = 0; i < grille->hauteur; i++) {
        for (int j = 0; j < grille->largeur; j++) {
            fprintf(f, "%d ", grille->cases[i][j]);
        }
        fprintf(f, "\n");
    }

    fclose(f);
    printf("Sauvegarde réalisée dans %s\n", nomFichier);
}
Grille *chargerGrilleSauvegarde(const char *nomFichier) {
    FILE *f = fopen(nomFichier, "r");
    if (!f) {
        printf("Aucune sauvegarde trouvée (%s).\n", nomFichier);
        return NULL;
    }

    int h, l;
    fscanf(f, "%d %d", &h, &l);

    Grille *g = malloc(sizeof(Grille));
    g->hauteur = h;
    g->largeur = l;
    g->nombre_cases_noires = 0;
    g->cases = malloc(h * sizeof(int *));

    for (int i = 0; i < h; i++) {
        g->cases[i] = malloc(l * sizeof(int));
        for (int j = 0; j < l; j++) {
            fscanf(f, "%d", &g->cases[i][j]);
            if (g->cases[i][j] == 1)
                g->nombre_cases_noires++;
        }
    }

    fclose(f);
    printf("Sauvegarde chargée depuis %s !\n", nomFichier);
    return g;
}
