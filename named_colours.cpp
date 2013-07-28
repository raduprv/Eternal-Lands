/*
	Store named colours

	Colour values are read from file and referenced by name.  The
	user can set the active GL colour either directly by name, or using
	an unique id looked up by name.  Alternatively, the colour tuple
	can be retrieved by name and stored locally.

	Author bluap/pjbroad June 2013
*/

#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <cstring>
#include <sstream>

#include "asc.h"
#include "elloggingwrapper.h"
#include "named_colours.h"
#include "io/elfilewrapper.h"

namespace ELGL_Colour
{
	//	A single colour tuple.
	// 
	class Colour_Tuple
	{
		public:
			Colour_Tuple(GLfloat r, GLfloat g, GLfloat b) { c[0] = r; c[1] = g, c[2] = b, c[3] = 1.0; }
			Colour_Tuple(GLfloat r, GLfloat g, GLfloat b, GLfloat a) { c[0] = r; c[1] = g, c[2] = b, c[3] = a;}
			void use(void) const { glColor4fv(c); }
			void get3v(GLfloat *buf) const { memcpy(buf, c, sizeof(GLfloat)*3); }
			void get4v(GLfloat *buf) const { memcpy(buf, c, sizeof(GLfloat)*4); }
		private:
			GLfloat c[4];
	};


	//	Container for colour tuples, maintains names and unique id.
	//	Access by the unique id is probably faster as it is purely an
	//	index into an array.  If you are really concerned with speed,
	//	get the colour tuple (by name) and store it yourself.
	//
	//	Note, a by-name lookup will return the null colour (black) if
	//	the name does not exist.  Using an invalid id, will cause an out
	//	of bounds access error.  You must get the id from this class.
	//
	class Colour_Container
	{
		public:
			Colour_Container(void) { add("null", Colour_Tuple(0.0f,0.0f,0.0f)); }
			void load(void) { load_xml(); load_default(); }
			void use(const char *name) const { use(get(name)); }
			void use(size_t colour) const { colours_by_index[colour].use(); }
			size_t get(const char *name) const;
			void get3v(const char *name, GLfloat *buf) const { colours_by_index[get(name)].get3v(buf); }
			void get4v(const char *name, GLfloat *buf) const { colours_by_index[get(name)].get4v(buf); }
		private:
			void load_xml(void);
			void load_default(void);
			void add(const char *name, Colour_Tuple colour);
			std::map<const std::string, size_t> colours_by_name;
			std::vector<Colour_Tuple> colours_by_index;
	};


	//	Get the colour id (index) by name.
	//
	size_t Colour_Container::get(const char *name) const
	{
		std::map<const std::string, size_t>::const_iterator it = colours_by_name.find(name);
		return (it == colours_by_name.end()) ?ELGL_INVALID_COLOUR :it->second;
	}


	//	Add a new colour to the container, if it does not already exist.
	//
	void Colour_Container::add(const char *name, Colour_Tuple colour)
	{
		if (get(name) == ELGL_INVALID_COLOUR)
		{
			colours_by_index.push_back(colour);
			colours_by_name[name] = colours_by_index.size() - 1;
		}
	}


	//	Load colours from the XML file.
	//
	void Colour_Container::load_xml(void)
	{
		char const *error_prefix = __PRETTY_FUNCTION__;
		std::string file_name = "named_colours.xml";

		if (!el_file_exists_anywhere(file_name.c_str()))
			return;

		xmlDocPtr doc;
		xmlNodePtr cur;

		if ((doc = xmlReadFile(file_name.c_str(), NULL, 0)) == NULL)
		{
			LOG_ERROR("%s : Can't open file [%s]\n", error_prefix, file_name.c_str() );
			return;
		}

		if ((cur = xmlDocGetRootElement (doc)) == NULL)
		{
			LOG_ERROR("%s : Empty xml document\n", error_prefix );
			xmlFreeDoc(doc);
			return;
		}

		if (xmlStrcasecmp (cur->name, (const xmlChar *) "named_colours"))
		{
			LOG_ERROR("%s : no named_colours element\n", error_prefix );
			xmlFreeDoc(doc);
			return;
		}

		for (cur = cur->xmlChildrenNode; cur; cur = cur->next)
		{
			if (!xmlStrcasecmp(cur->name, (const xmlChar *)"colour"))
			{
				const int NUM_STR = 5;
				char *read_strings[NUM_STR] = {NULL, NULL, NULL, NULL, NULL};
				const char *names[NUM_STR] = {"name", "r", "g", "b", "a"};
				for (int i=0; i<NUM_STR; i++)
				{
					char *tmp = (char*)xmlGetProp(cur, (xmlChar *)names[i]);
					if (!tmp)
						continue;
					MY_XMLSTRCPY(&read_strings[i], tmp);
					xmlFree(tmp);
				}
				if (read_strings[0])
				{
					GLfloat col_vals[NUM_STR-1] = { -1.0f, -1.0f, -1.0f, 1.0f};
					for (int i=1; i<NUM_STR; i++)
					{
						if (read_strings[i])
						{
							GLfloat tmpcol = -1.0f;
							std::stringstream ss(read_strings[i]);
							if( (ss >> tmpcol).fail() )
								continue;
							col_vals[i-1] = tmpcol;
						}
					}
					bool valid = true;
					for (int i=0; i<NUM_STR-1; i++)
						if (col_vals[i] < 0)
							valid = false;
					if (valid && (NUM_STR == 5))
						add(read_strings[0], Colour_Tuple(col_vals[0],col_vals[1],col_vals[2],col_vals[3]));
				}
				for (int i=0; i<NUM_STR; i++)
					if (read_strings[i])
						free(read_strings[i]);
			}
		}
		xmlFreeDoc(doc);
	}


	//	Load some default colours.
	//	These values are used after the XML file so will not replace
	//	colours with the same name.
	//
	void Colour_Container::load_default(void)
	{
		add("minimap.npc", Colour_Tuple(0.0f, 0.0f ,1.0));
		add("minimap.yourself", Colour_Tuple(0.0f,1.0f,0.0f));
		add("minimap.pkable",Colour_Tuple(1.0f,0.0f,0.0f));
		add("minimap.buddy", Colour_Tuple(0.0f,0.9f,1.0f));
		add("minimap.deadcreature", Colour_Tuple(0.8f, 0.8f, 0.0f));
		add("minimap.creature", Colour_Tuple(1.0f, 1.0f, 0.0f));
		add("minimap.mine", Colour_Tuple(0.15f, 0.65f, 0.45f));
		add("minimap.servermark", Colour_Tuple(0.33f,0.6f,1.0f));
		add("minimap.otherplayer", Colour_Tuple(1.0f, 1.0f, 1.0f));
		add("global.mousehighlight", Colour_Tuple(0.5f, 0.5f, 1.0f));
		add("global.mouseselected", Colour_Tuple(0.4f, 0.8f, 1.0f));
	}

} // end namespace

static ELGL_Colour::Colour_Container colours;

// The external interface
extern "C"
{
	size_t ELGL_INVALID_COLOUR = 0;
	void init_named_colours(void) { colours.load(); }
	void elglColourI(size_t index) { colours.use(index); }
	void elglColourN(const char *name) { colours.use(name); }
	size_t elglGetColourId(const char *name) { return colours.get(name); }
	void elglGetColour3v(const char *name, GLfloat *buf) { colours.get3v(name, buf); }
	void elglGetColour4v(const char *name, GLfloat *buf) { colours.get4v(name, buf); }
}
