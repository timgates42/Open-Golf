#ifndef _COURSE_H
#define _COURSE_H

#include <assert.h>

#include "3rd_party/vec/vec.h"
#include "3rd_party/sokol/sokol_app.h"
#include "3rd_party/sokol/sokol_gfx.h"

#include "mcore/mfile.h"
#include "mcore/maths.h"

#include "golf/assets.h"
#include "golf/data_stream.h"
#include "golf/renderer.h"

struct lightmap_image {
    unsigned char *data; 
    sg_image sg_image; 
};
typedef vec_t(struct lightmap_image) vec_lightmap_image_t;

struct lightmap {
    int width, height;
    vec_lightmap_image_t images;

    vec_vec2_t uvs; 
    int buf_len;
    sg_buffer uvs_buf;
};
typedef vec_t(struct lightmap*) vec_lightmap_ptr_t;

void lightmap_init(struct lightmap *lightmap, int width, int height, int num_elements, int num_images);
void lightmap_deinit(struct lightmap *lightmap);
void lightmap_resize(struct lightmap *lightmap, int width, int height, int num_images);
void lightmap_update_image(struct lightmap *lightmap);
void lightmap_update_uvs_buffer(struct lightmap *lightmap, int num_elements);
void lightmap_save_image(struct lightmap *lightmap, const char *filename);

struct terrain_model_material {
    vec3 color0, color1;
};

enum terrain_model_auto_texture {
    AUTO_TEXTURE_NONE,
    AUTO_TEXTURE_WOOD_OUT,
    AUTO_TEXTURE_WOOD_IN,
    AUTO_TEXTURE_WOOD_TOP,
    AUTO_TEXTURE_GRASS,
};

struct terrain_model_face {
    enum terrain_model_auto_texture auto_texture;
    int smooth_normal;
    vec2 texture_coords[4];
    float texture_coord_scale;
    int num_points;
    int x, y, z, w;
    int mat_idx;
    float cor, friction, vel_scale;
};
typedef vec_t(struct terrain_model_face) vec_terrain_model_face_t;

struct terrain_model_face create_terrain_model_face(int num_points, int mat_idx, int smooth_normal,
        int x, int y, int z, int w, vec2 tc0, vec2 tc1, vec2 tc2, vec2 tc3, float texture_coord_scale,
        float cor, float friction, float vel_scale, enum terrain_model_auto_texture auto__texture);

#define MAX_NUM_TERRAIN_MODEL_MATERIALS 5
struct terrain_model {
    struct terrain_model_material materials[MAX_NUM_TERRAIN_MODEL_MATERIALS];
    vec_vec3_t points;
    vec_terrain_model_face_t faces;

    char generator_name[MFILE_MAX_NAME + 1];
    map_float_t generator_params;
    
    int num_elements, buf_len;
    sg_buffer positions_buf, normals_buf, texture_coords_buf, material_idxs_buf;
};

void terrain_model_init(struct terrain_model *model, int num_elements);
void terrain_model_deinit(struct terrain_model *model);
void terrain_model_copy(struct terrain_model *model, struct terrain_model *model_to_copy);
int terrain_model_add_point(struct terrain_model *model, vec3 point, int idx);
vec3 terrain_model_get_point(struct terrain_model *model, int point_idx);
int terrain_model_get_point_idx(struct terrain_model *model, vec3 *point);
void terrain_model_delete_point(struct terrain_model *model, int point_idx);
int terrain_model_add_face(struct terrain_model *model, struct terrain_model_face face, int idx);
struct terrain_model_face terrain_model_get_face(struct terrain_model *model, int face_idx);
int terrain_model_get_face_idx(struct terrain_model *model, struct terrain_model_face *face);
void terrain_model_delete_face(struct terrain_model *model, int face_idx);
void terrain_model_update_buffers(struct terrain_model *model);
void terrain_model_export(struct terrain_model *model, mfile_t *file);
bool terrain_model_import(struct terrain_model *model, mfile_t *file);
void terrain_model_make_square(struct terrain_model *model);
void terrain_model_generate_triangle_data(struct terrain_model *model,
        vec_vec3_t *positions, vec_vec3_t *normals, vec_vec2_t *texture_coords,
        vec_float_t *material_idx);

struct terrain_entity {
    vec3 position, scale;
    quat orientation;
    struct terrain_model terrain_model;
    struct lightmap lightmap;
};
typedef vec_t(struct terrain_entity) vec_terrain_entity_t;
void terrain_entity_init(struct terrain_entity *entity, int num_elements, int lightmap_width, int lightmap_height);
void terrain_entity_deinit(struct terrain_entity *entity);
void terrain_entity_copy(struct terrain_entity *entity, struct terrain_entity *entity_to_copy);
mat4 terrain_entity_get_transform(struct terrain_entity *entity);

enum movement_type {
    MOVEMENT_TYPE_PENDULUM,
    MOVEMENT_TYPE_TO_AND_FROM,
    MOVEMENT_TYPE_RAMP,
    MOVEMENT_TYPE_ROTATION,
};
struct movement_data {
    enum movement_type type;
    struct {
        float theta0;
    } pendulum;
    struct {
        vec3 p0, p1;
    } to_and_from;
    struct {
        float theta0, theta1;
        float transition_length;
        vec3 rotation_axis;
    } ramp;
    struct {
        float theta0;
        vec3 axis;
    } rotation;
    float length;
};
typedef vec_t(struct movement_data) vec_movement_data_t;
struct multi_terrain_entity {
    vec3 static_position, static_scale;
    quat static_orientation;
    struct terrain_model static_terrain_model;
    struct lightmap static_lightmap;

    struct movement_data movement_data;
    vec3 moving_position, moving_scale;
    quat moving_orientation;
    struct terrain_model moving_terrain_model;
    struct lightmap moving_lightmap;
};
typedef vec_t(struct multi_terrain_entity) vec_multi_terrain_entity_t;
void multi_terrain_entity_init(struct multi_terrain_entity *entity, int num_static_elements, 
        int num_moving_elements, int lightmap_width, int lightmap_height);
void multi_terrain_entity_deinit(struct multi_terrain_entity *entity);
void multi_terrain_entity_copy(struct multi_terrain_entity *entity, struct multi_terrain_entity *entity_to_copy);
mat4 multi_terrain_entity_get_static_transform(struct multi_terrain_entity *entity);
mat4 multi_terrain_entity_get_moving_transform(struct multi_terrain_entity *entity, float t);
vec3 multi_terrain_entity_get_moving_velocity(struct multi_terrain_entity *entity, float t, vec3 world_point);

struct ball_start_entity {
    vec3 position;
    struct model *model;
};
typedef vec_t(struct ball_start_entity) vec_ball_start_entity_t;
mat4 ball_start_entity_get_transform(struct ball_start_entity *entity);

struct cup_entity {
    vec3 position, in_hole_delta;
    float radius, in_hole_radius;
    struct lightmap lightmap;
    struct model *model;
};
mat4 cup_entity_get_transform(struct cup_entity *entity);

struct camera_zone_entity {
    vec3 position;
    vec2 size;
    quat orientation;
    bool look_towards_cup;
};
typedef vec_t(struct camera_zone_entity) vec_camera_zone_entity_t;
mat4 camera_zone_entity_get_transform(struct camera_zone_entity *entity);

struct beginning_camera_animation_entity {
    vec3 start_position;
};
mat4 beginning_camera_animation_entity_get_transform(struct beginning_camera_animation_entity *entity);

struct ground_entity {
    vec3 position;
    struct terrain_model model;
    struct lightmap lightmap;
};
void ground_entity_init(struct ground_entity *entity);
mat4 ground_entity_get_transform(struct ground_entity *entity);

struct environment_entity {
    bool is_tiled;
    vec3 position, scale;
    quat orientation;
    struct model *model;
};
typedef vec_t(struct environment_entity) vec_environment_entity_t;
void environment_entity_init(struct environment_entity *entity, const char *model_name);
void environment_entity_deinit(struct environment_entity *entity);
mat4 environment_entity_get_transform(struct environment_entity *entity);

struct water_entity {
    vec3 position, scale;
    quat orientation;
    struct terrain_model model;
    struct lightmap lightmap;
};
typedef vec_t(struct water_entity) vec_water_entity_t;
void water_entity_init(struct water_entity *entity, int num_elements, int lightmap_width, int lightmap_height);
void water_entity_deinit(struct water_entity *entity);
mat4 water_entity_get_transform(struct water_entity *entity);

struct hole {
    char filepath[MFILE_MAX_PATH];
    vec_terrain_entity_t terrain_entities;
    vec_multi_terrain_entity_t multi_terrain_entities;
    vec_camera_zone_entity_t camera_zone_entities;
    vec_environment_entity_t environment_entities;
    vec_water_entity_t water_entities;
    struct cup_entity cup_entity;
    struct ball_start_entity ball_start_entity;
    struct beginning_camera_animation_entity beginning_camera_animation_entity;
    struct lightmap environment_lightmap;
};

void hole_init(struct hole *hole);
void hole_reset(struct hole *hole);
void hole_load(struct hole *hole, mfile_t *file);
void hole_save(struct hole *hole, mfile_t *file);
void hole_serialize(struct hole *hole, struct data_stream *stream, bool include_lightmaps);
void hole_deserialize(struct hole *hole, struct data_stream *stream, bool include_lightmaps);
int hole_add_camera_zone_entity(struct hole *hole);
void hole_generate_lightmaps(struct hole *hole, int num_dilates, int num_smooths);
void hole_update_buffers(struct hole *hole);

#endif
