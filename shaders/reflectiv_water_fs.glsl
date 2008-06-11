#define	USE_CUBIC_FILTER
uniform sampler2D tile_texture;
uniform sampler2D reflection_texture;
#ifdef	USE_SHADOW
uniform sampler2DShadow shadow_texture;
#endif	// USE_SHADOW
uniform float time;
uniform float blend;
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

void main (void) 
{
	vec4 color, light, shadow_tex_pos;
	vec2 reflection_tex_coord;
#ifdef	USE_SHADOW
	vec4 shadow_tex_coord;
	vec4 shadow;
#endif	// USE_SHADOW
	vec2 tile_tex_coord;

#ifdef	USE_SHADOW
	shadow_tex_coord = gl_TexCoord[0];
	tile_tex_coord = gl_TexCoord[1].st;
	reflection_tex_coord = gl_TexCoord[2].xy / gl_TexCoord[2].w;
#else	// USE_SHADOW
	tile_tex_coord = gl_TexCoord[0].st;
	reflection_tex_coord = gl_TexCoord[1].xy / gl_TexCoord[1].w;
#endif	// USE_SHADOW

#ifdef	USE_NOISE
	vec3 noise_tex_coord;
	vec2 noise_diplacment;

	noise_tex_coord = vec3(gl_TexCoord[3].st, time);

	noise_diplacment = texture3D(noise_texture, noise_tex_coord).ga * 2.0 - 1.0;

	reflection_tex_coord += noise_diplacment * noise_scale.xy;
	tile_tex_coord += noise_diplacment * noise_scale.zw;
#endif	// USE_NOISE

#ifdef	USE_CUBIC_FILTER
	vec2 coord_hg = reflection_tex_coord * size - vec2(0.5, 0.5);
	vec3 hg_x = texture1D(hg_texture, coord_hg.x).xyz;
	vec3 hg_y = texture1D(hg_texture, coord_hg.y).xyz;

	vec2 reflection_tex_coord10 = reflection_tex_coord + hg_x.x * texel_size_x;
	vec2 reflection_tex_coord00 = reflection_tex_coord - hg_x.y * texel_size_x;
	vec2 reflection_tex_coord11 = reflection_tex_coord + hg_y.x * texel_size_y;
	vec2 reflection_tex_coord01 = reflection_tex_coord + hg_y.x * texel_size_y;
	reflection_tex_coord10 -= hg_y.y * texel_size_y;
	reflection_tex_coord00 -= hg_y.y * texel_size_y;

	vec4 color00 = texture2D(reflection_texture, reflection_tex_coord00);
	vec4 color10 = texture2D(reflection_texture, reflection_tex_coord10);
	vec4 color01 = texture2D(reflection_texture, reflection_tex_coord01);
	vec4 color11 = texture2D(reflection_texture, reflection_tex_coord11);

	color00 = mix(color00, color01, hg_y.z);
	color10 = mix(color10, color11, hg_y.z);
	color = mix(color00, color10, hg_x.z);
#else	// USE_CUBIC_FILTER
	color = texture2D(reflection_texture, reflection_tex_coord);
#endif	// USE_CUBIC_FILTER

#ifdef	USE_SHADOW
	shadow = shadow2DProj(shadow_texture, shadow_tex_coord);
	light = gl_LightSource[7].ambient + gl_LightModel.ambient;
 	light = mix(light, gl_Color, shadow);
#else	// USE_SHADOW
	light = gl_Color;
#endif	// USE_SHADOW

	gl_FragColor = light * texture2D(tile_texture, tile_tex_coord);
	gl_FragColor = mix(gl_FragColor, color, blend);

#ifdef USE_FOG
	const float LOG2 = 1.442695;
	float fog = gl_Fog.density*gl_FogFragCoord;
	fog = exp2(-fog*fog*LOG2);
	gl_FragColor = mix(gl_Fog.color, gl_FragColor, fog);
#endif // USE_FOG
}
