#include "BufferManager.hpp"

BufferManager::BufferManager(void) : _buffer(""), _max_line_length(510)
{
}

BufferManager::BufferManager(const BufferManager &other)
{
    *this = other;
}

BufferManager &BufferManager::operator=(const BufferManager &other)
{
    if (this != &other)
    {
        _buffer = other._buffer;
        _max_line_length = other._max_line_length;
    }
    return (*this);
}

BufferManager::~BufferManager(void)
{
}

void BufferManager::set_max_line_length(size_t max_len)
{
    _max_line_length = max_len;
}

void BufferManager::append_data(const std::string &chunk)
{
    _buffer += chunk;
}

std::vector<std::string> BufferManager::extract_complete_lines()
{
    std::vector<std::string> lines;

    if (_buffer.empty())
        return (lines);

    size_t pos;

    while ((pos = _buffer.find('\n')) != std::string::npos)
    {
        std::string line = _buffer.substr(0, pos);
        _buffer.erase(0, pos + 1);

        if (!line.empty() && line[line.size() - 1] == '\r')
            line.erase(line.size() - 1);

        if (line.size() > _max_line_length)
            line.resize(_max_line_length);

        lines.push_back(line);
    }

    if (_buffer.size() > _max_line_length)
    {
        lines.push_back(_buffer.substr(0, _max_line_length));
        _buffer.clear();
    }

    return (lines);
}

bool BufferManager::has_buffered_data() const
{
    return (!_buffer.empty());
}

std::string BufferManager::get_buffer_copy() const
{
    return (_buffer);
}

void BufferManager::clear_buffer()
{
    _buffer.clear();
}
