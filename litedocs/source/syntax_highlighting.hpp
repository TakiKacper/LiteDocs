#pragma once

namespace litedocs_internal
{
	/*
		Rules on how to highlight
	*/
	struct highlighting_rules
	{
		struct rule 
		{ 
			std::string color;
			virtual ~rule() {}; 
		};

		struct keywords_rule : public rule
		{
			std::unordered_set<std::string> keywords;
		};

		struct pairs_rule : public rule
		{
			std::string begin;
			std::string end;
		};

		struct regex_rule : public rule
		{
			std::regex regex;	
		};

		std::vector<rule*> rules;
		std::vector<std::string> breaks;

		~highlighting_rules()
		{
			for (auto* _rule : rules)
				delete _rule;
		}
	};

	highlighting_rules* load_highlighting_rules_from_json(const nlohmann::json& json)
	{
		auto hg = std::make_unique<highlighting_rules>();

		try
		{
			for (auto& rule : json.at("rules"))
			{
				if (rule.at("type") == "keywords")
				{
					auto rule_obj = new highlighting_rules::keywords_rule;
					hg->rules.push_back(rule_obj);

					for (auto& keyword : rule.at("keywords"))
						rule_obj->keywords.insert(keyword);

					rule_obj->color = rule.at("color");

					if (!is_good_hex_color(rule_obj->color))
						throw std::exception("Invalid color");
				}
				else if (rule.at("type") == "pairs")
				{
					auto rule_obj = new highlighting_rules::pairs_rule;
					hg->rules.push_back(rule_obj);

					rule_obj->begin = rule.at("begin");
					rule_obj->end = rule.at("end");
					rule_obj->color = rule.at("color");

					if (!is_good_hex_color(rule_obj->color))
						throw std::exception("Invalid color");
				}
				else if (rule.at("type") == "regex")
				{
					auto rule_obj = new highlighting_rules::regex_rule;
					hg->rules.push_back(rule_obj);

					rule_obj->regex = rule.at("regex");
					rule_obj->color = rule.at("color");

					if (!is_good_hex_color(rule_obj->color))
						throw std::exception("Invalid color");
				}
				else
				{
					throw std::exception("Unknown type");
				}
			}

			for (auto& _break : json.at("breaks"))
				if (_break != "")
					hg->breaks.push_back(_break);

			std::sort(hg->breaks.begin(), hg->breaks.end(), 
				[](const std::string& a, const std::string& b) {return a.size() > b.size();});
		}
		catch (const std::exception&)
		{
			return nullptr;
		}

		return hg.release();
	}

	void try_to_load_highlighting_rules(const std::string& language_name)
	{
		std::string dir = get_executable_dir();
		dir += "/langs/";
		dir += language_name;
		dir += ".json";

		std::filesystem::path path(dir);

		if (!std::filesystem::exists(path)) goto _try_to_load_highlighting_rules_fail;

		{
			auto file =  std::fstream(path);
			if (!file.good()) goto _try_to_load_highlighting_rules_fail;

			auto rules_json = nlohmann::json::parse(file);
			auto rules = load_highlighting_rules_from_json(rules_json);
		
			if (rules == nullptr) goto _try_to_load_highlighting_rules_fail;

			highlighted_languages.insert({ language_name, rules });

			return;
		}

	_try_to_load_highlighting_rules_fail:

		highlighted_languages.insert({ language_name, nullptr });
	}

	std::string apply_rules(highlighting_rules* rules, const std::string& source, size_t code_begin, size_t code_end)
	{
		auto& iterator = code_begin;
		std::stringstream ss;

		auto dump_whitespaces = [&]()
		{
			size_t begin = iterator;

			while (iterator < code_end)
			{
				auto& c = source.at(iterator);
				if (c != ' ' && c != '\t' && c != '\n') break;
				iterator++;
			}

			ss << source.substr(begin, iterator - begin);
		};

		auto check_should_break = [&](std::string& _break) -> bool
		{
			if (code_end - iterator < _break.size()) return false;

			size_t i = 0;
			while (i < _break.size())
			{
				if (source.at(iterator + i) != _break.at(i)) break;
				i++;
			}

			if (i == _break.size()) return true;
			return false;
		};

		std::string* buffor_token = nullptr;

		auto get_token = [&]()
		{
			if (buffor_token != nullptr)
			{
				iterator += buffor_token->size();

				auto local = buffor_token;
				buffor_token = nullptr;
				return *local;
			}

			size_t begin = iterator;

			while (iterator < code_end)
			{
				auto& c = source.at(iterator);

				for (auto& b : rules->breaks)
					if (check_should_break(b))
					{
						buffor_token = &b;
						goto _get_token_return;
					}
				iterator++;
			}

		_get_token_return:
			return source.substr(begin, iterator - begin);
		};

		auto handle_keyword_rule = [&](highlighting_rules::keywords_rule* r, std::string& token) -> bool
		{
			if (r->keywords.find(token) != r->keywords.end())
			{
				ss << "<span style=\"color:";
				ss << r->color;
				ss << ";\">";
				ss << token;
				ss << "</span>";

				return true;
			}

			return false;
		};

		auto handle_pairs_rule = [&](highlighting_rules::pairs_rule* r, std::string& token) -> bool
		{
			if (r->begin == token)
			{
				ss << "<span style=\"color:";
				ss << r->color;
				ss << ";\">";

				ss << token;

				while (iterator < code_end)
				{
					auto token2 = get_token();
					ss << token2;

					if (token2 == r->end) break;
				}

				ss << "</span>";

				return true;
			}

			return false;
		};

		auto handle_regex_rule = [&](highlighting_rules::regex_rule* r, std::string& token) -> bool
		{
			if (!std::regex_match(token, r->regex)) return false;

			ss << "<span style=\"color:";
			ss << r->color;
			ss << ";\">";
			ss << token;
			ss << "</span>";

			return true;
		};

		while (iterator < code_end)
		{
			auto token = get_token();

			bool rule_found = false;
			for (auto& rule : rules->rules)
			{
				auto as_keyword_rule = dynamic_cast<highlighting_rules::keywords_rule*>(rule);

				if (as_keyword_rule != nullptr)
					if (handle_keyword_rule(as_keyword_rule, token)) 
					{
						rule_found = true;
						break;
					}

				auto as_pairs_rule = dynamic_cast<highlighting_rules::pairs_rule*>(rule);

				if (as_pairs_rule != nullptr)
					if (handle_pairs_rule(as_pairs_rule, token))
					{
						rule_found = true;
						break;
					}

				auto as_regex_rule = dynamic_cast<highlighting_rules::regex_rule*>(rule);

				if (as_regex_rule != nullptr)
					if (handle_regex_rule(as_regex_rule, token))
					{
						rule_found = true;
						break;
					}
			}

			if (!rule_found)
				ss << token;
		}

		return ss.str();
	}

	std::string higlight_syntax(const std::string& language_name, const std::string & source, size_t code_begin, size_t code_end)
	{
		auto itr = highlighted_languages.find(language_name);

		if (itr == highlighted_languages.end())
		{
			try_to_load_highlighting_rules(language_name);
			itr = highlighted_languages.find(language_name);;
		}

		if (itr->second == nullptr)
		{
			return source.substr(code_begin, code_end - code_begin);
		}

		return apply_rules(itr->second, source, code_begin, code_end);
	};
}