/**
 * File: pipe_msg.hpp
 * Routines used for receiving/sending data through fifo pipes.
 * Pavlos Spanoudakis (sdi1800184)
 */

#ifndef PIPE_MSG_HPP
#define PIPE_MSG_HPP

/* Defined Message Type codes. */
#define TRAVEL_REQUEST 1
#define SEARCH_STATUS 2
#define TRAVEL_REQUEST_ACCEPTED 3
#define TRAVEL_REQUEST_REJECTED 4
#define TRAVEL_REQUEST_INVALID 5
#define CITIZEN_FOUND 6
#define CITIZEN_NOT_FOUND 7

class BloomFilter;
class Date;

void sendMessageType(int pipe_fd, char req_type, char *buffer, unsigned int buffer_size);

void sendBloomFilter(int pipe_fd, BloomFilter *filter, char *buffer, unsigned int buffer_size);

void sendInt(int pipe_fd, const unsigned int &i, char *buffer, unsigned int buffer_size);

void sendShortInt(int pipe_fd, const unsigned short int &i, char *buffer, unsigned int buffer_size);

void sendLongInt(int pipe_fd, const unsigned long int &i, char *buffer, unsigned int buffer_size);

void sendString(int pipe_fd, const char *string, char *buffer, unsigned int buffer_size);

void sendDate(int pipe_fd, const Date &date, char *buffer, unsigned int buffer_size);

void receiveMessageType(int pipe_fd, char &req_type, char *buffer, unsigned int buffer_size);

void receiveString(int pipe_fd, char *&string, char *buffer, unsigned int buffer_size);

// receive might not be needed
void receiveBloomFilter(int pipe_fd, BloomFilter &filter, char *buffer, unsigned int buffer_size);
void updateBloomFilter(int pipe_fd, BloomFilter *filter, char *buffer, unsigned int buffer_size);

void receiveInt(int pipe_fd, unsigned int &i, char *buffer, unsigned int buffer_size);

void receiveShortInt(int pipe_fd, unsigned short int &i, char *buffer, unsigned int buffer_size);

void receiveLongInt(int pipe_fd, unsigned long int &i, char *buffer, unsigned int buffer_size);

void receiveDate(int pipe_fd, Date &date, char *buffer, unsigned int buffer_size);

#endif
