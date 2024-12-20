/*
 ============================================================================
 Name        : esonero2_client.c
 Author      : Niccolò Lamonaca
 Version     :
 Copyright   : Your copyright notice
 ============================================================================
 */


#include <time.h> // For time-related functions (repeated, consider consolidation).
#if defined WIN32
#include <winsock.h> // Windows socket programming.
#else
#include <string.h>   // For string manipulation functions (repeated, consider consolidation).
#include <unistd.h>   // For POSIX API functions.
#include <sys/types.h> // Data types for socket programming.
#include <sys/socket.h> // Socket functions and structures.
#include <arpa/inet.h>  // Functions for manipulating IP addresses.
#include <netinet/in.h> // Network structures like sockaddr_in.
#include <netdb.h>      // DNS resolution.
#include <ctype.h>      // Character type functions.
#include <stdlib.h>     // General utility functions.
#define closesocket close // Compatibility definition for UNIX.
#endif

#include <stdio.h>  // Standard input/output (repeated, consider consolidation).
#include <stdlib.h> // General utilities (repeated, consider consolidation).
#include "protocollo.h" // Protocol definitions.

//prototipe of the function that will print the help menu
void view_help_guide();

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

//this is the main function of the program
int main(void) {
#if defined WIN32
    WSADATA wsa_data;
    if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0) {
        errorhandler("Error at WSAStartup()");
        return 0;
    }
    typedef int socklen_t; // Windows richiede int invece di socklen_t
#endif

    // Utilizzo dei valori predefiniti da protocollo.h
    const char *server_name = DEFAULT_SERVER;
    int port = DEFAULT_PORT;

    // Create the UDP client socket
    int client_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (client_socket < 0) {
        errorhandler("Error creating socket");
        clearwinsock();
        return -1;
    }

    // Set server address and port
    struct sockaddr_in server_address;
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);

    // Resolution of server address
    struct hostent *host;
    host = gethostbyname(server_name);
    if (host == NULL) {
        errorhandler("Error resolving host");
        clearwinsock();
        return EXIT_FAILURE;
    }

    // Copy the IP address of the server
    memcpy(&server_address.sin_addr.s_addr, host->h_addr_list[0], host->h_length);

    // print information about the connection
    printf("\n\nConnecting to server %s (IP: %s, port: %d)\n", server_name, inet_ntoa(*(struct in_addr *)host->h_addr), port);

    char buffer[BUFFMAX];
    request req;

    printf("\n--- PASSWORD GENERATOR ---\n");
    while (1) {
        printf("\nEnter request or q to quit or h for help: ");
        fgets(buffer, sizeof(buffer), stdin);

        // remove the new line character
        buffer[strcspn(buffer, "\n")] = '\0';

        // command q managemenet. If you type q the programm will end
        if (buffer[0] == 'q' && buffer[1] == '\0') {
            printf("\nExiting client...\n");
            break;
        }

        // command h management. If u type h it will print the help guide menu
        if (buffer[0] == 'h' && buffer[1] == '\0') {
        	//since this code block is inside a while loop, after the menu is printed the loop will ask again to the user to type in a password type and its length to generate a new one
            view_help_guide(); //call to the function that will print the help menu
            continue;
        }

        // Parse the user input
        char command; // Variable to store the command character
        int length;   // Variable to store the length of the password

        // Validate input format and range for the length
        if (sscanf(buffer, "%c %d", &command, &length) != 2 || length < MIN || length > MAX) {
            printf("Invalid input. Length must be between %d and %d.\n", MIN, MAX);
            continue; // Restart the loop to ask for new input
        }

        // Validate the command character against allowed options
        if (sscanf(buffer, "%c %d", &command, &length) != 2 ||
            ((command != 'n') && (command != 'a') && (command != 'm') &&
             (command != 's') && (command != 'u'))) {
            printf("Invalid input. Incorrect option. Available options: n, a, m, s, u.\n");
            continue; // Restart the loop to ask for new input
        }

        // Prepare the request structure to send to the server
        req.request_type = command; // Set the command type
        req.length = length;        // Set the requested length for the password

        // Send the request to the server
        if (sendto(client_socket, (const char *)&req, sizeof(req), 0,
                   (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
            perror("Error sending data"); // Display an error message if sending fails
            continue; // Restart the loop to allow retry
        }

        // Receive the response from the server
        socklen_t server_address_length = sizeof(server_address); // Variable to store the server address length
        int recv_len = recvfrom(client_socket, (char *)&req, sizeof(req), 0,

        		(struct sockaddr *)&server_address, &server_address_length);

        // Check if the server response was received successfully
        if (recv_len > 0) {
            req.password[req.length] = '\0';  // Ensure the password is null-terminated
            printf("Generated Password: %s\n", req.password); // Print the generated password
        } else {
            printf("Error receiving data.\n"); // Display an error message if data reception fails
        }

    }

    closesocket(client_socket);
    clearwinsock();
    return 0; //end the program
}


//this function print the help menu that will show up when the user type the char 'h'
void view_help_guide() {
//in this function all the sentences will be contained in a box made with the symbols '+', ' | ' and '-'
    printf("\n+---------------------------------------------------------------+\n");
    printf("|                 Password Generator Help Menu                  |\n");
    printf("+---------------------------------------------------------------+\n");
    printf("| Commands:                                                     |\n");
    printf("|   h        : show this help menu                              |\n");
    printf("|   n LENGTH : generate numeric password (digits only)          |\n");
    printf("|   a LENGTH : generate alphabetic password (lowercase letters) |\n");
    printf("|   m LENGTH : generate mixed password (lowercase and numbers)  |\n");
    printf("|   s LENGTH : generate secure password (uppercase, lowercase,  |\n");
    printf("|               numbers, symbols)                               |\n");
    printf("|   u LENGTH : generate unambiguous secure password (no         |\n");
    printf("|               similar-looking characters)                     |\n");
    printf("|   q        : quit application                                 |\n");
    printf("+---------------------------------------------------------------+\n");
    printf("| LENGTH must be between %d and %d characters                    |\n", MIN, MAX);
    printf("+---------------------------------------------------------------+\n");
    printf("| Ambiguous characters excluded in 'u' option:                  |\n");
    printf("|   0 O o (zero and letters O)                                  |\n");
    printf("|   1 l I i (one and letters l, I)                              |\n");
    printf("|   2 Z z (two and letter Z)                                    |\n");
    printf("|   5 S s (five and letter S)                                   |\n");
    printf("|   8 B (eight and letter B)                                    |\n");
    printf("+---------------------------------------------------------------+\n");
}


