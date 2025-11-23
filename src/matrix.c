#include "matrix.h"
#include <math.h>

#include <stdlib.h>

/* createMatrixFromAdjacency: construit une matrice de transition depuis la liste d'adjacence */
t_matrix createMatrixFromAdjacency(liste_adjacence la) {
    t_matrix matrix = createEmptyMatrix(la.taille);

    for (int i = 0; i < la.taille; i++) {
        cellule* tmp = la.tab[i].head;
        while (tmp) {
            matrix.data[i][tmp->arrivee - 1] = tmp->proba;
            tmp = tmp->suivant;
        }
    }

    return matrix;
}

/* printMatrix: affiche une matrice sur la sortie standard */
void printMatrix(t_matrix matrix) {
    for (int i = 0; i < matrix.rows; i++) {
        for (int j = 0; j < matrix.cols; j++) {
            printf("%.2f ", matrix.data[i][j]);
        }
        printf("\n");
    }
}

/* createEmptyMatrix: alloue et initialise une matrice carrée vide de taille 'size' */
t_matrix createEmptyMatrix(int size) {
    t_matrix matrix;
    matrix.rows = size;
    matrix.cols = size;
    matrix.data = (float**)malloc(size * sizeof(float*));
    for (int i = 0; i < size; i++) {
        matrix.data[i] = (float*)calloc(size, sizeof(float));
    }
    return matrix;
}

/* copyMatrix: copie les éléments de src dans dest (mêmes dimensions) */
void copyMatrix(t_matrix dest, t_matrix src) {
    for (int i = 0; i < src.rows; i++) {
        for (int j = 0; j < src.cols; j++) {
            dest.data[i][j] = src.data[i][j];
        }
    }
}

/* multiplyMatrices: calcule result = m1 * m2 (produit matriciel) */
t_matrix multiplyMatrices(t_matrix m1, t_matrix m2, t_matrix result) {
    for (int i = 0; i < m1.rows; i++) {
        for (int j = 0; j < m2.cols; j++) {
            result.data[i][j] = 0.0f;
            for (int k = 0; k < m1.cols; k++) {
                result.data[i][j] += m1.data[i][k] * m2.data[k][j];
            }
        }
    }
    return result;
}

/* diffMatrices: renvoie la somme des différences absolues entre deux matrices (norme L1) */
float diffMatrices(t_matrix m1, t_matrix m2) {
    float diff = 0.0f;
    for (int i = 0; i < m1.rows; i++) {
        for (int j = 0; j < m1.cols; j++) {
            diff += fabs(m1.data[i][j] - m2.data[i][j]);
        }
    }
    return diff;
}

/* subMatrix: extrait la sous-matrice correspondant à la i-ème classe de la partition */
t_matrix subMatrix(t_matrix matrix, t_partition part, int compo_index) {
    int size = part.classes[compo_index].size;
    t_matrix submatrix = createEmptyMatrix(size);

    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            int row = part.classes[compo_index].members[i] - 1;
            int col = part.classes[compo_index].members[j] - 1;
            submatrix.data[i][j] = matrix.data[row][col];
        }
    }

    return submatrix;
}

// Algorithme pour calculer le PGCD de plusieurs entiers, il utilise l'algorithme d'Euclide de manière itérative

int gcd(int *vals, int nbvals) { 
    if (nbvals == 0) return 0;
    int result = vals[0];
    for (int i = 1; i < nbvals; i++) {
        int a = result;
        int b = vals[i];
        while (b != 0) {
            int temp = b;
            b = a % b;
            a = temp;
        }
        result = a;
    }
    return result;
}

/* gcd
Calcule le plus grand commun diviseur (PGCD) d'un tableau d'entiers `vals`
 */

int getPeriod(t_matrix sub_matrix) {
    int n = sub_matrix.rows;
    int *periods = (int *)malloc(n * sizeof(int));
    int period_count = 0;
    int cpt = 1;
    t_matrix power_matrix = createEmptyMatrix(n);
    t_matrix result_matrix = createEmptyMatrix(n);
    copyMatrix(power_matrix, sub_matrix);

    for (cpt = 1; cpt <= n; cpt++) {
        int diag_nonzero = 0;
        for (int i = 0; i < n; i++) {
            if (power_matrix.data[i][i] > 0.0f) {
                diag_nonzero = 1;
            }
        }
        if (diag_nonzero) {
            periods[period_count] = cpt;
            period_count++;
        }
        multiplyMatrices(power_matrix, sub_matrix, result_matrix);
        copyMatrix(power_matrix, result_matrix);
    }

    int period = gcd(periods, period_count);
    free(periods);
    // libération correcte des matrices allouées par createEmptyMatrix
    for (int i = 0; i < power_matrix.rows; i++) {
        free(power_matrix.data[i]);
    }
    free(power_matrix.data);
    for (int i = 0; i < result_matrix.rows; i++) {
        free(result_matrix.data[i]);
    }
    free(result_matrix.data);
    return period;
}

/* getPeriod
 Détermine la période d'une sous-matrice de transition `sub_matrix`.
 Méthode implémentée :
 1) On calcule successivement les puissances sub_matrix^1, sub_matrix^2, ..., sub_matrix^n
    (stockées dans `power_matrix`).
 2) Pour chaque puissance t on examine la diagonale : si un élément
    diagonal est strictement positif, on enregistre t dans le tableau `periods`.
 3) Après avoir collecté tous les t (jusqu'à n), on retourne le PGCD des t trouvés
    via la fonction `gcd`.
 */

/*
   Vecteurs / distributions
*/

/* createZeroVector: alloue et initialise un vecteur de taille n à zéro */
float *createZeroVector(int n) {
    float *v = (float*)calloc(n, sizeof(float));
    return v;
}

/* freeVector: libère un vecteur alloué */
void freeVector(float *v) {
    if (v) free(v);
}

/* copyVector: copie n éléments de src vers dest */
void copyVector(float *dest, float *src, int n) {
    for (int i = 0; i < n; i++) {
        dest[i] = src[i];
    }
}

/* multiplyVectorMatrix: calcule out = vec * mat (vecteur ligne) */
void multiplyVectorMatrix(float *vec, t_matrix mat, float *out) {
    int n = mat.cols; // suppose matrice carrée
    for (int j = 0; j < n; j++) {
        out[j] = 0.0f;
    }
    for (int i = 0; i < mat.rows; i++) {
        float vi = vec[i];
        if (vi == 0.0f) continue;
        for (int j = 0; j < mat.cols; j++) {
            out[j] += vi * mat.data[i][j];
        }
    }
}

/* diffVectors: somme des différences absolues entre deux vecteurs (norme L1) */
float diffVectors(float *v1, float *v2, int n) {
    float d = 0.0f;
    for (int i = 0; i < n; i++) {
        d += fabsf(v1[i] - v2[i]);
    }
    return d;
}

/* computeStationaryDistribution: méthode itérative pour obtenir la distribution stationnaire */
float *computeStationaryDistribution(t_matrix mat, float epsilon, int max_iter) {
    int n = mat.rows;
    float *p = createZeroVector(n);
    float *tmp = createZeroVector(n);
    /* démarrage avec une distribution uniforme */
    for (int i = 0; i < n; i++) {
        p[i] = 1.0f / n;
    }

    int iter = 0;
    while (iter < max_iter) {
        multiplyVectorMatrix(p, mat, tmp);
        float d = diffVectors(p, tmp, n);
        copyVector(p, tmp, n);
        if (d < epsilon) break;
        iter++;
    }
    freeVector(tmp);
    return p; /* le code appelant doit libérer */
}
