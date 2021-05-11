#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <cerrno>
#include <unistd.h>
#include <string>

#include "pipe_msg.hpp"
#include "include/bloom_filter.hpp"
#include "app/app_utils.hpp"

/*
using std::memcpy;
using std::memset;
using std::free;
*/
/*
void sendBloomFilter(int pipe_fd, BloomFilter &filter, int buffer_size)
{
    for(unsigned long int i = 0; i < filter.numBytes; i += buffer_size)
    {
        write(pipe_fd, filter.bits + i, buffer_size);
    }
}*/

void sendMessageType(int pipe_fd, char req_type, char *buffer, unsigned int buffer_size)
{
    unsigned int bytes_to_write, bytes_left;
    int written;
    for(unsigned int sent_bytes = 0; sent_bytes < sizeof(char); sent_bytes += bytes_to_write)
    {
        bytes_left = sizeof(char) - sent_bytes;
        bytes_to_write = bytes_left < buffer_size ? bytes_left : buffer_size;
        memcpy(buffer, &req_type + sent_bytes, bytes_to_write);
        written = write(pipe_fd, buffer, bytes_to_write);
        if (written < bytes_to_write)
        {
            if (written == -1)
            {
                if (errno == EINTR)
                {
                    bytes_to_write = 0;
                }
            }
            else
            {
                bytes_to_write = written;
            }
        }
    }
}

void sendBloomFilter(int pipe_fd, BloomFilter *filter, char *buffer, unsigned int buffer_size)
{
    unsigned int bytes_to_write, bytes_left;
    int written;
    for(unsigned long sent_bytes = 0; sent_bytes < filter->numBytes; sent_bytes += bytes_to_write)
    {
        bytes_left = filter->numBytes - sent_bytes;
        bytes_to_write = bytes_left < buffer_size ? bytes_left : buffer_size;
        memcpy(buffer, filter->bits + sent_bytes, bytes_to_write);
        written = write(pipe_fd, buffer, bytes_to_write);
        if (written < bytes_to_write)
        {
            if (written == -1)
            {
                if (errno == EINTR)
                {
                    bytes_to_write = 0;
                }
            }
            else
            {
                bytes_to_write = written;
            }
        }
    }
}

void sendInt(int pipe_fd, const unsigned int &i, char *buffer, unsigned int buffer_size)
{
    unsigned int bytes_to_write, bytes_left;
    int written;
    for(unsigned int sent_bytes = 0; sent_bytes < sizeof(unsigned int); sent_bytes += bytes_to_write)
    {
        bytes_left = sizeof(unsigned int) - sent_bytes;
        bytes_to_write = bytes_left < buffer_size ? bytes_left : buffer_size;
        memcpy(buffer, (char*)&i + sent_bytes, bytes_to_write);
        written = write(pipe_fd, buffer, bytes_to_write);
        if (written < bytes_to_write)
        {
            if (written == -1)
            {
                if (errno == EINTR)
                {
                    bytes_to_write = 0;
                }
            }
            else
            {
                bytes_to_write = written;
            }
        }
    }
}

void sendShortInt(int pipe_fd, const unsigned short int &i, char *buffer, unsigned int buffer_size)
{
    unsigned int bytes_to_write, bytes_left;
    int written;
    for(unsigned int sent_bytes = 0; sent_bytes < sizeof(unsigned short int); sent_bytes += bytes_to_write)
    {
        bytes_left = sizeof(unsigned short int) - sent_bytes;
        bytes_to_write = bytes_left < buffer_size ? bytes_left : buffer_size;
        memcpy(buffer, (char*)&i + sent_bytes, bytes_to_write);
        written = write(pipe_fd, buffer, bytes_to_write);
        if (written < bytes_to_write)
        {
            if (written == -1)
            {
                if (errno == EINTR)
                {
                    bytes_to_write = 0;
                }
            }
            else
            {
                bytes_to_write = written;
            }
        }
    }
}

void sendLongInt(int pipe_fd, const unsigned long int &i, char *buffer, unsigned int buffer_size)
{
    unsigned int bytes_to_write, bytes_left;
    int written;
    for(unsigned int sent_bytes = 0; sent_bytes < sizeof(unsigned long int); sent_bytes += bytes_to_write)
    {
        bytes_left = sizeof(unsigned long int) - sent_bytes;
        bytes_to_write = bytes_left < buffer_size ? bytes_left : buffer_size;
        memcpy(buffer, (char*)&i + sent_bytes, bytes_to_write);
        written = write(pipe_fd, buffer, bytes_to_write);
        if (written < bytes_to_write)
        {
            if (written == -1)
            {
                if (errno == EINTR)
                {
                    bytes_to_write = 0;
                }
            }
            else
            {
                bytes_to_write = written;
            }
        }
    }
}

void sendString(int pipe_fd, const char *string, char *buffer, unsigned int buffer_size)
{
    unsigned int len = strlen(string) + 1;
    sendInt(pipe_fd, len, buffer, buffer_size);
    unsigned int bytes_to_write, bytes_left;
    int written;
    for (unsigned long sent_bytes = 0; sent_bytes < len; sent_bytes += bytes_to_write)
    {
        bytes_left = len - sent_bytes;
        bytes_to_write = bytes_left < buffer_size ? bytes_left : buffer_size;
        memcpy(buffer, string + sent_bytes, bytes_to_write);
        written = write(pipe_fd, buffer, bytes_to_write);
        if (written < bytes_to_write)
        {
            if (written == -1)
            {
                if (errno == EINTR)
                {
                    bytes_to_write = 0;
                }
                else
                {
                    //problem
                }                
            }
            else
            {
                bytes_to_write = written;
            }
        }
    }    
}

void sendDate(int pipe_fd, const Date &date, char *buffer, unsigned int buffer_size)
{
    sendShortInt(pipe_fd, date.day, buffer, buffer_size);
    sendShortInt(pipe_fd, date.month, buffer, buffer_size);
    sendShortInt(pipe_fd, date.year, buffer, buffer_size);
}

void receiveString(int pipe_fd, char *&string, char *buffer, unsigned int buffer_size)
{
    unsigned int string_length;
    receiveInt(pipe_fd, string_length, buffer, buffer_size);
    string = NULL;
    void *realloc_res;
    unsigned long curr_string_size = 0, received_bytes, bytes_to_read, bytes_left = string_length;
    while (bytes_left > 0)
    {
        bytes_to_read = bytes_left < buffer_size ? bytes_left : buffer_size;
        received_bytes = read(pipe_fd, buffer, bytes_to_read);
        if (received_bytes < 0)
        {
            if (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK)
            {
                received_bytes = 0;
            }
            else
            {
                // problem
            }
        }
        else
        {
            realloc_res = realloc(string, curr_string_size + received_bytes);
            if (realloc_res == NULL)
            {
                free(string);
                // problem
            }
            string = (char*)realloc_res;
            memcpy(string + curr_string_size, buffer, received_bytes);
        }
        bytes_left -= received_bytes;
        curr_string_size += received_bytes;
    }
    if (string[string_length - 1] != '\0')
    // In case something went wrong...
    {
        string[string_length - 1] = '\0';
    }
}

void receiveString(int pipe_fd, std::string &dest_str, char *buffer, unsigned int buffer_size)
{
    // dest_str.clear();
    unsigned int string_length;
    receiveInt(pipe_fd, string_length, buffer, buffer_size);
    void *realloc_res;
    unsigned long received_bytes, bytes_to_read, bytes_left = string_length;
    while (bytes_left > 0)
    {
        bytes_to_read = bytes_left < buffer_size ? bytes_left : buffer_size;
        received_bytes = read(pipe_fd, buffer, bytes_to_read);
        if (received_bytes < 0)
        {
            if (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK)
            {
                received_bytes = 0;
            }
            else
            {
                // problem
            }
        }
        else
        {
            dest_str.append(buffer, received_bytes);
        }
        bytes_left -= received_bytes;
    }
    // Null termination?
}

void receiveBloomFilter(int pipe_fd, BloomFilter &filter, char *buffer, unsigned int buffer_size)
{
    unsigned int bytes_to_read, bytes_left, total_bytes;
    int received_bytes;
    for(unsigned long int total_bytes = 0; total_bytes < filter.numBytes; total_bytes += received_bytes)
    {
        bytes_left = filter.numBytes - total_bytes;
        bytes_to_read = bytes_left < buffer_size ? bytes_left : buffer_size;
        received_bytes = read(pipe_fd, buffer, bytes_to_read);
        if (received_bytes < 0)
        {
            if (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK)
            {
                received_bytes = 0;
            }
            else
            {
                // problem
            }            
        }
        else
        {
            memcpy(filter.bits + total_bytes, buffer, received_bytes);
        }
    }
}

void updateBloomFilter(int pipe_fd, BloomFilter &filter, char *buffer, unsigned int buffer_size)
{
    unsigned int bytes_to_read, bytes_left, total_bytes;
    int received_bytes;
    for(unsigned long int total_bytes = 0; total_bytes < filter.numBytes; total_bytes += received_bytes)
    {
        bytes_left = filter.numBytes - total_bytes;
        bytes_to_read = bytes_left < buffer_size ? bytes_left : buffer_size;
        received_bytes = read(pipe_fd, buffer, bytes_to_read);
        if (received_bytes < 0)
        {
            if (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK)
            {
                received_bytes = 0;
            }
            else
            {
                // problem
            }            
        }
        else
        {
            for (int i = 0; i < received_bytes; i++)
            {
                filter.bits[total_bytes + i] = filter.bits[total_bytes + i] | buffer[i]; 
            }
        }
    }
}

void receiveMessageType(int pipe_fd, char &req_type, char *buffer, unsigned int buffer_size)
{
    unsigned int bytes_to_read, bytes_left;
    int received_bytes;
    for(unsigned long total_bytes = 0; total_bytes < sizeof(char); total_bytes += received_bytes)
    {
        bytes_left = sizeof(char) - total_bytes;
        bytes_to_read = bytes_left < buffer_size ? bytes_left : buffer_size;
        received_bytes = read(pipe_fd, buffer, bytes_to_read);
        if (received_bytes < 0)
        {
            if (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK)
            {
                received_bytes = 0;
            }
            else
            {
                // problem
            }            
        }
        else
        {
            memcpy(&req_type + total_bytes, buffer, received_bytes);
        }
    }
}

void receiveInt(int pipe_fd, unsigned int &i, char *buffer, unsigned int buffer_size)
{
    unsigned int bytes_to_read, bytes_left;
    int received_bytes;
    for(unsigned long total_bytes = 0; total_bytes < sizeof(unsigned int); total_bytes += received_bytes)
    {
        bytes_left = sizeof(unsigned int) - total_bytes;
        bytes_to_read = bytes_left < buffer_size ? bytes_left : buffer_size;
        received_bytes = read(pipe_fd, buffer, bytes_to_read);
        if (received_bytes < 0)
        {
            if (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK)
            {
                received_bytes = 0;
            }
            else
            {
                // problem
            }            
        }
        else
        {
            memcpy((char*)&i + total_bytes, buffer, received_bytes);
        }
    }
}

void receiveShortInt(int pipe_fd, unsigned short int &i, char *buffer, unsigned int buffer_size)
{
    unsigned int bytes_to_read, bytes_left;
    int received_bytes;
    for(unsigned long total_bytes = 0; total_bytes < sizeof(unsigned short int); total_bytes += received_bytes)
    {
        bytes_left = sizeof(unsigned short int) - total_bytes;
        bytes_to_read = bytes_left < buffer_size ? bytes_left : buffer_size;
        received_bytes = read(pipe_fd, buffer, bytes_to_read);
        if (received_bytes < 0)
        {
            if (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK)
            {
                received_bytes = 0;
            }
            else
            {
                // problem
            }            
        }
        else
        {
            memcpy((char*)&i + total_bytes, buffer, received_bytes);
        }
    }
}

void receiveLongInt(int pipe_fd, unsigned long int &i, char *buffer, unsigned int buffer_size)
{
    unsigned int bytes_to_read, bytes_left;
    int received_bytes;
    for(unsigned long total_bytes = 0; total_bytes < sizeof(unsigned long int); total_bytes += received_bytes)
    {
        bytes_left = sizeof(unsigned long int) - total_bytes;
        bytes_to_read = bytes_left < buffer_size ? bytes_left : buffer_size;
        received_bytes = read(pipe_fd, buffer, bytes_to_read);
        if (received_bytes < 0)
        {
            if (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK)
            {
                received_bytes = 0;
            }
            else
            {
                // problem
            }            
        }
        else
        {
            memcpy((char*)&i + total_bytes, buffer, received_bytes);
        }
    }
}

void receiveDate(int pipe_fd, Date &date, char *buffer, unsigned int buffer_size)
{
    receiveShortInt(pipe_fd, date.day, buffer, buffer_size);
    receiveShortInt(pipe_fd, date.month, buffer, buffer_size);
    receiveShortInt(pipe_fd, date.year, buffer, buffer_size);
}
