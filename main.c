#include "fonctions.h"
#include <time.h>

int main(int argc, char *argv[]) {
    (void)argc;
    (void)argv;

    srand((unsigned int)time(NULL));

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
    Grille *grille_solution = NULL;
    Grille *grille_jeu = NULL;
    int largeurFenetre = DEFAULT_WINDOW_W;
    int hauteurFenetre = DEFAULT_WINDOW_H;
    int largeurGrillePixels = 0;
    int hauteurGrillePixels = 0;


    SDL_Window *fenetre = SDL_CreateWindow(
        "Picross",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        largeurFenetre, hauteurFenetre,
        SDL_WINDOW_RESIZABLE
    );
    if (!fenetre) {
        printf("Impossible de créer la fenêtre.\n");
        Mix_CloseAudio();
        TTF_Quit();
        SDL_Quit();
        return -1;
    }

    SDL_Renderer *rendu = SDL_CreateRenderer(fenetre, -1, SDL_RENDERER_ACCELERATED);
    if (!rendu) {
        printf("Impossible de créer le renderer.\n");
        SDL_DestroyWindow(fenetre);
        Mix_CloseAudio();
        TTF_Quit();
        SDL_Quit();
        return -1;
    }
    Menu_principal(rendu, fenetre, &grille_solution, &grille_jeu,largeurGrillePixels, hauteurGrillePixels);

    SDL_DestroyRenderer(rendu);
    SDL_DestroyWindow(fenetre);

    if (grille_jeu) libererGrille(grille_jeu);
    if (grille_solution) libererGrille(grille_solution);

    Mix_CloseAudio();
    TTF_Quit();
    SDL_Quit();

    return 0;
}
