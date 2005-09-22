#ifdef	TERRAIN
#include <stdlib.h>
#include <stdio.h>
#include <strings.h>
#include "global.h"

static inline void load_shader(GLhandleARB ShaderObject, char* filename, char* flags)
{
	int shader_size, buffer_size, flags_size;
	FILE *file;
	char* buffer;
	
	file = fopen(filename, "rb");
	
	fseek(file, 0L, SEEK_END);
	shader_size = ftell(file);
	fseek(file, 0L, SEEK_SET);
#if	1	
	flags_size = strlen(flags);
	buffer_size = shader_size + flags_size;
	buffer = (char*)malloc(buffer_size);
	
	memcpy(buffer, flags, flags_size);

	fread(&buffer[flags_size], 1, shader_size, file);
	
	fclose (file);
	
	ELglShaderSourceARB(ShaderObject, 1, (const char**)&buffer, &buffer_size);

#else
	buffer = (char*)malloc(shader_size);
	
	fread(buffer, 1, shader_size, file);
	
	fclose (file);
	
	ELglShaderSourceARB(ShaderObject, 1, &buffer, &shader_size);
#endif	
	free(buffer);
}


GLhandleARB init_normal_mapping_shader()
{
	GLhandleARB ProgramObject, VertexShaderObject, FragmentShaderObject;

	ProgramObject = ELglCreateProgramObjectARB();

	VertexShaderObject = ELglCreateShaderObjectARB(GL_VERTEX_SHADER_ARB);
	FragmentShaderObject = ELglCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);

	load_shader(VertexShaderObject, "shader/normal_mapping_vs.gls", "");
	load_shader(FragmentShaderObject, "shader/normal_mapping_fs.gls", "");

	ELglCompileShaderARB(VertexShaderObject);
	ELglCompileShaderARB(FragmentShaderObject);

	ELglAttachObjectARB(ProgramObject, VertexShaderObject);
	ELglAttachObjectARB(ProgramObject, FragmentShaderObject);	

	ELglDeleteObjectARB(VertexShaderObject);
	ELglDeleteObjectARB(FragmentShaderObject);

	ELglLinkProgramARB(ProgramObject);
	return ProgramObject;
}

void free_shader(GLhandleARB ProgramObject)
{
	ELglDeleteProgramsARB(1, &ProgramObject);
}
#endif
