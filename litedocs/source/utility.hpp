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

	std::string get_executable_dir();
}

#if defined(_WIN32)
#include <windows.h>
#include <Shlwapi.h>
#include <io.h> 

#define access _access_s
#endif

#ifdef __linux__
#include <limits.h>
#include <libgen.h>
#include <unistd.h>

#if defined(__sun)
#define PROC_SELF_EXE "/proc/self/path/a.out"
#else
#define PROC_SELF_EXE "/proc/self/exe"
#endif

#endif

#if defined(_WIN32)
std::string litedocs_internal::get_executable_dir() {
	char rawPathName[MAX_PATH];
	GetModuleFileNameA(NULL, rawPathName, MAX_PATH);

	auto path = std::string(rawPathName);
	size_t index = path.find_last_of('\\', path.size());

	return path.substr(0, index + 1);
}
#endif

#ifdef __linux__
std::string litedocs_internal::get_executable_dir() {
	std::string executablePath = getExecutablePath();
	char* executablePathStr = new char[executablePath.length() + 1];
	strcpy(executablePathStr, executablePath.c_str());
	char* executableDir = dirname(executablePathStr);
	delete[] executablePathStr;
	return std::string(executableDir);
}
#endif