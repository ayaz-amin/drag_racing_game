#include "ui.h"

void init_button(button_t* button, float position_x, float position_y, const char* file_path)
{
    button->m_texture = LoadTexture(file_path);
    button->m_texture_rec = {
        position_x - 0.5 * button->m_texture.width, 
        position_y - 0.25 * button->m_texture.height, 
        button->m_texture.width, button->m_texture.height / 2
    };
    button->m_source_frame = {
        0, 0, button->m_texture.width, button->m_texture.height / 2
    };
    button->m_button_state = 0;
    button->m_is_pressed = false;
}

void update_button(button_t* button, Vector2 mouse_position)
{
    button->m_is_pressed = false;

    if(CheckCollisionPointRec(mouse_position, button->m_texture_rec))
    {
        button->m_button_state = 1;
        if(IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) button->m_is_pressed = true;
    } else {
        button->m_button_state = 0;
    }

    button->m_source_frame.y = button->m_button_state * button->m_texture_rec.height;
}

void button_render(button_t* button)
{
    DrawTextureRec(
        button->m_texture,
        button->m_source_frame,
        {
            button->m_texture_rec.x,
            button->m_texture_rec.y
        },
        WHITE
    );
}

void unload_button(button_t* button)
{
    UnloadTexture(button->m_texture);
}