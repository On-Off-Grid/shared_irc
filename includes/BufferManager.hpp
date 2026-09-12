#ifndef BUFFERMANAGER_HPP
#define BUFFERMANAGER_HPP

#include <string>
#include <vector>

class BufferManager
{
    private:
        std::string _buffer;
        size_t      _max_line_length;

    public:
        BufferManager(void);
        BufferManager(const BufferManager &other);
        BufferManager &operator=(const BufferManager &other);
        ~BufferManager(void);

        void set_max_line_length(size_t max_len);

        void append_data(const std::string &chunk);

        std::vector<std::string> extract_complete_lines();

        bool has_buffered_data() const;
        std::string get_buffer_copy() const;
        void clear_buffer();
};

#endif
