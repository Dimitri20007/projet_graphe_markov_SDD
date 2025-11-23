#include "graph.h"

/* -------------------------
    Partie 1 : utilitaires
    -------------------------*/

/* getId: retourne un identifiant lisible pour le sommet (A, B, ..., AA...). */
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

/* verifierMarkov: vérifie la somme des probabilités sortantes par sommet. */
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

/* exporterMermaid: exporte la liste d'adjacence au format Mermaid. */
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

/* init_tarjan_vertices: initialise les structures nécessaires à Tarjan. */
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

/* init_stack: initialise une pile dynamique. */
void init_stack(int_stack *s) {
    s->cap = 16;
    s->data = (int*)malloc(s->cap * sizeof(int));
    s->top = 0;
}

/* push_stack: empile une valeur sur la pile (agrandit si besoin). */
void push_stack(int_stack *s, int v) {
    if (s->top >= s->cap) {
        s->cap *= 2;
        s->data = (int*)realloc(s->data, s->cap * sizeof(int));
    }
    s->data[s->top++] = v;
}

/* pop_stack: dépile et renvoie la valeur (ou -1 si vide). */
int pop_stack(int_stack *s) {
    if (s->top == 0) {
        return -1;
    }
    return s->data[--s->top];
}

/* stack_empty: retourne 1 si la pile est vide, sinon 0. */
int stack_empty(int_stack *s) {
    return s->top == 0;
}

/* free_stack: libère la mémoire de la pile. */
void free_stack(int_stack *s) {
    if (s->data) {
        free(s->data);
    }
    s->data = NULL;
    s->cap = s->top = 0;
}

/* init_partition: initialise une partition vide de classes. */
void init_partition(t_partition *p) {
    p->cap = 8;
    p->nb = 0;
    p->classes = (t_classe*)malloc(p->cap * sizeof(t_classe));
    if (!p->classes) { perror("alloc partition"); exit(EXIT_FAILURE); }
}

/* init_classe: initialise une classe et son tableau de membres. */
void init_classe(t_classe *c, const char *name) {
    strncpy(c->name, name, sizeof(c->name)-1);
    c->name[sizeof(c->name)-1] = '\0';
    c->capacity = 8;
    c->size = 0;
    c->members = (int*)malloc(c->capacity * sizeof(int));
    if (!c->members) { perror("alloc class members"); exit(EXIT_FAILURE); }
}

/* add_member_to_classe: ajoute un sommet à la classe (réalloue si besoin). */
void add_member_to_classe(t_classe *c, int vertex) {
    if (c->size >= c->capacity) {
        c->capacity *= 2;
        c->members = (int*)realloc(c->members, c->capacity * sizeof(int));
    }
    c->members[c->size++] = vertex;
}

/* add_class: ajoute une classe à la partition (réalloue si nécessaire). */
void add_class(t_partition *p, t_classe c) {
    if (p->nb >= p->cap) {
        p->cap *= 2;
        p->classes = (t_classe*)realloc(p->classes, p->cap * sizeof(t_classe));
    }
    p->classes[p->nb++] = c;
}

/* free_partition: libère la mémoire utilisée par la partition. */
void free_partition(t_partition *p) {
    if (!p) return;
    for (int i = 0; i < p->nb; i++) {
        if (p->classes[i].members) free(p->classes[i].members);
    }
    if (p->classes) free(p->classes);
    p->classes = NULL;
    p->nb = p->cap = 0;
}

/* strongconnect: sous-routine récursive de Tarjan.
 *
 * Algorithme : assignation d'index/lowlink, empilement, exploration
 * récursive des voisins, et dépilement pour former une composante lorsque
 * lowlink == index. Utilise une pile pour détecter les sommets en cours.
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
    Diagramme de Hasse (liens entre classes)
    -------------------------*/
/* init_link_array: initialise la structure dynamique de liens entre classes. */
void init_link_array(t_link_array *la) {
    la->cap = 8;
    la->size = 0;
    la->links = (t_link*)malloc(la->cap * sizeof(t_link));
    if (!la->links) { perror("alloc links"); exit(EXIT_FAILURE); }
}

/* link_exists: retourne 1 si le lien from->to existe déjà, sinon 0. */
int link_exists(t_link_array *la, int from, int to) {
    for (int i = 0; i < la->size; i++) {
        if (la->links[i].from == from && la->links[i].to == to) {
            return 1;
        }
    }
    return 0;
}

/* add_link_if_not_exists: ajoute un lien from->to si absent (évite auto-liens). */
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

/* build_vertex_to_class_map: renvoie une table map[v] -> permet de savoir a quel classe appartient un sommet. */
int* build_vertex_to_class_map(t_partition *p, int n) {
    int *map = (int*)malloc((n+1) * sizeof(int));
    if (!map) { perror("alloc map"); exit(EXIT_FAILURE); }
    for (int i = 1; i <= n; i++){
        map[i] = -1;
    } 
    for (int ci = 0; ci < p->nb; ci++) {
        for (int j = 0; j < p->classes[ci].size; j++) {
            int v = p->classes[ci].members[j];
            map[v] = ci + 1;
        }
    }
    return map;
}

/* create_links_from_partition: construit les liens entre classes à partir des arêtes. */
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

/* free_link_array: libère la mémoire du tableau de liens. */
void free_link_array(t_link_array *la) {
    if (!la) return;
    if (la->links) free(la->links);
    la->links = NULL;
    la->size = la->cap = 0;
}

/* exporterMermaidHasse: exporte le diagramme de Hasse au format Mermaid. */
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
        /* Utiliser un saut de ligne HTML pour les labels Mermaid et citer l'étiquette */
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

/* renvoie 1 si une classe a des flèches sortantes, 0 sinon.
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

/* affiche pour chaque classe si elle est transitoire ou persistante et si il y a un état absorbant.
 * Indique aussi si le graphe est irréductible (une seule classe).
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