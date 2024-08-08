#pragma once

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