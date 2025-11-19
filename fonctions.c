#include "fonctions.h"
#include <string.h>

static int **allouerCases(int hauteur, int largeur) {
    int **cases = malloc(hauteur * sizeof(int *));
    if (!cases) return NULL;

    for (int i = 0; i < hauteur; ++i) {
        cases[i] = calloc(largeur, sizeof(int));
        if (!cases[i]) {
            for (int j = 0; j < i; ++j) {
                free(cases[j]);
            }
            free(cases);
            return NULL;
        }
    }
    return cases;
}

Grille *chargerGrillesolution(FILE *fichier) {
    if (!fichier) return NULL;

    char ligne[1024];
    int **cases = NULL;
    int hauteur = 0;
    int largeur = 0;
    int capacite = 0;
    int nb_noires = 0;

    while (fgets(ligne, sizeof(ligne), fichier)) {
        size_t len = strlen(ligne);
        while (len > 0 &&
               (ligne[len - 1] == '\n' || ligne[len - 1] == '\r')) {
            ligne[--len] = '\0';
        }

        int count_bits = 0;
        for (size_t i = 0; i < len; ++i) {
            if (ligne[i] == '0' || ligne[i] == '1') {
                count_bits++;
            }
        }
        if (count_bits == 0) {
            continue;
        }

        if (largeur == 0) {
            largeur = count_bits;
        } else if (largeur != count_bits) {
            fprintf(stderr, "Ligne non rectangulaire dans test.txt\n");
            for (int r = 0; r < hauteur; ++r) {
                free(cases[r]);
            }
            free(cases);
            return NULL;
        }

        if (hauteur >= capacite) {
            int nouvelle_capacite = (capacite == 0) ? 4 : capacite * 2;
            int **nouv = realloc(cases, nouvelle_capacite * sizeof(int *));
            if (!nouv) {
                fprintf(stderr, "Erreur realloc pour les lignes de la grille\n");
                for (int r = 0; r < hauteur; ++r) {
                    free(cases[r]);
                }
                free(cases);
                return NULL;
            }
            cases = nouv;
            capacite = nouvelle_capacite;
        }

        int *row = malloc(largeur * sizeof(int));
        if (!row) {
            fprintf(stderr, "Erreur malloc pour une ligne de grille\n");
            for (int r = 0; r < hauteur; ++r) {
                free(cases[r]);
            }
            free(cases);
            return NULL;
        }

        int c = 0;
        for (size_t i = 0; i < len && c < largeur; ++i) {
            if (ligne[i] == '0' || ligne[i] == '1') {
                row[c] = (ligne[i] == '1') ? 1 : 0;
                if (row[c] == 1) nb_noires++;
                c++;
            }
        }

        cases[hauteur++] = row;
    }

    if (hauteur == 0 || largeur == 0) {
        fprintf(stderr, "Aucune donnée de grille lue dans test.txt\n");
        free(cases);
        return NULL;
    }

<<<<<<< HEAD
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
=======
    Grille *g = malloc(sizeof(Grille));
    if (!g) {
        fprintf(stderr, "Erreur malloc pour la structure Grille\n");
        for (int r = 0; r < hauteur; ++r) {
            free(cases[r]);
>>>>>>> cdbe6a620893c58f0961a9f255db84d437b5bf74
        }
        free(cases);
        return NULL;
    }

    g->cases = cases;
    g->hauteur = hauteur;
    g->largeur = largeur;
    g->taille = hauteur * largeur;
    g->nombre_cases_noires = nb_noires;

    return g;
}

<<<<<<< HEAD
void Verification(Grille * grille_solution, Grille * grille_jeu){

    int compteur=0;
=======
Grille *chargerGrille(Grille *grille_solution) {
    if (!grille_solution) return NULL;
>>>>>>> cdbe6a620893c58f0961a9f255db84d437b5bf74

    Grille *g = malloc(sizeof(Grille));
    if (!g) return NULL;

    g->hauteur = grille_solution->hauteur;
    g->largeur = grille_solution->largeur;
    g->taille  = g->hauteur * g->largeur;
    g->nombre_cases_noires = 0;

    g->cases = allouerCases(g->hauteur, g->largeur);
    if (!g->cases) {
        free(g);
        return NULL;
    }

    return g;
}

void libererGrille(Grille *grille) {
    if (!grille) return;

    if (grille->cases) {
        for (int i = 0; i < grille->hauteur; ++i) {
            free(grille->cases[i]);
        }
        free(grille->cases);
    }
    free(grille);
}

void dessinerGrille(SDL_Renderer *rendu, Grille *grille) {
    if (!grille || !rendu) return;

    for (int y = 0; y < grille->hauteur; ++y) {
        for (int x = 0; x < grille->largeur; ++x) {
            SDL_Rect rect;
            rect.x = x * TAILLE_CASE;
            rect.y = y * TAILLE_CASE;
            rect.w = TAILLE_CASE;
            rect.h = TAILLE_CASE;

            if (grille->cases[y][x] == 1) {
                SDL_SetRenderDrawColor(rendu, 0, 0, 0, 255);     
            } else {
                SDL_SetRenderDrawColor(rendu, 255, 255, 255, 255); 
            }
            SDL_RenderFillRect(rendu, &rect);

            SDL_SetRenderDrawColor(rendu, 0, 0, 0, 255);
            SDL_RenderDrawRect(rendu, &rect);
        }
    }
}

void gererClic(Grille *grille, int x, int y) {
<<<<<<< HEAD
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
=======
    if (!grille) return;

    int col = x / TAILLE_CASE;
    int lig = y / TAILLE_CASE;

    if (lig < 0 || lig >= grille->hauteur ||
        col < 0 || col >= grille->largeur) {
        return;
    }

    grille->cases[lig][col] = (grille->cases[lig][col] == 0) ? 1 : 0;
}

void Verification(Grille *grille_solution, Grille *grille_jeu) {
    if (!grille_solution || !grille_jeu) return;

    if (grille_solution->hauteur != grille_jeu->hauteur ||
        grille_solution->largeur != grille_jeu->largeur) {
        printf("Flop ! (dimensions différentes)\n");
        return;
    }

    int ok = 1;
    for (int y = 0; y < grille_solution->hauteur && ok; ++y) {
        for (int x = 0; x < grille_solution->largeur; ++x) {
            if (grille_solution->cases[y][x] != grille_jeu->cases[y][x]) {
                ok = 0;
                break;
            }
        }
    }

    if (ok) {
        printf("Tout bon !\n");
    } else {
        printf("Flop !\n");
    }
}

int read_grid_from_txt(const char *path, int ***out_grid, size_t *out_rows, size_t *out_cols) {
    if (!path || !out_grid || !out_rows || !out_cols) {
        return 1;
    }

    FILE *f = fopen(path, "r");
    if (!f) {
        return 2;
    }

    char ligne[1024];
    int **grid = NULL;
    size_t rows = 0, cols = 0, capacity = 0;

    while (fgets(ligne, sizeof(ligne), f)) {
        size_t len = strlen(ligne);
        while (len > 0 && (ligne[len - 1] == '\n' || ligne[len - 1] == '\r')) {
            ligne[--len] = '\0';
        }

        size_t count_bits = 0;
        for (size_t i = 0; i < len; ++i) {
            if (ligne[i] == '0' || ligne[i] == '1') {
                count_bits++;
            }
        }
        if (count_bits == 0) {
            continue;
        }

        if (cols == 0) {
            cols = count_bits;
        } else if (cols != count_bits) {
            fclose(f);
            for (size_t r = 0; r < rows; ++r) {
                free(grid[r]);
            }
            free(grid);
            return 4;
        }

        int *row = malloc(cols * sizeof(int));
        if (!row) {
            fclose(f);
            for (size_t r = 0; r < rows; ++r) {
                free(grid[r]);
            }
            free(grid);
            return 5;
        }

        size_t c = 0;
        for (size_t i = 0; i < len && c < cols; ++i) {
            if (ligne[i] == '0' || ligne[i] == '1') {
                row[c++] = (ligne[i] == '1') ? 1 : 0;
            }
        }


        if (rows >= capacity) {
            size_t new_cap = (capacity == 0) ? 4 : capacity * 2;
            int **new_grid = realloc(grid, new_cap * sizeof(int *));
            if (!new_grid) {
                fclose(f);
                free(row);
                for (size_t r = 0; r < rows; ++r) {
                    free(grid[r]);
                }
                free(grid);
                return 6;
            }
            grid = new_grid;
            capacity = new_cap;
        }

        grid[rows++] = row;
    }

    fclose(f);

    if (rows == 0 || cols == 0) {
        free(grid);
        return 7;
    }

    *out_grid = grid;
    *out_rows = rows;
    *out_cols = cols;
    return 0;
}

void free_grid(int **grid, size_t rows) {
    if (!grid) {
        return;
    }
    for (size_t r = 0; r < rows; ++r) {
        free(grid[r]);
    }
    free(grid);
}

void print_grid(const int * const *grid, size_t rows, size_t cols) {
    if (!grid) {
        return;
    }
    printf("Grille (%zu x %zu):\n", rows, cols);
    for (size_t r = 0; r < rows; ++r) {
        for (size_t c = 0; c < cols; ++c) {
            printf("%d", grid[r][c]);
        }
        printf("\n");
    }
}

int compare_grids(const int * const *g1, const int * const *g2, size_t rows, size_t cols) {
    if (!g1 || !g2) {
        return 0;
    }

    for (size_t r = 0; r < rows; ++r) {
        for (size_t c = 0; c < cols; ++c) {
            if (g1[r][c] != g2[r][c]) return 0;
        }
    }
    return 1;
}

int check_grids_from_files(const char *player_path, const char *solution_path) { 
    int **g1 = NULL, **g2 = NULL;
    size_t r1 = 0, c1 = 0, r2 = 0, c2 = 0;

    int rc1 = read_grid_from_txt(player_path, &g1, &r1, &c1);
    int rc2 = read_grid_from_txt(solution_path, &g2, &r2, &c2);
    if (rc1 != 0 || rc2 != 0) {
        if (g1) {
            free_grid(g1, r1);
        }
        if (g2) {
            free_grid(g2, r2);
        }
        return -1;
    }

    if (r1 != r2 || c1 != c2) {
        free_grid(g1, r1);
        free_grid(g2, r2);
        return 0;
    }

    int same = compare_grids((const int * const *)g1, (const int * const *)g2, r1, c1);

    free_grid(g1, r1);
    free_grid(g2, r2);
    return same ? 1 : 0;
}
>>>>>>> cdbe6a620893c58f0961a9f255db84d437b5bf74
