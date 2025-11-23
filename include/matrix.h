#ifndef MATRIX_H
#define MATRIX_H

#include "graph.h"

// Structure représentant une matrice de floats
typedef struct {
    int rows;
    int cols;
    float **data;
} t_matrix;

// Fonctions pour la manipulation des matrices
void printMatrix(t_matrix matrix);

// Création et manipulation des matrices
t_matrix createMatrixFromAdjacency(liste_adjacence la);
t_matrix createEmptyMatrix(int size);
void copyMatrix(t_matrix dest, t_matrix src);
t_matrix multiplyMatrices(t_matrix m1, t_matrix m2, t_matrix result);
float diffMatrices(t_matrix m1, t_matrix m2);
t_matrix subMatrix(t_matrix matrix, t_partition part, int compo_index);
int getPeriod(t_matrix sub_matrix);

/* Distribution / vecteurs */
float *createZeroVector(int n);
void freeVector(float *v);
void copyVector(float *dest, float *src, int n);
void multiplyVectorMatrix(float *vec, t_matrix mat, float *out);
float diffVectors(float *v1, float *v2, int n);
float *computeStationaryDistribution(t_matrix mat, float epsilon, int max_iter);

#endif
