#include <libxml/parser.h>
#include "elfilewrapper.h"
#include "xmlcallbacks.h"
#include "../errors.h"

int el_xml_input_match(char const *file_name)
{
#ifdef ANDROID
	return (el_open (file_name))?1:0;
#else
	return el_file_exists_anywhere (file_name);
#endif
}

void *el_xml_input_open(char const *file_name)
{
#ifdef ANDROID
	return el_open(file_name);
#else
	return el_open_anywhere (file_name);
#endif
}

int el_xml_input_read(void *context, char *buffer, int len)
{
	return el_read(context, len, buffer);
}

int el_xml_input_close(void *context)
{
	el_close(context);

	return 1;
}

void xml_register_el_input_callbacks()
{
	xmlRegisterInputCallbacks(el_xml_input_match, el_xml_input_open,
		el_xml_input_read, el_xml_input_close);
}

