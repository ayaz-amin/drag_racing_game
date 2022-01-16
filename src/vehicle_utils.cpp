#define FRAME_SPEED 15

#include "vehicle_utils.h"

void render_vehicle(vehicle_t* vehicle)
{
    if(vehicle->m_velocity == 0.0f)
    {
        vehicle->m_frame_count = 0;
        vehicle->m_current_frame = 0;
    } else {
        vehicle->m_frame_count++;
        if(vehicle->m_frame_count >= 60/FRAME_SPEED)
        {
            vehicle->m_frame_count = 1;
            vehicle->m_current_frame++;
            if(vehicle->m_current_frame > 4) vehicle->m_current_frame = 1;
        }
    }
    
    vehicle->m_frame_rec.x = (float)vehicle->m_current_frame * (float)vehicle->m_texture.width / 5;

    DrawTextureRec(vehicle->m_texture, vehicle->m_frame_rec, {vehicle->m_position_x, vehicle->m_position_y}, WHITE);
}