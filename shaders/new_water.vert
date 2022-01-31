in vec2 position;

out vec2 vs_tile_tex_coords;
#ifdef USE_SHADOW
out vec4 vs_pos_light_space;
#endif // USE_SHADOW
#ifdef USE_NOISE
out vec3 vs_noise_tex_coords;
#endif // USE_NOISE

uniform mat4 projection;
uniform mat4 model_view;
uniform float water_depth_offset;
uniform vec2 water_movement;
#ifdef USE_SHADOW
uniform mat4 shadow_texgen_mat;
uniform vec3 camera_pos;
#endif // USE_SHADOW
#ifdef USE_NOISE
uniform float time;
#endif // USE_NOISE

void main()
{
	vec4 position4 = vec4(position, water_depth_offset, 1.0);
	gl_Position = projection * model_view * position4;

	vs_tile_tex_coords = (3.0/50.0) * position + water_movement;
#ifdef USE_SHADOW
	vs_pos_light_space = shadow_texgen_mat * (position4 + vec4(trunc(camera_pos), 0.0));
#endif
#ifdef USE_NOISE
	vs_noise_tex_coords = vec3(position/3.0, time);
#endif // USE_NOISE
}
