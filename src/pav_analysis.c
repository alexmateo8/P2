#include <math.h>
#include "pav_analysis.h"

float compute_power(const float *x, unsigned int N) {
    float total = 0;

    for(int i=0; i<N ; i++){
        total = total + x[i] * x[i];
    }

    total = total / N;
    total = 10 * log10(total);
    return total;
}

float compute_am(const float *x, unsigned int N) {
    float total = 0;

    for(int i=0; i<N ; i++){
        total = total + fabs(x[i]);
    }

    total = total / N;

    return total;
}

float compute_zcr(const float *x, unsigned int N, float fm) {
    float ZCR = 0;

    for(int i= 1; i<N ; i++ ){
        if((x[i]>0 && x[i-1]<=0) || (x[i]<0 && x[i-1]>=0)){
            ZCR = ZCR + 1;
        }
    }

    ZCR = ZCR * fm / (2*(N-1));
    return ZCR;
}