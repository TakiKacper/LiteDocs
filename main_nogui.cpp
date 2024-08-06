
#define LITEDOCS_IMPLEMENTATION
#define MARKDOWN_PARSER_IMPLEMENTATION
#include "include/nlohmann/json.hpp"
#include "markdown_parser.hpp"
#include "source/litedocs.hpp"

#include <iostream>
#include <fstream>
#include <filesystem>

void save_page(litedocs::generated_page* page, const std::string& project_path)
{
	std::string build_dir = project_path + "/build/";
	std::string name;

	for (auto& s : *page->sections)
		name += *s + "\\";
	name += page->page_name + ".html";

	auto path = std::filesystem::path(build_dir + name);

	std::filesystem::create_directories(path.parent_path());

	auto f = std::ofstream(path);
	f << *page->content;
	f.close();

	std::cout << "\n[Saved] " << name;
}

litedocs::loaded_file load_file(std::string filename, const std::string& project_path)
{
	litedocs::loaded_file result;

	std::string path = project_path + "/" + filename;
	std::fstream t(path);

	if (!t.good())
		return result;

	t.seekg(0, std::ios::end);
	size_t size = t.tellg();
	result.content = std::string(size, ' ');
	t.seekg(0);
	t.read(&result.content[0], size);

	t.close();

	result.success = true;

	return result;
}

void message_callback(const std::string& message)
{
	std::cout << message << '\n';
}

int main(int argc, char* argv[])
{
	std::vector<std::string> arguments;

	for (int i = 1; i < argc; i++)
		arguments.push_back(argv[i]);

	std::filesystem::path project_filepath;

	if (arguments.size() > 1)
	{
		std::cout << "\n[Error] Expected one argument, the litedocs project filepath";
		return 0;
	}

	if (arguments.size() == 1)
	{
		project_filepath = arguments.back();
		if (!std::filesystem::exists(project_filepath) || std::filesystem::is_directory(project_filepath))
		{
			std::cout << "\n[Error] Invalid path";
			return 0;
		}
	}
	else
	{
		while (true)
		{
			std::cout << "Litedocs project filepath (.json file): ";
			std::cin >> project_filepath;

			if (std::filesystem::exists(project_filepath) && !std::filesystem::is_directory(project_filepath)) break;
			std::cout << "\n[Error] Invalid project filepath\n";
		}
	}

	//Generate build folder
	std::filesystem::path build_directory = project_filepath.parent_path().string() + "/build";

	std::filesystem::remove_all(build_directory);
	std::filesystem::create_directories(build_directory);

	litedocs::generate_docs(project_filepath.string(), load_file, save_page, message_callback);

	return 0;
}