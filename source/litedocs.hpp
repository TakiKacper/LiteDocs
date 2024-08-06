#pragma once
#include <string>
#include <list>

//Define LITEDOCS_IMPLEMENTATION to implementation litedocs in given compilation unit
//Also include nlohmann/json.hpp" and markdown_parser.hpp

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

namespace litedocs_internal
{
	struct page_order_node
	{
		bool is_go_down = false;	//create subsection
		bool is_go_up = false;		//leave subsection

		std::string file;					//file as given in json file
		std::string page_name;				//file with cuted extension and path
		std::string page_name_undescores;	//page name but with spaces replaced with underscores
	};

	struct project
	{
		std::string name;
		std::string site_language_tag;

		std::string navbar_color;
		std::string sidebar_text_color;
		std::string sidebar_hover_color;
		std::string sidebar_background;
		std::string background_color;

		std::vector<page_order_node> pages_order;
	};
}

namespace litedocs_internal
{
	bool is_good_hex_color(const std::string& color) 
	{
		if (color[0] != '#' || (color.length() != 7 && color.length() != 4))
			return false;

		for (size_t i = 1; i < color.length(); ++i)
			if (!std::isxdigit(color[i]))
				return false;

		return true;
	}

	std::string remove_file_extension(const std::string& filename)
	{
		size_t lastdot = filename.find_last_of(".");
		if (lastdot == std::string::npos) return filename;
		return filename.substr(0, lastdot);
	}

	void replace_spaces_with_underscores(std::string& input) 
	{
		for (size_t i = 0; i < input.size(); ++i)
			if (input[i] == ' ') 
				input[i] = '_';
	}

	bool recursive_get_pages_order(
		std::vector<page_order_node>& pages_order, 
		nlohmann::json& pages
	)
	{
		pages_order.push_back({});

		if (pages.is_string())
		{
			pages_order.back().file = pages;
			pages_order.back().page_name = remove_file_extension(pages);
			pages_order.back().page_name_undescores = pages_order.back().page_name;

			replace_spaces_with_underscores(pages_order.back().page_name_undescores);
		}
		else if (pages.is_array())
		{
			pages_order.back().is_go_down = true;

			for (auto& page : pages)
				recursive_get_pages_order(pages_order, page);

			pages_order.push_back({});
			pages_order.back().is_go_up = true;
		}
		else return false;
		return true;
	}

	std::string format_string(const std::string& format, std::vector<const std::string*> args)
	{
		struct string_view
		{
			const std::string* target;
			size_t begin;
			size_t end;
		};
		std::vector<string_view> fragments;

		size_t search_format_offset = 0;
		size_t previous_format_offset = 0;
		size_t arg_id = 0;

		while (true)
		{
			if (arg_id == args.size()) break;

			search_format_offset = format.find("{}", search_format_offset);
			if (search_format_offset == format.npos) break;

			fragments.push_back({});
			auto& format_text = fragments.back();

			format_text.target = &format;
			format_text.begin = previous_format_offset;
			format_text.end = search_format_offset;

			fragments.push_back({});
			auto& arg = fragments.back();

			arg.target = args[arg_id];
			arg.begin = 0;
			arg.end = args[arg_id]->size();

			arg_id++;

			search_format_offset += 2;
			previous_format_offset = search_format_offset;
		}

		fragments.push_back({});
		auto& rest_of_format = fragments.back();

		rest_of_format.target = &format;
		rest_of_format.begin = previous_format_offset;
		rest_of_format.end = format.size();

		size_t out_string_size = 0;
		for (auto& frg : fragments)
			out_string_size += frg.end - frg.begin;

		std::string result;
		result.reserve(out_string_size);

		for (auto& frg : fragments)
			result += frg.target->substr(frg.begin, frg.end - frg.begin);

		return result;
	}

	bool read_project(
		project& project,
		const nlohmann::json& project_json,
		litedocs::message_callback message
	)
	{
		try
		{
			//Config
			project.name = project_json.at("name");
			project.site_language_tag = project_json.at("site_language_tag");

			//Style
			auto style = project_json.at("style");

			project.navbar_color		 =	style.at("navbar_color");
			project.sidebar_text_color	 =	style.at("sidebar_text_color");
			project.sidebar_hover_color  =	style.at("sidebar_hover_color");
			project.sidebar_background	 =	style.at("sidebar_background");
			project.background_color	 =	style.at("background_color");

			if (!is_good_hex_color(project.navbar_color))			{ message("Error: Invalid navbar color");				return false; }
			if (!is_good_hex_color(project.sidebar_text_color))		{ message("Error: Invalid sidebar text color");			return false; }
			if (!is_good_hex_color(project.sidebar_hover_color))	{ message("Error: Invalid sidebar hover color");		return false; }
			if (!is_good_hex_color(project.sidebar_background))		{ message("Error: Invalid sidebar background color");	return false; }
			if (!is_good_hex_color(project.background_color))		{ message("Error: Invalid background color");			return false; }

			//Pages order
			auto pages = project_json.at("pages_order");
			if (!pages.is_array()) { message("Error: Pages order is supposed to be an array"); return false; }

			if (!recursive_get_pages_order(project.pages_order, pages)) { message("Error: Invalid pages order"); return false; };
		}
		catch (const std::exception& exc)
		{
			message("Error: " + std::string(exc.what()));
			return false;
		}

		return true;
	}

	std::pair<std::string, std::string> project_filepath_to_folder_and_filename(const std::string& str)
	{
		size_t found;
		found = str.find_last_of("/\\");

		return { str.substr(0, found), str.substr(found + 1) };
	}


	void generate_unclosed_head(std::string& head, const project& project);
	void generate_navbar(std::string& navbar, const project& project);
	void generate_sidebar(std::string& sidebar, const project& project);
	void generate_content(std::stringstream& outstream, const std::string& content, const project& project);
}

#define throw_error(condition, _message) do { if (condition) {if (message != nullptr) message(_message); return false;} } while(0);

bool litedocs::generate_docs(
	const std::string& project_file_filepath,
	load_file_callback load_file,
	save_page_callback save_file,
	message_callback message
)
{
	std::string project_filename;
	std::string project_folder;

	{
		auto x = litedocs_internal::project_filepath_to_folder_and_filename(project_file_filepath);
		project_folder = std::move(x.first);
		project_filename = std::move(x.second);
	}

	loaded_file project_file = load_file(project_filename, project_folder);
	throw_error(!project_file.success, "[Error] Missing project file");

	litedocs_internal::project project;
	auto project_json = nlohmann::json::parse(project_file.content);

	throw_error(!litedocs_internal::read_project(project, project_json, message), "[Error] Failed to load project");

	std::string head;	 litedocs_internal::generate_unclosed_head(head, project);
	std::string navbar;  litedocs_internal::generate_navbar(navbar, project);
	std::string sidebar; litedocs_internal::generate_sidebar(sidebar, project);

	//put header
	//begin body
	//put navbar
	//put main
	//put sidebar
	//put content
	//end main
	//end body

	std::string base = "./";
	std::vector<const std::string*> sections;

	for (size_t i = 0; i < project.pages_order.size(); i++)
	{
		const auto& page = project.pages_order.at(i);

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

namespace litedocs_internal
{
	extern const std::string head_format;

	void generate_unclosed_head(std::string& head, const project& project)
	{
		head = format_string(
			head_format, 
			{ 
				&project.site_language_tag, 
				&project.name,
				&project.navbar_color, 
				&project.sidebar_background, 
				&project.sidebar_text_color,
				&project.sidebar_hover_color 
			}
		);
	}
}

/*
Args order:
	site lang tag
	project name
	navbar bg color
	sidebar bg color
	sidebar text color
	sidebar hover color
*/
const std::string litedocs_internal::head_format = R"(
<!--Generate Head-->
<!DOCTYPE html>
<html lang = {}>
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>{}</title>
    <style>
        body {
            margin: 0;
            font-family: Arial, sans-serif;
        }
        .navbar {
            width: 100%;
            background-color: {};
            color: white;
            padding: 10px 20px;
            box-shadow: 0 2px 5px rgba(0,0,0,0.1);
            position: fixed;
            top: 0;
            left: 0;
            z-index: 1000;
        }
        .navbar h1 {
            margin: 0;
        }
        .main {
            display: flex;
            height: calc(100vh - 50px);
            margin-top: 50px;
        }
        .sidebar {
            width: 15%;
            background-color: {};
            padding: 20px;
            box-shadow: 2px 0 5px rgba(0,0,0,0.1);
            overflow-y: auto;
        }
        .content {
            width: 75%;
            padding: 20px;
			padding-left: 60px;
            overflow-y: auto;
        }
        .sidebar ul {
            list-style-type: none;
            padding: 0;
        }
        .sidebar ul li {
            margin-bottom: 10px;
        }
        .sidebar ul li a {
            text-decoration: none;
            color: {};
            display: block;
            padding: 10px;
            border-radius: 5px;
            transition: background-color 0.3s;
        }
        .sidebar ul li a:hover {
            background-color: {};
        }
        .sidebar ul ul {
            list-style-type: none;
            padding-left: 20px;
            font-size: 0.9em;
        }
        .sidebar ul ul li a {
            padding: 5px 10px;
        }
    </style>
)";

namespace litedocs_internal
{
	extern const std::string navbar_format;

	void generate_navbar(std::string& navbar, const project& project)
	{
		navbar = format_string(
			navbar_format,
			{
				&project.name
			}
		);
	}
}

/*
Args order:
	Project Name
*/
const std::string litedocs_internal::navbar_format = R"(
    <!--Generate Navbar-->
    <div class="navbar">
        <h1>{}</h1>
    </div>
)";

namespace litedocs_internal
{
	extern const std::string sidebar_format;
	extern const std::string sidebar_item_begin_format;
	extern const std::string sidebar_item_end_mark;
	extern const std::string sidebar_subsection_begin_mark;
	extern const std::string sidebar_subsection_end_mark;

	void generate_sidebar(std::string& sidebar, const project& project)
	{
		std::stringstream sidebar_stream;
		std::vector<const std::string*> sections;

		for (size_t i = 0; i < project.pages_order.size(); i++)
		{
			const auto& page = project.pages_order.at(i);

			if (page.is_go_down)
			{
				if (i != 0)
					sections.push_back(&project.pages_order.at(i - 1).page_name_undescores);

				sidebar_stream << sidebar_subsection_begin_mark;			
			}
			else if (page.is_go_up)
			{
				sidebar_stream << sidebar_subsection_end_mark << sidebar_item_end_mark;

				if (sections.size())
					sections.pop_back();
			}
			else
			{
				std::stringstream link_stream;

				for (auto& upsection : sections)
					link_stream << *upsection << '/';

				link_stream << page.page_name_undescores << ".html";

				std::string link = link_stream.str();

				sidebar_stream << format_string(
					sidebar_item_begin_format,
					{
						&link,
						&page.page_name
					}
				);
			}
		}

		std::string items = sidebar_stream.str();

		sidebar = format_string(
			sidebar_format,
			{
				&items
			}
		);
	}
}

/*
Args order:
	Items
*/
const std::string litedocs_internal::sidebar_format = R"(
	<!-- Generate Sidebar-->
        <div class="sidebar">
            <ul>
				{}
            </ul>
        </div>
)";

const std::string litedocs_internal::sidebar_item_end_mark = R"(
	</li>
)";

/*
Args order:
	Link
	Item display name
*/
const std::string litedocs_internal::sidebar_item_begin_format = R"(
	<li><a href={}>{}</a>
)";

const std::string litedocs_internal::sidebar_subsection_begin_mark = R"(
	<ul>
)";

const std::string litedocs_internal::sidebar_subsection_end_mark = R"(
	</ul>
)";

namespace litedocs_internal
{
	extern const std::string content_format;

	void generate_content(std::stringstream& outstream, const std::string& content, const project& project)
	{
		outstream << "<!-- Generate Content -->";
		outstream << R"(<div class="content">)";

		outstream << markdown_parsing::markdown_to_html(content);

		outstream << R"(</div>)";
	}
}

#endif // LITEDOCS_IMPLEMENTATION

