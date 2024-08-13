### This project is still in development!
# LiteDocs

LiteDocs is a documentation generator that converts pure markdown into a working html + css websites.  

# Features
- Easy configuration with single json file per project
- Syntax Highlighting configured with json files (you can add your own custom languages!)
- Pretty Fast

# Usage
1. Grab the LiteDocs executable (compile it yourself or get one in releases tab)
2. Create a folder for your project
3. Write your docs in markdown
4. Add ``[project_name].json`` file to the project folder
  ```json
{
    "name" : "Example Project",
    "site_language_tag" : "en",

    "style" : {
        "navbar_color" : "#026562",

        "sidebar_text_color" : "#FFFFFF",
        "sidebar_hover_color" : "#026562",
        "sidebar_background" : "#1E1E1E",

        "content_text_color" : "#FFFFFF",
        "content_background" : "#121212",

        "code_block_frame_color" : "#026562",
        "code_block_background" : "#1E1E1E"
    },
    "pages_order" : [
        "example.md",
        [
            "subsection.md"
        ]
    ]
}
```
4. Configure pages order, colors etc.
5. Run LiteDocs executable
6. Enjoy your sites, saved in ``[your project folder]/build``!
