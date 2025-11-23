#include "graph.h"
#include "hasse.h"
#include "matrix.h"
#include "locale.h"
#include <windows.h>

/*point d'entrée du programme et flux principal d'exécution */
int main() {
    // pour la syntaxe UTF-8 sous Windows
    setlocale(LC_ALL, "fr_FR.UTF-8");
    SetConsoleOutputCP(CP_UTF8);



    // 1) Charger le graphe (Partie 1)
        /* Permettre à l'utilisateur de choisir un fichier dans le dossier data/
           On vérifie l'existence du fichier avant d'appeler readGraph (qui exit() en cas d'erreur). */
        char filename[256];
        char path[512];
        FILE *f = NULL;
        liste_adjacence la;
        while (1) {
            printf("Entrez le nom du fichier dans le dossier data (ex: exemple_meteo.txt) : ");
            if (scanf("%255s", filename) != 1) {
                fprintf(stderr, "Erreur lecture du nom de fichier.\n");
                return 1;
            }
            snprintf(path, sizeof(path), "data/%s", filename);
            f = fopen(path, "r");
            if (!f) {
                fprintf(stderr, "Impossible d'ouvrir '%s'. Veuillez entrer un nom de fichier valide.\n", path);
                continue;
            } else {
                fclose(f);
                la = readGraph(path);
                break;
            }
        }
    printf("\n=== Affichage du graphe (liste d'adjacence) ===\n");
    afficherListeAdjacence(la);
    printf("\n=== Vérification du graphe ===\n");
    verifierMarkov(la);
    printf("\n=== Exportation vers Mermaid (graphe original) ===\n");
    exporterMermaid(la, "data/graphe_mermaid.mmd");

    // 2) Partie 2 : Tarjan pour trouver les classes
    printf("\n=== Exécution de l'algorithme de Tarjan (composantes fortement connexes) ===\n");
    t_partition partition = tarjan(la);
    for (int i = 0; i < partition.nb; i++) {
        printf("Composante %s: {", partition.classes[i].name);
        for (int j = 0; j < partition.classes[i].size; j++) {
            printf("%d", partition.classes[i].members[j]);
            if (j + 1 < partition.classes[i].size) printf(",");
        }
        printf("}\n");
    }

    int n = la.taille;
    int *vertex_to_class = build_vertex_to_class_map(&partition, n);

    t_link_array hasse = create_links_from_partition(la, &partition, vertex_to_class);
    removeTransitiveLinks(&hasse);

    printf("\n=== Liens (classes) détectés ===\n");
    for (int i = 0; i < hasse.size; i++) {
        printf("C%d -> C%d\n", hasse.links[i].from, hasse.links[i].to);
    }

    exporterMermaidHasse(&partition, &hasse, "data/hasse_mermaid.mmd");

    compute_and_print_characteristics(&partition, &hasse);

    // Partie 3: Calculs matriciels
    t_matrix matrix = createMatrixFromAdjacency(la);
    printf("\n=== Matrice d'adjacence ===\n");
    printMatrix(matrix);

    // m3
    t_matrix m3 = createEmptyMatrix(matrix.rows);
    t_matrix temp1 = createEmptyMatrix(matrix.rows);
    t_matrix temp2 = createEmptyMatrix(matrix.rows);
    copyMatrix(temp1, matrix);
    multiplyMatrices(temp1, matrix, temp2);
    multiplyMatrices(temp2, matrix, m3);
    printf("\n=== M^3 ===\n");
    printMatrix(m3);

    //m7
    t_matrix m7 = createEmptyMatrix(matrix.rows);
    t_matrix temp3 = createEmptyMatrix(matrix.rows);
    copyMatrix(temp3, m3);
    for (int i = 0; i < 4; i++) {
        multiplyMatrices(temp3, matrix, m7);
        copyMatrix(temp3, m7);
    }
    printf("\n=== M^7 ===\n");
    printMatrix(m7);

    // convergence de M^n
    float epsilon = 0.01f;
    t_matrix prev = createEmptyMatrix(matrix.rows);
    t_matrix current = createEmptyMatrix(matrix.rows);
    copyMatrix(current, matrix);
    int power = 1;
    int max_iter = 10000; /* protection contre boucle infinie */
    while (power < max_iter) {
        copyMatrix(prev, current);
        multiplyMatrices(prev, matrix, current);
        power++;
        if (diffMatrices(prev, current) < epsilon) {
            break;
        }
    }
    if (power >= max_iter) {
        printf("⚠️  Avertissement : convergence non atteinte après %d itérations\n", max_iter);
    }
    printf("\n=== M^%d (convergence) ===\n", power);
    printMatrix(current);

    // Calcul des sous-matrices et périodes
    for (int i = 0; i < partition.nb; i++) {
        t_matrix submatrix = subMatrix(matrix, partition, i);
        printf("\n=== Sous-matrice pour la classe %s ===\n", partition.classes[i].name);
        printMatrix(submatrix);
        int period = getPeriod(submatrix);
        printf("Période de la classe %s: %d\n", partition.classes[i].name, period);

        /* Afficher sommes des lignes de la sous-matrice (vérifier stochastique ou non) */
        printf("Sommes des lignes de la sous-matrice (par état dans la classe) :\n");
        for (int r = 0; r < submatrix.rows; r++) {
            float s = 0.0f;
            for (int c = 0; c < submatrix.cols; c++) {
                s += submatrix.data[r][c];
            }
            char status[64];
            if (s > 0.999f && s < 1.001f) {
                strcpy(status, "(stochastique)");
            } else if (s < 0.999f) {
                strcpy(status, "(sous-stochastique: masse sortante possible)");
            } else {
                strcpy(status, "(>1 erreur?)");
            }
            printf("  Etat %d (dans classe) : somme = %.6f %s\n",
                   partition.classes[i].members[r], s, status);
        }

        /* Calculer la limite p0 * S^k pour p0 uniforme sur la classe (même pour classes transitoires) */
        {
            int m = submatrix.rows;
            float *pcur = createZeroVector(m);
            float *pnext = createZeroVector(m);
            if (m > 0) {
                for (int t = 0; t < m; t++) {
                    pcur[t] = 1.0f / m; /* uniforme */
                }
                int it = 0;
                int max_it_local = 100000;
                float eps_local = 1e-6f;
                while (it < max_it_local) {
                    multiplyVectorMatrix(pcur, submatrix, pnext);
                    float d = diffVectors(pcur, pnext, m);
                    copyVector(pcur, pnext, m);
                    if (d < eps_local) {
                        break;
                    }
                    it++;
                }
                float mass = 0.0f;
                for (int t = 0; t < m; t++) {
                    mass += pcur[t];
                }
                // stochastique signifie que la somme des composantes doit être 1(prochement), ce qui permet de savoir si la classe est fermée ou non
                printf("Limite (approx) pour p0 uniforme sur la classe (it=%d) : somme des composantes = %.8f\n", it, mass);
                for (int t = 0; t < m; t++) {
                    printf("  Etat %d : %.8f\n", partition.classes[i].members[t], pcur[t]);
                }
            }
            freeVector(pcur);
            freeVector(pnext);
        }
        /* Déterminer si la classe est persistante (pas de flèches sortantes dans le Hasse réduit) */
        if(compute_caracteristics(&partition, &hasse, i+1) == 0) {
            printf("-> La classe %s est persistante.\n", partition.classes[i].name);
        } else {
            printf("-> La classe %s est transitoire.\n", partition.classes[i].name);
        }

        /* libération de la sous-matrice */
        for (int rr = 0; rr < submatrix.rows; rr++) free(submatrix.data[rr]);
        free(submatrix.data);
    }

    /* libérer matrices temporaires utilisées plus haut */
    for (int i = 0; i < m3.rows; i++) {
        free(m3.data[i]);
    }
    free(m3.data);

    for (int i = 0; i < temp1.rows; i++) {
        free(temp1.data[i]);
    }
    free(temp1.data);

    for (int i = 0; i < temp2.rows; i++) {
        free(temp2.data[i]);
    }
    free(temp2.data);

    for (int i = 0; i < m7.rows; i++) {
        free(m7.data[i]);
    }
    free(m7.data);

    for (int i = 0; i < temp3.rows; i++) {
        free(temp3.data[i]);
    }
    free(temp3.data);

    for (int i = 0; i < prev.rows; i++) {
        free(prev.data[i]);
    }
    free(prev.data);

    for (int i = 0; i < current.rows; i++) {
        free(current.data[i]);
    }
    free(current.data);

    // Cleanup
    free(vertex_to_class);
    free_link_array(&hasse);
    free_partition(&partition);
    free_liste_adjacence(&la);
    for (int i = 0; i < matrix.rows; i++) {
        free(matrix.data[i]);
    }
    free(matrix.data);

    printf("\nTerminé.\n");
    printf("→ Copiez les fichiers '.mmd' dans https://www.mermaidchart.com/\n");

    return 0;
}
