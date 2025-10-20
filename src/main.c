#include "graph.h"

int main() {
    // Charger le graphe depuis le fichier exemple
    liste_adjacence la = readGraph("data/exemple1.txt");

    printf("\n=== Affichage du graphe (liste d'adjacence) ===\n");
    afficherListeAdjacence(la);

    printf("\n=== Vérification du graphe ===\n");
    verifierMarkov(la);

    printf("\n=== Exportation vers Mermaid ===\n");
    exporterMermaid(la, "data/graphe_mermaid.mmd");

    printf("\n➡️ Copiez le contenu de 'data/graphe_mermaid.mmd' sur https://www.mermaidchart.com/\n");

    return 0;
}
