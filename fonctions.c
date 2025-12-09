#include "fonctions.h"
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
        while (len > 0 && (ligne[len - 1] == '\n' || ligne[len - 1] == '\r')) ligne[--len] = '\0';
        int count_bits = 0;
        for (size_t i = 0; i < len; ++i) if (ligne[i] == '0' || ligne[i] == '1') count_bits++;
        if (count_bits == 0) continue;
        if (largeur == 0) largeur = count_bits;
        else if (largeur != count_bits) {
            for (int r = 0; r < hauteur; ++r) free(cases[r]);
            free(cases);
            return NULL;
        }
        if (hauteur >= capacite) {
            int nouvelle_capacite = (capacite == 0) ? 4 : capacite * 2;
            int **nouv = realloc(cases, nouvelle_capacite * sizeof(int *));
            if (!nouv) {
                for (int r = 0; r < hauteur; ++r) free(cases[r]);
                free(cases);
                return NULL;
            }
            cases = nouv;
            capacite = nouvelle_capacite;
        }
        int *row = malloc(largeur * sizeof(int));
        if (!row) {
            for (int r = 0; r < hauteur; ++r) free(cases[r]);
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
        free(cases);
        return NULL;
    }
    Grille *g = malloc(sizeof(Grille));
    if (!g) {
        for (int r = 0; r < hauteur; ++r) free(cases[r]);
        free(cases);
        return NULL;
    }
    g->cases = cases;
    g->hauteur = hauteur;
    g->largeur = largeur;
    g->taille = hauteur * largeur;
    g->nombre_cases_noires = nb_noires;

   

    g->indices_lignes = NULL;
    g->indices_colonnes = NULL;
    g->nb_indices_lignes = NULL;
    g->nb_indices_colonnes = NULL;

    return g;
}

Grille *chargerGrille(Grille *grille_solution) {
    if (!grille_solution) return NULL;
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
    g->indices_lignes = NULL;
    g->indices_colonnes = NULL;
    g->nb_indices_lignes = NULL;
    g->nb_indices_colonnes = NULL;
    
    return g;
}

int *calculerIndices(int *ligne, int taille, int *nb_indices)
{
    int *indices = malloc(taille * sizeof(int));
    int count = 0;
    int n = 0;
    for (int i = 0; i < taille; ++i) {
        if (ligne[i] == 1) n++;
        else if (n > 0) { indices[count++] = n; n = 0; }
    }
    if (n > 0) indices[count++] = n;
    *nb_indices = count;
    return indices;
}

void libererIndices(Grille *grille) {
    if (!grille) return;
    
    if (grille->indices_lignes) {
        for (int i = 0; i < grille->hauteur; ++i) {
            free(grille->indices_lignes[i]);
        }
        free(grille->indices_lignes);
    }
    if (grille->indices_colonnes) {
        for (int i = 0; i < grille->largeur; ++i) {
            free(grille->indices_colonnes[i]);
        }
        free(grille->indices_colonnes);
    }
    free(grille->nb_indices_lignes);
    free(grille->nb_indices_colonnes);
    
    grille->indices_lignes = NULL;
    grille->indices_colonnes = NULL;
    grille->nb_indices_lignes = NULL;
    grille->nb_indices_colonnes = NULL;
}

void calculerEtStockerTousIndices(Grille *grille_solution) {
    if (!grille_solution) return;
    
    int H = grille_solution->hauteur;
    int L = grille_solution->largeur;
    
    grille_solution->indices_lignes = malloc(H * sizeof(int*));
    grille_solution->indices_colonnes = malloc(L * sizeof(int*));
    grille_solution->nb_indices_lignes = malloc(H * sizeof(int));
    grille_solution->nb_indices_colonnes = malloc(L * sizeof(int));
    
    if (!grille_solution->indices_lignes || !grille_solution->indices_colonnes || 
        !grille_solution->nb_indices_lignes || !grille_solution->nb_indices_colonnes) {
        libererIndices(grille_solution); 
        return; 
    }

    for (int y = 0; y < H; ++y) {
        int nb;
        int *indices = calculerIndices(grille_solution->cases[y], L, &nb);
        grille_solution->indices_lignes[y] = indices;
        grille_solution->nb_indices_lignes[y] = nb;
    }

    for (int x = 0; x < L; ++x) {
        int col[H];
        for (int y = 0; y < H; ++y) {
            col[y] = grille_solution->cases[y][x];
        }
        
        int nb;
        int *indices = calculerIndices(col, H, &nb);
        grille_solution->indices_colonnes[x] = indices;
        grille_solution->nb_indices_colonnes[x] = nb;
    }
}

void libererGrille(Grille *grille) {
    if (!grille) return;
    libererIndices(grille); 

    if (grille->cases) {
        for (int i = 0; i < grille->hauteur; ++i) {
            free(grille->cases[i]);
        }
        free(grille->cases);
    }
    free(grille);
}

void dessinerGrille(SDL_Renderer *renderer, Grille *grille_jeu, Grille *grille_solution) {
    if (!grille_jeu || !grille_solution || !renderer) return;

    if (grille_solution->hauteur == 0 || grille_solution->largeur == 0) return;
    
    SDL_Rect fondGauche = {0, OFFSET_Y, OFFSET_X, grille_solution->hauteur * TAILLE_CASE};
    SDL_SetRenderDrawColor(renderer, 220, 220, 220, 255);
    SDL_RenderFillRect(renderer, &fondGauche);

    SDL_Rect fondHaut = {OFFSET_X, 0, grille_solution->largeur * TAILLE_CASE, OFFSET_Y};
    SDL_SetRenderDrawColor(renderer, 220, 220, 220, 255);
    SDL_RenderFillRect(renderer, &fondHaut);
    
    SDL_Rect coinHautGauche = {0, 0, OFFSET_X, OFFSET_Y};
    SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255); 
    SDL_RenderFillRect(renderer, &coinHautGauche);

    for (int y = 0; y < grille_jeu->hauteur; ++y) {
        for (int x = 0; x < grille_jeu->largeur; ++x) {
            SDL_Rect rect = {x * TAILLE_CASE + OFFSET_X + 1, y * TAILLE_CASE + OFFSET_Y + 1, TAILLE_CASE - 2, TAILLE_CASE - 2};
            
       
            if (grille_jeu->cases[y][x] == ETAT_NOIR)
            { 
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            } else { 
                SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            }
            SDL_RenderFillRect(renderer, &rect);
            
        
            if (grille_jeu->cases[y][x] == ETAT_MARQUE) { 
                
             
                int x1 = x * TAILLE_CASE + OFFSET_X;
                int y1 = y * TAILLE_CASE + OFFSET_Y;
                int x2 = x1 + TAILLE_CASE;
                int y2 = y1 + TAILLE_CASE;
                const int PADDING = 5; 
                
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); 
                SDL_RenderDrawLine(renderer, x1 + PADDING, y1 + PADDING, x2 - PADDING, y2 - PADDING);
                SDL_RenderDrawLine(renderer, x2 - PADDING, y1 + PADDING, x1 + PADDING, y2 - PADDING);
            }
        }
    }
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    for (int y = 0; y <= grille_solution->hauteur; ++y) {
        SDL_RenderDrawLine(renderer, 0, OFFSET_Y + y * TAILLE_CASE, OFFSET_X + grille_solution->largeur * TAILLE_CASE, OFFSET_Y + y * TAILLE_CASE);
    }
    for (int x = 0; x <= grille_solution->largeur; ++x) {
        SDL_RenderDrawLine(renderer, OFFSET_X + x * TAILLE_CASE, 0, OFFSET_X + x * TAILLE_CASE, OFFSET_Y + grille_solution->hauteur * TAILLE_CASE);
    }
    SDL_RenderDrawLine(renderer, 0, OFFSET_Y, OFFSET_X + grille_solution->largeur * TAILLE_CASE, OFFSET_Y);
    SDL_RenderDrawLine(renderer, OFFSET_X, 0, OFFSET_X, OFFSET_Y + grille_solution->hauteur * TAILLE_CASE);
    TTF_Font *font = TTF_OpenFont("fonts/sans.ttf", 18);
    if (!font) return; 

    SDL_Color couleurTexte = {0, 0, 0, 255};

    for (int y = 0; y < grille_solution->hauteur; ++y) {
        int nb;
        int *indices = calculerIndices(grille_solution->cases[y], grille_solution->largeur, &nb);
        int espace = 25;

        for (int i = 0; i < nb; ++i) {
            char buf[12];
            snprintf(buf, sizeof(buf), "%d", indices[i]);
            SDL_Surface *surf = TTF_RenderText_Solid(font, buf, couleurTexte);
            if(surf) {
                SDL_Texture *tex = SDL_CreateTextureFromSurface(renderer, surf);
                int tw = surf->w;
                int th = surf->h;
                SDL_FreeSurface(surf);
                
                int pos_x = OFFSET_X - ((nb - i) * espace) + (espace - tw) / 2;
                int pos_y = y * TAILLE_CASE + OFFSET_Y + (TAILLE_CASE - th) / 2;

                SDL_Rect r = { pos_x, pos_y, tw, th };
                SDL_RenderCopy(renderer, tex, NULL, &r);
                SDL_DestroyTexture(tex);
            }
        }
        free(indices);
    }

    for (int x = 0; x < grille_solution->largeur; ++x) {
        int col[grille_solution->hauteur];
        for (int y = 0; y < grille_solution->hauteur; ++y) col[y] = grille_solution->cases[y][x];
        int nb;
        int *indices = calculerIndices(col, grille_solution->hauteur, &nb);
        int espace = 20;

        for (int i = 0; i < nb; ++i) {
            char buf[12];
            snprintf(buf, sizeof(buf), "%d", indices[i]);
            SDL_Surface *surf = TTF_RenderText_Solid(font, buf, couleurTexte);
            if(surf) {
                SDL_Texture *tex = SDL_CreateTextureFromSurface(renderer, surf);
                int tw = surf->w;
                int th = surf->h;
                SDL_FreeSurface(surf);
                
                int pos_x = x * TAILLE_CASE + OFFSET_X + (TAILLE_CASE - tw) / 2;
                int pos_y = OFFSET_Y - ((nb - i) * espace) + (espace - th) / 2;

                SDL_Rect r = { pos_x, pos_y, tw, th };
                SDL_RenderCopy(renderer, tex, NULL, &r);
                SDL_DestroyTexture(tex);
            }
        }
        free(indices);
    }

    TTF_CloseFont(font);
}

void gererClicDroit(Grille *grille, int x, int y) {
    if (!grille) return;

    int col = (x - OFFSET_X) / TAILLE_CASE;
    int lig = (y - OFFSET_Y) / TAILLE_CASE;

    if (lig < 0 || lig >= grille->hauteur || col < 0 || col >= grille->largeur) return;
    if (grille->cases[lig][col] == ETAT_VIDE) {
        grille->cases[lig][col] = ETAT_MARQUE;
    } else if (grille->cases[lig][col] == ETAT_MARQUE) {
        grille->cases[lig][col] = ETAT_VIDE;
    }
}

void gererClic(Grille *grille, int x, int y) {
    if (!grille) return;

    int col = (x - OFFSET_X) / TAILLE_CASE;
    int lig = (y - OFFSET_Y) / TAILLE_CASE;

    if (lig < 0 || lig >= grille->hauteur || col < 0 || col >= grille->largeur) return;
    if (grille->cases[lig][col] == ETAT_MARQUE) {
        grille->cases[lig][col] = ETAT_VIDE;
    }
    grille->cases[lig][col] = (grille->cases[lig][col] == ETAT_NOIR) ? ETAT_VIDE : ETAT_NOIR;
}

void Verification(Grille *grille_solution, Grille *grille_jeu) {
    if (!grille_solution || !grille_jeu) return;

    if (grille_solution->hauteur != grille_jeu->hauteur || grille_solution->largeur != grille_jeu->largeur) {
        printf("Flop ! (dimensions différentes)\n");
        return;
    }

    int ok = 1;

    for (int y = 0; y < grille_solution->hauteur; ++y) {
        for (int x = 0; x < grille_solution->largeur; ++x) {
            
            int jeu_case = grille_jeu->cases[y][x];
            int sol_case = grille_solution->cases[y][x];
            if (sol_case == ETAT_NOIR) {
          
                if (jeu_case != ETAT_NOIR) {
                    ok = 0;
                    break; 
                }
            } 
      
            else { 
          
                if (jeu_case == ETAT_NOIR) {
                    ok = 0;
                    break; 
                }
            }
        }
        
        if (!ok) break; 
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

void afficherEcranFin(SDL_Renderer *renderer, const char *message) {
    int w = 0, h = 0;
    SDL_GetRendererOutputSize(renderer, &w, &h);

    int enCours = 1;
    SDL_Event e;

    while (enCours) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                enCours = 0;
            } else if (e.type == SDL_KEYDOWN ||
                       e.type == SDL_MOUSEBUTTONDOWN) {
                enCours = 0;
            }
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

  
        TTF_Font *font = TTF_OpenFont("fonts/font.otf", 24);
        if (font) {
            SDL_Color white = {255, 255, 255, 255};
            SDL_Surface *surf = TTF_RenderText_Solid(font, message, white);
            if (surf) {
                SDL_Texture *tex = SDL_CreateTextureFromSurface(renderer, surf);
                int tw = surf->w;
                int th = surf->h;
                SDL_FreeSurface(surf);
                SDL_Rect r = {w/2 - tw/2, h/2 - th/2, tw, th};
                SDL_RenderCopy(renderer, tex, NULL, &r);
                SDL_DestroyTexture(tex);
            }
            TTF_CloseFont(font);
        }


        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }
}

int grillesIdentiques(Grille *grille_solution, Grille *grille_jeu) {
    if (!grille_solution || !grille_jeu) return 0;

    if (grille_solution->hauteur != grille_jeu->hauteur ||
        grille_solution->largeur != grille_jeu->largeur) {
        return 0;
    }

    for (int y = 0; y < grille_solution->hauteur; ++y) {
        for (int x = 0; x < grille_solution->largeur; ++x) {
            if (grille_solution->cases[y][x] != grille_jeu->cases[y][x]) {
                return 0;
            }
        }
    }
    return 1;
}

SDL_Texture * texture_init(const char * fichier, SDL_Renderer * renderer)
{
    SDL_Texture *texture = IMG_LoadTexture(renderer, fichier);
    if (!texture){
    printf("Impossible de charger la texture %s : %s\n", fichier, IMG_GetError());
    }
    return texture;
}

Bouton creer_bouton(const char *fichier, SDL_Renderer *renderer, const char *label, TTF_Font *font, int x, int y)
{
    Bouton b;

    b.texture = texture_init(fichier, renderer);
    b.rect.x = x;
    b.rect.y = y;
    b.rect.w = BTN_W;
    b.rect.h = BTN_H;

    SDL_Color blanc = {255, 255, 255, 255};
    SDL_Surface *surf_texte = TTF_RenderText_Solid(font, label, blanc);
    b.texte = SDL_CreateTextureFromSurface(renderer, surf_texte);
    SDL_FreeSurface(surf_texte);

    int text_w, text_h;
    SDL_QueryTexture(b.texte, NULL, NULL, &text_w, &text_h);

    if (text_w > b.rect.w) b.rect.w = text_w + 20;
    if (text_h > b.rect.h) b.rect.h = text_h + 10;
    b.rect_texte.w = text_w;
    b.rect_texte.h = text_h;
    b.rect_texte.x = b.rect.x + (b.rect.w - text_w)/2;
    b.rect_texte.y = b.rect.y + (b.rect.h - text_h)/2;

    return b;
}

bool Clique(int souris_x, int souris_y, Bouton b)
{
    if (souris_x >= b.rect.x && souris_y >= b.rect.y && souris_x <= b.rect.x + b.rect.w && souris_y <= b.rect.y + b.rect.h)
    {
        return true;
    }
    return false;
}

void Menu_principal(SDL_Renderer *renderer, SDL_Window *window, Grille **grille_solution_ptr, Grille **grille_jeu_ptr, int largeurGrillePixels, int hauteurGrillePixels)
{
    (void)window;

    Mix_Music *musique = Mix_LoadMUS("music/musique.wav");
    if (musique) Mix_PlayMusic(musique, -1);

    SDL_Texture *texture_fond = texture_init("fond.jpg", renderer);
    if (!texture_fond) return;

    TTF_Font *font = TTF_OpenFont("fonts/font.otf", 36);
    if (!font) return;

    SDL_Color blanc = {255, 255, 255, 255};

    SDL_Surface *surf_titre = TTF_RenderText_Solid(font, "Picross Game", blanc);
    SDL_Texture *texture_titre = SDL_CreateTextureFromSurface(renderer, surf_titre);
    SDL_FreeSurface(surf_titre);
    int titre_w, titre_h;
    SDL_QueryTexture(texture_titre, NULL, NULL, &titre_w, &titre_h);
    SDL_Rect rect_titre = { (DEFAULT_WINDOW_W - titre_w) / 2, 50, titre_w, titre_h }; 

    Bouton btn_jouer   = creer_bouton("textures/btn_jouer.jpg", renderer, "Jouer", font, 100, 200);
    Bouton btn_param   = creer_bouton("textures/param.jpg", renderer, "Options", font, 100, 270);
    Bouton btn_quitter = creer_bouton("textures/quit.jpg", renderer, "Quitter", font, 100, 340);

    SDL_Event event;
    int running = 1;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT)
                running = 0;
            else if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
                int mx = event.button.x, my = event.button.y;
                if (Clique(mx, my, btn_jouer))
            
    
                    Menu_mode(renderer, window, grille_solution_ptr, grille_jeu_ptr, largeurGrillePixels, hauteurGrillePixels);
                else if (Clique(mx, my, btn_param))
                    Menu_parametres(renderer, window);
                else if (Clique(mx, my, btn_quitter))
                    running = 0;
            }
        }

        SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
        SDL_RenderClear(renderer);

        SDL_RenderCopy(renderer, texture_fond, NULL, NULL);
        SDL_RenderCopy(renderer, texture_titre, NULL, &rect_titre);

        SDL_RenderCopy(renderer, btn_jouer.texture, NULL, &btn_jouer.rect);
        SDL_RenderCopy(renderer, btn_jouer.texte, NULL, &btn_jouer.rect_texte);

        SDL_RenderCopy(renderer, btn_param.texture, NULL, &btn_param.rect);
        SDL_RenderCopy(renderer, btn_param.texte, NULL, &btn_param.rect_texte);

        SDL_RenderCopy(renderer, btn_quitter.texture, NULL, &btn_quitter.rect);
        SDL_RenderCopy(renderer, btn_quitter.texte, NULL, &btn_quitter.rect_texte);

        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    SDL_DestroyTexture(texture_titre);
    SDL_DestroyTexture(texture_fond);

    SDL_DestroyTexture(btn_jouer.texture);
    SDL_DestroyTexture(btn_jouer.texte);
    SDL_DestroyTexture(btn_param.texture);
    SDL_DestroyTexture(btn_param.texte);
    SDL_DestroyTexture(btn_quitter.texture);
    SDL_DestroyTexture(btn_quitter.texte);

    TTF_CloseFont(font);
    if (musique) Mix_FreeMusic(musique);
}


void Menu_jouer(SDL_Renderer *renderer, SDL_Window *window, Grille *grille_solution, Grille *grille_jeu, int largeurGrillePixels, int hauteurGrillePixels)
{
    (void)window;
    if (!grille_solution || !grille_jeu) {
        printf("Erreur: Tentative de jouer sans grille chargée.\n");
        return;
    }

    TTF_Font *font = TTF_OpenFont("fonts/sans.ttf", 24);
    if (!font) {
        printf("Erreur: Impossible de charger la police (Menu_jouer).\n");
        return; 
    }

    SDL_GetWindowSize(window, &largeurGrillePixels, &hauteurGrillePixels);

    Bouton boutonFinis = creer_bouton("textures/btn_jouer.jpg", renderer, "Verifier", font, largeurGrillePixels - 150, 10);
    Bouton boutonIndice = creer_bouton("textures/btn_jouer.jpg", renderer, "Indice", font, largeurGrillePixels - 150 - BTN_W - 10, 10);

    SDL_Event e;
    int enCours = 1;

    while(enCours) {
        while(SDL_PollEvent(&e)) {
            if(e.type == SDL_QUIT) enCours = 0;
        
            else if(e.type == SDL_MOUSEBUTTONDOWN) {
                int mx = e.button.x, my = e.button.y;
                
           
                if (e.button.button == SDL_BUTTON_LEFT) {
                    if(Clique(mx, my, boutonFinis)) {
                        int ok = grillesIdentiques(grille_solution, grille_jeu);
                        if(ok) afficherEcranFin(renderer, "Tout bon !");
                        else afficherEcranFin(renderer, "Flop !");
                    } 
                    else if (Clique(mx, my, boutonIndice)) {
                        indice(grille_solution, grille_jeu);
                    }
                    else {
                        gererClic(grille_jeu, mx, my); 
                    }
                }
                else if (e.button.button == SDL_BUTTON_RIGHT) {
                    gererClicDroit(grille_jeu, mx, my); 
                }
            }
        }

        SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);
        SDL_RenderClear(renderer);
        dessinerGrille(renderer, grille_jeu, grille_solution);

        SDL_RenderCopy(renderer, boutonFinis.texture, NULL, &boutonFinis.rect);
        SDL_RenderCopy(renderer, boutonFinis.texte, NULL, &boutonFinis.rect_texte);

        SDL_RenderCopy(renderer, boutonIndice.texture, NULL, &boutonIndice.rect);
        SDL_RenderCopy(renderer, boutonIndice.texte, NULL, &boutonIndice.rect_texte);
    
        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    SDL_DestroyTexture(boutonFinis.texture);
    SDL_DestroyTexture(boutonFinis.texte);
    SDL_DestroyTexture(boutonIndice.texture);
    SDL_DestroyTexture(boutonIndice.texte);
    TTF_CloseFont(font);
}

void Menu_parametres(SDL_Renderer *renderer, SDL_Window *window)
{
    if (!renderer || !window) return;

    TTF_Font *font = TTF_OpenFont("fonts/font.otf", 24); 
    if (!font) return;

    SDL_Color blanc = {255, 255, 255, 255};

    SDL_Texture *texture_fond = texture_init("fond.jpg", renderer);
    if (!texture_fond) return;
    Bouton btn_retour = creer_bouton("textures/btn_jouer.jpg", renderer, "Retour", font, 50, 50);

    const int BAR_W = 200;
    const int BAR_H = 10;
    const int BAR_X = 270;
    const int BAR_Y = 160;
    const int CURSOR_SIZE = 15;
    
    int volume = Mix_VolumeMusic(-1);
    char txt_volume[32];

    SDL_Texture *tex_volume = NULL;
    SDL_Rect rect_volume = {50, 150, 200, 30};
    
    bool dragging_volume = false; 
    SDL_Event event;
    int running = 1;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = 0;
            
            if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
                int mx = event.button.x, my = event.button.y;
                
                if (mx >= BAR_X && mx <= BAR_X + BAR_W && my >= BAR_Y - CURSOR_SIZE && my <= BAR_Y + BAR_H + CURSOR_SIZE) {
                    dragging_volume = true;
                }
                
                if (Clique(mx, my, btn_retour)) {
                    running = 0;
                }
            }
            else if (event.type == SDL_MOUSEBUTTONUP && event.button.button == SDL_BUTTON_LEFT)
            {
                dragging_volume = false;
            }
            
            else if (event.type == SDL_MOUSEMOTION)
            {
                if (dragging_volume) {
                    int mx = event.motion.x;
                    int new_cursor_x = mx;
                    if (new_cursor_x < BAR_X) new_cursor_x = BAR_X;
                    if (new_cursor_x > BAR_X + BAR_W) new_cursor_x = BAR_X + BAR_W;

                    float ratio = (float)(new_cursor_x - BAR_X) / BAR_W;
                    volume = (int)(ratio * MIX_MAX_VOLUME); 
                    
                    Mix_VolumeMusic(volume);
                }
            }
        }

        SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
        SDL_RenderClear(renderer);

        SDL_RenderCopy(renderer, texture_fond, NULL, NULL);
        
        if(tex_volume) SDL_DestroyTexture(tex_volume);
        snprintf(txt_volume, sizeof(txt_volume), "Volume : (%d)", volume);
        SDL_Surface *surf_vol = TTF_RenderText_Solid(font, txt_volume, blanc);
        tex_volume = SDL_CreateTextureFromSurface(renderer, surf_vol);
        SDL_FreeSurface(surf_vol);
        SDL_RenderCopy(renderer, tex_volume, NULL, &rect_volume);
        
        SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
        SDL_Rect bar_rect = {BAR_X, BAR_Y, BAR_W, BAR_H};
        SDL_RenderFillRect(renderer, &bar_rect);

        int cursor_x = BAR_X + (int)((float)volume / MIX_MAX_VOLUME * BAR_W);

        SDL_Rect cursor_rect = {cursor_x - 5, BAR_Y - 5, 10, BAR_H + 10};
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); 
        SDL_RenderFillRect(renderer, &cursor_rect);

        SDL_RenderCopy(renderer, btn_retour.texture, NULL, &btn_retour.rect);
        SDL_RenderCopy(renderer, btn_retour.texte, NULL, &btn_retour.rect_texte);

        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }
    SDL_DestroyTexture(texture_fond);
    SDL_DestroyTexture(btn_retour.texture);
    SDL_DestroyTexture(btn_retour.texte);
    if(tex_volume) SDL_DestroyTexture(tex_volume);
    TTF_CloseFont(font);
}

Grille *genererGrilleAleatoire(int taille) {

    
    
    Grille *g = malloc(sizeof(Grille));
    if (!g) return NULL;

    g->hauteur = taille;
    g->largeur = taille;
    g->taille = taille * taille;
    g->nombre_cases_noires = 0;
    g->cases = allouerCases(taille, taille);
    if (!g->cases) {
        free(g);
        return NULL;
    }

    for (int y = 0; y < taille; ++y) {
        for (int x = 0; x < taille; ++x) {
            if ((rand() % 2) == 0)
            {
                g->cases[y][x] = ETAT_NOIR; 
                g->nombre_cases_noires++;
            } 
            else
            {
                g->cases[y][x] = ETAT_VIDE; 
            }
        }
    }
    g->indices_lignes = NULL;
    g->indices_colonnes = NULL;
    g->nb_indices_lignes = NULL;
    g->nb_indices_colonnes = NULL;

    return g;
}

void Menu_mode(SDL_Renderer *renderer, SDL_Window *window, Grille **grille_solution_ptr, Grille **grille_jeu_ptr, int largeurGrillePixels, int hauteurGrillePixels)
{

    TTF_Font *font_titre = TTF_OpenFont("fonts/font.otf", 36); 
    TTF_Font *font_btn = TTF_OpenFont("fonts/font.otf", 24);
    
    if (!font_titre || !font_btn) {
        if (font_titre) TTF_CloseFont(font_titre);
        if (font_btn) TTF_CloseFont(font_btn);
        return;
    }

    SDL_Color blanc = {255, 255, 255, 255};

    SDL_Surface *surf_titre = TTF_RenderText_Solid(font_titre, "Choix du mode de jeu", blanc);
    if (!surf_titre) {
        TTF_CloseFont(font_titre);
        TTF_CloseFont(font_btn);
        return;
    }
    
    SDL_Texture *texture_titre = SDL_CreateTextureFromSurface(renderer, surf_titre);
    SDL_FreeSurface(surf_titre);
    
    int titre_w, titre_h;
    SDL_QueryTexture(texture_titre, NULL, NULL, &titre_w, &titre_h);
    
    int w, h;
    SDL_GetWindowSize(window, &w, &h);
    SDL_Rect rect_titre = { (w - titre_w) / 2, 50, titre_w, titre_h };

    SDL_Texture *texture_fond = texture_init("fond.jpg", renderer);
    if (!texture_fond) {
        SDL_DestroyTexture(texture_titre);
        TTF_CloseFont(font_titre);
        TTF_CloseFont(font_btn);
        return;
    }

    Bouton btn_predefini = creer_bouton("textures/btn_jouer.jpg", renderer, "Predefini", font_btn, 100, 200);
    Bouton btn_hexagones = creer_bouton("textures/btn_jouer.jpg", renderer, "Hexagones", font_btn, 100, 270);
    Bouton btn_aleatoire = creer_bouton("textures/btn_jouer.jpg", renderer, "Aleatoire", font_btn, 100, 340);
    Bouton btn_retour   = creer_bouton("textures/quit.jpg", renderer, "Retour", font_btn, 100, 410);

    SDL_Event event;
    int running = 1;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = 0;
            else if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
                int mx = event.button.x, my = event.button.y;
                
                if (Clique(mx, my, btn_predefini)) {
                  
                    Menu_selection_niveau_predefini(renderer, window, grille_solution_ptr, grille_jeu_ptr, largeurGrillePixels, hauteurGrillePixels);
                    running = 0; 
                } 
                else if (Clique(mx, my, btn_hexagones)) {
                 
                    printf("Mode Hexagones selectionne (Non implémente).\n");
                   
                } 
                else if (Clique(mx, my, btn_aleatoire)) {
                    printf("Mode Aleatoire selectionne.\n");
                    if (*grille_jeu_ptr) libererGrille(*grille_jeu_ptr);
                    if (*grille_solution_ptr) libererGrille(*grille_solution_ptr);
                    *grille_solution_ptr = genererGrilleAleatoire(10);
                    
                    if (!*grille_solution_ptr) {
                        printf("Erreur: Échec de la génération aléatoire.\n");
                        running = 0; 
                        continue;
                    } 
                    *grille_jeu_ptr = chargerGrille(*grille_solution_ptr);
                    calculerEtStockerTousIndices(*grille_solution_ptr);
                    Menu_jouer(renderer, window, *grille_solution_ptr, *grille_jeu_ptr, largeurGrillePixels, hauteurGrillePixels);
                    
                } 
                else if (Clique(mx, my, btn_retour)) {
                    running = 0; 
                }
            }
        }
        
        SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);
        SDL_RenderClear(renderer);
        
        if (texture_fond) {
             SDL_RenderCopy(renderer, texture_fond, NULL, NULL);
        }

        SDL_RenderCopy(renderer, texture_titre, NULL, &rect_titre);
        
        SDL_RenderCopy(renderer, btn_predefini.texture, NULL, &btn_predefini.rect);
        SDL_RenderCopy(renderer, btn_predefini.texte, NULL, &btn_predefini.rect_texte);

        SDL_RenderCopy(renderer, btn_hexagones.texture, NULL, &btn_hexagones.rect);
        SDL_RenderCopy(renderer, btn_hexagones.texte, NULL, &btn_hexagones.rect_texte);
        
        SDL_RenderCopy(renderer, btn_aleatoire.texture, NULL, &btn_aleatoire.rect);
        SDL_RenderCopy(renderer, btn_aleatoire.texte, NULL, &btn_aleatoire.rect_texte);
        
        SDL_RenderCopy(renderer, btn_retour.texture, NULL, &btn_retour.rect);
        SDL_RenderCopy(renderer, btn_retour.texte, NULL, &btn_retour.rect_texte);

        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    SDL_DestroyTexture(texture_titre);
    SDL_DestroyTexture(texture_fond);
    SDL_DestroyTexture(btn_predefini.texture);
    SDL_DestroyTexture(btn_predefini.texte);
    SDL_DestroyTexture(btn_hexagones.texture);
    SDL_DestroyTexture(btn_hexagones.texte);
    SDL_DestroyTexture(btn_aleatoire.texture);
    SDL_DestroyTexture(btn_aleatoire.texte);
    SDL_DestroyTexture(btn_retour.texture);
    SDL_DestroyTexture(btn_retour.texte);
    TTF_CloseFont(font_titre);
    TTF_CloseFont(font_btn);
}


void Menu_selection_niveau_predefini(SDL_Renderer *renderer, SDL_Window *window, Grille **grille_solution_ptr, Grille **grille_jeu_ptr, int largeurGrillePixels, int hauteurGrillePixels)
{
 
    TTF_Font *font_titre = TTF_OpenFont("fonts/font.otf", 36); 
    TTF_Font *font_btn = TTF_OpenFont("fonts/sans.ttf", 24);
    
    if (!font_titre || !font_btn) {
        if (font_titre) TTF_CloseFont(font_titre);
        if (font_btn) TTF_CloseFont(font_btn);
        return;
    }
    SDL_Color blanc = {255, 255, 255, 255};
    SDL_Surface *surf_titre = TTF_RenderText_Solid(font_titre, "Selectionnez le Niveau", blanc);
    if (!surf_titre) {
        TTF_CloseFont(font_titre);
        TTF_CloseFont(font_btn);
        return;
    }
    
    SDL_Texture *texture_titre = SDL_CreateTextureFromSurface(renderer, surf_titre);
    SDL_FreeSurface(surf_titre);
    
    int titre_w, titre_h;
    SDL_QueryTexture(texture_titre, NULL, NULL, &titre_w, &titre_h);
    
    int w, h;
    SDL_GetWindowSize(window, &w, &h);
    SDL_Rect rect_titre = { (w - titre_w) / 2, 50, titre_w, titre_h };


    SDL_Texture *texture_fond = texture_init("fond.jpg", renderer);
    if (!texture_fond) {
        SDL_DestroyTexture(texture_titre);
        TTF_CloseFont(font_titre);
        TTF_CloseFont(font_btn);
        return;
    }
    
    Bouton btn_niv1 = creer_bouton("textures/btn_jouer.jpg", renderer, "Niveau 1", font_btn, 100, 200);
    Bouton btn_niv2 = creer_bouton("textures/btn_jouer.jpg", renderer, "Niveau 2", font_btn, 100, 270);
    Bouton btn_niv3 = creer_bouton("textures/btn_jouer.jpg", renderer, "Niveau 3", font_btn, 100, 340);
    Bouton btn_niv4 = creer_bouton("textures/btn_jouer.jpg", renderer, "Niveau 4", font_btn, 100, 410);
    Bouton btn_retour   = creer_bouton("textures/quit.jpg", renderer, "Retour", font_btn, 100, 480);

    SDL_Event event;
    int running = 1;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = 0;
            else if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
                int mx = event.button.x, my = event.button.y;
                
                char chemin_fichier[100];
                int niveau_selectionne = 0;

                if (Clique(mx, my, btn_niv1)) niveau_selectionne = 1;
                else if (Clique(mx, my, btn_niv2)) niveau_selectionne = 2;
                else if (Clique(mx, my, btn_niv3)) niveau_selectionne = 3;
                else if (Clique(mx, my, btn_niv4)) niveau_selectionne = 4;
                else if (Clique(mx, my, btn_retour)) {
                    running = 0; 
                    continue;
                }

                if (niveau_selectionne > 0) {
                    snprintf(chemin_fichier, sizeof(chemin_fichier), "predefinis/predefini_niv%d.txt", niveau_selectionne);
                    
                    printf("Tentative de chargement du niveau : %s\n", chemin_fichier);
    
                    FILE *f = fopen(chemin_fichier, "r");
                    
                    if (f) {
                        if (*grille_jeu_ptr) libererGrille(*grille_jeu_ptr);
                        if (*grille_solution_ptr) libererGrille(*grille_solution_ptr);
                        *grille_solution_ptr = chargerGrillesolution(f);
                        fclose(f);
                        
                        if (!*grille_solution_ptr) {
                            printf("Erreur: Échec du chargement de la nouvelle grille solution.\n");
                            continue; 
                        }
                        *grille_jeu_ptr = chargerGrille(*grille_solution_ptr);
                        
                        if (!*grille_jeu_ptr){
                            printf("Erreur: Échec de la création de la grille de jeu.\n");
                            libererGrille(*grille_solution_ptr);
                            *grille_solution_ptr = NULL;
                            continue; 
                        }
                        calculerEtStockerTousIndices(*grille_solution_ptr);
                        Menu_jouer(renderer, window, *grille_solution_ptr, *grille_jeu_ptr, largeurGrillePixels, hauteurGrillePixels);
                        
                        running = 0;
                        
                    } else {
                       printf("Erreur: Impossible d'ouvrir le fichier %s\n", chemin_fichier);
                    }
                }
            }
        }
        
        SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);
        SDL_RenderClear(renderer);
        
        if (texture_fond) {
             SDL_RenderCopy(renderer, texture_fond, NULL, NULL);
        }

        SDL_RenderCopy(renderer, texture_titre, NULL, &rect_titre);
        
        SDL_RenderCopy(renderer, btn_niv1.texture, NULL, &btn_niv1.rect);
        SDL_RenderCopy(renderer, btn_niv1.texte, NULL, &btn_niv1.rect_texte);
        
        SDL_RenderCopy(renderer, btn_niv2.texture, NULL, &btn_niv2.rect);
        SDL_RenderCopy(renderer, btn_niv2.texte, NULL, &btn_niv2.rect_texte);
        
        SDL_RenderCopy(renderer, btn_niv3.texture, NULL, &btn_niv3.rect);
        SDL_RenderCopy(renderer, btn_niv3.texte, NULL, &btn_niv3.rect_texte);

         SDL_RenderCopy(renderer, btn_niv4.texture, NULL, &btn_niv4.rect);
        SDL_RenderCopy(renderer, btn_niv4.texte, NULL, &btn_niv4.rect_texte);
        
        SDL_RenderCopy(renderer, btn_retour.texture, NULL, &btn_retour.rect);
        SDL_RenderCopy(renderer, btn_retour.texte, NULL, &btn_retour.rect_texte);

        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    SDL_DestroyTexture(texture_titre);
    SDL_DestroyTexture(texture_fond);
    SDL_DestroyTexture(btn_niv1.texture);
    SDL_DestroyTexture(btn_niv1.texte);
    SDL_DestroyTexture(btn_niv2.texture);
    SDL_DestroyTexture(btn_niv2.texte);
    SDL_DestroyTexture(btn_niv3.texture);
    SDL_DestroyTexture(btn_niv3.texte);
    SDL_DestroyTexture(btn_niv4.texture);
    SDL_DestroyTexture(btn_niv4.texte);
    SDL_DestroyTexture(btn_retour.texture);
    SDL_DestroyTexture(btn_retour.texte);
    TTF_CloseFont(font_titre);
    TTF_CloseFont(font_btn);
}

void indice(Grille *grille_solution, Grille *grille_jeu)
{
    int indices_donnes = 0;
    for (int y = 0; y < grille_solution->hauteur; ++y) {
        for (int x = 0; x < grille_solution->largeur; ++x) {
            if (grille_solution->cases[y][x] == ETAT_NOIR && grille_jeu->cases[y][x] != ETAT_NOIR) 
            {

                grille_jeu->cases[y][x] = ETAT_NOIR;
                indices_donnes++;
                if (indices_donnes >= NOMBRE_INDICES) {
                    return;
                }
            }
        }
    }
}