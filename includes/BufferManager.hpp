#ifndef BUFFERMANAGER_HPP
#define BUFFERMANAGER_HPP

#include <map>
#include <string>
#include <vector>

typedef int fd_t;

class BufferManager
{
    private:
        std::map<fd_t, std::string> _buffers;
        size_t                      _max_line_length;

    public:
        BufferManager(void);
        BufferManager(const BufferManager &other);
        BufferManager &operator=(const BufferManager &other);
        ~BufferManager(void);

        void set_max_line_length(size_t max_len);

        void add_client(fd_t fd);
        void remove_client(fd_t fd);
        bool has_client(fd_t fd) const;

        void append_data(fd_t fd, const std::string &chunk);

        std::vector<std::string> extract_complete_lines(fd_t fd);

        bool has_buffered_data(fd_t fd) const;
        std::string get_buffer_copy(fd_t fd) const;
        void clear_buffer(fd_t fd);
};

#endif
