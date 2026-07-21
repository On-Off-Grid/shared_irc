#include "BufferManager.hpp"

BufferManager::BufferManager(void) : _max_line_length(510)
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
        _buffers = other._buffers;
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

void BufferManager::add_client(fd_t fd)
{
    _buffers[fd] = "";
}

void BufferManager::remove_client(fd_t fd)
{
    _buffers.erase(fd);
}

bool BufferManager::has_client(fd_t fd) const
{
    return (_buffers.find(fd) != _buffers.end());
}

void BufferManager::append_data(fd_t fd, const std::string &chunk)
{
    if (!has_client(fd))
        add_client(fd);
    _buffers[fd] += chunk;
}

std::vector<std::string> BufferManager::extract_complete_lines(fd_t fd)
{
    std::vector<std::string> lines;
    std::map<fd_t, std::string>::iterator it = _buffers.find(fd);

    if (it == _buffers.end() || it->second.empty())
        return (lines);

    std::string &buf = it->second;
    size_t pos;

    while ((pos = buf.find('\n')) != std::string::npos)
    {
        std::string line = buf.substr(0, pos);
        buf.erase(0, pos + 1);

        if (!line.empty() && line[line.size() - 1] == '\r')
            line.erase(line.size() - 1);

        if (line.size() > _max_line_length)
            line.resize(_max_line_length);

        lines.push_back(line);
    }

    if (buf.size() > _max_line_length)
    {
        lines.push_back(buf.substr(0, _max_line_length));
        buf.clear();
    }

    return (lines);
}

bool BufferManager::has_buffered_data(fd_t fd) const
{
    std::map<fd_t, std::string>::const_iterator it;

    it = _buffers.find(fd);
    if (it == _buffers.end())
        return (false);
    return (!it->second.empty());
}

std::string BufferManager::get_buffer_copy(fd_t fd) const
{
    std::map<fd_t, std::string>::const_iterator it;

    it = _buffers.find(fd);
    if (it == _buffers.end())
        return ("");
    return (it->second);
}

void BufferManager::clear_buffer(fd_t fd)
{
    if (has_client(fd))
        _buffers[fd].clear();
}
