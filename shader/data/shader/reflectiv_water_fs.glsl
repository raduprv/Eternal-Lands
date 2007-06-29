uniform sampler2D tile_texture;
uniform sampler2D reflection_texture;
uniform sampler3D noise_texture;
uniform vec4 noise_scale;
uniform float time;
uniform sampler2DShadow shadow_texture;
uniform float blend;

void main (void) 
{
	vec4 color, light, shadow_tex_pos;
	vec4 reflection_tex_coord;
#ifdef	USE_SHADOW
	vec4 shadow_tex_coord;
#endif	// USE_SHADOW
	vec2 tile_tex_coord;
	float shadow;

#ifdef	USE_SHADOW
	shadow_tex_coord = gl_TexCoord[0];
	tile_tex_coord = gl_TexCoord[1].st;
	reflection_tex_coord = gl_TexCoord[2];
#else	// USE_SHADOW
	tile_tex_coord = gl_TexCoord[0].st;
	reflection_tex_coord = gl_TexCoord[1];
#endif	// USE_SHADOW

#ifdef	USE_NOISE
	vec3 noise_tex_coord;
	vec2 noise_diplacment;

	noise_tex_coord = vec3(gl_TexCoord[3].st, time);

	noise_diplacment = texture3D(noise_texture, noise_tex_coord).ga * 2.0 - 1.0;

	reflection_tex_coord += vec4(noise_diplacment * noise_scale.xy, 0.0, 0.0) * reflection_tex_coord.w;
	tile_tex_coord += noise_diplacment * noise_scale.zw;
#endif	// USE_NOISE

	color = texture2DProj(reflection_texture, reflection_tex_coord);

	color = mix(texture2D(tile_texture, tile_tex_coord), color, blend);

#ifdef	USE_SHADOW
	shadow = shadow2DProj(shadow_texture, shadow_tex_coord).r;
	light = mix(gl_FrontLightModelProduct.sceneColor, gl_Color, shadow);
#else	// USE_SHADOW
	light = gl_Color;
#endif	// USE_SHADOW

	gl_FragColor = light * color;
}

