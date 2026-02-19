#include "game.h"
#include "v_graphics.h"
#include "v_input.h"
#include "v_matrix.h"
#include "v_vector.h"
#include "v_colors.h"
#include "v_config.h"
#include "v_primitives.h"

#include <stddef.h>


const mesh_t *active_mesh = &MESH_CUBE;

uint8_t rot_x = 0;
uint8_t rot_y = 0;
vec3_t camera = {0, 0, INT_TO_F16(4)};

uint16_t apply_lighting(uint16_t base_color, fix16_t normal_z) 
{
    int intensity = normal_z; 
    if (intensity < 0) intensity = -intensity; 
    intensity += F16_ONE / 4; 
    if (intensity > F16_ONE) intensity = F16_ONE;

    int r = (base_color >> 11) & 0x1F;
    int g = (base_color >> 5) & 0x3F;
    int b = base_color & 0x1F;

    r = (r * intensity) >> 16;
    g = (g * intensity) >> 16;
    b = (b * intensity) >> 16;

    if (base_color == V_WHITE) 
    {
        g += 3; // Inject a little extra green
        if (g > 63) g = 63;
    }

    return (r << 11) | (g << 5) | b;
}

void game_load(void)
{
}

void game_update(float dt)
{
  uint8_t k = input_get();
  if (k & INPUT_UP) 
    rot_x -= 2;
  if (k & INPUT_DOWN) 
    rot_x += 2;
  if (k & INPUT_LEFT) 
    rot_y -= 2;
  if (k & INPUT_RIGHT) 
    rot_y += 2;

  if (k & INPUT_A) 
    camera.z = f16_sub(camera.z, FLT_TO_F16(0.1f));
  if (k & INPUT_B) 
    camera.z = f16_add(camera.z, FLT_TO_F16(0.1f));

  static bool toggle_prev = false;
  bool toggle = (k & INPUT_LEFT) && (k & INPUT_RIGHT);

  if(toggle && !toggle_prev)
  {
    static int mode = 1;
    mode = (mode + 1) % 3;
    engine_set_mode((render_mode_t)mode);
  }
  toggle_prev = toggle;
}

void game_draw(render_mode_t mode)
{
    gfx_clear(V_BLACK);

    mat4_t mat_rot = mat4_mul(mat4_rotate_x(rot_x), mat4_rotate_y(rot_y));

    // We dynamically size these arrays based on the loaded mesh
    int num_verts = active_mesh->num_vertices;
    vec3_t t_verts[num_verts];
    vec2_t p_verts[num_verts];

    int cx = V_DISPLAY_WIDTH/2;
    int cy = V_DISPLAY_HEIGHT/2;
    fix16_t fov = INT_TO_F16(150);

    // Transform and Project
    for(int i = 0; i < num_verts; i++) 
    {
        t_verts[i] = mat4_mul_vec3(mat_rot, active_mesh->vertices[i]);
        t_verts[i].z = f16_add(t_verts[i].z, camera.z);
        if(t_verts[i].z < FLT_TO_F16(0.5f)) t_verts[i].z = FLT_TO_F16(0.5f);
        
        p_verts[i].x = f16_add(f16_mul(t_verts[i].x, f16_div(fov, t_verts[i].z)), INT_TO_F16(cx));
        p_verts[i].y = f16_add(f16_mul(t_verts[i].y, f16_div(fov, t_verts[i].z)), INT_TO_F16(cy));
    }

    // Render Solid Faces
    if (mode == RENDER_SOLID || mode == RENDER_BOTH) 
    {
        for(int i = 0; i < active_mesh->num_faces; i++) 
        {
            // Get the indices for the 3 points of this triangle
            int i1 = active_mesh->faces[i][0];
            int i2 = active_mesh->faces[i][1];
            int i3 = active_mesh->faces[i][2];

            vec3_t normal = vec3_normal(t_verts[i1], t_verts[i2], t_verts[i3]);
            
            // Backface Culling
            if (normal.z < 0) 
            { 
                int x1 = F16_TO_INT(p_verts[i1].x);
                int y1 = F16_TO_INT(p_verts[i1].y);
                int x2 = F16_TO_INT(p_verts[i2].x);
                int y2 = F16_TO_INT(p_verts[i2].y);
                int x3 = F16_TO_INT(p_verts[i3].x);
                int y3 = F16_TO_INT(p_verts[i3].y);

                // Pure monochromatic lighting
                uint16_t shaded_color = apply_lighting(V_WHITE, normal.z);
                gfx_fill_triangle(x1, y1, x2, y2, x3, y3, shaded_color);
            }
        }
    }

    // Render Classic Wireframe
    if (mode == RENDER_WIRE || mode == RENDER_BOTH) 
    {
        for(int i = 0; i < active_mesh->num_edges; i++) 
        {
            int i1 = active_mesh->edges[i][0];
            int i2 = active_mesh->edges[i][1];

            int x1 = F16_TO_INT(p_verts[i1].x);
            int y1 = F16_TO_INT(p_verts[i1].y);
            int x2 = F16_TO_INT(p_verts[i2].x);
            int y2 = F16_TO_INT(p_verts[i2].y);

            uint16_t color = (mode == RENDER_BOTH) ? V_BLACK : V_GREEN; 
            gfx_draw_line(x1, y1, x2, y2, color);
        }
    }
}

game_config_t cube_game = {
  .on_load = NULL,
  .on_update = game_update,
  .on_draw = game_draw
};
