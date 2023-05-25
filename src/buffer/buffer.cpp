#include "buffer/buffer.h"

using tubekit::buffer::buffer;

buffer::buffer(u_int64_t limit_max)
{
    m_size = limit_max < 10 ? limit_max : 10;
    m_limit_max = limit_max;
    m_buffer = (char *)malloc(m_size);
    m_read_ptr = m_buffer;
    m_write_ptr = m_buffer;
    m_last_read = time(nullptr);
    m_last_write = time(nullptr);
}

buffer::~buffer()
{
    if (m_buffer)
    {
        free(m_buffer);
    }
}

time_t buffer::get_last_read_gap()
{
    time_t now = time(nullptr);
    return now - m_last_read;
}

time_t buffer::get_last_write_gap()
{
    time_t now = time(nullptr);
    return now - m_last_write;
}

u_int64_t buffer::read(char *dest, u_int64_t size)
{
    std::lock_guard<std::mutex> guard(m_mutex);
    if (dest == nullptr || size == 0)
    {
        throw std::runtime_error("dest == nullptr || size == 0");
    }
    // read size of bytes from m_buffer
    u_int64_t readed_size = 0;
    while (size > 0 && m_read_ptr != m_write_ptr)
    {
        dest[readed_size] = *m_read_ptr;
        readed_size++, size--;
        m_read_ptr++;
    }
    // update m_last_read
    m_last_read = time(nullptr);
    return readed_size;
}

u_int64_t buffer::write(const char *source, u_int64_t size)
{
    std::lock_guard<std::mutex> guard(m_mutex);
    if (source == nullptr || size == 0)
    {
        throw std::runtime_error("dest == nullptr || size == 0");
    }
    // check buffer size
    if (!check_and_write(source, size))
    {
        // move to before
        move_to_before();
        if (check_and_write(source, size))
        {
            m_last_write = time(nullptr);
            return size;
        }
        else
        {
            // realloc
            u_int64_t should_add_size = size - after_size();
            if (should_add_size + m_size > m_limit_max)
            {
                throw std::runtime_error("should_add_size + m_size > m_limit_max");
            }
            char *new_buffer = (char *)realloc(m_buffer, m_size + should_add_size);
            if (new_buffer)
            {
                m_buffer = new_buffer;
                m_size = m_size + should_add_size;
            }
            if (check_and_write(source, size))
            {
                m_last_write = time(nullptr);
                return size;
            }
            throw std::runtime_error("realloc error");
        }
    }
    return size;
}

bool buffer::check_and_write(const char *source, u_int64_t size)
{
    u_int64_t m_after_size = after_size();
    if (size <= m_after_size)
    {
        memcpy(m_write_ptr, source, size);
        m_write_ptr = m_write_ptr + size;
        m_last_write = time(nullptr);
        return true;
    }
    return false;
}

void buffer::move_to_before()
{
    char *tmp = m_buffer;
    char *ptr = m_read_ptr;
    while (ptr != m_write_ptr)
    {
        *tmp = *ptr;
        tmp++;
        ptr++;
    }
    m_read_ptr = m_buffer;
    m_write_ptr = tmp;
}

u_int64_t buffer::after_size()
{
    char *last_buffer_ptr = m_buffer + m_size - 1;
    u_int64_t after_size = last_buffer_ptr - m_write_ptr + 1;
    return after_size;
}