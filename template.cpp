#include "template.h"

bool is_uint(const std::string &s)
{
	const std::regex test{"^\\d+$"};
	return std::regex_match(s, test);
}

int int_or_ref(const std::string &value, const std::unordered_map<std::string, std::string> &vars)
{
	if (is_uint(value)) return util::s2t<int>(value);
	else if (vars.count(value) && is_uint(vars.at(value))) return util::s2t<int>(vars.at(value));
	else throw std::runtime_error{"Value " + value + " is not an integer or a reference to one"};
	
}

namespace templ
{
	std::vector<std::string> split(const std::string &in, unsigned int sects, const std::string &id, const std::string &sep)
	{
		std::vector<std::string> ret = util::strsplit(in, "\n" + sep + "\n");
		if (sects > 0 && ret.size() != sects) throw std::runtime_error{"Expected " + util::t2s(sects) + " sections in template " + id + ", but got " + util::t2s(ret.size())};
		return ret;
	}

	std::vector<std::string> split(const std::string &in, const std::string &sep)
	{
		return split(in, 0, "", sep);
	}

	bool test(const std::string &expr, const std::unordered_map<std::string, std::string> &vars)
	{
		const static std::regex re_compare{"^(\\w+)(=|==|!=)(.+)\\?$"}, re_mathcomp{"^(\\w+)(>|<|>=|<=)(\\w+)\\?$"}, re_exist{"^(\\w+)\\?$"};
		std::smatch match{};
		if (std::regex_match(expr, match, re_compare))
		{
			if (! vars.count(match[1])) return false;
			if (match[2] == "=" || match[2] == "==")  return vars.at(match[1]) == match[3];
			if (match[2] == "!=") return vars.at(match[1]) != match[3];
			throw std::runtime_error("Internal error choosing comparison operator");
		}
		if (std::regex_match(expr, match, re_mathcomp))
		{
			int l = int_or_ref(match[1], vars);
			int r = int_or_ref(match[3], vars);
			if (match[2] == ">") return l > r;
			if (match[2] == "<") return l < r;
			if (match[2] == ">=") return l >= r;
			if (match[2] == "<=") return l <= r;
			throw std::runtime_error{"Internal error choosing comparison operator"};
		}
		if (std::regex_match(expr, match, re_exist)) return vars.count(match[1]);
		throw std::runtime_error{"Invalid test expression " + expr};
	}

	std::string eval(const std::string &expr, const std::unordered_map<std::string, std::string> &vars)
	{
		const static std::regex re_ternequal{"^(\\w+)=(.+?)\\?(.*?):(.*)$"}, re_ternary{"^(\\w+)\\?(.*?):(.*)$"}, re_arith{"^(\\w+)(\\+|-|\\*|/|%)(\\d+)$"}, re_sub{"^(\\w+)$"};
		std::smatch match{};
		if (std::regex_match(expr, match, re_ternequal)) return (vars.count(match[1]) && vars.at(match[1]) == match[2]) ? match[3] : match[4];
		if (std::regex_match(expr, match, re_ternary)) return (vars.count(match[1])) ? match[2] : match[3];
		if (std::regex_match(expr, match, re_arith))
		{
			int l = int_or_ref(match[1], vars);
			int r = int_or_ref(match[3], vars);
			int ret;
			if (match[2] == "+") ret = l + r;
			else if (match[2] == "-") ret = l - r;
			else if (match[2] == "*") ret = l * r;
			else if (match[2] == "/") ret = l / r;
			else if (match[2] == "%") ret = l % r;
			else throw std::runtime_error{"Internal error choosing arithmetic operator"};
			return util::t2s(ret);
		}
		if (std::regex_match(expr, match, re_sub)) return (vars.count(match[1])) ? vars.at(match[1]) : "";
		throw std::runtime_error{"Invalid evaluation expression " + expr};
	}

	std::string render(const std::string &in, const std::unordered_map<std::string, std::string> &vars)
	{
		const static std::regex re_token{"\\{\\{(.*?)\\}\\}"};
		std::ostringstream ret{};
		int count = 0, nsilent = 0;
		std::vector<bool> echo{};
		for (std::sregex_token_iterator iter{in.begin(), in.end(), re_token, {-1, 1}}; iter != std::sregex_token_iterator{}; iter++)
		{
			const std::string &cur = iter->str();
			std::string res = "";
			if (count++ % 2 == 0) res = cur;
			else
			{
				if (! cur.size()) continue;
				if (cur[0] == '#')
				{
					echo.push_back(test(cur.substr(1), vars));
					nsilent += echo.back() ? 0 : 1;
				}
				else if (cur[0] == '/') // Do we want to restrict what comes after the slash?
				{
					if (! echo.size()) throw std::runtime_error{"Unmatched end of block in template render"};
					nsilent -= echo.back() ? 0 : 1;
					echo.pop_back();
				}
				else res = eval(cur, vars);
			}
			if (! nsilent) ret << res;
		}
		return ret.str();
	}
}
