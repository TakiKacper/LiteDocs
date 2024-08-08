#pragma once

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

	bool recursive_get_pages_order(
		std::vector<page_order_node>& pages_order,
		nlohmann::json& pages
	);

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

			project.navbar_color = style.at("navbar_color");
			project.sidebar_text_color = style.at("sidebar_text_color");
			project.sidebar_hover_color = style.at("sidebar_hover_color");
			project.sidebar_background = style.at("sidebar_background");
			project.background_color = style.at("background_color");

			if (!is_good_hex_color(project.navbar_color)) { message("Error: Invalid navbar color");				return false; }
			if (!is_good_hex_color(project.sidebar_text_color)) { message("Error: Invalid sidebar text color");			return false; }
			if (!is_good_hex_color(project.sidebar_hover_color)) { message("Error: Invalid sidebar hover color");		return false; }
			if (!is_good_hex_color(project.sidebar_background)) { message("Error: Invalid sidebar background color");	return false; }
			if (!is_good_hex_color(project.background_color)) { message("Error: Invalid background color");			return false; }

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
}