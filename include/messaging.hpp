/**
 * File: messaging.hpp
 * Routines used for receiving/sending data.
 * Pavlos Spanoudakis (sdi1800184)
 */

#ifndef MSG_HPP
#define MSG_HPP

class BloomFilter;
class Date;

/* Routines used for sending data. */

void sendMessageType(int fd, char req_type, char *buffer, unsigned int buffer_size);

void sendBloomFilter(int fd, BloomFilter *filter, char *buffer, unsigned int buffer_size);

void sendInt(int fd, const unsigned int &i, char *buffer, unsigned int buffer_size);

void sendShortInt(int fd, const unsigned short int &i, char *buffer, unsigned int buffer_size);

void sendLongInt(int fd, const unsigned long int &i, char *buffer, unsigned int buffer_size);

void sendString(int fd, const char *string, char *buffer, unsigned int buffer_size);

void sendDate(int fd, const Date &date, char *buffer, unsigned int buffer_size);

/* Routines used for receiving data. */

void receiveMessageType(int fd, char &req_type, char *buffer, unsigned int buffer_size);

void receiveString(int fd, char *&string, char *buffer, unsigned int buffer_size);

void updateBloomFilter(int fd, BloomFilter *filter, char *buffer, unsigned int buffer_size);

void receiveInt(int fd, unsigned int &i, char *buffer, unsigned int buffer_size);

void receiveShortInt(int fd, unsigned short int &i, char *buffer, unsigned int buffer_size);

void receiveLongInt(int fd, unsigned long int &i, char *buffer, unsigned int buffer_size);

void receiveDate(int fd, Date &date, char *buffer, unsigned int buffer_size);

#endif
