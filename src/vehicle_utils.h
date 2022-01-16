#pragma once

#include <raylib.h>

typedef struct
{
    int m_gear_size;
    int m_current_gear;
    int* m_torque_curve;
    float* m_gear_ratios;
    float m_differential;
} engine_t;

typedef struct
{
    Texture2D m_texture;
    Rectangle m_frame_rec;
    int m_frame_count;
    int m_current_frame;

    // Forces

    float m_traction;
    float m_drag;
    float m_rolling_resistance;
    float m_longitudinal;

    // Physical properties

    float m_mass;
    float m_friction_coefficient;
    float m_frontal_area;

    float m_drag_coefficient;
    float m_rolling_resistance_coefficient;

    engine_t m_engine;
    float m_engine_torque;
    float m_engine_rpm;

    float m_wheel_radius;

    // Movement
    
    float m_braking_force;
    float m_acceleration;
    float m_velocity;

    // float m_top_speed;

    float m_position_x;
    float m_position_y;
} vehicle_t;

void init_engine(engine_t* engine);
void update_vehicle(vehicle_t* vehicle);
void render_vehicle(vehicle_t* vehicle);