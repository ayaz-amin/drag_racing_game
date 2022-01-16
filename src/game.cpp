#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

#define AIR_DENSITY 1.29f
#define FIXED_DELTA_TIME 0.02f

#include "game.h"
#include "ui.h"
#include "vehicle_utils.h"
#include "math_utils.h"

#include <stdio.h>
#include <math.h>
#include <raylib.h>

#include "rlImGui/rlImGui.h"
#include "rlImGui/imgui.h"

enum UIstate
{
    MENU_STATE,
    GARAGE_STATE,
    GAME_STATE,
    PAUSE_STATE,
    EXIT_STATE
};

Vector2 g_mouse_pos; 

UIstate g_ui_state = MENU_STATE;

button_t g_back_button;
button_t g_free_roam_button;
button_t g_garage_button;

Texture2D g_main_menu_tex;

Texture2D g_garage_tex;

Texture2D g_track_tex;
Texture2D g_cursor_tex;
Texture2D g_background_tex;
float scrolling_road = 0.0f;

Camera2D g_player_camera;

float delta_time;

void init_player(vehicle_t* player)
{
    player->m_texture = LoadTexture("assets/vx90tt.png");
    player->m_frame_rec = {
        0.0f, 0.0f, (float)player->m_texture.width / 5, (float)player->m_texture.height
    };
    player->m_frame_count = 0;
    player->m_current_frame = 0;

    player->m_traction = 0.0f;
    player->m_drag = 0.0f;
    player->m_rolling_resistance = 0.0f;
    player->m_longitudinal = 0.0f;

    player->m_mass = 1200.0f;
    player->m_friction_coefficient = 0.3f;
    player->m_frontal_area = 2.23f;

    player->m_drag_coefficient = 0.5f * player->m_friction_coefficient * player->m_frontal_area * AIR_DENSITY;
    player->m_rolling_resistance_coefficient = 0.01f;

    player->m_engine_hp = 450.0f;
    player->m_min_rpm = 900.0f;
    player->m_max_rpm = 7000.0f;
    player->m_engine_rpm = player->m_min_rpm;
    player->m_instant_torque = (player->m_engine_hp * 5252) / player->m_engine_rpm;
    player->m_engine_torque = player->m_engine_torque;
    player->m_differential = 3.75f;

    player->m_top_speed = top_speed(
        AIR_DENSITY, player->m_drag_coefficient,
        player->m_frontal_area, player->m_rolling_resistance_coefficient,
        player->m_mass, 9.81f, player->m_engine_hp
    );

    player->m_wheel_radius = 0.33f;

    player->m_braking_force = -5000.0f;
    player->m_acceleration = 0.0f;
    player->m_velocity = 0.0f;

    player->m_position_x = 0.0f;
    player->m_position_y = 0.0f;
}

void update_main_menu()
{
    update_button(&g_free_roam_button, g_mouse_pos);
    update_button(&g_garage_button, g_mouse_pos);
    if(g_free_roam_button.m_is_pressed) g_ui_state = GAME_STATE;
    if(g_garage_button.m_is_pressed) g_ui_state = GARAGE_STATE;
}

void draw_main_menu()
{
    DrawTexture(g_main_menu_tex, 0, 0, WHITE);
    button_render(&g_free_roam_button);
    button_render(&g_garage_button);
}

void update_garage(vehicle_t* vehicle)
{
    update_button(&g_back_button, g_mouse_pos);
    if(g_back_button.m_is_pressed) g_ui_state = MENU_STATE;
    vehicle->m_drag_coefficient = 0.5f * vehicle->m_friction_coefficient * vehicle->m_frontal_area * AIR_DENSITY;
    vehicle->m_engine_torque = (vehicle->m_engine_hp * 5252) / vehicle->m_max_rpm;
    // vehicle->m_top_speed = cbrt(
    //     (2 * vehicle->m_engine_hp * 745.7f) / (AIR_DENSITY * vehicle->m_drag_coefficient * vehicle->m_frontal_area)
    //     );

    vehicle->m_top_speed = top_speed(
        AIR_DENSITY, vehicle->m_drag_coefficient,
        vehicle->m_frontal_area, vehicle->m_rolling_resistance_coefficient,
        vehicle->m_mass, 9.81f, vehicle->m_engine_hp
    );
}

bool imgui_window_close = false;

void draw_garage(vehicle_t* vehicle)
{
    DrawTexture(g_garage_tex, 0, 0, WHITE);
    DrawTextureRec(
        vehicle->m_texture, {0.0f, 0.0f, vehicle->m_texture.width / 5, vehicle->m_texture.height},
        {
            0.5 * WINDOW_WIDTH - 0.1 * vehicle->m_texture.width,
            0.75 * WINDOW_HEIGHT - 0.5 * vehicle->m_texture.height
        },
        WHITE
    );
    button_render(&g_back_button);

    BeginRLImGui();
    ImGui::Begin("Vehicle editor", &imgui_window_close, ImGuiWindowFlags_MenuBar);
        
        ImGui::SliderFloat("Vehicle mass", &vehicle->m_mass, 500.0f, 2000.0f);
        ImGui::SliderFloat("Coefficient of friction", &vehicle->m_friction_coefficient, 0.15f, 1.0f);
        ImGui::SliderFloat("Frontal Area", &vehicle->m_frontal_area, 1.0f, 3.0f);
        ImGui::SliderFloat("Engine Horsepower", &vehicle->m_engine_hp, 200.0f, 1500.0f);
        ImGui::SliderFloat("Wheel radius", &vehicle->m_wheel_radius, 0.1f, 1.0f);
        ImGui::Text("Theoretical top speed (kph): %f", vehicle->m_top_speed * 3.6);
        ImGui::Text("Theoretical top speed (mph): %f", vehicle->m_top_speed * 2.237);
    
    ImGui::End();
    EndRLImGui();
}

void update_race(vehicle_t* player, vehicle_t* opponent)
{
    // Update player vehicle

    if(IsKeyDown(KEY_UP))
    {
        player->m_traction = (
            (player->m_engine_torque * player->m_differential * 0.7f) / player->m_wheel_radius
        );
    } else if(IsKeyDown(KEY_DOWN)) {
        player->m_traction = player->m_braking_force;
    } else {
        player->m_traction = 0.0f;
    }

    player->m_drag = -player->m_drag_coefficient * player->m_velocity * abs(player->m_velocity);

    player->m_rolling_resistance = -player->m_rolling_resistance_coefficient * player->m_mass * 9.81f;

    player->m_longitudinal = player->m_traction + player->m_drag + player->m_rolling_resistance;
    
    player->m_acceleration = player->m_longitudinal / player->m_mass;

    player->m_velocity += player->m_acceleration * FIXED_DELTA_TIME;

    if(player->m_velocity <= 0) player->m_velocity = 0.0f;
    // if(player->m_velocity > player->m_top_speed) player->m_velocity = player->m_top_speed;

    player->m_engine_rpm = (player->m_velocity / player->m_wheel_radius) * player->m_differential * 9.549296585513; // 60 / (2 pi)

    if(player->m_engine_rpm <= player->m_min_rpm) {
        player->m_engine_torque = player->m_instant_torque;
    } else if(player->m_engine_rpm > player->m_min_rpm && player->m_engine_rpm <= player->m_max_rpm) {
        player->m_engine_torque = (player->m_engine_hp * 5252) / player->m_engine_rpm;
    } else if(player->m_engine_rpm > player->m_max_rpm) {
        player->m_engine_rpm = player->m_max_rpm;
    }

    player->m_position_x += player->m_velocity * FIXED_DELTA_TIME;

    // Update opponent vehicle

    opponent->m_traction = (
        (opponent->m_engine_torque * opponent->m_differential * 0.7f) / opponent->m_wheel_radius
    );

    opponent->m_drag = -opponent->m_drag_coefficient * opponent->m_velocity * abs(opponent->m_velocity);

    opponent->m_rolling_resistance = -opponent->m_rolling_resistance_coefficient * opponent->m_mass * 9.81f;

    opponent->m_longitudinal = opponent->m_traction + opponent->m_drag + opponent->m_rolling_resistance;
    
    opponent->m_acceleration = opponent->m_longitudinal / opponent->m_mass;

    opponent->m_velocity += opponent->m_acceleration * FIXED_DELTA_TIME;

    if(opponent->m_velocity <= 0) opponent->m_velocity = 0.0f;

    opponent->m_engine_rpm = (
        (opponent->m_velocity / opponent->m_wheel_radius) * opponent->m_differential * 60
    ) / (6.283185);

    if(opponent->m_engine_rpm <= opponent->m_min_rpm)
    {
        opponent->m_engine_torque = opponent->m_instant_torque;
    } else if(opponent->m_engine_rpm > opponent->m_min_rpm && opponent->m_engine_rpm <= opponent->m_max_rpm) {
        opponent->m_engine_torque = (opponent->m_engine_hp * 5252) / opponent->m_engine_rpm;
    } else if(opponent->m_engine_rpm > opponent->m_max_rpm) {
        opponent->m_engine_rpm = opponent->m_max_rpm;
    }

    opponent->m_engine_torque = (opponent->m_engine_hp * 5252) / opponent->m_engine_rpm;

    opponent->m_position_x += opponent->m_velocity * FIXED_DELTA_TIME;

    g_player_camera.target.x = lerp(g_player_camera.target.x, player->m_position_x, 10.0f * delta_time);

    scrolling_road -= 10 * delta_time * player->m_velocity;
    if(scrolling_road <= -g_track_tex.width) scrolling_road = 0.0f;

    update_button(&g_back_button, g_mouse_pos);
    if(g_back_button.m_is_pressed)
    {
        player->m_frame_count = 0;
        player->m_current_frame = 0;
        player->m_traction = 0.0f;
        player->m_drag = 0.0f;
        player->m_rolling_resistance = 0.0f;
        player->m_longitudinal = 0.0f;
        player->m_engine_rpm = 1000.0f;
        player->m_acceleration = 0.0f;
        player->m_velocity = 0.0f;
        player->m_position_x = 0.0f;

        g_player_camera.target.x = player->m_position_x;
        
        opponent->m_frame_count = 0;
        opponent->m_current_frame = 0;
        opponent->m_traction = 0.0f;
        opponent->m_drag = 0.0f;
        opponent->m_rolling_resistance = 0.0f;
        opponent->m_longitudinal = 0.0f;
        opponent->m_engine_rpm = 1000.0f;
        opponent->m_acceleration = 0.0f;
        opponent->m_velocity = 0.0f;
        opponent->m_position_x = 0.0f;

        g_ui_state = MENU_STATE;
    }
}

char velocity_buffer[64];
char gear_buffer[64];
char rpm_buffer[64];

void draw_race(vehicle_t* player, vehicle_t* opponent)
{
    DrawTexture(g_background_tex, 0, 0, WHITE);

    DrawTextureEx(g_track_tex, Vector2{scrolling_road, 0}, 0.0f, 1.0f, WHITE);
    DrawTextureEx(g_track_tex, Vector2{g_track_tex.width + scrolling_road, 0}, 0.0f, 1.0f, WHITE);

    BeginMode2D(g_player_camera);
        render_vehicle(player);
        render_vehicle(opponent);
    EndMode2D();

    button_render(&g_back_button);

    snprintf(velocity_buffer, sizeof(velocity_buffer), "Velocity (km/h): %f", player->m_velocity * 3.6);
    snprintf(rpm_buffer, sizeof(rpm_buffer), "RPM: %f", player->m_engine_rpm);

    DrawText(velocity_buffer, 300, 200, 20, BLUE);
    DrawText(gear_buffer, 600, 200, 20, BLUE);
    DrawText(rpm_buffer, 100, 200, 20, BLUE);
}

void update_app(vehicle_t* player, vehicle_t* opponent)
{
    switch(g_ui_state)
    {
        case MENU_STATE :
            update_main_menu();
            break;
        case GAME_STATE:
            update_race(player, opponent);
            break;
        case GARAGE_STATE:
            update_garage(player);
    }
}

void draw_app(vehicle_t* player, vehicle_t* opponent)
{
    switch(g_ui_state)
    {
        case MENU_STATE:
            draw_main_menu();
            break;
        case GAME_STATE:
            draw_race(player, opponent);
            break;
        case GARAGE_STATE:
            draw_garage(player);
    }
}

void run_app()
{
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Double Clutch");
    SetTargetFPS(120);
    HideCursor();

    vehicle_t player;
    init_player(&player);

    vehicle_t opponent;
    opponent.m_texture = LoadTexture("assets/tp23.png");
    opponent.m_frame_rec = {
        0.0f, 0.0f, (float)opponent.m_texture.width / 5, (float)opponent.m_texture.height
    };
    opponent.m_frame_count = 0;
    opponent.m_current_frame = 0;

    opponent.m_traction = 0.0f;
    opponent.m_drag = 0.0f;
    opponent.m_rolling_resistance = 0.0f;
    opponent.m_longitudinal = 0.0f;

    opponent.m_mass = 1250.0f;
    opponent.m_friction_coefficient = 0.30f;
    opponent.m_frontal_area = 2.21f;

    opponent.m_drag_coefficient = 0.5f * opponent.m_friction_coefficient * opponent.m_frontal_area * AIR_DENSITY;
    opponent.m_rolling_resistance_coefficient = 0.01f;

    opponent.m_engine_hp = 397.0f;
    opponent.m_min_rpm = 900.0f;
    opponent.m_max_rpm = 6900.0f;
    opponent.m_engine_rpm = opponent.m_min_rpm;
    opponent.m_instant_torque = (opponent.m_engine_hp * 5252) / opponent.m_engine_rpm;
    opponent.m_engine_torque = opponent.m_instant_torque;
    opponent.m_differential = 3.55f;

    opponent.m_wheel_radius = 0.33f;

    opponent.m_braking_force = -4000.0f;
    opponent.m_acceleration = 0.0f;
    opponent.m_velocity = 0.0f;
        
    opponent.m_position_x = 0;
    opponent.m_position_y = 100;

    SetupRLImGui(true);

    init_button(&g_back_button, 0.1f * WINDOW_WIDTH, 0.1f * WINDOW_HEIGHT, "assets/ui/neon_back_button.png");
    init_button(&g_free_roam_button, 0.25f * WINDOW_WIDTH, 0.75f * WINDOW_HEIGHT, "assets/ui/free_roam_button.png");
    init_button(&g_garage_button, 0.25f * WINDOW_WIDTH, 0.5f * WINDOW_HEIGHT, "assets/ui/garage_button.png");

    g_main_menu_tex = LoadTexture("assets/background/main_menu.png");

    g_garage_tex = LoadTexture("assets/background/garage.png");

    g_cursor_tex = LoadTexture("assets/cursor.png");
    g_background_tex = LoadTexture("assets/background/mountain.png");
    g_track_tex = LoadTexture("assets/background/road.png");
    
    g_player_camera = Camera2D{
        Vector2{WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2},
        Vector2{0, 0},
        0.0f,
        1.0f
    };

    while(!WindowShouldClose())
    {
        g_mouse_pos = GetMousePosition();
        delta_time = (float)GetFrameTime();

        update_app(&player, &opponent);
        
        ClearBackground(RAYWHITE);
        BeginDrawing();

        draw_app(&player, &opponent);

        DrawTexture(g_cursor_tex, g_mouse_pos.x, g_mouse_pos.y, WHITE);

        DrawFPS(0, 0);

        EndDrawing();
    }

    ShutdownRLImGui();

    UnloadTexture(opponent.m_texture);
    UnloadTexture(player.m_texture);
    UnloadTexture(g_background_tex);
    UnloadTexture(g_cursor_tex);
    UnloadTexture(g_garage_button.m_texture);
    UnloadTexture(g_free_roam_button.m_texture);
    CloseWindow();
}