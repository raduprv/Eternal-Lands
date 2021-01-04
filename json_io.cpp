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

#include "chat.h"
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
	static int exit_error(const char *function, size_t line, const std::string& message, int error_code)
		{ LOG_ERROR("%s:%ld %s", function, line, message.c_str()); return error_code; }
	static void info_message(const char *function, size_t line, std::string message)
		{ LOG_INFO("%s:%ld %s", function, line, message.c_str()); }
	static void console_message(const std::string& file_type, const std::string& message)
		{ std::string full_message = "Problem with " + file_type + ": " + message; LOG_TO_CONSOLE(c_red3, full_message.c_str()); }
	static void file_format_error(const std::string& file_type)
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
			const char * class_name_str = "Recipes";
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
			JSON_IO::file_format_error(class_name_str);
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
		{
			JSON_IO::file_format_error(class_name_str);
			return JSON_IO::exit_error(__PRETTY_FUNCTION__, __LINE__, "Not saving, because we had a load error.  Fix the problem first.", -1);
		}

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
			const char * class_name_str = "Quickspells";
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
			JSON_IO::file_format_error(class_name_str);
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
		{
			JSON_IO::file_format_error(class_name_str);
			return JSON_IO::exit_error(__PRETTY_FUNCTION__, __LINE__, "Not saving, because we had a load error.  Fix the problem first.", -1);
		}

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
			const char * class_name_str = "Counters";
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
			JSON_IO::file_format_error(class_name_str);
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
		{
			JSON_IO::file_format_error(class_name_str);
			return JSON_IO::exit_error(__PRETTY_FUNCTION__, __LINE__, "Not saving, because we had a load error.  Fix the problem first.", -1);
		}

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


namespace JSON_IO_Channel_Colours
{
	using json = nlohmann::json;


	//	A Class to load and save the the channel colours in json format.
	//
	class Channel_Colours
	{
		public:
			Channel_Colours(void) : parse_error(false) {}
			int load(const char *file_name, channelcolor *the_channel_colours, size_t max_channel_colours);
			int save(const char *file_name, const channelcolor *the_channel_colours, size_t max_channel_colours);
		private:
			bool parse_error;		// there was an error populating the json object
			const char * class_name_str = "Channel Colours";
	};


	//	Load the channel colours.
	//	Assumed that he channel colour arrar has been initialised.
	//	Return the number of sets actually read, or -1 on error.
	//
	int Channel_Colours::load(const char *file_name, channelcolor *the_channel_colours, size_t max_channel_colours)
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
			JSON_IO::file_format_error(class_name_str);
			return JSON_IO::exit_error(__PRETTY_FUNCTION__, __LINE__, e.what(), -1);
		}

		if (!read_json["channel_colours"].is_array())
			return JSON_IO::exit_error(__PRETTY_FUNCTION__, __LINE__, "Missing channel_colours[]", -1);

		for (size_t i = 0; i < read_json["channel_colours"].size() && i < max_channel_colours; i++)
		{
			json channel_colour_set = read_json["channel_colours"][i];
			if (!channel_colour_set["channel"].is_number_unsigned() || !channel_colour_set["colour"].is_number_integer() ||
					(channel_colour_set["colour"] < 0))
				continue;
			the_channel_colours[i].nr = channel_colour_set["channel"];
			the_channel_colours[i].color = channel_colour_set["colour"];
		}

		return (read_json["channel_colours"].size() < max_channel_colours) ?read_json["channel_colours"].size() :max_channel_colours;
	}


	//	Save the channel colours.
	//	Return 0, or -1 on error.
	//
	int Channel_Colours::save(const char *file_name, const channelcolor *the_channel_colours, size_t max_channel_colours)
	{
		JSON_IO::info_message(__PRETTY_FUNCTION__, __LINE__, " [" + std::string(file_name) + "]");

		if (parse_error)
		{
			JSON_IO::file_format_error(class_name_str);
			return JSON_IO::exit_error(__PRETTY_FUNCTION__, __LINE__, "Not saving, because we had a load error.  Fix the problem first.", -1);
		}

		json write_json;

		json channel_colours_list = json::array();
		for (size_t i = 0; i < max_channel_colours; i++)
		{
			if (the_channel_colours[i].color < 0)
				continue;
			json channel_colour_set;
			channel_colour_set["channel"] = the_channel_colours[i].nr;
			channel_colour_set["colour"] = the_channel_colours[i].color;
			channel_colours_list.push_back(channel_colour_set);
		}
		write_json["channel_colours"] = channel_colours_list;

		std::ofstream out_file(file_name);
		if (out_file)
		{
			out_file << std::setw(JSON_IO::get_json_indent()) << write_json << std::endl;
			return 0;
		}
		else
			return JSON_IO::exit_error(__PRETTY_FUNCTION__, __LINE__, "Failed to write json [" + std::string(file_name) + "]", -1);
	}
}


namespace JSON_IO_Character_Options
{
	using json = nlohmann::json;


	//	A Class to load and save the character options in json format.
	//	Character options if present override values in the ini file.
	//
	class Character_Options
	{
		public:
			Character_Options(void) : parse_error(false), loaded(false), modified(false) {}
			void set_file_name(const char *file_name) { the_file_name = std::string(file_name); }
			int load(void);
			int save(void);
			template <class TheType> TheType get(const char *var_name, TheType default_var_value) const;
			template <class TheType> void set(const char *var_name, TheType value);
			bool exists(const char *var_name) const;
			void remove(const char *var_name);
		private:
			bool have_options_list(void) const { return ((json_data.find(ini_options_list_str) != json_data.end()) && json_data[ini_options_list_str].is_array()); }
			bool parse_error;		// there was an error populating the json object
			bool loaded;			// true if the file is already loaded
			bool modified;			// if true, the object has been modified and needs to be saved
			json json_data;			// the json object as read from file, or empty if no file
			std::string the_file_name;
			const char * class_name_str = "Character Options";
			const char * ini_options_list_str = "ini_options";
			const char * name_str = "name";
			const char * value_str = "value";
	};


	//	Load the character options.
	//	Return 0 or -1 on error.
	//
	int Character_Options::load(void)
	{
		if (loaded)
			return 0;

		JSON_IO::info_message(__PRETTY_FUNCTION__, __LINE__, " [" + the_file_name + "]");

		std::ifstream in_file(the_file_name);
		if (!in_file)
			return JSON_IO::exit_error(__PRETTY_FUNCTION__, __LINE__, "Failed to open [" + the_file_name + "]", -1);

		try
		{
			in_file >> json_data;
		}
		catch (json::exception& e)
		{
			parse_error = true;
			JSON_IO::file_format_error(class_name_str);
			return JSON_IO::exit_error(__PRETTY_FUNCTION__, __LINE__, e.what(), -1);
		}

		loaded = true;

		return 0;
	}


	//	Save the character options.
	//	Return 0, or -1 on error.
	//
	int Character_Options::save(void)
	{
		if (!modified)
			return 0;

		JSON_IO::info_message(__PRETTY_FUNCTION__, __LINE__, " [" + the_file_name + "]");

		if (parse_error)
		{
			JSON_IO::file_format_error(class_name_str);
			return JSON_IO::exit_error(__PRETTY_FUNCTION__, __LINE__, "Not saving, because we had a load error.  Fix the problem first.", -1);
		}

		std::ofstream out_file(the_file_name);
		if (out_file)
		{
			out_file << std::setw(JSON_IO::get_json_indent()) << json_data << std::endl;
			modified = false;
			return 0;
		}
		else
			return JSON_IO::exit_error(__PRETTY_FUNCTION__, __LINE__, "Failed to write json [" + the_file_name + "]", -1);
	}


	//	Return true if the specificed option exists for the character name
	//
	bool Character_Options::exists(const char *var_name) const
	{
		if (!have_options_list())
			return false;

		for (auto var : json_data[ini_options_list_str].items())
			if (var.value()[name_str] == var_name)
				return true;

		return false;
	}


	//	Return true if the specificed option exists for the character name
	//
	void Character_Options::remove(const char *var_name)
	{
		if (!have_options_list())
			return;

		// cannot get .erase() to work so do it the hard way....
		bool removed_var = false;
		json new_options_list = json::array();
		for (auto var : json_data[ini_options_list_str].items())
			if (var.value()[name_str] == var_name)
				removed_var = true;
			else
				new_options_list.push_back(var.value());
		if (removed_var)
		{
			json_data[ini_options_list_str] = new_options_list;
			modified = true;
		}
	}


	//	Get the named value, if not found or invalid, the default value is returned.
	//
	template <class TheType> TheType Character_Options::get(const char *var_name, TheType default_value) const
	{
		if (!have_options_list())
			return default_value;

		for (auto var : json_data[ini_options_list_str].items())
			if (var.value()[name_str] == var_name)
				return var.value()[value_str];

		return default_value;
	}

	//	Set the named value, creating the array if required
	//
	template <class TheType> void Character_Options::set(const char *var_name, TheType value)
	{
		modified = true;

		if (!have_options_list())
			json_data[ini_options_list_str] = json::array();

		for (auto var : json_data[ini_options_list_str].items())
			if (var.value()[name_str] == var_name)
			{
				var.value()[value_str] = value;
				return;
			}
		json new_value;
		new_value[name_str] = var_name;
		new_value[value_str] = value;
		json_data[ini_options_list_str].push_back(new_value);
		return;
	}
}


namespace JSON_IO_Client_State
{
	using json = nlohmann::json;

	//	A Class to load and save the Client State.
	//	The state is items we need to save for the user
	//	but which are not options for them to set manually.
	//
	class Client_State
	{
		public:
			Client_State(void) : parse_error(false) {}
			int load(const char *file_name);
			int save(const char *file_name);
			template <class TheType> TheType get(const char *section_name, const char *var_name, TheType default_var_value) const;
			template <class TheType> void set(const char *section_name, const char *var_name, TheType value);
		private:
			bool parse_error;		// there was an error populating the json object
			const char * class_name_str = "Client State";
			json state_read;
			json state_write;
	};


	//	Load the Client State.
	//	Return 0 on success or -1 on error.
	//
	int Client_State::load(const char *file_name)
	{
		JSON_IO::info_message(__PRETTY_FUNCTION__, __LINE__, " [" + std::string(file_name) + "]");

		std::ifstream in_file(file_name);
		if (!in_file)
			return JSON_IO::exit_error(__PRETTY_FUNCTION__, __LINE__, "Failed to open [" + std::string(file_name) + "]", -1);

		try
		{
			in_file >> state_read;
		}
		catch (json::exception& e)
		{
			parse_error = true;
			JSON_IO::file_format_error(class_name_str);
			return JSON_IO::exit_error(__PRETTY_FUNCTION__, __LINE__, e.what(), -1);
		}

		return 0;
	}


	//	Save the Client State.
	//	Return 0, or -1 on error.
	//
	int Client_State::save(const char *file_name)
	{
		JSON_IO::info_message(__PRETTY_FUNCTION__, __LINE__, " [" + std::string(file_name) + "]");

		if (parse_error)
		{
			JSON_IO::file_format_error(class_name_str);
			return JSON_IO::exit_error(__PRETTY_FUNCTION__, __LINE__, "Not saving, because we had a load error.  Fix the problem first.", -1);
		}

		std::ofstream out_file(file_name);
		if (out_file)
		{
			out_file << std::setw(JSON_IO::get_json_indent()) << state_write << std::endl;
			return 0;
		}
		else
			return JSON_IO::exit_error(__PRETTY_FUNCTION__, __LINE__, "Failed to write json [" + std::string(file_name) + "]", -1);
	}


	//	Get the named value, if not found or invalid, the default value is returned.
	//
	template <class TheType> TheType Client_State::get(const char *section_name, const char *var_name, TheType default_value) const
	{
		if (state_read.contains(section_name) && state_read[section_name].contains(var_name))
			return state_read[section_name][var_name].get<TheType>();
		return default_value;
	}

	//	Set the named value, creating the array if required
	//
	template <class TheType> void Client_State::set(const char *section_name, const char *var_name, TheType value)
	{
		state_write[section_name][var_name] = value;
	}

}


//	The instance of the manufacture recipe object.
static JSON_IO_Recipes::Recipes recipes;

//	The instance of the quickspells object.
static JSON_IO_Quickspells::Quickspells quickspells;

//	The instance of the counters object.
static JSON_IO_Counters::Counters counters;

//	The instance of the channel colours object.
static JSON_IO_Channel_Colours::Channel_Colours channel_colours;

//	The instance of the character options object.
static JSON_IO_Character_Options::Character_Options character_options;

//	The instance of the Client State ojbect.
static JSON_IO_Client_State::Client_State cstate;

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

	// channel colours
	int json_load_channel_colours(const char *file_name, channelcolor *the_channel_colours, size_t max_channel_colours)
		{ return channel_colours.load(file_name, the_channel_colours, max_channel_colours); }
	int json_save_channel_colours(const char *file_name, const channelcolor *the_channel_colours, size_t max_channel_colours)
		{ return channel_colours.save(file_name, the_channel_colours, max_channel_colours); }

	// character options
	void json_character_options_set_file_name(const char *file_name)
		{ character_options.set_file_name(file_name); }
	int json_character_options_load_file(void)
		{ return character_options.load(); }
	int json_character_options_save_file(void)
		{ return character_options.save(); }
	int json_character_options_exists(const char *var_name)
		{ return (character_options.exists(var_name)) ?1 :0; }
	void json_character_options_remove(const char *var_name)
		{ character_options.remove(var_name); }
	int json_character_options_get_int(const char *var_name, int default_value )
		{ return character_options.get(var_name, default_value); }
	void json_character_options_set_int(const char *var_name, int value)
		{ character_options.set(var_name, value); }
	float json_character_options_get_float(const char *var_name, float default_value )
		{ return character_options.get(var_name, default_value); }
	void json_character_options_set_float(const char *var_name, float value)
		{ character_options.set(var_name, value); }
	int json_character_options_get_bool(const char *var_name, int default_value )
		{ return (character_options.get(var_name, default_value)) ?1 :0; }
	void json_character_options_set_bool(const char *var_name, int value)
		{ character_options.set(var_name, static_cast<bool>(value)); }

	// Client State
	int json_load_cstate(const char *file_name)
		{ return cstate.load(file_name); }
	int json_save_cstate(const char *file_name)
		{ return cstate.save(file_name); }
	// get/set int
	int json_cstate_get_int(const char *section_name, const char *var_name, int default_value)
		{ return cstate.get(section_name, var_name, default_value); }
	void json_cstate_set_int(const char *section_name, const char *var_name, int value)
		{ cstate.set(section_name, var_name, value); }
	// get/set unsigned int
	unsigned int json_cstate_get_unsigned_int(const char *section_name, const char *var_name, unsigned int default_value)
		{ return cstate.get(section_name, var_name, default_value); }
	void json_cstate_set_unsigned_int(const char *section_name, const char *var_name, unsigned int value)
		{ cstate.set(section_name, var_name, value); }
	// get/set float
	float json_cstate_get_float(const char *section_name, const char *var_name, float default_value)
		{ return cstate.get(section_name, var_name, default_value); }
	void json_cstate_set_float(const char *section_name, const char *var_name, float value)
		{ cstate.set(section_name, var_name, value); }
	// get_set bool
	int json_cstate_get_bool(const char *section_name, const char *var_name, int default_value)
		{ return ((cstate.get(section_name, var_name, static_cast<bool>(default_value))) ?1 :0); }
	void json_cstate_set_bool(const char *section_name, const char *var_name, int value)
		{ cstate.set(section_name, var_name, static_cast<bool>(value)); }
}
