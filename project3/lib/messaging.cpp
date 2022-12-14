/**
 * File: messaging.cpp
 * Implementations of routines used for receiving/sending data.
 * Pavlos Spanoudakis (sdi1800184)
 */

#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <cerrno>
#include <unistd.h>
#include <netinet/in.h>

#include "../include/messaging.hpp"
#include "../include/bloom_filter.hpp"
#include "../app/app_utils.hpp"

/**
 * @brief Writes the specified message type in the file with the given file descriptor,
 * using the given buffer with the specified size.
 * 
 * @param fd The file descriptor of the file to write the data.
 * @param req_type The message type to write to the file.
 * @param buffer The buffer to copy the data before writing to the file.
 * @param buffer_size The size of the buffer.
 */
void sendMessageType(int fd, char req_type, char *buffer, unsigned int buffer_size)
{
    unsigned int bytes_to_write, bytes_left;
    int written;
    for(unsigned int sent_bytes = 0; sent_bytes < sizeof(char); sent_bytes += bytes_to_write)
    {
        bytes_left = sizeof(char) - sent_bytes;
        bytes_to_write = bytes_left < buffer_size ? bytes_left : buffer_size;
        memcpy(buffer, &req_type + sent_bytes, bytes_to_write);
        written = write(fd, buffer, bytes_to_write);
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
                    perror("Fatal error while writing to file.\n");
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
 * @brief Writes the bytes array of the specified Bloom Filter in the file
 *  with the given file descriptor, using the given buffer with the specified size.
 * 
 * @param fd The file descriptor of the file to write the data.
 * @param filter The Bloom Filter to send.
 * @param buffer The buffer to copy the data before writing to the file.
 * @param buffer_size The size of the buffer.
 */
void sendBloomFilter(int fd, BloomFilter *filter, char *buffer, unsigned int buffer_size)
{
    unsigned int bytes_to_write, bytes_left;
    int written;
    for(unsigned long sent_bytes = 0; sent_bytes < filter->numBytes; sent_bytes += bytes_to_write)
    {
        bytes_left = filter->numBytes - sent_bytes;
        bytes_to_write = bytes_left < buffer_size ? bytes_left : buffer_size;
        memcpy(buffer, filter->bits + sent_bytes, bytes_to_write);
        written = write(fd, buffer, bytes_to_write);
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
                    perror("Fatal error while writing to file.\n");
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
 * @brief Writes the specified integer in the file with the given file descriptor,
 * using the given buffer with the specified size.
 * 
 * @param fd The file descriptor of the file to write the data.
 * @param i The integer to send.
 * @param buffer The buffer to copy the data before writing to the file.
 * @param buffer_size The size of the buffer.
 */
void sendInt(int fd, const unsigned int i, char *buffer, unsigned int buffer_size)
{
    uint32_t net_i = ntohl(i);
    unsigned int bytes_to_write, bytes_left;
    int written;
    for(unsigned int sent_bytes = 0; sent_bytes < sizeof(uint32_t); sent_bytes += bytes_to_write)
    {
        bytes_left = sizeof(uint32_t) - sent_bytes;
        bytes_to_write = bytes_left < buffer_size ? bytes_left : buffer_size;
        memcpy(buffer, (char*)&net_i + sent_bytes, bytes_to_write);
        written = write(fd, buffer, bytes_to_write);
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
                    perror("Fatal error while writing to file.\n");
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
 * @brief Writes the specified short integer in the file
 *  with the given file descriptor, using the given buffer with the specified size.
 * 
 * @param fd The file descriptor of the file to write the data.
 * @param i The short integer to send.
 * @param buffer The buffer to copy the data before writing to the file.
 * @param buffer_size The size of the buffer.
 */
void sendShortInt(int fd, const unsigned short int i, char *buffer, unsigned int buffer_size)
{
    uint16_t net_i = htons(i);
    unsigned int bytes_to_write, bytes_left;
    int written;
    for(unsigned int sent_bytes = 0; sent_bytes < sizeof(uint16_t); sent_bytes += bytes_to_write)
    {
        bytes_left = sizeof(uint16_t) - sent_bytes;
        bytes_to_write = bytes_left < buffer_size ? bytes_left : buffer_size;
        memcpy(buffer, (char*)&net_i + sent_bytes, bytes_to_write);
        written = write(fd, buffer, bytes_to_write);
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
                    perror("Fatal error while writing to file.\n");
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
 * @brief Sends the specified string through the file
 *  with the given file descriptor, using the given buffer with the specified size.
 * 
 * @param fd The file descriptor of the file to write the data.
 * @param string The string to send.
 * @param buffer The buffer to copy the data before writing to the file.
 * @param buffer_size The size of the buffer.
 */
void sendString(int fd, const char *string, char *buffer, unsigned int buffer_size)
{
    unsigned int len = strlen(string) + 1;
    sendInt(fd, len, buffer, buffer_size);
    unsigned int bytes_to_write, bytes_left;
    int written;
    for (unsigned long sent_bytes = 0; sent_bytes < len; sent_bytes += bytes_to_write)
    {
        bytes_left = len - sent_bytes;
        bytes_to_write = bytes_left < buffer_size ? bytes_left : buffer_size;
        memcpy(buffer, string + sent_bytes, bytes_to_write);
        written = write(fd, buffer, bytes_to_write);
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
                    perror("Fatal error while writing to file.\n");
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
 * @brief Sends the specified Date through the file
 *  with the given file descriptor, using the given buffer with the specified size.
 * 
 * @param fd The file descriptor of the file to write the data.
 * @param date The date to send.
 * @param buffer The buffer to copy the data before writing to the file.
 * @param buffer_size The size of the buffer.
 */
void sendDate(int fd, const Date &date, char *buffer, unsigned int buffer_size)
{
    sendShortInt(fd, date.day, buffer, buffer_size);
    sendShortInt(fd, date.month, buffer, buffer_size);
    sendShortInt(fd, date.year, buffer, buffer_size);
}

/**
 * @brief Reads a string from the file with the given file descriptor,
 * using the given buffer with the specified size, and stores it in a heap character array
 * pointed by the given string pointer. The pointer must be free-ed after use.
 * 
 * @param fd The file descriptor of the file to read data from.
 * @param string This will point to the received data.
 * @param buffer The buffer to store newly received data from the file.
 * @param buffer_size The size of the buffer.
 */
void receiveString(int fd, char *&string, char *buffer, unsigned int buffer_size)
{
    unsigned int string_length;
    receiveInt(fd, string_length, buffer, buffer_size);
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
        received_bytes = read(fd, buffer, bytes_to_read);
        if (received_bytes < 0)
        {
            if (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK)
            {
                received_bytes = 0;
            }
            else
            {
                perror("Fatal error while reading from file.\n");
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

/**
 * @brief Reads a Bloom Filter byte array from the file with the given file descriptor,
 * using the given buffer with the specified size, and "updates" the byte array of the given Bloom Filter
 * using bitwise-OR.
 * 
 * @param fd The file descriptor of the file to read data from.
 * @param filter The Bloom Filter to be updated based on the received data.
 * @param buffer The buffer to store newly received data from the file.
 * @param buffer_size The size of the buffer.
 */
void updateBloomFilter(int fd, BloomFilter *filter, char *buffer, unsigned int buffer_size)
{
    unsigned int bytes_to_read, bytes_left;
    int received_bytes;
    for(unsigned long int total_bytes = 0; total_bytes < filter->numBytes; total_bytes += received_bytes)
    {
        bytes_left = filter->numBytes - total_bytes;
        bytes_to_read = bytes_left < buffer_size ? bytes_left : buffer_size;
        received_bytes = read(fd, buffer, bytes_to_read);
        if (received_bytes < 0)
        {
            if (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK)
            {
                received_bytes = 0;
            }
            else
            {
                perror("Fatal error while reading from file.\n");
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
 * @brief Reads a Message Type from the file with the given file descriptor,
 * using the given buffer with the specified size, and stores it in the given char variable.
 * 
 * @param fd The file descriptor of the file to read data from.
 * @param req_type The variable to store the received Message Type.
 * @param buffer The buffer to store newly received data from the file.
 * @param buffer_size The size of the buffer.
 */
void receiveMessageType(int fd, char &req_type, char *buffer, unsigned int buffer_size)
{
    unsigned int bytes_to_read, bytes_left;
    int received_bytes;
    for(unsigned long total_bytes = 0; total_bytes < sizeof(char); total_bytes += received_bytes)
    {
        bytes_left = sizeof(char) - total_bytes;
        bytes_to_read = bytes_left < buffer_size ? bytes_left : buffer_size;
        received_bytes = read(fd, buffer, bytes_to_read);
        if (received_bytes < 0)
        {
            if (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK)
            {
                received_bytes = 0;
            }
            else
            {
                perror("Fatal error while reading from file.\n");
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
 * @brief Reads an integer from the file with the given file descriptor,
 * using the given buffer with the specified size, and stores it in the given integer variable.
 * 
 * @param fd The file descriptor of the file to read data from.
 * @param i The variable to store the received integer
 * @param buffer The buffer to store newly received data from the file.
 * @param buffer_size The size of the buffer.
 */
void receiveInt(int fd, unsigned int &i, char *buffer, unsigned int buffer_size)
{
    uint32_t net_i;
    unsigned int bytes_to_read, bytes_left;
    int received_bytes;
    for(unsigned long total_bytes = 0; total_bytes < sizeof(unsigned int); total_bytes += received_bytes)
    {
        bytes_left = sizeof(unsigned int) - total_bytes;
        bytes_to_read = bytes_left < buffer_size ? bytes_left : buffer_size;
        received_bytes = read(fd, buffer, bytes_to_read);
        if (received_bytes < 0)
        {
            if (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK)
            {
                received_bytes = 0;
            }
            else
            {
                perror("Fatal error while reading from file.\n");
                exit(EXIT_FAILURE);
            }          
        }
        else
        {
            memcpy((char*)&net_i + total_bytes, buffer, received_bytes);
        }
    }
    i = ntohl(net_i);
}

/**
 * @brief Reads a short integer from the file with the given file descriptor,
 * using the given buffer with the specified size, and stores it in the given short integer variable.
 * 
 * @param fd The file descriptor of the file to read data from.
 * @param i The variable to store the received short integer
 * @param buffer The buffer to store newly received data from the file.
 * @param buffer_size The size of the buffer.
 */
void receiveShortInt(int fd, unsigned short int &i, char *buffer, unsigned int buffer_size)
{
    uint16_t net_i;
    unsigned int bytes_to_read, bytes_left;
    int received_bytes;
    for(unsigned long total_bytes = 0; total_bytes < sizeof(unsigned short int); total_bytes += received_bytes)
    {
        bytes_left = sizeof(unsigned short int) - total_bytes;
        bytes_to_read = bytes_left < buffer_size ? bytes_left : buffer_size;
        received_bytes = read(fd, buffer, bytes_to_read);
        if (received_bytes < 0)
        {
            if (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK)
            {
                received_bytes = 0;
            }
            else
            {
                perror("Fatal error while reading from file.\n");
                exit(EXIT_FAILURE);
            }
        }
        else
        {
            memcpy((char*)&net_i + total_bytes, buffer, received_bytes);
        }
    }
    i = ntohs(net_i);
}

/**
 * @brief Reads a Date from the file with the given file descriptor,
 * using the given buffer with the specified size, and stores it in the specified Date object.
 * 
 * @param fd The file descriptor of the file to read data from.
 * @param date The date object to store the received date.
 * @param buffer The buffer to store newly received data from the file.
 * @param buffer_size The size of the buffer.
 */
void receiveDate(int fd, Date &date, char *buffer, unsigned int buffer_size)
{
    receiveShortInt(fd, date.day, buffer, buffer_size);
    receiveShortInt(fd, date.month, buffer, buffer_size);
    receiveShortInt(fd, date.year, buffer, buffer_size);
}
