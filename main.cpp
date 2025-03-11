#include <iostream>
#include <fstream>
#include <deque>
#include <random>
#include <unordered_map>

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480
#define SDL_MAIN_HANDLED
#include "SDL.h"
#include "SDL_ttf.h"

#include "common.h"
#include "audio/audio.h"
#include "ui/ui.h"
#include "ui/components.h"


struct RGB {
    int r;
    int g;
    int b;
};
RGB getColorFromNumber(int number) {
    // Define an array of colors
    RGB colors[] = {
        {255, 0, 0},  // red
        {255, 165, 0},  // orange
        {255, 255, 0},  // yellow
        {0, 128, 0},  // green
        {0, 0, 255},  // blue
        {75, 0, 130},  // indigo
        {238, 130, 238},  // violet
        {128, 0, 128},  // purple
        {255, 0, 255},  // magenta
        {0, 0, 0}  // black
    };
    int numColors = 10;  // number of colors

    // Return the color at the index corresponding to the input number modulo the number of colors
    return colors[number % numColors];
}


//std::ofstream l("log.out");

SDL_Window* window = NULL;
SDL_Surface* screenSurface = NULL;
SDL_Renderer* renderer;
TTF_Font* defaultfont;
int cursor_x, cursor_y;

flashingText open_flashing_text;
textRender cursor_info_text;
textRender fft_info_text;

bool view_fft_or_chunks = false;

bool ui_init()
{
    if( SDL_Init( SDL_INIT_VIDEO && SDL_INIT_AUDIO ) < 0 )
    {
        printf( "Nu s-a putut initializa SDL: %s\n", SDL_GetError() );
        return 1;
    }
    if (SDL_Init(SDL_INIT_AUDIO) < 0)
    {
        printf( "Nu s-a putut initializa SDL: %s\n", SDL_GetError() );
        return 1;
    }
    window = SDL_CreateWindow( "FFT Player", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN );
    if( window == NULL )
    {
        printf( "Fereastra SDL nu a putut fi creata: %s\n", SDL_GetError() );
        return 1;
    }
    screenSurface = SDL_GetWindowSurface( window );
    if (TTF_Init() == -1)
    {
        printf("Nu s-a putut initializa SDL_ttf: %s\n", TTF_GetError());
        return 1;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer)
    {
        printf("Nu s-a putut crea renderer: %s\n", SDL_GetError());
        return 1;
    }

    defaultfont = TTF_OpenFont("./TitilliumWeb.ttf", 18);
    open_flashing_text.x = 150;
    open_flashing_text.y = 0;
    open_flashing_text.interval = 800;
    open_flashing_text.color = {51, 181, 229};
    open_flashing_text.text = "Niciun fisier ales. Trageti un fisier WAV pe EXE pentru a deschide.";
    open_flashing_text.font = defaultfont;
    open_flashing_text.load(defaultfont, renderer);
    open_flashing_text.visible = false;

    amp_set.x = 10;
    amp_set.y = SCREEN_HEIGHT - 25 - 9;
    amp_set.current_value = 1;
    amp_set.label = "Amp: ";
    amp_set.font = defaultfont;
    amp_set.load(renderer);

    play_speed_set.x = 90;
    play_speed_set.y = SCREEN_HEIGHT - 25 - 9;
    play_speed_set.current_value = 1;
    play_speed_set.label = "Viteza: ";
    play_speed_set.font = defaultfont;
    play_speed_set.load(renderer);

    chunk_play_set.x = 170;
    chunk_play_set.y = SCREEN_HEIGHT - 25 - 9;
    chunk_play_set.current_value = -1;
    chunk_play_set.label = "Bucata redata: ";
    chunk_play_set.font = defaultfont;
    chunk_play_set.load(renderer);

    separation_thresh_set.x = 320;
    separation_thresh_set.y = SCREEN_HEIGHT - 25 - 9;
    separation_thresh_set.current_value = 150;
    separation_thresh_set.label = "Prag sep: ";
    separation_thresh_set.font = defaultfont;
    separation_thresh_set.load(renderer);

    cursor_info_text.x = 0;
    cursor_info_text.y = 0;
    cursor_info_text.color = {255, 255, 255};
    cursor_info_text.update(defaultfont, renderer);

    fft_info_text.x = 0;
    fft_info_text.y = 0;
    fft_info_text.color = {255, 255, 255};
    fft_info_text.update(defaultfont, renderer);
    return 0;
}
// Offseturi pentru vizualizare
int fft_offset = 0, fft_width = 1;
bool ui_loop()
{
    SDL_Event e;
	while (SDL_PollEvent(&e) != 0) {
        amp_set.handle_input(e);
        play_speed_set.handle_input(e);
        chunk_play_set.handle_input(e);
        separation_thresh_set.handle_input(e);
        switch (e.type)
            {
            case SDL_KEYDOWN:
                if(e.key.keysym.scancode == SDL_SCANCODE_S)
                    fft_offset += 10;
                if(e.key.keysym.scancode == SDL_SCANCODE_W)
                    fft_offset -= 10;
                if(e.key.keysym.scancode == SDL_SCANCODE_A)
                    fft_width -= 1;
                if(e.key.keysym.scancode == SDL_SCANCODE_D)
                    fft_width += 1;
                if(e.key.keysym.scancode == SDL_SCANCODE_F)
                    view_fft_or_chunks = !view_fft_or_chunks;
                if(e.key.keysym.scancode == SDL_SCANCODE_SPACE)
                    pause = !pause;
            case SDL_MOUSEMOTION:
                cursor_x = e.button.x;
                cursor_y = e.button.y;


            }
        if (e.type == SDL_QUIT)
                return false;
	}

    //SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    //SDL_RenderClear(renderer);

    //open_flashing_text.render(renderer);



    // Vizualizare
    //std::cout << (NUM_SAMPLES * COMBINED_BUFFER_SIZE) / SCREEN_HEIGHT << " bins per pixel" << std::endl;




    // RANDARE STANDARD
    /*if(audio::frequency_bins_deque.size() > 1)
    {
        int i=audio::frequency_bins_deque.size()-1, x;
        if(audio::frequency_bins_deque.size() / SCREEN_WIDTH > 0)
            x = audio::frequency_bins_deque.size() - (SCREEN_WIDTH * (audio::frequency_bins_deque.size() / SCREEN_WIDTH));
        else
            x = audio::frequency_bins_deque.size()-1;
        if(SCREEN_WIDTH * fft_width + fft_offset < NUM_SAMPLES * COMBINED_BUFFER_SIZE)
        {
            for(int j = 0; j < SCREEN_WIDTH * fft_width; j += fft_width)
            {
                if(audio::frequency_bins_deque[i].first[j] < 255)
                    SDL_SetRenderDrawColor(renderer, (Uint8)audio::frequency_bins_deque[i].first[j], (Uint8)audio::frequency_bins_deque[i].first[j], (Uint8)audio::frequency_bins_deque[i].first[j], 255);
                else
                    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
                SDL_RenderDrawLine(renderer, x, j / fft_width + fft_offset, x, j / fft_width + fft_offset);
                if(cursor_y - (j / fft_width + fft_offset) == 0)
                {
                    cursor_info_text.text = "Cursor: " + std::to_string(j * (44100 / (NUM_SAMPLES * COMBINED_BUFFER_SIZE))) + "Hz " + std::to_string(audio::frequency_bins_deque[i].first[j]);
                    cursor_info_text.update(defaultfont, renderer);
                }
            }
        }
    }*/

    // RANDARE GRUPARE FRECVENTE
    if(audio::frequency_bins_deque.size() > 1 && !pause)
    {
        int i=audio::frequency_bins_deque.size()-1, x;
        if(audio::frequency_bins_deque.size() / SCREEN_WIDTH > 0)
            x = audio::frequency_bins_deque.size() - (SCREEN_WIDTH * (audio::frequency_bins_deque.size() / SCREEN_WIDTH));
        else
            x = audio::frequency_bins_deque.size()-1;
        if(SCREEN_WIDTH * fft_width + fft_offset < NUM_SAMPLES * COMBINED_BUFFER_SIZE)
        {
            for(int j = 0; j < SCREEN_WIDTH * fft_width; j += fft_width)
            {
                if(view_fft_or_chunks)
                {
                    //std::cout << "y = " << j / fft_width + fft_offset << " index = " << audio::frequency_bins_deque[i].second << std::endl;
                    if(audio::frequency_bins_deque[i].second[j] == audio::play_chunk_index || audio::play_chunk_index == -1)
                    {
                        RGB color = getColorFromNumber(audio::frequency_bins_deque[i].second[j]);
                        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 127);
                        SDL_RenderDrawPoint(renderer, x, j / fft_width + fft_offset);
                    }
                }
                else
                {
                    if(audio::frequency_bins_deque[i].first[j] < 255)
                        SDL_SetRenderDrawColor(renderer, (Uint8)audio::frequency_bins_deque[i].first[j], (Uint8)audio::frequency_bins_deque[i].first[j], (Uint8)audio::frequency_bins_deque[i].first[j], 255);
                    else
                        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 127);
                    SDL_RenderDrawLine(renderer, x, j / fft_width + fft_offset, x, j / fft_width + fft_offset);
                }


                if(cursor_y - (j / fft_width + fft_offset) == 0)
                {
                    cursor_info_text.text = "Cursor: " + std::to_string(j * (44100 / (NUM_SAMPLES * COMBINED_BUFFER_SIZE))) + "Hz " + std::to_string(audio::frequency_bins_deque[i].first[j]);
                    cursor_info_text.update(defaultfont, renderer);
                }
            }
        }
    }


    // Interfata

    // Bare de referinta
    SDL_SetRenderDrawColor(renderer, 25, 50, 25, 255);
    SDL_RenderDrawLine(renderer, 0, fft_offset, SCREEN_WIDTH, fft_offset);
    SDL_RenderDrawLine(renderer,
                       0, SCREEN_WIDTH * fft_width + fft_offset,
                       SCREEN_WIDTH, SCREEN_WIDTH * fft_width + fft_offset);

    // Afisare offset si scara
    fft_info_text.x = cursor_info_text.texW + 10;
    fft_info_text.text = "Offset: " + std::to_string(fft_offset) + " Scara: " + std::to_string(fft_width);
    fft_info_text.update(defaultfont, renderer);


    /// Oare este ineficient sa declar si sa folosesc variabilele de dimensiuni astfel?
    SDL_Rect bottom_bar;
    bottom_bar.x = 0;
    bottom_bar.y = SCREEN_HEIGHT - 35;
    bottom_bar.w = SCREEN_WIDTH;
    bottom_bar.h = 50;
    SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);
    SDL_RenderFillRect(renderer, &bottom_bar);

    SDL_Rect top_bar;
    top_bar.x = 0;
    top_bar.y = 0;
    top_bar.w = SCREEN_WIDTH;
    top_bar.h = 28;
    SDL_RenderFillRect(renderer, &top_bar);

    amp_set.render(renderer);
    play_speed_set.render(renderer);
    separation_thresh_set.render(renderer);
    cursor_info_text.render(renderer);
    fft_info_text.render(renderer);
    chunk_play_set.render(renderer);
    audio::play_chunk_index = chunk_play_set.current_value;

    if(open_flashing_text.text != "")
        open_flashing_text.render(renderer);

    SDL_RenderPresent(renderer);
    return true;
}

int main(int argc, char* argv[])
{
    ui_init();
    if(argc > 1)
    {
        audio::init_read_audio(argv[1]);
    }
    else
    {
        pause = true;
        open_flashing_text.visible = true;
    }
    while(ui_loop())
    {
        //SDL_Delay(10);
        if(!pause)
            if(audio::play_audio_loop_test() > 0)
                break;
    }

    return 0;
}
