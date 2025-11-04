#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>

#define TAILLE_CASE 40

/*
 * Fonctions à placer dans fonctions.c (code) et fonctions.h (déclarations) :
 *
 * Grille *chargerGrillesolution(const char *nomFichier);
 * void libererGrille(Grille *grille);
 * void dessinerGrille(SDL_Renderer *rendu, Grille *grille);
 * void gererClic(Grille *grille, int x, int y);
 */

typedef struct Grille {
    int **cases;
    int hauteur;
    int largeur;
    int taille;
    int nombre_cases_noires;
} Grille;


Grille *chargerGrille(Grille * grille_solution) {




    Grille *grille_jeu = malloc(sizeof(Grille));

    grille_jeu->hauteur = grille_solution->hauteur;
    grille_jeu->largeur = grille_solution->largeur;
    grille_jeu->nombre_cases_noires=0;

        if(grille_solution->hauteur != grille_jeu->hauteur)
    {
        printf("Erreur de hauteur de la grille de jeu\n");
    }
    if(grille_solution->largeur != grille_jeu->largeur)
    {
        printf("Erreur de largeur de la grille de jeu\n");
    }


    grille_jeu->cases = malloc(grille_jeu->hauteur * sizeof(int *));

    for (int i = 0; i < grille_jeu->hauteur; i++)
    {
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
            if (colonnes == 0) colonnes = colonnesActuelles;
            lignes++;
            colonnesActuelles = 0;
        }
    }


    if (colonnesActuelles > 0)
    {
        lignes++;
        if (colonnes == 0) colonnes = colonnesActuelles;
    }

    rewind(fichier);

    Grille *grille_solution = malloc(sizeof(Grille));
    grille_solution->hauteur = lignes;
    grille_solution->largeur = colonnes;
    grille_solution->nombre_cases_noires=0;
    grille_solution->cases = malloc(grille_solution->hauteur * sizeof(int *));
    for (int i = 0; i < grille_solution->hauteur; i++)
        grille_solution->cases[i] = malloc(grille_solution->largeur * sizeof(int));

    for (int i = 0; i < grille_solution->hauteur; i++) {
        for (int j = 0; j < grille_solution->largeur; j++) {
            int c;
            do {
                c = fgetc(fichier);
            } while (c != '0' && c != '1' && c != EOF);
            if (c == EOF) break;
            grille_solution->cases[i][j] = c - '0';

            if(c==1)
            {
                grille_solution->nombre_cases_noires++; 
            }
        }
    }

    fclose(fichier);
    return grille_solution;
}

void Verification(Grille * grille_solution, Grille * grille_jeu)
{


    int compteur=0;

    for(int i=0;i<grille_jeu->hauteur;i++)
    {
    
            if( grille_jeu->cases[i] = grille_solution->cases[i])
            {
                compteur ++;
            }
    
    }

    if (compteur = grille_solution->nombre_cases_noires)
    {
        printf("yay\n");
    }
    
}
void libererGrille(Grille *grille) {
    for (int i = 0; i < grille->hauteur; i++)
        free(grille->cases[i]);
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

int main(int argc, char *argv[]) {

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        printf("Erreur SDL_Init : %s\n", SDL_GetError());
        return -1;
    }

    printf("SDL initialisé avec succès !\n");

    FILE *fichier = fopen("test.txt", "r");
    if (!fichier) {
        printf("Impossible d’ouvrir le fichier test.txt\n");
        SDL_Quit();
        return -1;
    }

    Grille *grille_solution = chargerGrillesolution(fichier);
    Grille * grille_jeu = chargerGrille(grille_solution);
    if (!grille_solution) {
        printf("Échec du chargement de la grille solution.\n");
        SDL_Quit();
        return -1;
    }
       if (!grille_jeu) {
        printf("Échec du chargement de la grille solution.\n");
        SDL_Quit();
        return -1;
    }

    int largeurFenetre = grille_solution->largeur * TAILLE_CASE;
    int hauteurFenetre = grille_solution->hauteur * TAILLE_CASE;

    SDL_Window *fenetre = SDL_CreateWindow("Affichage de la grille",SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,largeurFenetre, hauteurFenetre, 0);

    if (!fenetre) {
        printf("Impossible de créer la fenêtre.\n");
        libererGrille(grille_jeu);
        libererGrille(grille_solution);
        SDL_Quit();
        return -1;
    }

    SDL_Renderer *rendu = SDL_CreateRenderer(fenetre, -1, SDL_RENDERER_ACCELERATED);
    if (!rendu) {
        printf("Impossible de créer le renderer.\n");
        SDL_DestroyWindow(fenetre);
        libererGrille(grille_jeu);
        libererGrille(grille_solution);
        SDL_Quit();
        return -1;
    }

    SDL_Event evenement;
    int enCours = 1;

    while (enCours) {
        while (SDL_PollEvent(&evenement)) {
            if (evenement.type == SDL_QUIT)
                enCours = 0;
            else if (evenement.type == SDL_MOUSEBUTTONDOWN)
                gererClic(grille_jeu, evenement.button.x, evenement.button.y);
                Verification(grille_jeu, grille_solution);
        }

        SDL_SetRenderDrawColor(rendu, 255, 255, 255, 255);
        SDL_RenderClear(rendu);
        dessinerGrille(rendu, grille_jeu);
        SDL_RenderPresent(rendu);

        SDL_Delay(16); 
    }

    SDL_DestroyRenderer(rendu);
    SDL_DestroyWindow(fenetre);
    libererGrille(grille_jeu);
    libererGrille(grille_solution);
    SDL_Quit();

    return 0;
}
