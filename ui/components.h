#ifndef COMPONENTS_H
#define COMPONENTS_H

#include <string>
//#include <functional>
#include "SDL.h"
#include "SDL_ttf.h"

class textRender {
public:
    int x, y;
    int texW, texH;
    SDL_Color color;
    std::string text;
    //std::function<const int& ()> onPress;
    void render(SDL_Renderer* renderer);
    void update(TTF_Font* font, SDL_Renderer* renderer);
    void handle_input(SDL_Event e);
    void dealocate();
private:
    SDL_Surface* surface = NULL;
    SDL_Texture* text_texture = NULL;
    SDL_Rect rect;
    bool loaded = false;
};

class flashingText {
public:
    int x, y, interval;
    SDL_Color color;
    std::string text;
    TTF_Font* font;
    bool visible;
    void load(TTF_Font* font, SDL_Renderer* renderer);
    void render(SDL_Renderer* renderer);
    void dealocate();
private:
    int cnt_interval;
    textRender displaytext;
    bool displayed;
};

class settingText {
public:
    int x, y;
    int current_value, step = 1;
    std::string label;
    TTF_Font* font;
    void load(SDL_Renderer* renderer);
    void render(SDL_Renderer* renderer);
    void handle_input(SDL_Event e);
    void dealocate();
private:
    textRender label_text;
    textRender value_text;
    bool queueValueTextUpdate;
    bool hovered;
};

#endif
