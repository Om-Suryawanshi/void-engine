#include "v_primitives.h"
#include "v_fixed.h"

static const vec3_t cube_verts[8] = {
    { INT_TO_F16(-1), INT_TO_F16(-1), INT_TO_F16( 1) }, { INT_TO_F16( 1), INT_TO_F16(-1), INT_TO_F16( 1) }, 
    { INT_TO_F16( 1), INT_TO_F16( 1), INT_TO_F16( 1) }, { INT_TO_F16(-1), INT_TO_F16( 1), INT_TO_F16( 1) }, 
    { INT_TO_F16(-1), INT_TO_F16(-1), INT_TO_F16(-1) }, { INT_TO_F16( 1), INT_TO_F16(-1), INT_TO_F16(-1) }, 
    { INT_TO_F16( 1), INT_TO_F16( 1), INT_TO_F16(-1) }, { INT_TO_F16(-1), INT_TO_F16( 1), INT_TO_F16(-1) }  
};

static const int cube_faces[12][3] = {
    {0, 1, 2}, {0, 2, 3}, {5, 4, 7}, {5, 7, 6},
    {4, 0, 3}, {4, 3, 7}, {1, 5, 6}, {1, 6, 2},
    {3, 2, 6}, {3, 6, 7}, {4, 5, 1}, {4, 1, 0}  
};

static const int cube_edges[12][2] = {
    {0,1}, {1,2}, {2,3}, {3,0}, {4,5}, {5,6}, 
    {6,7}, {7,4}, {0,4}, {1,5}, {2,6}, {3,7} 
};

const mesh_t MESH_CUBE = {
    .vertices = cube_verts, .num_vertices = 8,
    .faces = cube_faces,    .num_faces = 12,
    .edges = cube_edges,    .num_edges = 12
};

static const vec3_t pyr_verts[5] = {
    { 0, INT_TO_F16(1), 0 }, // Top point
    { INT_TO_F16(-1), INT_TO_F16(-1), INT_TO_F16( 1) }, // Base corners
    { INT_TO_F16( 1), INT_TO_F16(-1), INT_TO_F16( 1) }, 
    { INT_TO_F16( 1), INT_TO_F16(-1), INT_TO_F16(-1) }, 
    { INT_TO_F16(-1), INT_TO_F16(-1), INT_TO_F16(-1) }  
};

static const int pyr_faces[6][3] = {
    {0, 1, 2}, {0, 2, 3}, {0, 3, 4}, {0, 4, 1}, // Sides
    {1, 4, 3}, {1, 3, 2}                        // Base
};

static const int pyr_edges[8][2] = {
    {0,1}, {0,2}, {0,3}, {0,4}, // Slopes
    {1,2}, {2,3}, {3,4}, {4,1}  // Base
};

const mesh_t MESH_PYRAMID = {
    .vertices = pyr_verts, .num_vertices = 5,
    .faces = pyr_faces,    .num_faces = 6,
    .edges = pyr_edges,    .num_edges = 8
};
