#ifndef AUDIO_H
#define AUDIO_H


#include <stdio.h>
#include <fstream>
#include <math.h>
#include <iostream>
#include <deque>
#include <vector>

#include "fftw3.h"
#include "portaudio.h"
#include "sndfile.h"

#define NUM_SAMPLES 1024
#define COMBINED_BUFFER_SIZE 16
#define SAMPLE_RATE 44100
#define PI 3.14159265358979323846

/*extern SF_INFO sfinfo;
extern SNDFILE* infile;
extern PaStream* stream;
extern float* buffer;
extern std::deque<float*> combined_buffer;
extern fftw_complex *in, *out;


double blackman_harris(int n, int N);*/

//extern double frequnecy_bins[NUM_SAMPLES * COMBINED_BUFFER_SIZE];
namespace audio
{
    extern std::deque<std::pair<std::vector<double>, std::vector<int>>> frequency_bins_deque;

    int init_read_audio(char* audio_path);
    int play_audio_loop();
    int dealocate_audio();

    int play_audio_loop_test();

    extern int play_chunk_index;
}
#endif // AUDIO_H
