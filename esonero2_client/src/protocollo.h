/*
 ============================================================================
 Name        : protocollo.h
 Author      : Niccolò Lamonaca
 Version     :
 Copyright   : Your copyright notice
 ============================================================================
 */

#ifndef PROTOCOLLO_H_
#define PROTOCOLLO_H_

#define BUFFMAX 255 // this define the size of the buffer
#define DEFAULT_IP "127.0.0.1" //here is defined the default ip
#define DEFAULT_SERVER "passwdgen.uniba.it" //here is defined the name associated to the default ip
#define DEFAULT_PORT 60000 //this define the default port
#define NO_ERROR 0 //this is like a "variable" used into some ifs


#define MAX 32 // this define the max length of the password
#define MIN 6 // this define the min length of the password

#define QLEN 5

// this is the struct for the protocol requests
typedef struct {
    char request_type;
    int length;
    char password[MAX + 1];
} request;

#endif /* PROTOCOLLO_H_ */
