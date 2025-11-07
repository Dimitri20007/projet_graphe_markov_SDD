#include "graph.h"
#include <string.h>

// === Génère l'identifiant (A, B, ..., Z, AA, AB, ...) ===
char* getId(int num) {
    static char id[3];
    id[0] = '\0';
    num--; // pour indexer à partir de 0

    if (num < 26) {
        id[0] = 'A' + num;
        id[1] = '\0';
    } else {
        id[0] = 'A' + (num / 26) - 1;
        id[1] = 'A' + (num % 26);
        id[2] = '\0';
    }
    return id;
}

// === Vérifie que la somme des probabilités sortantes vaut environ 1 ===
void verifierMarkov(liste_adjacence la) {
    int valide = 1;

    for (int i = 0; i < la.taille; i++) {
        float somme = 0.0;
        cellule* tmp = la.tab[i].head;

        while (tmp) {
            somme += tmp->proba;
            tmp = tmp->suivant;
        }

        if (somme < 0.99 || somme > 1.01) {
            printf("⚠️  Sommet %d : somme = %.2f (invalide)\n", i + 1, somme);
            valide = 0;
        }
    }

    if (valide)
        printf("Le graphe est un graphe de Markov valide.\n");
    else
        printf("Le graphe n'est pas un graphe de Markov.\n");
}

// === Exporte la liste d’adjacence au format Mermaid ===
void exporterMermaid(liste_adjacence la, const char* filename) {
    FILE* file = fopen(filename, "wt");
    if (!file) {
        perror("Erreur à l'ouverture du fichier de sortie Mermaid");
        exit(EXIT_FAILURE);
    }

    // En-tête de configuration Mermaid
    fprintf(file, "---\n");
    fprintf(file, "config:\n layout: elk\n theme: neo\n look: neo\n---\n");
    fprintf(file, "flowchart LR\n");

    // Création des nœuds
    for (int i = 0; i < la.taille; i++) {
        fprintf(file, "%s((%d))\n", getId(i + 1), i + 1);
    }

    // Création des arêtes
    for (int i = 0; i < la.taille; i++) {
        cellule* tmp = la.tab[i].head;
        while (tmp) {
            static char id[3];
            strcpy(id, getId(tmp->arrivee));
            fprintf(file, "%s -->|%.2f|%s\n",
                    getId(i + 1), tmp->proba, id);
            tmp = tmp->suivant;
        }
    }

    fclose(file);
    printf("Fichier Mermaid généré : %s\n", filename);
}
