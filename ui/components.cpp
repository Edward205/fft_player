#include "components.h"

#include <string>
#include <iostream>

#include "SDL.h"
#include "SDL_ttf.h"

void textRender::render(SDL_Renderer* renderer)
{
    rect.x = x;
    rect.y = y;
    SDL_RenderCopy(renderer, text_texture, NULL, &rect);
};
void textRender::update(TTF_Font* font, SDL_Renderer* renderer)
{
    rect.x = x;
    rect.y = y;
    char* c = const_cast<char*>(text.c_str());

    if (loaded)
    {
        SDL_DestroyTexture(text_texture);
        SDL_FreeSurface(surface);
    }
    surface = TTF_RenderText_Blended(font, c, color);
    loaded = true;
    text_texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_QueryTexture(text_texture, NULL, NULL, &rect.w, &rect.h);
    texW = rect.w;
    texH = rect.h;
}
void textRender::handle_input(SDL_Event e)
{
    int relative_x = texW / 2 + x, relative_y = texH / 2 + y;

    switch (e.key.keysym.sym)
    {
        // neimplementat
        // pentru taste apasate pe buton
    }
    switch (e.type)
    {
    case SDL_MOUSEBUTTONUP:
        if(e.button.x > relative_x - texW / 2 && e.button.x < relative_x + texW / 2 && e.button.y > relative_y - texH / 2 && e.button.y < relative_y + texH / 2)
            break;
    }
}
void textRender::dealocate()
{
    if (!loaded)
    {
        SDL_FreeSurface(surface);
        SDL_DestroyTexture(text_texture);
    }
    loaded = false;
}


void flashingText::load(TTF_Font* font, SDL_Renderer* renderer)
{
    displayed = true;
    cnt_interval = 0;
    displaytext.x = x;
    displaytext.y = y;
    displaytext.color = color;
    displaytext.text = text;
    displaytext.update(font, renderer);
}
void flashingText::render(SDL_Renderer* renderer)
{
    if(visible)
    {
        if(cnt_interval == interval)
        {
            displayed = !displayed;
            cnt_interval = 0;
        }
        else
            ++cnt_interval;
        if(displayed)
            displaytext.render(renderer);
    }
}

void settingText::load(SDL_Renderer* renderer)
{
    label_text.x = x;
	label_text.y = y;
	label_text.text = label;
	SDL_Color white;
	white.r = 255;
	white.g = 255;
	white.b = 255;
	white.a = 255;
	label_text.color = white;
	label_text.update(font, renderer);

    value_text.x = x + label_text.texW + 5;
	value_text.y = y;
	value_text.color = white;
	value_text.text = std::to_string(current_value);
	value_text.update(font, renderer);

}
void settingText::render(SDL_Renderer* renderer)
{
    label_text.render(renderer);
    value_text.render(renderer);
    if(queueValueTextUpdate)
    {
        value_text.text = std::to_string(current_value);
        value_text.update(font, renderer);
        queueValueTextUpdate = false;
    }

}
void settingText::handle_input(SDL_Event e)
{
    switch (e.type) {
    case SDL_MOUSEMOTION:
        if(e.button.x > label_text.x && e.button.x < label_text.x + label_text.texW &&
           e.button.y > label_text.y && e.button.y < label_text.y + label_text.texH)
            hovered = true;
        else
            hovered = false;

    case SDL_KEYDOWN:
        if(hovered)
        {
            if(e.key.keysym.scancode == SDL_SCANCODE_RIGHT)
            {
                current_value += step * 2;
                queueValueTextUpdate = true;
            }
            else
            if(e.key.keysym.scancode == SDL_SCANCODE_LEFT)
            {
                current_value -= step * 2;
                queueValueTextUpdate = true;
            }

            if(e.key.keysym.scancode == SDL_SCANCODE_UP)
            {
                current_value += step;
                queueValueTextUpdate = true;
            }
            else
            if(e.key.keysym.scancode == SDL_SCANCODE_DOWN)
            {
                current_value -= step;
                queueValueTextUpdate = true;
            }
        }
        break;

    case SDL_MOUSEWHEEL:
        if(hovered)
        {
            if(e.wheel.y > 0) // scroll up
            {
                 current_value += step;
                 queueValueTextUpdate = true;
            }
            else if(e.wheel.y < 0) // scroll down
            {
                 current_value -= step;
                 queueValueTextUpdate = true;
            }

        }
        break;
    }
}
