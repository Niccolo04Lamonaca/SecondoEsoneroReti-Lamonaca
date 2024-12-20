/*
 ============================================================================
 Name        : esonero2_server.c
 Author      : Niccolò Lamonaca
 Version     :
 Copyright   : Your copyright notice
 ============================================================================
 */

#include <time.h> // For time-related functions.

// Conditional inclusion of headers depending on the operating system.
#if defined WIN32
#include <winsock.h> // For Windows socket programming.
#else
#include <string.h>   // For string manipulation functions.
#include <unistd.h>   // For POSIX API functions like close(), read(), write().
#include <sys/types.h> // For data types used in socket programming.
#include <sys/socket.h> // For socket functions and structures.
#include <arpa/inet.h>  // For internet operations like htons() and inet_pton().
#include <netinet/in.h> // For sockaddr_in and other networking structures.
#include <netdb.h>      // For DNS resolution and network database operations.
#include <ctype.h>      // For character type functions like isdigit().
#include <stdlib.h>     // For general utility functions like malloc(), free().
#include <time.h>       // For time-related functions
#define closesocket close // Define closesocket to close for compatibility with UNIX.
#endif

#include <stdio.h>  // For standard input/output functions like printf(), scanf().
#include <stdlib.h> // For memory management and other utility functions.
#include <string.h> // For string manipulation functions (repeated, could be removed if redundant).
#include "protocollo.h" // Include the custom header file for protocol definitions.

// Function to handle and print error messages
void errorhandler(const char *errorMessage) {
    perror(errorMessage);
}

// Clears socket resources
void clearwinsock() {
#if defined WIN32
    WSACleanup();
#endif
}

//prototipes of the functions that will generate the passwords
void generate_numeric(char *password, int length);
void generate_alpha(char *password, int length);
void generate_mixed(char *password, int length);
void generate_secure(char *password, int length);
void generate_unambiguous(char *password, int length);

//this is the main function of the program
int main(void) {
    #if defined WIN32
        // Initialize Windows Sockets API on Windows systems
        WSADATA wsa_data;
        if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0) {
            errorhandler("Error at WSAStartup()");
            return 0;
        }
    #endif

    // Initialize random seed for password generation
    srand((unsigned int)time(NULL));

    // Create a UDP socket
    int server_socket = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (server_socket < 0) {
        errorhandler("Error creating socket");
        clearwinsock();
        return -1;
    }

    // Define server and client address structures
    struct sockaddr_in server_address;
    struct sockaddr_in client_address;
    unsigned int client_address_length = sizeof(client_address);

    // Use a default server address for testing
    memset(&server_address, 0, sizeof(server_address)); // Initialize server address to zero
    server_address.sin_family = AF_INET;                // Set address family to Internet
    server_address.sin_port = htons(DEFAULT_PORT);      // Convert port number to network byte order
    server_address.sin_addr.s_addr = inet_addr(DEFAULT_IP); // Set default IP address

    // Bind the socket to the server address
    if (bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        errorhandler("bind() failed");
        closesocket(server_socket);
        clearwinsock();
        return EXIT_FAILURE;
    }

    printf("\nServer ready listening on port: %d\n", DEFAULT_PORT);

    // Infinite loop to handle incoming requests
    while (1) {
        char buffer[BUFFMAX]; // Buffer to hold received data
        int recv_len = recvfrom(server_socket, buffer, BUFFMAX, 0, (struct sockaddr *)&client_address, &client_address_length);
        if (recv_len < 0) {
            errorhandler("recvfrom() failed");
            continue; // Continue to next iteration on failure
        }

        // Print client details
        printf("\n\n - New request from %s:%d\n", inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port));

        // Deserialize the request
        request req;
        memcpy(&req, buffer, sizeof(req));
        if (req.request_type == 'q') { // Handle disconnection request
            printf("Client requested disconnection\n");
            continue;
        }

        // Validate requested password length
        if (req.length < MIN || req.length > MAX) {
            printf("Invalid password length\n");
            continue;
        }

        // Generate the requested password type based on the request
        switch (req.request_type) {
            case 'n': // Numeric password
                generate_numeric(req.password, req.length);
                break;
            case 'a': // Alphabetic password
                generate_alpha(req.password, req.length);
                break;
            case 'm': // Mixed alphanumeric password
                generate_mixed(req.password, req.length);
                break;
            case 's': // Secure password with special characters
                generate_secure(req.password, req.length);
                break;
            case 'u': // Unambiguous password (no confusing characters)
                generate_unambiguous(req.password, req.length);
                break;
            default: // Handle invalid request type
                printf("Invalid request type\n");
                continue;
        }

        // Send the generated password back to the client
        sendto(server_socket, (const char *)&req, sizeof(req), 0, (struct sockaddr *)&client_address, client_address_length);
    }

    // Close the server socket and clean up resources
    closesocket(server_socket);
    clearwinsock();
    return 0;
}

// Generate a numeric password
void generate_numeric(char *password, int length) {
    for (int i = 0; i < length; ++i)
        password[i] = '0' + (rand() % 10); // Random digit from 0 to 9
    password[length] = '\0'; // Null-terminate the string
}

// Generate an alphabetic password
void generate_alpha(char *password, int length) {
    for (int i = 0; i < length; ++i)
        password[i] = 'a' + (rand() % 26); // Random lowercase letter
    password[length] = '\0'; // Null-terminate the string
}

// Generate a mixed alphanumeric password
void generate_mixed(char *password, int length) {
    for (int i = 0; i < length; ++i) {
        if (rand() % 2) // Randomly choose between letter and digit
            password[i] = 'a' + (rand() % 26); // Random letter
        else
            password[i] = '0' + (rand() % 10); // Random digit
    }
    password[length] = '\0'; // Null-terminate the string
}

// Generate a secure password with special characters
void generate_secure(char *password, int length) {
    const char charset[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789!@#$%^&*()";
    int charset_size = sizeof(charset) - 1; // Number of characters in the charset
    for (int i = 0; i < length; ++i)
        password[i] = charset[rand() % charset_size]; // Random character from the charset
    password[length] = '\0'; // Null-terminate the string
}

// Generate an unambiguous password (no confusing characters like 'O' and '0')
void generate_unambiguous(char *password, int length) {
    const char charset[] = "ABCDEFGHJKLMNPQRSTUVWXYZabcdefghjkmnpqrstuvwxyz234679";
    int charset_size = sizeof(charset) - 1;
    for (int i = 0; i < length; ++i)
        password[i] = charset[rand() % charset_size]; // No ambiguous characters
    password[length] = '\0'; // Null-terminate the string
}




