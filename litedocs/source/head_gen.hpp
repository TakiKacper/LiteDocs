#pragma once

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
            width: 85%;
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
        .code_border {
            border: 2px solid #026562;
            border-radius: 5px;
            background-color: #1E1E1E;
            padding: 20px;
            width: fit-screen; 
            margin: 20px 40px 0px 0px;
            font-family: Arial, sans-serif;
        }
    </style>
)";
