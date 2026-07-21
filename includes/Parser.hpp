#ifndef PARSER_HPP
#define PARSER_HPP

#include "ParsedCommand.hpp"
#include <string>

class Parser
{
    public:
        Parser(void);
        Parser(const Parser &other);
        Parser &operator=(const Parser &other);
        ~Parser(void);

        ParsedCommand parse_line(const std::string &line) const;
};

#endif
