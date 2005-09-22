#ifdef	TERRAIN
#include <stdlib.h>
#include <stdio.h>
#include <strings.h>
#include <GL/gl.h>
#include <GL/glext.h>
#include "shader.h"

static inline void load_shader(GLhandleARB ShaderObject, char* filename, char* flags)
{
	unsigned int shader_size, buffer_size, flags_size;
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
	
	glShaderSourceARB(ShaderObject, 1, &buffer, &buffer_size);

#else
	buffer = (char*)malloc(shader_size);
	
	fread(buffer, 1, shader_size, file);
	
	fclose (file);
	
	glShaderSourceARB(ShaderObject, 1, &buffer, &shader_size);
#endif	
	free(buffer);
}


void printf_shader_log(GLhandleARB glObject)
{
	unsigned int blen, slen;
	GLcharARB* InfoLog;
	
	glGetObjectParameterivARB(glObject, GL_OBJECT_INFO_LOG_LENGTH_ARB , &blen);
	
	if (blen > 1)
	{
		InfoLog = (GLcharARB*)malloc(blen*sizeof(GLcharARB));
		glGetInfoLogARB(glObject, blen, &slen, InfoLog);
		printf("\n%s\n", InfoLog);
		free(InfoLog);		
	}
	else
	{
		printf("\n--\n");
	}
}

GLhandleARB init_normal_mapping_shader()
{
	GLhandleARB ProgramObject, VertexShaderObject, FragmentShaderObject;

	ProgramObject = glCreateProgramObjectARB();

	VertexShaderObject = glCreateShaderObjectARB(GL_VERTEX_SHADER_ARB);
	FragmentShaderObject = glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);

	load_shader(VertexShaderObject, "shader/normal_mapping_vs.gls", "");
	load_shader(FragmentShaderObject, "shader/normal_mapping_fs.gls", "");

	glCompileShaderARB(VertexShaderObject);
	glCompileShaderARB(FragmentShaderObject);

	glAttachObjectARB(ProgramObject, VertexShaderObject);
	glAttachObjectARB(ProgramObject, FragmentShaderObject);	

	glDeleteObjectARB(VertexShaderObject);
	glDeleteObjectARB(FragmentShaderObject);

	glLinkProgramARB(ProgramObject);
	printf_shader_log(ProgramObject);
	return ProgramObject;
}

void free_shader(GLhandleARB ProgramObject)
{
	glDeleteObjectARB(ProgramObject);
}
#endif
