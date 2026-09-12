#include "Parser.hpp"
#include <cctype>
#include <sstream>

Parser::Parser(void)
{
}

Parser::Parser(const Parser &other)
{
	*this = other;
}

Parser &Parser::operator=(const Parser &other)
{
	(void)other;
	return (*this);
}

Parser::~Parser(void)
{
}

static std::string to_upper_ascii(const std::string &input)
{
	std::string out = input;
	for (size_t i = 0; i < out.size(); ++i)
	{
		out[i] = static_cast<char>(std::toupper(static_cast<unsigned char>(out[i])));
	}
	return (out);
}

static void init_result(ParsedCommand &out, const std::string &line)
{
	out.prefix = "";
	out.command = "";
	out.params.clear();
	out.trailing = "";
	out.raw_line = line;
	out.has_prefix = false;
	out.has_trailing = false;
	out.is_valid = false;
}

ParsedCommand Parser::parse_line(const std::string &line) const
{
	ParsedCommand out;

	init_result(out, line);

	std::string str = line;

	while (!str.empty() && (str[str.size() - 1] == '\r' || str[str.size() - 1] == '\n'))
		str.erase(str.size() - 1);

	size_t i = 0;
	size_t len = str.size();

	while (i < len && str[i] == ' ')
		i++;

	if (i >= len)
		return (out);

	if (str[i] == ':')
	{
		i++;
		size_t start = i;
		while (i < len && str[i] != ' ')
			i++;
		out.prefix = str.substr(start, i - start);
		out.has_prefix = true;

		while (i < len && str[i] == ' ')
			i++;
	}

	if (i >= len)
		return (out);

	size_t cmd_start = i;
	while (i < len && str[i] != ' ')
		i++;

	out.command = to_upper_ascii(str.substr(cmd_start, i - cmd_start));
	if (out.command.empty())
		return (out);

	out.is_valid = true;

	while (i < len && str[i] == ' ')
		i++;

	while (i < len)
	{
		if (str[i] == ':')
		{
			out.trailing = str.substr(i + 1);
			out.has_trailing = true;
			out.params.push_back(out.trailing);
			break;
		}

		size_t param_start = i;
		while (i < len && str[i] != ' ')
			i++;

		std::string param = str.substr(param_start, i - param_start);
		out.params.push_back(param);

		while (i < len && str[i] == ' ')
			i++;
	}

	return (out);
}
