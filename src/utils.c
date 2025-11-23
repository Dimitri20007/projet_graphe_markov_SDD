#include "graph.h"

/* -------------------------
    Partie 1 : utilitaires
    -------------------------*/

/* getId
 * Retourne un identifiant court pour un sommet (A, B, ..., Z, AA, AB, ...).
 * Utilisé pour l'export Mermaid afin d'avoir des noms lisibles.
 * Entrée : num = numéro du sommet en base 1.
 * Retour : pointeur statique vers la chaîne (ne pas free()). */
char* getId(int num) {
    static char id[8];
    id[0] = '\0';
    if (num <= 0) {
        strcpy(id, "?");
        return id;
    }
    num--; // 0-based
    if (num < 26) {
        id[0] = 'A' + num;
        id[1] = '\0';
    } else {
        int first = (num / 26) - 1;
        int second = num % 26;
        id[0] = 'A' + first;
        id[1] = 'A' + second;
        id[2] = '\0';
    }
    return id;
}

/* verifierMarkov
 * Vérifie que pour chaque sommet la somme des probabilités sortantes est
 * proche de 1 (test de conformité d'une matrice de transition de Markov).
 * Affiche un message pour chaque sommet qui n'est pas conforme.
 *
 * Ceci permet de détecter des erreurs de lecture ou de normalisation.
 */
void verifierMarkov(liste_adjacence la) {
    int valide = 1;
    for (int i = 0; i < la.taille; i++) {
        float somme = 0.0f;
        cellule* tmp = la.tab[i].head;
        while (tmp) {
            somme += tmp->proba;
            tmp = tmp->suivant;
        }
        if (somme < 0.99f || somme > 1.01f) {
            printf("⚠️  Sommet %d : somme = %.2f (invalide)\n", i + 1, somme);
            valide = 0;
        }
    }
    if (valide)
        printf("Le graphe est un graphe de Markov valide.\n");
    else
        printf("Le graphe n'est pas un graphe de Markov.\n");
}

/* exporterMermaid
 * Exporte la liste d'adjacence dans un fichier au format Mermaid (flowchart).
 * Le fichier peut ensuite être collé dans un rendu Mermaid pour visualiser
 * le graphe pondéré.
 */
void exporterMermaid(liste_adjacence la, const char* filename) {
    FILE* file = fopen(filename, "wt");
    if (!file) {
        perror("Erreur à l'ouverture du fichier de sortie Mermaid");
        exit(EXIT_FAILURE);
    }
    fprintf(file, "---\n");
    fprintf(file, "config:\n layout: elk\n theme: neo\n look: neo\n---\n");
    fprintf(file, "flowchart LR\n");
    for (int i = 0; i < la.taille; i++) {
        fprintf(file, "%s((%d))\n", getId(i + 1), i + 1);
    }
    for (int i = 0; i < la.taille; i++) {
        cellule* tmp = la.tab[i].head;
        while (tmp) {
            char idto[8];
            strcpy(idto, getId(tmp->arrivee));
            fprintf(file, "%s -->|%.2f|%s\n", getId(i + 1), tmp->proba, idto);
            tmp = tmp->suivant;
        }
    }
    fclose(file);
    printf("Fichier Mermaid généré : %s\n", filename);
}

/* -------------------------
    Partie 2 : Tarjan (composantes fortement connexes)
    -------------------------*/

/* init_tarjan_vertices
 * Initialise un tableau de structures utilisées par l'algorithme de Tarjan.
 * Chaque sommet reçoit : id (1..n), index=-1 (non visité), lowlink=-1 et onstack=0.
 */
t_tarjan_vertex* init_tarjan_vertices(int n) {
    t_tarjan_vertex *arr = (t_tarjan_vertex*)malloc(n * sizeof(t_tarjan_vertex));
    if (!arr) {
        perror("Allocation tarjan vertices");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < n; i++) {
        arr[i].id = i + 1;   // sommet 1..n
        arr[i].index = -1;
        arr[i].lowlink = -1;
        arr[i].onstack = 0;
    }
    return arr;
}

/* init_stack
 * Initialise une pile dynamique d'entiers utilisée par Tarjan.
 */
void init_stack(int_stack *s) {
    s->cap = 16;
    s->data = (int*)malloc(s->cap * sizeof(int));
    s->top = 0;
}

/* push_stack
 * Empile la valeur `v` sur la pile `s`. Si nécessaire agrandit le buffer.
 */
void push_stack(int_stack *s, int v) {
    if (s->top >= s->cap) {
        s->cap *= 2;
        s->data = (int*)realloc(s->data, s->cap * sizeof(int));
    }
    s->data[s->top++] = v;
}

/* pop_stack
 * Dépile et retourne la valeur au sommet, ou -1 si vide.
 */
int pop_stack(int_stack *s) {
    if (s->top == 0) return -1;
    return s->data[--s->top];
}

/* stack_empty
 * Retourne 1 si la pile est vide, 0 sinon.
 */
int stack_empty(int_stack *s) {
    return s->top == 0;
}

/* free_stack
 * Libère la mémoire associée à la pile.
 */
void free_stack(int_stack *s) {
    if (s->data) free(s->data);
    s->data = NULL;
    s->cap = s->top = 0;
}

/* init_partition
 * Initialise une structure de partition (tableau de classes) vide.
 */
void init_partition(t_partition *p) {
    p->cap = 8;
    p->nb = 0;
    p->classes = (t_classe*)malloc(p->cap * sizeof(t_classe));
    if (!p->classes) { perror("alloc partition"); exit(EXIT_FAILURE); }
}

/* init_classe
 * Initialise une classe (t_classe) avec un nom et allocation initiale
 * du tableau des membres. La classe est prête à recevoir des sommets.
 */
void init_classe(t_classe *c, const char *name) {
    strncpy(c->name, name, sizeof(c->name)-1);
    c->name[sizeof(c->name)-1] = '\0';
    c->capacity = 8;
    c->size = 0;
    c->members = (int*)malloc(c->capacity * sizeof(int));
    if (!c->members) { perror("alloc class members"); exit(EXIT_FAILURE); }
}

/* add_member_to_classe
 * Ajoute le sommet `vertex` (1-based) à la classe `c`, en réallouant
 * le tableau si nécessaire.
 */
void add_member_to_classe(t_classe *c, int vertex) {
    if (c->size >= c->capacity) {
        c->capacity *= 2;
        c->members = (int*)realloc(c->members, c->capacity * sizeof(int));
    }
    c->members[c->size++] = vertex;
}

/* add_class
 * Ajoute une classe construite `c` à la partition `p`.
 */
void add_class(t_partition *p, t_classe c) {
    if (p->nb >= p->cap) {
        p->cap *= 2;
        p->classes = (t_classe*)realloc(p->classes, p->cap * sizeof(t_classe));
    }
    p->classes[p->nb++] = c;
}

/* free_partition
 * Libère toutes les ressources associées à la partition (membres et tableau
 * de classes).
 */
void free_partition(t_partition *p) {
    if (!p) return;
    for (int i = 0; i < p->nb; i++) {
        if (p->classes[i].members) free(p->classes[i].members);
    }
    if (p->classes) free(p->classes);
    p->classes = NULL;
    p->nb = p->cap = 0;
}

/* strongconnect (Tarjan)
 * Implémentation récursive de la sous-routine 'strongconnect' de l'algorithme
 * de Tarjan pour trouver les composantes fortement connexes (SCC).
 *
 * Paramètres :
 *  - v_idx : indice du sommet courant dans les tableaux (0-based)
 *  - V : tableau de t_tarjan_vertex (index, lowlink, onstack)
 *  - index_ptr : pointeur sur le compteur d'index temporel
 *  - S : pile d'entiers contenant les sommets actuellement sur la pile
 *  - partition : structure où seront ajoutées les classes trouvées
 *  - la : liste d'adjacence du graphe
 *
 * Principe :
 * 1) Assigner à v un index et un lowlink = index, puis incrémenter index.
 * 2) Empiler v sur S et marquer onstack.
 * 3) Pour chaque voisin w de v :
 *    - si w n'a pas été visité (index == -1), appeler strongconnect(w) puis
 *      mettre à jour lowlink(v) = min(lowlink(v), lowlink(w)).
 *    - sinon si w est sur la pile, mettre à jour lowlink(v) = min(lowlink(v), index(w)).
 * 4) Si lowlink(v) == index(v), alors v est la racine d'une SCC :
 *    dépiler les sommets jusqu'à v pour former la classe.
 */
static void strongconnect(int v_idx, t_tarjan_vertex *V, int *index_ptr, int_stack *S, t_partition *partition, liste_adjacence *la) {
    V[v_idx].index = (*index_ptr);
    V[v_idx].lowlink = (*index_ptr);
    (*index_ptr)++;
    /* Empiler le sommet courant (id 1-based) */
    push_stack(S, V[v_idx].id);
    V[v_idx].onstack = 1;

    /* Parcours des voisins du sommet v */
    cellule *cur = la->tab[v_idx].head;
    while (cur) {
        int w = cur->arrivee; // numéro de sommet 1-based
        int w_idx = w - 1;    // indice 0-based pour les tableaux
        if (V[w_idx].index == -1) {
            /* w non visité : exploration récursive */
            strongconnect(w_idx, V, index_ptr, S, partition, la);
            /* Après retour, propager la valeur lowlink si nécessaire */
            if (V[w_idx].lowlink < V[v_idx].lowlink)
                V[v_idx].lowlink = V[w_idx].lowlink;
        } else if (V[w_idx].onstack) {
            /* w est sur la pile => arête arrière vers une racine de SCC en cours */
            if (V[w_idx].index < V[v_idx].lowlink)
                V[v_idx].lowlink = V[w_idx].index;
        }
        cur = cur->suivant;
    }

    /* Si v est une racine (lowlink == index), on dépile pour construire la SCC */
    if (V[v_idx].lowlink == V[v_idx].index) {
        t_classe c;
        char tmpname[16];
        snprintf(tmpname, sizeof(tmpname), "C%d", partition->nb + 1);
        init_classe(&c, tmpname);
        while (1) {
            int w = pop_stack(S);
            if (w == -1) break; /* sécurité */
            int w_idx2 = w - 1;
            V[w_idx2].onstack = 0;
            add_member_to_classe(&c, w);
            if (w == V[v_idx].id) break; /* jusqu'à atteindre v */
        }
        add_class(partition, c);
    }
}

t_partition tarjan(liste_adjacence la) {
    int n = la.taille;
    t_partition partition;
    init_partition(&partition);
    t_tarjan_vertex *V = init_tarjan_vertices(n);
    int index = 0;
    int_stack S;
    init_stack(&S);
    for (int v = 0; v < n; v++) {
        if (V[v].index == -1) {
            strongconnect(v, V, &index, &S, &partition, &la);
        }
    }
    free_stack(&S);
    free(V);
    return partition;
}

/* -------------------------
   Hasse diagram (links between classes)
   -------------------------*/
/* init_link_array
 * Initialise une structure `t_link_array` qui contient un tableau dynamique
 * de liens entre classes (utilisé pour représenter le diagramme de Hasse).
 * La capacité initiale est fixée à 8 et la mémoire est allouée.
 */
void init_link_array(t_link_array *la) {
    la->cap = 8;
    la->size = 0;
    la->links = (t_link*)malloc(la->cap * sizeof(t_link));
    if (!la->links) { perror("alloc links"); exit(EXIT_FAILURE); }
}

/* link_exists
 * Retourne 1 si un lien (from -> to) existe déjà dans `la`, 0 sinon.
 * Permet d'éviter l'ajout de doublons dans le tableau de liens.
 */
int link_exists(t_link_array *la, int from, int to) {
    for (int i = 0; i < la->size; i++) {
        if (la->links[i].from == from && la->links[i].to == to) return 1;
    }
    return 0;
}

/* add_link_if_not_exists
 * Ajoute le lien dirigé `from -> to` dans `la` si ce lien n'existe pas
 * déjà et n'est pas un auto-lien (from == to). Le tableau est redimensionné
 * automatiquement lorsque la capacité est atteinte.
 */
void add_link_if_not_exists(t_link_array *la, int from, int to) {
    if (from == to) return;
    if (link_exists(la, from, to)) return;
    if (la->size >= la->cap) {
        la->cap *= 2;
        la->links = (t_link*)realloc(la->links, la->cap * sizeof(t_link));
    }
    la->links[la->size].from = from;
    la->links[la->size].to = to;
    la->size++;
}

/* build_vertex_to_class_map
 * Construit et retourne un tableau `map` de taille n+1 (indexation 1-based)
 * où map[v] contient l'indice de la classe (1-based) à laquelle appartient
 * le sommet `v`. Les entrées non affectées sont initialisées à -1.
 *
 * Utilité : permet de convertir rapidement un sommet en sa classe pour créer
 * les liens entre classes (diagramme de Hasse) à partir de la liste d'adjacence.
 */
int* build_vertex_to_class_map(t_partition *p, int n) {
    int *map = (int*)malloc((n+1) * sizeof(int));
    if (!map) { perror("alloc map"); exit(EXIT_FAILURE); }
    for (int i = 1; i <= n; i++) map[i] = -1;
    for (int ci = 0; ci < p->nb; ci++) {
        for (int j = 0; j < p->classes[ci].size; j++) {
            int v = p->classes[ci].members[j];
            map[v] = ci + 1;
        }
    }
    return map;
}

/* create_links_from_partition
 * À partir d'une liste d'adjacence `la` et d'une partition `p` (avec la table
 * `vertex_to_class`), construit un t_link_array contenant les liens entre
 * classes : pour chaque arête u->v reliant deux sommets de classes différentes
 * on ajoute un lien Ci -> Cj (où Ci et Cj sont les indices de classes 1-based).
 *
 * Le résultat est un ensemble de liens sans doublons (grâce à add_link_if_not_exists).
 */
t_link_array create_links_from_partition(liste_adjacence la, t_partition *p, int *vertex_to_class) {
    t_link_array arr;
    init_link_array(&arr);
    int n = la.taille;
    for (int i = 0; i < n; i++) {
        int Ci = vertex_to_class[i+1];
        cellule *cur = la.tab[i].head;
        while (cur) {
            int j = cur->arrivee;
            int Cj = vertex_to_class[j];
            if (Ci != Cj) {
                add_link_if_not_exists(&arr, Ci, Cj);
            }
            cur = cur->suivant;
        }
    }
    return arr;
}

/* free_link_array
 * Libère la mémoire du tableau de liens et remet la structure dans un état
 * vide (sécurise les valeurs internes).
 */
void free_link_array(t_link_array *la) {
    if (!la) return;
    if (la->links) free(la->links);
    la->links = NULL;
    la->size = la->cap = 0;
}

/* exporterMermaidHasse
 * Exporte le diagramme de Hasse (classes + liens) au format Mermaid.
 * Pour chaque classe, on construit une étiquette contenant le nom de la
 * classe et la liste de ses membres, puis on écrit les arcs Cx --> Cy pour
 * chaque lien présent dans `links`.
 *
 * Remarque : cette fonction se contente d'écrire les liens fournis ; si vous
 * voulez le vrai diagramme de Hasse réduit (sans arcs transitifs), assurez
 * vous d'avoir préalablement appelé `supprimer_arcs_transitifs` sur `links`.
 */
void exporterMermaidHasse(t_partition *p, t_link_array *links, const char *filename) {
    FILE *f = fopen(filename, "wt");
    if (!f) { perror("open hasse file"); exit(EXIT_FAILURE); }
    fprintf(f, "---\n");
    fprintf(f, "config:\n layout: elk\n theme: neo\n look: neo\n---\n");
    fprintf(f, "flowchart LR\n");
    for (int i = 0; i < p->nb; i++) {
        char members[200];
        members[0] = '\0';
        for (int j = 0; j < p->classes[i].size; j++) {
            char buf[16];
            if (j == 0) snprintf(buf, sizeof(buf), "%d", p->classes[i].members[j]);
            else snprintf(buf, sizeof(buf), ",%d", p->classes[i].members[j]);
            strncat(members, buf, sizeof(members) - strlen(members) - 1);
        }
        /* Use HTML line break for Mermaid labels and quote the label */
        char label[256];
        snprintf(label, sizeof(label), "%s<br>{%s}", p->classes[i].name, members);
        fprintf(f, "C%d[\"%s\"]\n", i+1, label);
    }
    for (int k = 0; k < links->size; k++) {
        int from = links->links[k].from;
        int to = links->links[k].to;
        fprintf(f, "C%d --> C%d\n", from, to);
    }
    fclose(f);
    printf("Fichier Mermaid Hasse généré : %s\n", filename);
}

/* compute_caracteristics
 * Pour une classe donnée (class_number 1-based), retourne 1 si la classe a
 * au moins une flèche sortante dans `links` (i.e. transitoire), 0 sinon.
 *
 * Méthode : construit un tableau temporaire has_out où has_out[c] = 1 si au
 * moins un lien a pour origine la classe c, puis renvoie la valeur pour
 * `class_number`.
 */
int compute_caracteristics(t_partition *p, t_link_array *links, int class_number){
    int nb = p->nb;
    int *has_out = (int*)calloc(nb+1, sizeof(int));
    for (int i = 0; i < links->size; i++) {
        int from = links->links[i].from;
        has_out[from] = 1;
    }
    int bool = has_out[class_number];
    free(has_out);

    return bool;
}

/* compute_and_print_characteristics
 * Parcourt les classes et affiche pour chacune :
 *  - si elle est transitoire (présence d'arcs sortants vers d'autres classes)
 *  - ou persistante (pas d'arcs sortants)
 *
 * Pour les classes persistantes de taille 1, indique explicitement si l'état
 * est absorbant.
 * En fin de fonction on donne une indication sur l'irréductibilité globale
 * (nb de classes = 1 signifie irréductible).
 */
void compute_and_print_characteristics(t_partition *p, t_link_array *links) {
    printf("\n=== Caractéristiques des classes ===\n");
    for (int i = 0; i < p->nb; i++) {
        int bool = compute_caracteristics(p, links, i+1);
        if (bool) {
            printf("%s : transitoire (flèches sortantes)\n", p->classes[i].name);
        } else {
            printf("%s : persistante (pas de flèche sortante)\n", p->classes[i].name);
            if (p->classes[i].size == 1) {
                printf(" -> Etat %d est absorbant.\n", p->classes[i].members[0]);
            }
        }
    }
    if (p->nb == 1) {
        printf("\nLe graphe est IRRÉDUCTIBLE (une seule classe).\n");
    } else {
        printf("\nLe graphe n'est pas irréductible (nb_classes = %d).\n", p->nb);
    }
}