#include "frequency_bin_typedef.h"

#define DISTRIBUTION_SPACING 50.0
#define HARMONICS_ARR_SIZE (size_t) 20

#define EULER M_E

double* get_harmonics(double peak){
    double* harmonics = malloc(sizeof(double) * HARMONICS_ARR_SIZE);
    for(size_t h = 0; h < HARMONICS_ARR_SIZE; h++){
        harmonics[h] = peak * (h + 1);
    }
    return harmonics;
}

double get_correlation(double f, frequency_bin peaks[], size_t peaks_arr_size){
    double c = 0;
    for(size_t s = 0; s < peaks_arr_size; s++){
        c += pow(EULER, -1 * pow((2 * (f - peaks[s][0]) / DISTRIBUTION_SPACING), 2));
    }
    return c;
}

double test_harmonics(frequency_bin peaks[], double harmonics[], size_t peaks_arr_size){
    double correlation = 0;
    for(size_t h = 0; h < HARMONICS_ARR_SIZE; h++){
        correlation += get_correlation(harmonics[h], peaks, peaks_arr_size);
    }
    return (double) correlation / HARMONICS_ARR_SIZE;
}

void note_probabilities(frequency_bin peaks[], size_t peaks_arr_size, size_t float_comparison_error){    
    double probability;
    for(int p = 0; p < peaks_arr_size; p++){
        if(peaks[p][1] > float_comparison_error){
            double* harmonics = get_harmonics(peaks[p][0]);
            probability = test_harmonics(peaks, harmonics, peaks_arr_size);

            free(harmonics);

            peaks[p][2] = probability;
        } else {
            peaks[p][2] = 0.0;
        }
    }
}