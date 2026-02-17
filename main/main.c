#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

// Engine Core
#include "v_display.h"
#include "v_graphics.h"
#include "v_input.h"
#include "v_config.h"
#include "v_colors.h"

// Math Kernel
#include "v_fixed.h"
#include "v_vector.h"
#include "v_matrix.h"

static const char *TAG = "VOID_ENGINE";

// --- 3D DATA DEFINITION ---

// A Cube has 8 corners (Vertices)
// We define them in "Local Space" (centered at 0,0,0)
#define NUM_VERTICES 8
vec3_t cube_vertices[NUM_VERTICES] = {
    { INT_TO_F16(-1), INT_TO_F16(-1), INT_TO_F16(-1) }, // 0: Left-Bot-Back
    { INT_TO_F16( 1), INT_TO_F16(-1), INT_TO_F16(-1) }, // 1: Right-Bot-Back
    { INT_TO_F16( 1), INT_TO_F16( 1), INT_TO_F16(-1) }, // 2: Right-Top-Back
    { INT_TO_F16(-1), INT_TO_F16( 1), INT_TO_F16(-1) }, // 3: Left-Top-Back
    { INT_TO_F16(-1), INT_TO_F16(-1), INT_TO_F16( 1) }, // 4: Left-Bot-Front
    { INT_TO_F16( 1), INT_TO_F16(-1), INT_TO_F16( 1) }, // 5: Right-Bot-Front
    { INT_TO_F16( 1), INT_TO_F16( 1), INT_TO_F16( 1) }, // 6: Right-Top-Front
    { INT_TO_F16(-1), INT_TO_F16( 1), INT_TO_F16( 1) }  // 7: Left-Top-Front
};

// A Cube has 12 Edges connecting those corners
// We store pairs of indices: {Start Vertex, End Vertex}
#define NUM_EDGES 12
int cube_edges[NUM_EDGES][2] = {
    {0,1}, {1,2}, {2,3}, {3,0}, // Back Face
    {4,5}, {5,6}, {6,7}, {7,4}, // Front Face
    {0,4}, {1,5}, {2,6}, {3,7}  // Connecting Lines
};

// --- GAME STATE ---
// Rotation angles (0-255)
uint8_t angle_x = 0;
uint8_t angle_y = 0;

// Camera Position (We move the cube away from camera)
vec3_t camera_pos = { 0, 0, INT_TO_F16(5) }; // Z = 5 units away

void app_main(void)
{
    ESP_LOGI(TAG, "Booting Void Engine 3D...");

    // 1. Initialize Subsystems
    display_init();
    input_init(); // Don't forget this!

    // Center of screen for projection
    int center_x = V_DISPLAY_WIDTH / 2;
    int center_y = V_DISPLAY_HEIGHT / 2;
    
    // Field of View Scale (Higher = Zoomed in)
    // 256.0f is a good default for 160x128
    fix16_t fov = INT_TO_F16(180); 

    while(1)
    {
        // --- 1. INPUT PHASE ---
        uint8_t keys = input_get();
        
        
        // Rotate logic
        if (keys & INPUT_UP)    angle_x -= 2;
        if (keys & INPUT_DOWN)  angle_x += 2;
        if (keys & INPUT_LEFT)  angle_y -= 2;
        if (keys & INPUT_RIGHT) angle_y += 2;

        // Zoom logic (Move Z)
        if (keys & INPUT_A) camera_pos.z = f16_sub(camera_pos.z, FLT_TO_F16(0.1f)); // Zoom In
        if (keys & INPUT_B) camera_pos.z = f16_add(camera_pos.z, FLT_TO_F16(0.1f)); // Zoom Out


        // --- 2. UPDATE/TRANSFORM PHASE ---
        
        // Calculate Rotation Matrices
        mat4_t rot_x = mat4_rotate_x(angle_x);
        mat4_t rot_y = mat4_rotate_y(angle_y);
        
        // Combine Rotations (X * Y)
        mat4_t model_matrix = mat4_mul(rot_x, rot_y);

        // Transform Vertices
        vec3_t transformed_verts[NUM_VERTICES];
        vec2_t projected_points[NUM_VERTICES];

        for (int i = 0; i < NUM_VERTICES; i++) {
            // A. Rotate Vertex
            vec3_t p = mat4_mul_vec3(model_matrix, cube_vertices[i]);
            
            p.z = f16_add(p.z, camera_pos.z);
            
            transformed_verts[i] = p;

            if (p.z == 0) p.z = 1; // Prevent divide by zero
            
            fix16_t z_inv = f16_div(fov, p.z);
            
            projected_points[i].x = f16_add(f16_mul(p.x, z_inv), INT_TO_F16(center_x));
            projected_points[i].y = f16_add(f16_mul(p.y, z_inv), INT_TO_F16(center_y));
        }

        
        gfx_clear(V_BLACK);

        for (int i = 0; i < NUM_EDGES; i++) {
            int i1 = cube_edges[i][0];
            int i2 = cube_edges[i][1];

            int x1 = F16_TO_INT(projected_points[i1].x);
            int y1 = F16_TO_INT(projected_points[i1].y);
            int x2 = F16_TO_INT(projected_points[i2].x);
            int y2 = F16_TO_INT(projected_points[i2].y);

            if (x1 > -50 && x1 < 200 && y1 > -50 && y1 < 200) {
                 gfx_draw_line(x1, y1, x2, y2, V_GREEN);
            }
        }
        
        gfx_draw_pixel(center_x, center_y, V_RED);

        display_draw();

        vTaskDelay(1);
        
    }
}
