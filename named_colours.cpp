#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <cstring>
#include <omp.h>

#include "named_colours.h"

namespace ELGL_Colour
{
	class Vector
	{
		public:
			Vector(GLfloat r, GLfloat g, GLfloat b) { c[0] = r; c[1] = g, c[2] = b, c[3] = 1.0; }
			Vector(GLfloat r, GLfloat g, GLfloat b, GLfloat a) { c[0] = r; c[1] = g, c[2] = b, c[3] = a;}
#if defined(ELC)
			void use(void) const { glColor4fv(c); }
#else
			void use(void) const { std::cout << "glColor4fv(" << c[0] << "," << c[1] << "," << c[2] << "," << c[3] << ");" << std::endl; }
#endif
			void get3v(GLfloat *buf) const { memcpy(buf, c, sizeof(GLfloat)*3); }
			void get4v(GLfloat *buf) const { memcpy(buf, c, sizeof(GLfloat)*4); }
		private:
			GLfloat c[4];
	};


	class Colours
	{
		public:
			Colours(void) { add("null", Vector(0.0f,0.0f,0.0f)); }
			void load(void);
			void use(const char *name) const { use(get(name)); }
			void use(size_t colour) const { colours_by_index[colour].use(); }
			size_t get(const char *name) const;
			void get3v(const char *name, GLfloat *buf) const { colours_by_index[get(name)].get3v(buf); }
			void get4v(const char *name, GLfloat *buf) const { colours_by_index[get(name)].get4v(buf); }
		private:
			void add(const char *name, Vector colours);
			std::map<const std::string, size_t> colours_by_name;
			std::vector<Vector> colours_by_index;
	};


	size_t Colours::get(const char *name) const
	{
		std::map<const std::string, size_t>::const_iterator it = colours_by_name.find(name);
		return (it == colours_by_name.end()) ?ELGL_INVALID_COLOUR :it->second;
	}

	void Colours::add(const char *name, Vector colours)
	{
		if (get(name) == ELGL_INVALID_COLOUR)
		{
			colours_by_index.push_back(colours);
			colours_by_name[name] = colours_by_index.size() - 1;
		}
	}


	void Colours::load(void)
	{
		add("minimap.npc", Vector(0.0f, 0.0f ,1.0));
		add("minimap.yourself", Vector(0.0f,1.0f,0.0f));
		add("minimap.pkable",Vector(1.0f,0.0f,0.0f));
		add("minimap.buddy", Vector(0.0f,0.9f,1.0f));
		add("minimap.deadcreature", Vector(0.8f, 0.8f, 0.0f));
		add("minimap.creature", Vector(1.0f, 1.0f, 0.0f));
		add("minimap.mine", Vector(0.15f, 0.65f, 0.45f));
		add("minimap.servermark", Vector(0.33f,0.6f,1.0f));
		add("minimap.otherplayer", Vector(1.0f, 1.0f, 1.0f));
		add("withalpha", Vector(1.0f, 1.0f, 1.0f, 0.5));
#if !defined(ELC)
		std::cout << "Loaded size by_index=" << colours_by_index.size() << " by_name=" << colours_by_name.size() << std::endl;
#endif
	}


} // end namespace

static ELGL_Colour::Colours colours;

// The external interface
extern "C"
{
	size_t ELGL_INVALID_COLOUR = 0;
	void init_named_colours(void) { colours.load(); }
	void elglColourI(size_t index) { colours.use(index); }
	void elglColourN(const char *name) { colours.use(name); }
	size_t elglGetColourIndex(const char *name) { return colours.get(name); }
	void elglGetColour3v(const char *name, GLfloat *buf) { colours.get3v(name, buf); }
	void elglGetColour4v(const char *name, GLfloat *buf) { colours.get4v(name, buf); }
}

#if !defined(ELC)
// test code
int main(int argc, char *argv[])
{
	double timer_start = omp_get_wtime();
	elglLoad();
	size_t my_colour = elglGetColourIndex("minimap.mine");
	elglColourI(my_colour);
	elglColourN("minimap.mine");
	elglColourN("xyz");
	elglColourN("withalpha");
	elglColourN("minimap.yourself");
	// this will cause an "Invalid read", not protected for speed
	// elglColourI(999);

	GLfloat c3[3] = { 0.1, 0.5, 0.7 };
	std::cout << c3[0] << ", " << c3[1] << ", " << c3[2] << std::endl;
	elglGetColour3v("other", c3);
	std::cout << c3[0] << ", " << c3[1] << ", " << c3[2] << std::endl;

	GLfloat c4[4] = { 0.1, 0.5, 0.7, 0.34 };
	std::cout << c4[0] << ", " << c4[1] << ", " << c4[2] << ", " << c4[3] << std::endl;
	elglGetColour4v("withalpha", c4);
	std::cout << c4[0] << ", " << c4[1] << ", " << c4[2] << ", " << c4[3] << std::endl;
	std::cout << timer_start << " " << omp_get_wtime() << std::endl;
}
#endif
