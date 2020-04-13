/*
	Provide functions to load and save configuration files using json format.

	Replacing previous binary formats with portable text files.
	See https://github.com/raduprv/Eternal-Lands/issues/71

	Author bluap/pjbroad April 2020
	Stay at home. Save lives. Protect the NHS.
*/
#include <iostream>
#include <fstream>
#include <string>
#include <iomanip>
#include <nlohmann/json.hpp>

#include "elloggingwrapper.h"
#include "counters.h"
#include "manufacture.h"
#include "text.h"

//	Helper functions
//
namespace JSON_IO
{
	static size_t get_json_indent(void)
		{ return 0; } // 0 is compact, non-zero give pretty output, 4 for example
	static int exit_error(const char *function, size_t line, std::string message, int error_code)
		{ LOG_ERROR("%s:%ld %s", function, line, message.c_str()); return error_code; }
	static void info_message(const char *function, size_t line, std::string message)
		{ LOG_INFO("%s:%ld %s", function, line, message.c_str()); }
	static void console_message(std::string file_type, std::string message)
		{ std::string full_message = "Problem with " + file_type + ": " + message; LOG_TO_CONSOLE(c_red1, full_message.c_str()); }
	static void file_format_error(std::string file_type)
		{ console_message(file_type, "File format error. " + file_type + " will not be saved until this is corrected."); }
}


namespace JSON_IO_Recipes
{
	using json = nlohmann::json;


	//	A Class to load and save manufacture recipes in json format.
	//
	class Recipes
	{
		public:
			Recipes(void) : opened(false), parse_error(false) {}
			int open(const char *file_name);
			int load(recipe_entry *recipes_store, size_t max_recipes);
			int save(const char *file_name, recipe_entry *recipes_store, size_t num_recipes, int current_recipe);
		private:
			bool opened;			// we have opened the file and populated the read_json object
			bool parse_error;		// there was an error populating the json object
			json read_json;			// the complete json object read from file
	};


	//	Read the json from file and return the number of recipes, or -1 for error.
	//
	int Recipes::open(const char *file_name)
	{
		JSON_IO::info_message(__PRETTY_FUNCTION__, __LINE__, " [" + std::string(file_name) + "]");

		std::ifstream in_file(file_name);
		if (!in_file)
			return JSON_IO::exit_error(__PRETTY_FUNCTION__, __LINE__, "Failed to open [" + std::string(file_name) + "]", -1);

		try
		{
			in_file >> read_json;
		}
		catch (json::exception& e)
		{
			parse_error = true;
			JSON_IO::file_format_error("Recipes");
			return JSON_IO::exit_error(__PRETTY_FUNCTION__, __LINE__, e.what(), -1);
		}

		if (!read_json["recipes"].is_array())
			return JSON_IO::exit_error(__PRETTY_FUNCTION__, __LINE__, "Missing recipes[]", -1);

		opened = true;
		return read_json["recipes"].size();
	}


	//	Parse the json read by open() and store into the recipe array.
	//	The recipe array must have been initialised to all zero - calloc().
	//	String memory is allocated here but must be freed by the caller.
	//	Missing fields should soft fail.
	//	Returns the active recipe, or -1 for an error.
	//
	int Recipes::load(recipe_entry *recipes_store, size_t max_recipes)
	{
		JSON_IO::info_message(__PRETTY_FUNCTION__, __LINE__, "");

		if (!recipes_store)
			return JSON_IO::exit_error(__PRETTY_FUNCTION__, __LINE__, "Recipe store is NULL", -1);

		if (!opened)
			return JSON_IO::exit_error(__PRETTY_FUNCTION__, __LINE__, "JSON object not open()ed", -1);

		size_t number_of_recipes = (read_json["recipes"].is_array()) ?read_json["recipes"].size() :0;
		int cur_recipe = 0;

		for (size_t i = 0; i < number_of_recipes && i < max_recipes; i++)
		{
			json recipe = read_json["recipes"][i];
			if (recipe.is_null())
				continue;
			json items = recipe["items"];
			if (!items.is_array())
				continue;
			for (size_t j = 0; j < NUM_MIX_SLOTS && j < items.size(); j++)
			{
				json the_item = items[j];
				if (the_item.is_null())
					continue;
				recipes_store[i].items[j].id = (the_item["id"].is_number_unsigned()) ?the_item["id"].get<Uint16>() :unset_item_uid;
				recipes_store[i].items[j].image_id = (the_item["image_id"].is_number_integer()) ?the_item["image_id"].get<int>() :0;
				recipes_store[i].items[j].quantity = (the_item["quantity"].is_number_integer()) ?the_item["quantity"].get<int>() :0;
			}
			if (!recipe["name"].is_string())
				continue;
			std::string name = recipe["name"].get<std::string>();
			if (name.size() > 0)
			{
				recipes_store[i].name = static_cast<char *>(malloc(name.size() + 1));
				strcpy(recipes_store[i].name, name.c_str());
			}
			if (recipe["current"].is_boolean())
				cur_recipe = i;
		}

		// we are done with the json object so free the memory
		read_json.clear();
		opened = false;

		return cur_recipe;
	}


	//	Save the recipe data in json format.
	//	Empty slots in a recipe are not saved.
	//	Ids that are unset, are not saved.
	//	Null names are saved as empty strings.
	//	Return 0 if successful otherwise -1.
	//
	int Recipes::save(const char *file_name, recipe_entry *recipes_store, size_t num_recipes, int current_recipe)
	{
		JSON_IO::info_message(__PRETTY_FUNCTION__, __LINE__, " [" + std::string(file_name) + "]");

		if (parse_error)
			return JSON_IO::exit_error(__PRETTY_FUNCTION__, __LINE__, "Not saving, because we had a load error.  Fix the problem first.", -1);

		json write_json;

		json recipe_list = json::array();
		for (size_t i=0; i<num_recipes; i++)
		{
			json recipe;
			json item_list = json::array();
			for (size_t j=0; j<NUM_MIX_SLOTS; j++)
			{
				if (recipes_store[i].items[j].quantity <= 0)
					continue;
				json one_item;
				if (recipes_store[i].items[j].id != unset_item_uid)
					one_item["id"] = recipes_store[i].items[j].id;
				one_item["image_id"] = recipes_store[i].items[j].image_id;
				one_item["quantity"] = recipes_store[i].items[j].quantity;
				item_list.push_back(one_item);
			}
			recipe["items"] = item_list;
			recipe["name"] = (recipes_store[i].name) ?recipes_store[i].name :"";
			if (current_recipe == static_cast<int>(i))
				recipe["current"] = true;
			recipe_list.push_back(recipe);
		}
		write_json["recipes"] = recipe_list;

		std::ofstream out_file(file_name);
		if (out_file)
		{
			out_file << std::setw(JSON_IO::get_json_indent()) << write_json << std::endl;
			return 0;
		}
		else
			return JSON_IO::exit_error(__PRETTY_FUNCTION__, __LINE__, "Failed to write json [" + std::string(file_name) + "]", -1);
	}

} // end JSON_IO_Recipes namespace


namespace JSON_IO_Quickspells
{
	using json = nlohmann::json;


	//	A Class to load and save quickspells in json format.
	//
	class Quickspells
	{
		public:
			Quickspells(void) : parse_error(false) {}
			int load(const char *file_name, int *spell_ids, size_t max_num_spell_id);
			int save(const char *file_name, Uint16 *spell_ids, size_t num_spell_id);
		private:
			bool parse_error;		// there was an error populating the json object
	};


	//	Load the quickspells ids up to the maximum.
	//	Return the number read or -1 for an error.
	//
	int Quickspells::load(const char *file_name, int *spell_ids, size_t max_num_spell_id)
	{
		JSON_IO::info_message(__PRETTY_FUNCTION__, __LINE__, " [" + std::string(file_name) + "]");

		std::ifstream in_file(file_name);
		if (!in_file)
			return JSON_IO::exit_error(__PRETTY_FUNCTION__, __LINE__, "Failed to open [" + std::string(file_name) + "]", -1);

		json read_json;

		try
		{
			in_file >> read_json;
		}
		catch (json::exception& e)
		{
			parse_error = true;
			JSON_IO::file_format_error("Quickspells");
			return JSON_IO::exit_error(__PRETTY_FUNCTION__, __LINE__, e.what(), -1);
		}

		if (!read_json["quickspells"].is_array())
			return JSON_IO::exit_error(__PRETTY_FUNCTION__, __LINE__, "Missing quickspells[]", -1);

		for (size_t i = 0; i < read_json["quickspells"].size() && i < max_num_spell_id; i++)
			spell_ids[i] = (read_json["quickspells"][i].is_number_integer()) ?read_json["quickspells"][i].get<int>() :-1;

		return (read_json["quickspells"].size() < max_num_spell_id) ?read_json["quickspells"].size() :max_num_spell_id;
	}


	//	Save the quickspells ids.
	//	Return 0, or -1 on an error
	//
	int Quickspells::save(const char *file_name, Uint16 *spell_ids, size_t num_spell_id)
	{
		JSON_IO::info_message(__PRETTY_FUNCTION__, __LINE__, " [" + std::string(file_name) + "]");

		if (parse_error)
			return JSON_IO::exit_error(__PRETTY_FUNCTION__, __LINE__, "Not saving, because we had a load error.  Fix the problem first.", -1);

		json write_json;

		json quickspells_list = json::array();
		for (size_t i = 0; i < num_spell_id; i++)
			quickspells_list.push_back(spell_ids[i]);
		write_json["quickspells"] = quickspells_list;

		std::ofstream out_file(file_name);
		if (out_file)
		{
			out_file << std::setw(JSON_IO::get_json_indent()) << write_json << std::endl;
			return 0;
		}
		else
			return JSON_IO::exit_error(__PRETTY_FUNCTION__, __LINE__, "Failed to write json [" + std::string(file_name) + "]", -1);
	}

} // end JSON_IO_Quickspells namespace


namespace JSON_IO_Counters
{
	using json = nlohmann::json;


	//	A Class to load and save the counters in json format.
	//
	class Counters
	{
		public:
			Counters(void) : parse_error(false) {}
			int load(const char *file_name, const char **cat_str, int *entries, size_t num_categories, struct Counter **the_counters);
			int save(const char *file_name, const char **cat_str, const int *entries, size_t num_categories, const struct Counter **the_counters);
		private:
			bool parse_error;		// there was an error populating the json object
	};


	//	Load the counters.
	//	Memory allocated here, must be freed by the caller.
	//	Return the number of categories actually read, or -1 on error.
	//
	int Counters::load(const char *file_name, const char **cat_str, int *entries, size_t num_categories, struct Counter **the_counters)
	{
		JSON_IO::info_message(__PRETTY_FUNCTION__, __LINE__, " [" + std::string(file_name) + "]");

		std::ifstream in_file(file_name);
		if (!in_file)
			return JSON_IO::exit_error(__PRETTY_FUNCTION__, __LINE__, "Failed to open [" + std::string(file_name) + "]", -1);

		json read_json;

		try
		{
			in_file >> read_json;
		}
		catch (json::exception& e)
		{
			parse_error = true;
			JSON_IO::file_format_error("Counters");
			return JSON_IO::exit_error(__PRETTY_FUNCTION__, __LINE__, e.what(), -1);
		}

		if (!read_json["categories"].is_array())
			return JSON_IO::exit_error(__PRETTY_FUNCTION__, __LINE__, "Missing categories[]", -1);

		for (size_t i = 0; i < read_json["categories"].size() && i < num_categories; i++)
		{
			if (!read_json["categories"][i]["name"].is_string() || (read_json["categories"][i]["name"] != std::string(cat_str[i])))
				continue;
			json entries_list = read_json["categories"][i]["entries"];
			if (!entries_list.is_array())
				continue;
			for (size_t j = 0; j < entries_list.size(); j++)
			{
				json entry = entries_list[j];
				if (!entry["name"].is_string())
					continue;
				std::string name = entry["name"].get<std::string>();
				if (name.empty())
					continue;
				entries[i]++;
				the_counters[i] = (struct Counter *)realloc(the_counters[i], entries[i] * sizeof(struct Counter));
				the_counters[i][j].name = static_cast<char *>(malloc(name.size() + 1));
				strcpy(the_counters[i][j].name, name.c_str());
				the_counters[i][j].n_session = 0;
				the_counters[i][j].n_total = (entry["n_total"].is_number_unsigned()) ?entry["n_total"].get<Uint32>() :0;
				the_counters[i][j].extra = (entry["extra"].is_number_unsigned()) ?entry["extra"].get<Uint32>() :0;
			}
		}

		return (read_json["categories"].size() < num_categories) ?read_json["categories"].size() :num_categories;
	}


	//	Save the counters.
	//	Return 0, or -1 on error.
	//
	int Counters::save(const char *file_name, const char **cat_str, const int *entries, size_t num_categories, const struct Counter **the_counters)
	{
		JSON_IO::info_message(__PRETTY_FUNCTION__, __LINE__, " [" + std::string(file_name) + "]");

		if (parse_error)
			return JSON_IO::exit_error(__PRETTY_FUNCTION__, __LINE__, "Not saving, because we had a load error.  Fix the problem first.", -1);

		json categories_list = json::array();
		for (size_t i = 0; i < num_categories; i++)
		{
			json category;
			category["name"] = cat_str[i];
			json entries_list = json::array();
			for (int j = 0; j < entries[i]; j++)
			{
				json entry;
				entry["name"] = the_counters[i][j].name;
				entry["n_total"] = the_counters[i][j].n_total;
				if (the_counters[i][j].extra > 0)
					entry["extra"] = the_counters[i][j].extra;
				entries_list.push_back(entry);
			}
			category["entries"] = entries_list;
			categories_list.push_back(category);
		}

		json write_json;
		write_json["categories"] = categories_list;

		std::ofstream out_file(file_name);
		if (out_file)
		{
			out_file << std::setw(JSON_IO::get_json_indent()) << write_json << std::endl;
			return 0;
		}
		else
			return JSON_IO::exit_error(__PRETTY_FUNCTION__, __LINE__, "Failed to write json [" + std::string(file_name) + "]", -1);
	}

} //end JSON_IO_Counters namespace


//	The instance of the manufacture recipe object.
static JSON_IO_Recipes::Recipes recipes;

//	The instance of the quickspells object.
static JSON_IO_Quickspells::Quickspells quickspells;

//	The instance of the counters object.
static JSON_IO_Counters::Counters counters;


//	The C interface
//
extern "C"
{
	// manufacture recipe functions
	int json_open_recipes(const char *file_name)
		{ return recipes.open(file_name); }
	int json_load_recipes(recipe_entry *recipes_store, size_t max_recipes)
		{ return recipes.load(recipes_store, max_recipes); }
	int json_save_recipes(const char *file_name, recipe_entry *recipes_store, size_t num_recipes, int current_recipe)
		{ return recipes.save(file_name, recipes_store, num_recipes, current_recipe); }

	// quickspells funcitons
	int json_load_quickspells(const char *file_name, int *spell_ids, size_t max_num_spell_id)
		{ return quickspells.load(file_name, spell_ids, max_num_spell_id); }
	int json_save_quickspells(const char *file_name, Uint16 *spell_ids, size_t num_spell_id)
		{ return quickspells.save(file_name, spell_ids, num_spell_id); }

	// counters functions
	int json_load_counters(const char *file_name, const char **cat_str, int *entries, size_t num_categories, struct Counter **the_counters)
		{ return counters.load(file_name, cat_str, entries, num_categories, the_counters); }
	int json_save_counters(const char *file_name, const char **cat_str, const int *entries, size_t num_categories, const struct Counter **the_counters)
		{ return counters.save(file_name, cat_str, entries, num_categories, the_counters); }
}
