#pragma once
#include <string>
#include <list>

//Define LITEDOCS_IMPLEMENTATION to implementation litedocs in given compilation unit
//Also include nlohmann/json.hpp" and "markdown_parser.hpp"

namespace litedocs
{
	struct loaded_file
	{
		bool success = false;
		std::string content;
	};

	struct generated_page
	{
		//Page name (without .html extension)
		std::string								page_name;

		//Sections to which given page belongs
		//The vector under the pointer may be changed during parsing other pages
		//So save the file immediately or copy the vector
		const std::vector<const std::string*>*	sections;

		//Generated page content in html
		const std::string*						content;
	};

	using save_page_callback = void(*)(generated_page* page, const std::string& project_path);
	using load_file_callback = loaded_file(*)(std::string filename, const std::string& project_path);
	using message_callback = void(*)(const std::string& message);

	bool generate_docs(
		const std::string& project_file_filepath, 
		load_file_callback load_file,
		save_page_callback save_file,
		message_callback message
	);
}

#ifdef LITEDOCS_IMPLEMENTATION

#include <cctype>
#include <sstream>
#include <string>
#include <vector>
#include <memory>
#include <unordered_set>
#include <fstream>
#include <regex>

#include "source/utility.hpp"
#include "source/project.hpp"

#include "source/syntax_highlighting.hpp"

#include "source/head_gen.hpp"
#include "source/navbar_gen.hpp"
#include "source/sidebar_gen.hpp"
#include "source/content_gen.hpp"

#define throw_error(condition, _message) do { if (condition) {if (message != nullptr) message(_message); return false;} } while(0);

bool litedocs::generate_docs(
	const std::string& project_file_filepath,
	load_file_callback load_file,
	save_page_callback save_file,
	message_callback message
)
{
	/*
		Load project
	*/

	std::string project_filename;
	std::string project_folder;

	{
		size_t found;
		found = project_file_filepath.find_last_of("/\\");

		project_folder = project_file_filepath.substr(0, found);
		project_filename = project_file_filepath.substr(found + 1);
	}

	loaded_file project_file = load_file(project_filename, project_folder);
	throw_error(!project_file.success, "[Error] Missing project file");

	nlohmann::json project_json;
	litedocs_internal::project project;
	try
	{
		project_json = nlohmann::json::parse(project_file.content);
	}
	catch (const std::exception& exc)
	{
		message("[Error] " + std::string(exc.what()));
		message("[Error] Failed to load project");
		return false;
	}
	
	throw_error(!litedocs_internal::read_project(project, project_json, message), "[Error] Failed to load project");

	/*
		Generate Head, Navbar and Sidebar
	*/

	std::string head;	 litedocs_internal::generate_unclosed_head(head, project);
	std::string navbar;  litedocs_internal::generate_navbar(navbar, project);
	std::string sidebar; litedocs_internal::generate_sidebar(sidebar, project);

	/*
		For each page, generate the content and the .html site
	*/

	std::string base = "./";
	std::vector<const std::string*> sections;

	for (size_t i = 0; i < project.pages_order.size(); i++)
	{
		const auto& page = project.pages_order.at(i);

		//Keep base a root directory using relative paths
		if (page.is_go_down && i != 0)
		{
			base += "../";
			sections.push_back(&project.pages_order.at(i - 1).page_name_undescores);
		}
		else if (page.is_go_up && sections.size() != 0)
		{
			sections.pop_back();
			base.erase(base.end() - 3, base.end());
		}

		if (page.is_go_down || page.is_go_up) continue;

		auto content_source = load_file(page.file, project_folder);
		throw_error(!content_source.success, "[Error] Failed to load file: " + page.file);

		std::stringstream out_stream;

		out_stream << head;

		out_stream << "<base href=\"";
		out_stream << base;
		out_stream << "\"/>";

		out_stream << "</head>";

		out_stream << "<body bgcolor=" << project.background_color << " >";
		out_stream << navbar;
		out_stream << "<div class=\"main\">";
		out_stream << sidebar;
		
		litedocs_internal::generate_content(out_stream, content_source.content, project);

		out_stream << R"(</div></body></html>)";

		std::string result = out_stream.str();

		generated_page gen_page;
		gen_page.page_name = page.page_name_undescores;
		gen_page.sections = &sections;
		gen_page.content = &result;

		save_file(&gen_page, project_folder);
	}

	return true;
}

#undef throw_error

#endif // LITEDOCS_IMPLEMENTATION

