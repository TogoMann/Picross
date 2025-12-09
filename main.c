#include "fonctions.h"

int main(int argc, char *argv[]) {
    (void)argc;
    (void)argv;

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
        printf("Erreur SDL_Init : %s\n", SDL_GetError());
        return -1;
    }

    if (TTF_Init() < 0) {
        printf("Erreur TTF_Init : %s\n", TTF_GetError());
        SDL_Quit();
        return -1;
    }

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        printf("Erreur Mix_OpenAudio : %s\n", Mix_GetError());
        TTF_Quit();
        SDL_Quit();
        return -1;
    }

    FILE *fichier = fopen("test.txt", "r");
    if (!fichier) {
        printf("Impossible d’ouvrir le fichier test.txt\n");
        Mix_CloseAudio();
        TTF_Quit();
        SDL_Quit();
        return -1;
    }

    Grille *grille_solution = chargerGrillesolution(fichier);
    fclose(fichier);

    if (!grille_solution) {
        printf("Échec du chargement de la grille solution.\n");
        Mix_CloseAudio();
        TTF_Quit();
        SDL_Quit();
        return -1;
    }

    Grille *grille_jeu = chargerGrille(grille_solution);
    if (!grille_jeu) {
        printf("Échec du chargement de la grille de jeu.\n");
        libererGrille(grille_solution);
        Mix_CloseAudio();
        TTF_Quit();
        SDL_Quit();
        return -1;
    }

    int largeurGrillePixels = grille_solution->largeur * TAILLE_CASE;
    int hauteurGrillePixels = grille_solution->hauteur * TAILLE_CASE;

    int largeurFenetre      = largeurGrillePixels + OFFSET_X + BTN_W + 50;
    int hauteurFenetre      = hauteurGrillePixels + OFFSET_Y + 50;

    SDL_Window *fenetre = SDL_CreateWindow(
        "Picross",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        largeurFenetre, hauteurFenetre,
        0
    );
    if (!fenetre) {
        printf("Impossible de créer la fenêtre.\n");
        libererGrille(grille_jeu);
        libererGrille(grille_solution);
        Mix_CloseAudio();
        TTF_Quit();
        SDL_Quit();
        return -1;
    }

    SDL_Renderer *rendu = SDL_CreateRenderer(fenetre, -1, SDL_RENDERER_ACCELERATED);
    if (!rendu) {
        printf("Impossible de créer le renderer.\n");
        SDL_DestroyWindow(fenetre);
        libererGrille(grille_jeu);
        libererGrille(grille_solution);
        Mix_CloseAudio();
        TTF_Quit();
        SDL_Quit();
        return -1;
    }

    Menu_principal(rendu, fenetre, grille_solution, grille_jeu,
                   largeurGrillePixels, hauteurGrillePixels);

    SDL_DestroyRenderer(rendu);
    SDL_DestroyWindow(fenetre);
    libererGrille(grille_jeu);
    libererGrille(grille_solution);

    Mix_CloseAudio();
    TTF_Quit();
    SDL_Quit();

    return 0;
}