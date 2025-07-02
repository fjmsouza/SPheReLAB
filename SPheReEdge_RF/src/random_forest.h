#ifndef RAMDOM_FOREST_H
#define RAMDOM_FOREST_H


#ifdef __cplusplus
extern "C" {
#endif

void add_vectors(double *v1, double *v2, int size, double *result);
void mul_vector_number(double *v1, double num, int size, double *result);
void score(double * input, double * output);

#ifdef __cplusplus
}
#endif

#endif
