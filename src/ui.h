#pragma once

#include <raylib.h>

typedef struct 
{
    Texture2D m_texture;
    Rectangle m_texture_rec;
    Rectangle m_source_frame;
    int m_button_state;
    bool m_is_pressed;
} button_t;

void init_button(button_t* button, float position_x, float position_y, const char* file_path);
void update_button(button_t* button, Vector2 mouse_position);
void button_render(button_t* button);
void unload_button(button_t* button);