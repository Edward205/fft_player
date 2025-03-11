#include "audio.h"
#include "../common.h"
int sample_cnt = 0;
namespace audio
{
SF_INFO sfinfo;
SNDFILE* infile;
PaStream* stream;
float* buffer;
std::deque<float*> combined_buffer;
fftw_complex *in, *out;
sf_count_t numSamples;

//detectie bucati
int frequency_chunk_index = 0;
bool in_chunk = true;
float diferenta = 0;
float radacina = 0;

std::deque<std::pair<std::vector<double>, std::vector<int>>> frequency_bins_deque;
int play_chunk_index = -1;

double blackman_harris(int n, int N)
{
    double a0, a1, a2, a3, seg1, seg2, seg3, w_n;
    a0 = 0.35875;
    a1 = 0.48829;
    a2 = 0.14128;
    a3 = 0.01168;

    seg1 = a1 * (double) cos( ((double) 2 * (double) M_PI * (double) n) / ((double) N - (double) 1) );
    seg2 = a2 * (double) cos( ((double) 4 * (double) M_PI * (double) n) / ((double) N - (double) 1) );
    seg3 = a3 * (double) cos( ((double) 6 * (double) M_PI * (double) n) / ((double) N - (double) 1) );

    w_n = a0 - seg1 + seg2 - seg3;
    return w_n;
}

int init_read_audio(char* audio_path)
{
    // Deschidem fisierul
    infile = sf_open(audio_path, SFM_READ, &sfinfo);
    if (!infile)
    {
        std::cerr << "Eroare la deschiderea fisierului: " << sf_strerror(infile) << std::endl;
        return 1;
    }

    // Initializare PortAudio
    PaError err = Pa_Initialize();
    if (err != paNoError)
    {
        std::cerr << "Eroare la initializarea PortAudio: " << Pa_GetErrorText(err) << std::endl;
        sf_close(infile);
        return 1;
    }

    // Initializare stream PortAudio
    err = Pa_OpenDefaultStream(&stream, 0, sfinfo.channels, paFloat32, sfinfo.samplerate, NUM_SAMPLES, NULL, NULL);
    if (err != paNoError)
    {
        std::cerr << "Eroare la initializarea streamului PortAudio: " << Pa_GetErrorText(err) << std::endl;
        sf_close(infile);
        Pa_Terminate();
        return 1;
    }

    // Alocam memorie pentru intrarea si iesirea FFT
    in = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * NUM_SAMPLES * COMBINED_BUFFER_SIZE);
    out = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * NUM_SAMPLES * COMBINED_BUFFER_SIZE);

    // Buffer pentru intrarea audio
    buffer = new float[NUM_SAMPLES * sfinfo.channels];


    // Incepem streamul PortAudio
    err = Pa_StartStream(stream);
    if (err != paNoError)
    {
        std::cerr << "Eroare la pornirea streamului PortAudio: " << Pa_GetErrorText(err) << std::endl;
        delete[] buffer;
        sf_close(infile);
        Pa_Terminate();
        return 1;
    }
}

int play_audio_loop_test()
{
    PaError err;
    bool read_ok = true;
    if(play_speed_set.current_value > 0)
    {
        if(sample_cnt % play_speed_set.current_value == 0)
        {
            read_ok = (numSamples = sf_read_float(infile, buffer, NUM_SAMPLES * sfinfo.channels) > 0);
            sample_cnt = 0;
        }
    }
    else
    {
        for(int i = 0; i < -play_speed_set.current_value; ++i)
            read_ok = (numSamples = sf_read_float(infile, buffer, NUM_SAMPLES * sfinfo.channels) > 0);
    }
    if( read_ok )
    {
        ++sample_cnt;
        combined_buffer.push_back(buffer);
        if(combined_buffer.size() >= COMBINED_BUFFER_SIZE)
        {
            // Hann window
            for(int i = 0; i < COMBINED_BUFFER_SIZE; ++i)
            {
                for (int j = 0; j <= NUM_SAMPLES; ++j)
                {
                    //double hann = (0.5 * (1 - cos(2 * PI * j / NUM_SAMPLES)));
                    // inmultiti cu variabila hann pt a aplica hann window
                    in[j][0] = 1.5 * combined_buffer[i][j];
                    in[j][1] = 0;
                }
            }

            // Efectuare FFT
            fftw_plan plan = fftw_plan_dft_1d(NUM_SAMPLES * COMBINED_BUFFER_SIZE, in, out, FFTW_FORWARD, FFTW_ESTIMATE);
            fftw_execute(plan);
            fftw_destroy_plan(plan);

            // Procesare rezultat (parcurgere frecvente)
            double max_magnitude = 0;
            int max_freq = 0;
            std::pair<std::vector<double>, std::vector<int>> frequnecy_bins;

            // Algoritm procesare (sunt pre obosit sa ii dau un nume)
            // Pasul 1 - detectam cea mai puternica frecventa si crestem vectorii care retin frecventele
            for (int i = 0; i <= NUM_SAMPLES * COMBINED_BUFFER_SIZE; i++)
            {
                double magnitude = sqrt(out[i][0]*out[i][0] + out[i][1]*out[i][1]);
                if(magnitude > max_magnitude)
                {
                    max_magnitude = magnitude;
                    max_freq = i;
                }

                frequnecy_bins.second.push_back(frequency_chunk_index);
                frequnecy_bins.first.push_back(magnitude * amp_set.current_value);

            }
            //std::cout << max_freq * (44100 / (NUM_SAMPLES * COMBINED_BUFFER_SIZE)) << " " << max_magnitude << std::endl;
            // Pasul 2 - efectuam un algoritm de fill pe cea mai puternica frecventa
            int upper_limit = max_freq, lower_limit = max_freq;
            for (int i = max_freq; i <= NUM_SAMPLES * COMBINED_BUFFER_SIZE; ++i)
            {
                double magnitude = sqrt(out[i][0]*out[i][0] + out[i][1]*out[i][1]);
                double dif = std::max(magnitude, max_magnitude) - std::min(magnitude, max_magnitude);
                if(dif > (double)separation_thresh_set.current_value)
                {
                    upper_limit = i;
                    break;
                }
            }
            for (int i = max_freq; i >= 0; --i)
            {
                double magnitude = sqrt(out[i][0]*out[i][0] + out[i][1]*out[i][1]);
                double dif = std::max(magnitude, max_magnitude) - std::min(magnitude, max_magnitude);
                if(dif > (double)separation_thresh_set.current_value)
                {
                    lower_limit = i;
                    break;
                }
            }

            // Pasul 3 - marcam totul intre upper_limit si lower_limit
            for(int i = lower_limit; i <= upper_limit; ++i)
                frequnecy_bins.second[i] = 1;

            // Stergere buffer vechi si adaugare buffer nou
            combined_buffer.pop_front();
            combined_buffer.push_back(buffer);
            frequency_bins_deque.push_back(frequnecy_bins);
            frequency_chunk_index = 0;

            // FFT invers
            fftw_complex* ifft_in = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * NUM_SAMPLES * COMBINED_BUFFER_SIZE);
            fftw_complex* ifft_out = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * NUM_SAMPLES * COMBINED_BUFFER_SIZE);

            // Modificare frecvente
            if(play_chunk_index != -1)
                for (int i = 0; i < NUM_SAMPLES * COMBINED_BUFFER_SIZE; i++)
                {
                    float frequency = i * SAMPLE_RATE / (NUM_SAMPLES * COMBINED_BUFFER_SIZE);  // Calculate the frequency of the i-th bin
                    //std::cout << i << " " << frequnecy_bins.second[i] << " " << play_chunk_index << std::endl;
                    if (frequnecy_bins.second[i] != play_chunk_index)
                    {
                        out[i][0] = 0.0;
                        out[i][1] = 0.0;
                    }
                }

            // Copiem frecventele modificate in vectorul de frecvente de intrare pentru FFT invers
            for (int i = 0; i < NUM_SAMPLES * COMBINED_BUFFER_SIZE; ++i)
            {
                ifft_in[i][0] = out[i][0];
                ifft_in[i][1] = out[i][1];
            }


            // Efectuare FFT invers
            fftw_plan ifft_plan = fftw_plan_dft_1d(NUM_SAMPLES * COMBINED_BUFFER_SIZE, ifft_in, ifft_out, FFTW_BACKWARD, FFTW_ESTIMATE);
            fftw_execute(ifft_plan);
            fftw_destroy_plan(ifft_plan);

            // Normalizare iesire
            for (int i = 0; i < NUM_SAMPLES * COMBINED_BUFFER_SIZE; i++)
            {
                ifft_out[i][0] /= NUM_SAMPLES * COMBINED_BUFFER_SIZE;
                ifft_out[i][1] /= NUM_SAMPLES * COMBINED_BUFFER_SIZE;
            }

            // Copiem iesirea FFT-ului invers intr-un vector de sampleuri
            float* modified_samples = new float[NUM_SAMPLES * COMBINED_BUFFER_SIZE];
            for (int i = 0; i < NUM_SAMPLES * COMBINED_BUFFER_SIZE; i++)
            {
                modified_samples[i] = ifft_out[i][0];
            }
            err = Pa_WriteStream(stream, modified_samples, NUM_SAMPLES);
            delete modified_samples;


        }
        if (err != paNoError)
        {
            std::cerr << "Eroare la scrierea streamului PortAudio: " << Pa_GetErrorText(err) << std::endl;
            //return 1;
        }
        return 0;
    }
    else
    {
        std::cout << "S-a ajuns la capatul fisierului" << std::endl;
        pause = true;
        //return 1;
    }
}
}
