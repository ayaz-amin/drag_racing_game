#define FRAME_SPEED 15

#include "vehicle_utils.h"
#include "math_utils.h"

void init_engine(engine_t* engine)
{
    engine->m_gear_size = 6;
    engine->m_current_gear = 0;
    int torque_curve[] = {100, 280, 325, 420, 460, 340, 300, 100};
    float gear_ratios[] = {5.8f, 4.5f, 3.74f, 2.8f, 1.6f, 0.79f};
    engine->m_torque_curve = torque_curve;
    engine->m_gear_ratios = gear_ratios;
    engine->m_differential = 4.25f;
}

void update_vehicle(vehicle_t* vehicle)
{
    float rpm = vehicle->m_engine_rpm;
    if(rpm < 1000)
    {
        vehicle->m_engine_torque = lerp(
            vehicle->m_engine.m_torque_curve[0], vehicle->m_engine.m_torque_curve[1], rpm / 1000
        );
    } else if(rpm < 2000) {
        vehicle->m_engine_torque = lerp(
            vehicle->m_engine.m_torque_curve[1], vehicle->m_engine.m_torque_curve[2], (rpm - 1000) / 1000
        );
    } else if(rpm < 3000) {
        vehicle->m_engine_torque = lerp(
            vehicle->m_engine.m_torque_curve[2], vehicle->m_engine.m_torque_curve[3], (rpm - 2000) / 1000
        );
    } else if(rpm < 4000) {
        vehicle->m_engine_torque = lerp(
            vehicle->m_engine.m_torque_curve[3], vehicle->m_engine.m_torque_curve[4], (rpm - 3000) / 1000
        );
    } else if(rpm < 5000) {
        vehicle->m_engine_torque = lerp(
            vehicle->m_engine.m_torque_curve[4], vehicle->m_engine.m_torque_curve[5], (rpm - 4000) / 1000
        );
    } else if(rpm < 6000) {
        vehicle->m_engine_torque = lerp(
            vehicle->m_engine.m_torque_curve[5], vehicle->m_engine.m_torque_curve[6], (rpm - 5000) / 1000
        );
    } else if(rpm < 7000) {
        vehicle->m_engine_torque = lerp(
            vehicle->m_engine.m_torque_curve[6], vehicle->m_engine.m_torque_curve[7], (rpm - 6000) / 1000
        );
    } else {
        vehicle->m_engine_torque = vehicle->m_engine.m_torque_curve[7];
    }

    if(rpm > 6200 && vehicle->m_engine.m_current_gear < vehicle->m_engine.m_gear_size - 1)
    {
        vehicle->m_engine.m_current_gear++;
    } else if(rpm < 2000 && vehicle->m_engine.m_current_gear > 0) {
        vehicle->m_engine.m_current_gear--;
    }
}

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