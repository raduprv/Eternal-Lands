in vec2 vs_tile_tex_coords;
in vec3 frag_position;
#ifdef USE_SHADOW
in vec4 vs_pos_light_space;
#endif // USE_SHADOW
#ifdef USE_NOISE
in vec3 vs_noise_tex_coords;
#endif // USE_NOISE

out vec4 frag_color;

struct DirectionalLight
{
	vec3 direction;
	vec3 ambient;
	vec3 diffuse;
	//vec3 specular;
};

struct PositionalLight
{
	vec3 position;
	float constant_attenuation;
	float linear_attenuation;
	float quadratic_attenuation;
	vec3 ambient;
	vec3 diffuse;
// 	vec3 specular;
};

// All water tiles are horizontally flat surfaces
const vec3 normal = vec3(0.0, 0.0, 1.0);
// gl_LightModel.ambient. Client never sets it, so use default value of:
const vec3 lightmodel_ambient = vec3(0.2, 0.2, 0.2);
// This is the value of glColor when the shader is called
const vec4 white = vec4(1.0, 1.0, 1.0, 1.0);

uniform sampler2D tile_texture;
uniform DirectionalLight directional_light;
uniform PositionalLight positional_lights[8];
uniform int nr_positional_lights;

#ifdef USE_REFLECTION
uniform sampler2D reflection_texture;
uniform vec4 viewport;
uniform float blend;
#endif // USE_REFLECTION

#ifdef	USE_SHADOW
uniform sampler2DShadow shadow_texture;
#endif	// USE_SHADOW

#ifdef	USE_NOISE
uniform sampler3D noise_texture;
uniform vec4 noise_scale;
#endif	// USE_NOISE

#ifdef	USE_CUBIC_FILTER
uniform sampler1D hg_texture;
uniform vec2 size;
uniform vec2 texel_size_x;
uniform vec2 texel_size_y;
#endif	// USE_CUBIC_FILTER

#ifdef USE_FOG
uniform float fog_density;
uniform vec4 fog_color;
#endif // USE_FOG

void calc_directional_light(DirectionalLight light, vec3 normal, out vec3 ambient, out vec3 diffuse)
{
	vec3 light_direction = normalize(-light.direction);
	float diff = max(dot(normal, light_direction), 0.0);
	ambient = light.ambient;
	diffuse = light.diffuse * diff;
}

void calc_positional_light(PositionalLight light, vec3 position, out vec3 ambient, out vec3 diffuse)
{
	vec3 light_direction = normalize(light.position - position);
	float diff = max(dot(normal, light_direction), 0.0);
	float dist = length(light.position - position);
	float attenuation = 1.0 / (light.constant_attenuation
	                          + light.linear_attenuation * dist
	                          + light.quadratic_attenuation * dist * dist);
	ambient = light.ambient * attenuation;
	diffuse = light.diffuse * diff * attenuation;
}

void main (void)
{
	vec3 ambient, diffuse;
	calc_directional_light(directional_light, normal, ambient, diffuse);
	for (int i = 0; i < nr_positional_lights; ++i)
	{
		vec3 p_ambient, p_diffuse;
		calc_positional_light(positional_lights[i], frag_position, p_ambient, p_diffuse);
		ambient += p_ambient;
		diffuse += p_diffuse;
	}
	ambient += lightmodel_ambient;
	vec4 light_color = vec4(ambient + diffuse, 1.0);

	vec2 tile_tex_coords = vs_tile_tex_coords;
	vec4 color, light;
#ifdef USE_REFLECTION
	vec2 reflection_tex_coords = (gl_FragCoord.xy - viewport.xy) / viewport.zw;
#endif // USE_REFLECTION

#ifdef USE_NOISE
	vec2 noise_displacement = texture(noise_texture, vs_noise_tex_coords).ga * 2.0 - 1.0;
 #ifdef USE_REFLECTION
	reflection_tex_coords += noise_displacement * noise_scale.xy;
 #endif // USE_REFLECTION
	tile_tex_coords += noise_displacement * noise_scale.zw;
#endif // USE_NOISE

#ifdef USE_SHADOW
	vec4 shadow_color = vec4(ambient, 1.0);
	float shadow = textureProj(shadow_texture, vs_pos_light_space);
// 	light = mix(shadow_color, light_color, shadow);
	light = mix(shadow_color, white, shadow);
#else // USE_SHADOW
// 	light = light_color;
	light = white;
#endif // USE_SHADOW
	vec4 tile_color = light * texture(tile_texture, tile_tex_coords);

#ifdef USE_REFLECTION
 #ifdef USE_CUBIC_FILTER
	vec2 coord_hg = reflection_tex_coords * size - vec2(0.5, 0.5);
	vec3 hg_x = texture(hg_texture, coord_hg.x).xyz;
	vec3 hg_y = texture(hg_texture, coord_hg.y).xyz;

	vec2 reflection_tex_coord10 = reflection_tex_coords + hg_x.x * texel_size_x;
	vec2 reflection_tex_coord00 = reflection_tex_coords - hg_x.y * texel_size_x;
	vec2 reflection_tex_coord11 = reflection_tex_coords + hg_y.x * texel_size_y;
	vec2 reflection_tex_coord01 = reflection_tex_coords + hg_y.x * texel_size_y;
	reflection_tex_coord10 -= hg_y.y * texel_size_y;
	reflection_tex_coord00 -= hg_y.y * texel_size_y;

	vec4 color00 = texture(reflection_texture, reflection_tex_coord00);
	vec4 color10 = texture(reflection_texture, reflection_tex_coord10);
	vec4 color01 = texture(reflection_texture, reflection_tex_coord01);
	vec4 color11 = texture(reflection_texture, reflection_tex_coord11);

	color00 = mix(color00, color01, hg_y.z);
	color10 = mix(color10, color11, hg_y.z);
	color = mix(color00, color10, hg_x.z);
 #else // USE_CUBIC_FILTER
	color = texture(reflection_texture, reflection_tex_coords);
 #endif // USE_CUBIC_FILTER
	frag_color = mix(tile_color, color, blend);
#else // USE_REFLECTION
	frag_color = tile_color;
#endif // USE_REFLECTION

#ifdef USE_FOG
	const float LOG2 = 1.442695;
	float fog = fog_density * gl_FragCoord.z / gl_FragCoord.w;
	fog = exp2(-fog*fog*LOG2);
	frag_color = mix(fog_color, frag_color, fog);
#endif // USE_FOG
}
