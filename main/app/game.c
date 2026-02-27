#include "game.h"
#include "v_graphics.h"
#include "v_input.h"
#include "v_matrix.h"
#include "v_vector.h"
#include "v_colors.h"
#include "v_config.h"
#include "v_primitives.h"
#include "v_entity.h"

#define MAX_ENTITIES 10
entity_t entities[MAX_ENTITIES];

vec3_t camera = {0, 0, INT_TO_F16(6)};

uint16_t apply_lighting(uint16_t base_color, fix16_t normal_z)
{
  int intensity = normal_z;
  if (intensity < 0)
    intensity = -intensity;

  intensity += F16_ONE / 4;
  if (intensity > F16_ONE)
    intensity = F16_ONE;

  int r = (base_color >> 11) & 0x1F;
  int g = (base_color >> 5) & 0x3F;
  int b = base_color & 0x1F;

  r = (r * intensity) >> 16;
  g = (g * intensity) >> 16;
  b = (b * intensity) >> 16;

  if (base_color == V_WHITE)
  {
    g += 3;
    if (g > 63)
      g = 63;
  }

  return (r << 11) | (g << 5) | b;
}

void game_load(void)
{
  for(int i = 0; i < MAX_ENTITIES; i++)
    entities[i].active = false;

  entities[0].active = true;
  entities[0].mesh = &MESH_PYRAMID;
  entities[0].pos = (vec3_t){0, INT_TO_F16(2), 0};
  entities[0].color = V_WHITE;

  entities[1].active = true;
  entities[1].mesh = &MESH_CUBE;
  entities[1].pos = (vec3_t){0, INT_TO_F16(-2), 0};
  entities[1].color = V_CYAN;
}

void game_update(float dt)
{
  uint8_t k = input_get();
  float rot_speed = 200.0f * dt;

  entities[1].rot.y += rot_speed;
  if (entities[1].rot.y >= 256.0f)
    entities[1].rot.y -= 256.0f;

  if(k & INPUT_LEFT)
    entities[0].pos.x = f16_sub(entities[0].pos.x, FLT_TO_F16(2.0f * dt));
  if(k & INPUT_RIGHT)
    entities[0].pos.x = f16_add(entities[0].pos.x, FLT_TO_F16(2.0f * dt));
  if(k & INPUT_UP)
    entities[0].pos.y = f16_sub(entities[0].pos.y, FLT_TO_F16(2.0f * dt));
  if(k & INPUT_DOWN)
    entities[0].pos.y = f16_add(entities[0].pos.y, FLT_TO_F16(2.0f * dt));

  fix16_t move_speed = FLT_TO_F16(4.0f * dt);
  if(k & INPUT_A)
    camera.z = f16_sub(camera.z, move_speed);
  if(k & INPUT_B)
    camera.z = f16_add(camera.z, move_speed);
}

void sort_entities(entity_t **list, int count)
{
  for(int i = 1; i < count; i++)
  {
    entity_t *key = list[i];
    fix16_t key_z = f16_add(key->pos.z, camera.z);

    int j = i - 1;
    while(j >= 0)
    {
      fix16_t j_z = f16_add(list[j]->pos.z, camera.z);
      if (j_z < key_z)
      {
        list[j+ 1] = list[j];
        j--;
      }
      else
      {
        break;
      }
    }
    list[j + 1] = key;
  }
}

void game_draw(render_mode_t mode)
{
  gfx_clear(V_BLACK);

  entity_t *draw_list[MAX_ENTITIES];
  int draw_count = 0;
  for(int i = 0; i < MAX_ENTITIES; i++)
  {
    if(entities[i].active)
      draw_list[draw_count++] = &entities[i];
  }

  sort_entities(draw_list, draw_count);

    int cx = V_DISPLAY_WIDTH/2;
    int cy = V_DISPLAY_HEIGHT/2;
    fix16_t fov = INT_TO_F16(150);
    const fix16_t near_z = FLT_TO_F16(0.5f);
    // Transform and Project
    for(int i = 0; i < num_verts; i++) 
    {
        t_verts[i] = mat4_mul_vec3(mat_rot, active_mesh->vertices[i]);
        t_verts[i].z = f16_add(t_verts[i].z, camera.z);
        
        if(t_verts[i].z < near_z) 
          t_verts[i].z = near_z;
        
        p_verts[i].x = f16_add(f16_mul(t_verts[i].x, f16_div(fov, t_verts[i].z)), INT_TO_F16(cx));
        p_verts[i].y = f16_add(f16_mul(t_verts[i].y, f16_div(fov, t_verts[i].z)), INT_TO_F16(cy));
    }

    for(int i = 0; i < num_verts; i++)
    {
      vec3_t p = mat4_mul_vec3(mat_rot, mesh->vertices[i]);
      p = vec3_add(p , ent->pos);
      p.z = f16_add(p.z, camera.z);

      if(p.z < near_plane)
      {
        v_culled[i] = true;
        p.z = near_plane;
      }
      else
      {
        v_culled[i] = false;
      }

      t_verts[i] = p;
      p_verts[i].x = f16_add(f16_mul(p.x, f16_div(fov, p.z)), INT_TO_F16(cx));
      p_verts[i].y = f16_add(f16_mul(p.y, f16_div(fov, p.z)), INT_TO_F16(cy));
    }

    for(int i = 0; i < mesh->num_faces; i++)
    {
      int i1 = mesh->faces[i][0];
      int i2 = mesh->faces[i][1];
      int i3 = mesh->faces[i][2];

      if(v_culled[i1] || v_culled[i2] || v_culled[i3]) 
        continue;

      vec3_t normal = vec3_normal(t_verts[i1], t_verts[i2], t_verts[i3]);

      if(normal.z < 0)
      {
        int x1 = F16_TO_INT(p_verts[i1].x);
        int y1 = F16_TO_INT(p_verts[i1].y);
        int x2 = F16_TO_INT(p_verts[i2].x);
        int y2 = F16_TO_INT(p_verts[i2].y);
        int x3 = F16_TO_INT(p_verts[i3].x);
        int y3 = F16_TO_INT(p_verts[i3].y);

        uint16_t shaded_color = apply_lighting(ent->color, normal.z);
        gfx_fill_triangle(x1, y1, x2, y2, x3, y3, shaded_color);
      }
    }
  }
}

game_config_t void_lander = {
  .on_load = game_load,
  .on_update = game_update,
  .on_draw = game_draw
};
