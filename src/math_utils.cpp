#include "math_utils.h"

#include <math.h>

float lerp(float a, float b, float t)
{
    return a + (b - a) * t;
}

float top_speed(
    float air_density,
    float drag_coefficient,
    float frontal_area,
    float rolling_resistance_coefficient,
    float vehicle_mass,
    float gravity,
    float engine_hp
)
{
    float _a = 0.5f * air_density * drag_coefficient * frontal_area;
    float _c = rolling_resistance_coefficient * vehicle_mass * gravity;
    float _d = engine_hp * 746;

    float _q = _d / (2 * _a);
    float _q_sqr = _q * _q;
    float _r = _c / (3 * _a);
    float _r_cbd = _r * _r * _r;

    return cbrt(
        _q + sqrt(_q_sqr + _r_cbd)
    ) + cbrt(
        _q - sqrt(_q_sqr + _r_cbd)
    );
}