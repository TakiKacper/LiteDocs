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
                &project.content_text_color,
                &project.navbar_color,
                &project.code_block_frame_color,
                &project.code_block_background,
                &project.sidebar_background,
                &project.sidebar_background,
                & project.sidebar_text_color,
                & project.sidebar_hover_color,
            }
        );
    }
}

/*
Args order:
    site lang tag
    project name
    content text color
    navbar bg color
    code block frame color
    code block background color
    sidebar bg color
    sidebar bg color (again)
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
            color: {};
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
            display: flex;
            align-items: center;
            justify-content: space-between;
        }
        .navbar h1 {
            margin: 0;
        }
        .main {
            display: flex;
            height: calc(100vh - 50px);
            margin-top: 50px;
        }
        .code_border {
            border: 2px solid {};
            border-radius: 5px;
            background-color: {};
            overflow-x: auto;
            padding: 20px;
            width: fit-screen; 
            margin: 20px 40px 0px 0px;
            font-family: Arial, sans-serif;
        }

    @media (min-width: 768px)
    {
        .sidebar {
            width: 15%;
            background-color: {};
            padding: 20px;
            box-shadow: 2px 0 5px rgba(0,0,0,0.1);
            overflow-y: auto;
            margin-right: 20px;
        }
        .content {
            width: 85%;
            padding: 20px;
			padding-left: 60px;
            overflow-y: auto;
        }
    }
    @media (max-width: 768px) 
    {
        sidebar {
            width: 80%;
            background-color: {};

            padding: 20px;
            box-shadow: 2px 0 5px rgba(0,0,0,0.1);
            overflow-y: auto;
            margin-right: 20px;
        }
        .content {
            width: 100%;
            padding: 20px;
			padding-left: 60px;
            overflow-y: auto;
        }
        .menu-btn {
            width: 30px;
            height: 22px;
            display: flex;
            flex-direction: column;
            justify-content: space-between;
            cursor: pointer;
            margin: 0px 30px 0px 0px;
        }
        .menu-btn div {
            width: 100%;
            height: 4px;
            background-color: white;
            border-radius: 2px;
        }
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
            font-size: 1.0em;
        }
    </style>
)";
