#include <stdio.h>


#define DEBUG

#include "kernels.h"

#include <gsl/gsl_sf_bessel.h>
#include <gsl/gsl_complex.h>
#include <gsl/gsl_vector.h>

int main(void) {
    printf("hii\n%f, %f\n", creal(a), cimag(a));
    printf("%f\n", gsl_sf_bessel_I0(1.0f));
    gsl_vector* v = gsl_vector_alloc(10);
    gsl_vector_set_all(v, 1.0);
    
}