#ifndef PIPE_MSG_HPP
#define PIPE_MSG_HPP

#define BLOOM_TRANSFER 1
#define TRAVEL_REQUEST 2
#define TRAVEL_STATS 3
#define SEARCH_STATUS 4
#define TRAVEL_REQUEST_ACCEPTED 5
#define TRAVEL_REQUEST_REJECTED 6
#define TRAVEL_REQUEST_INVALID 7
#define CITIZEN_FOUND 8
#define CITIZEN_NOT_FOUND 9

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
void updateBloomFilter(int pipe_fd, BloomFilter &filter, char *buffer, unsigned int buffer_size);

void receiveInt(int pipe_fd, unsigned int &i, char *buffer, unsigned int buffer_size);

void receiveShortInt(int pipe_fd, unsigned short int &i, char *buffer, unsigned int buffer_size);

void receiveLongInt(int pipe_fd, unsigned long int &i, char *buffer, unsigned int buffer_size);

void receiveDate(int pipe_fd, Date &date, char *buffer, unsigned int buffer_size);

#endif
