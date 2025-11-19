#ifndef FONCTIONS_H
#define FONCTIONS_H

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>

#define WINDOW_W 800
#define WINDOW_H 600

#define BTN_W 100
#define BTN_H 40

#define TAILLE_CASE 40

typedef struct Grille {
    int **cases;
    int hauteur;
    int largeur;
    int taille;
    int nombre_cases_noires;
} Grille;


Grille *chargerGrille(Grille * grille_solution);
Grille *chargerGrillesolution(FILE *fichier);
void Verification(Grille * grille_solution, Grille * grille_jeu);
void libererGrille(Grille *grille);
void dessinerGrille(SDL_Renderer *rendu, Grille *grille);
void gererClic(Grille *grille, int x, int y);
int  read_grid_from_txt(const char *path, int ***out_grid, size_t *out_rows, size_t *out_cols); void free_grid(int **grid, size_t rows);
void print_grid(const int * const *grid, size_t rows, size_t cols);
int  compare_grids(const int * const *g1, const int * const *g2, size_t rows, size_t cols);
int  check_grids_from_files(const char *player_path, const char *solution_path);
void sauvegarderGrille(const char *nomFichier, Grille *grille);
Grille *chargerGrilleSauvegarde(const char *nomFichier);
#endif
