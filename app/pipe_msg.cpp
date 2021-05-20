/**
 * File: pipe_msg.cpp
 * Implementations of routines used for receiving/sending data through fifo pipes.
 * Pavlos Spanoudakis (sdi1800184)
 */

#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <cerrno>
#include <unistd.h>
#include <string>

#include "pipe_msg.hpp"
#include "../include/bloom_filter.hpp"
#include "app_utils.hpp"

/**
 * @brief Writes the specified message type in the fifo pipe with the given file descriptor,
 * using the given buffer with the specified size.
 * 
 * @param pipe_fd The file descriptor of the fifo pipe to write the data.
 * @param req_type The message type to write to the pipe.
 * @param buffer The buffer to copy the data before writing to the pipe.
 * @param buffer_size The size of the buffer.
 */
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
                else
                {
                    perror("Fatal error while writing to pipe.\n");
                    exit(EXIT_FAILURE);
                }
            }
            else
            {
                bytes_to_write = written;
            }
        }
    }
}

/**
 * @brief Writes the bytes array of the specified Bloom Filter in the fifo pipe
 *  with the given file descriptor, using the given buffer with the specified size.
 * 
 * @param pipe_fd The file descriptor of the fifo pipe to write the data.
 * @param filter The Bloom Filter to send.
 * @param buffer The buffer to copy the data before writing to the pipe.
 * @param buffer_size The size of the buffer.
 */
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
                else
                {
                    perror("Fatal error while writing to pipe.\n");
                    exit(EXIT_FAILURE);
                }
            }
            else
            {
                bytes_to_write = written;
            }
        }
    }
}

/**
 * @brief Writes the specified integer in the fifo pipe
 *  with the given file descriptor, using the given buffer with the specified size.
 * 
 * @param pipe_fd The file descriptor of the fifo pipe to write the data.
 * @param i The integer to send.
 * @param buffer The buffer to copy the data before writing to the pipe.
 * @param buffer_size The size of the buffer.
 */
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
                else
                {
                    perror("Fatal error while writing to pipe.\n");
                    exit(EXIT_FAILURE);
                }
            }
            else
            {
                bytes_to_write = written;
            }
        }
    }
}

/**
 * @brief Writes the specified short integer in the fifo pipe
 *  with the given file descriptor, using the given buffer with the specified size.
 * 
 * @param pipe_fd The file descriptor of the fifo pipe to write the data.
 * @param i The short integer to send.
 * @param buffer The buffer to copy the data before writing to the pipe.
 * @param buffer_size The size of the buffer.
 */
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
                else
                {
                    perror("Fatal error while writing to pipe.\n");
                    exit(EXIT_FAILURE);
                }
            }
            else
            {
                bytes_to_write = written;
            }
        }
    }
}

/**
 * @brief Writes the specified long integer in the fifo pipe
 *  with the given file descriptor, using the given buffer with the specified size.
 * 
 * @param pipe_fd The file descriptor of the fifo pipe to write the data.
 * @param i The long integer to send.
 * @param buffer The buffer to copy the data before writing to the pipe.
 * @param buffer_size The size of the buffer.
 */
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
                else
                {
                    perror("Fatal error while writing to pipe.\n");
                    exit(EXIT_FAILURE);
                }
            }
            else
            {
                bytes_to_write = written;
            }
        }
    }
}

/**
 * @brief Sends the specified string through the fifo pipe
 *  with the given file descriptor, using the given buffer with the specified size.
 * 
 * @param pipe_fd The file descriptor of the fifo pipe to write the data.
 * @param string The string to send.
 * @param buffer The buffer to copy the data before writing to the pipe.
 * @param buffer_size The size of the buffer.
 */
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
                    perror("Fatal error while writing to pipe.\n");
                    exit(EXIT_FAILURE);
                }                
            }
            else
            {
                bytes_to_write = written;
            }
        }
    }    
}


/**
 * @brief Sends the specified Date through the fifo pipe
 *  with the given file descriptor, using the given buffer with the specified size.
 * 
 * @param pipe_fd The file descriptor of the fifo pipe to write the data.
 * @param date The date to send.
 * @param buffer The buffer to copy the data before writing to the pipe.
 * @param buffer_size The size of the buffer.
 */
void sendDate(int pipe_fd, const Date &date, char *buffer, unsigned int buffer_size)
{
    sendShortInt(pipe_fd, date.day, buffer, buffer_size);
    sendShortInt(pipe_fd, date.month, buffer, buffer_size);
    sendShortInt(pipe_fd, date.year, buffer, buffer_size);
}


/**
 * @brief Reads a string from the fifo pipe with the given file descriptor,
 * using the given buffer with the specified size, and stores it in a heap character array
 * pointed by the given string pointer. The pointer must be free-ed after use.
 * 
 * @param pipe_fd The file descriptor of the fifo pipe to read data from.
 * @param string This will point to the received data.
 * @param buffer The buffer to store newly received data from the pipe.
 * @param buffer_size The size of the buffer.
 */
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
                perror("Fatal error while reading from pipe.\n");
                exit(EXIT_FAILURE);
            }
        }
        else
        {
            realloc_res = realloc(string, curr_string_size + received_bytes);
            if (realloc_res == NULL)
            {
                perror("Failed realloc() call.\n");
                exit(EXIT_FAILURE);
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

void receiveStringAlt(int pipe_fd, char *&string, char *buffer, unsigned int buffer_size)
{
    unsigned int string_length;
    receiveInt(pipe_fd, string_length, buffer, buffer_size);
    string = (char*)malloc(string_length);
    if (string == NULL)
    {
        perror("Failed malloc() call.\n");
        exit(EXIT_FAILURE);
    }
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
                perror("Fatal error while reading from pipe.\n");
                exit(EXIT_FAILURE);
            }
        }
        else
        {
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
                perror("Fatal error while reading from pipe.\n");
                exit(EXIT_FAILURE);
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
    unsigned int bytes_to_read, bytes_left;
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
                perror("Fatal error while reading from pipe.\n");
                exit(EXIT_FAILURE);
            }           
        }
        else
        {
            memcpy(filter.bits + total_bytes, buffer, received_bytes);
        }
    }
}

/**
 * @brief Reads a Bloom Filter byte array from the fifo pipe with the given file descriptor,
 * using the given buffer with the specified size, and "updates" the byte array of the given Bloom Filter
 * using bitwise-OR.
 * 
 * @param pipe_fd The file descriptor of the fifo pipe to read data from.
 * @param filter The Bloom Filter to be updated based on the received data.
 * @param buffer The buffer to store newly received data from the pipe.
 * @param buffer_size The size of the buffer.
 */
void updateBloomFilter(int pipe_fd, BloomFilter *filter, char *buffer, unsigned int buffer_size)
{
    unsigned int bytes_to_read, bytes_left;
    int received_bytes;
    for(unsigned long int total_bytes = 0; total_bytes < filter->numBytes; total_bytes += received_bytes)
    {
        bytes_left = filter->numBytes - total_bytes;
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
                perror("Fatal error while reading from pipe.\n");
                exit(EXIT_FAILURE);
            }            
        }
        else
        {
            for (int i = 0; i < received_bytes; i++)
            {
                filter->bits[total_bytes + i] = filter->bits[total_bytes + i] | buffer[i]; 
            }
        }
    }
}

/**
 * @brief Reads a Message Type from the fifo pipe with the given file descriptor,
 * using the given buffer with the specified size, and stores it in the given char variable.
 * 
 * @param pipe_fd The file descriptor of the fifo pipe to read data from.
 * @param req_type The variable to store the received Message Type.
 * @param buffer The buffer to store newly received data from the pipe.
 * @param buffer_size The size of the buffer.
 */
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
                perror("Fatal error while reading from pipe.\n");
                exit(EXIT_FAILURE);
            }          
        }
        else
        {
            memcpy(&req_type + total_bytes, buffer, received_bytes);
        }
    }
}

/**
 * @brief Reads an integer from the fifo pipe with the given file descriptor,
 * using the given buffer with the specified size, and stores it in the given integer variable.
 * 
 * @param pipe_fd The file descriptor of the fifo pipe to read data from.
 * @param i The variable to store the received integer
 * @param buffer The buffer to store newly received data from the pipe.
 * @param buffer_size The size of the buffer.
 */
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
                perror("Fatal error while reading from pipe.\n");
                exit(EXIT_FAILURE);
            }          
        }
        else
        {
            memcpy((char*)&i + total_bytes, buffer, received_bytes);
        }
    }
}

/**
 * @brief Reads a short integer from the fifo pipe with the given file descriptor,
 * using the given buffer with the specified size, and stores it in the given short integer variable.
 * 
 * @param pipe_fd The file descriptor of the fifo pipe to read data from.
 * @param i The variable to store the received short integer
 * @param buffer The buffer to store newly received data from the pipe.
 * @param buffer_size The size of the buffer.
 */
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
                perror("Fatal error while reading from pipe.\n");
                exit(EXIT_FAILURE);
            }
        }
        else
        {
            memcpy((char*)&i + total_bytes, buffer, received_bytes);
        }
    }
}

/**
 * @brief Reads a long integer from the fifo pipe with the given file descriptor,
 * using the given buffer with the specified size, and stores it in the given long integer variable.
 * 
 * @param pipe_fd The file descriptor of the fifo pipe to read data from.
 * @param i The variable to store the received long integer
 * @param buffer The buffer to store newly received data from the pipe.
 * @param buffer_size The size of the buffer.
 */
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
                perror("Fatal error while reading from pipe.\n");
                exit(EXIT_FAILURE);
            }
        }
        else
        {
            memcpy((char*)&i + total_bytes, buffer, received_bytes);
        }
    }
}

/**
 * @brief Reads a Date from the fifo pipe with the given file descriptor,
 * using the given buffer with the specified size, and stores it in the specified Date object.
 * 
 * @param pipe_fd The file descriptor of the fifo pipe to read data from.
 * @param date The date object to store the received date.
 * @param buffer The buffer to store newly received data from the pipe.
 * @param buffer_size The size of the buffer.
 */
void receiveDate(int pipe_fd, Date &date, char *buffer, unsigned int buffer_size)
{
    receiveShortInt(pipe_fd, date.day, buffer, buffer_size);
    receiveShortInt(pipe_fd, date.month, buffer, buffer_size);
    receiveShortInt(pipe_fd, date.year, buffer, buffer_size);
}
