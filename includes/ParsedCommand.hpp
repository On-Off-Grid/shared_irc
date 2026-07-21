#ifndef PARSEDCOMMAND_HPP
#define PARSEDCOMMAND_HPP

#include <string>
#include <vector>

struct ParsedCommand
{
    std::string              prefix;
    std::string              command;
    std::vector<std::string> params;
    std::string              trailing;
    std::string              raw_line;
    bool                     has_prefix;
    bool                     has_trailing;
    bool                     is_valid;
};

#endif

