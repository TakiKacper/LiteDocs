#pragma once

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