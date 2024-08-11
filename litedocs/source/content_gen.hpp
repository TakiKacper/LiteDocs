#pragma once

namespace litedocs_internal
{
	extern const std::string content_format;

	void generate_content(std::stringstream& outstream, const std::string& content, const project& project)
	{
		outstream << "<!-- Generate Content -->";
		outstream << R"(<div class="content">)";

		outstream << markdown_parsing::markdown_to_html(content, html_tags_override);

		outstream << R"(</div>)";
	}
}