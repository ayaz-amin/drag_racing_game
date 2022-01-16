#pragma once

float lerp(float a, float b, float t);
float top_speed(
    float air_density,
    float drag_coefficient,
    float frontal_area,
    float rolling_resistance_coefficient,
    float vehicle_mass,
    float gravity,
    float engine_hp
);