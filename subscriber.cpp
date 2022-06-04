/**
 * @file subscriber.cpp
 * @author Dumitrescu Alexandra 323CA
 * @brief Subscriber
 * @version 0.1
 * @date 2022-05-07
 * 
 * @copyright Copyright (c) Dumitrescu Alexandra 2022
 * 
 */
#include "include/helpers.h"
#include "include/constants.h"
#include "include/command_parser.h"
#include "include/post.h"

using namespace std;


void usage(char *file)
{
    /*
        ./subscriber <ID> <SERVER_IP> <SERVER_PORT>
    */
	fprintf(stderr, "Usage: %s id_client server_address server_port\n", file);
	exit(0);
}

/*
    Client
    1. Receives commands from STDIN and sends to the server
        1.1 Exit
        1.2 Subscribe topic SF
        1.3 Unsubscribe topic
    2. Received messages from the server
        2.1 With subscribed topics
        2.2 With Exit message
        2.3 With ID_IN_USE error message
    

*/
int main(int argc, char *argv[]) {
    setvbuf(stdout, NULL, _IONBF, BUFSIZ);
    
    int socket_fd, return_value;
    struct sockaddr_in server_address;
    char buffer[TOTAL_LEN];

    if(argc < 4) {
        usage(argv[0]);
    }

    /*
        Open socket for TCP and diable nagle
    */
    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    DIE(socket_fd < 0, "Error in opening client's socket.");
    int neagle = 1;
    setsockopt(socket_fd, IPPROTO_TCP, TCP_NODELAY, &neagle, sizeof(int));

    /*
        Obtain the address and the port of the server
    */
    memset((char *) &server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(atoi(argv[3]));
    return_value = inet_aton(argv[2], &server_address.sin_addr);
    DIE(return_value == 0, "Error in inet aton.");

    /*
        Connect the socket with the server
    */
    return_value = connect(socket_fd, (struct sockaddr*) &server_address, sizeof(server_address));
    DIE(return_value < 0, "Error in connecting client");

    /*
        In order for the server to know the client's ID, we send an initial
        packet tot the server with the assigned ID.
    */
    struct Send_Header send_ID_header;
    send_ID_header.operation = ID_CODE;
    send_ID_header.size = strlen(argv[1]) + 1;
    return_value = send(socket_fd, &send_ID_header, sizeof(struct Send_Header), 0);
    DIE(return_value < 0, "Error in sending header");
    return_value = send(socket_fd, argv[1], send_ID_header.size, 0);
    DIE(return_value < 0, "Error in sending name");

    /*
        Create set file descriptor for multiplexing and add
        the STDIN port and client's socket.
    */
    fd_set read_fds, tmp_fds;
    FD_ZERO(&read_fds);
    FD_ZERO(&tmp_fds);
	FD_SET(socket_fd, &read_fds);
	FD_SET(0, &read_fds);


    while(1) {
        tmp_fds = read_fds;

        /*
            Multiplexing
        */
        return_value = select(socket_fd + 1, &tmp_fds, NULL, NULL, NULL);
        DIE(return_value < 0, "Error in select");

        /*
            STDIN commands
        */
        if(FD_ISSET(STDIN, &tmp_fds)) {
            /*
                Exit request - closes the client
            */
            fgets(buffer, BUFLEN - 1, stdin);
            if(strcmp(buffer, EXIT_REQUEST) == 0) {
                break;
            } else if(strncmp(buffer, SUBSCRIBE_REQUEST, strlen(SUBSCRIBE_REQUEST)) == 0) {
                /*
                    Received a subscribe request from stdin and send a packet to
                    the server.
                */
                char aux[BUFLEN];
                aux[0] = '\0';
                strcpy(aux, buffer);

                /*
                    Set the Header
                */
                struct Send_Header header;
                header.operation = SUBSCRIBE_CODE;
                header.size = strlen(buffer) + 1;

                /*
                    If the command is valid, then send the composed Header and
                    received message from STDIN
                */
                if(check_command_format(aux, 3) == true) {
                    return_value = send(socket_fd, &header, sizeof(struct Send_Header), 0);
                    DIE(return_value < 0, "Error in sending header");

                    return_value = send(socket_fd, buffer, header.size, 0);
                    DIE(return_value < 0, "Error in sending packet.");

                    cout << "Subscribed to topic." << endl;
                } else {
                    cerr << "Invalid subscribe command!" << endl;
                }
                continue;
            } else if(strncmp(buffer, UNSUBSCRIBE_REQUEST, strlen(UNSUBSCRIBE_REQUEST)) == 0) {
                /*
                    Receive an unsubscribe message
                */
                char aux[BUFLEN];
                aux[0] = '\0';
                strcpy(aux, buffer);

                /*
                    Compose the header
                */
                struct Send_Header header;
                header.operation = UNSUBSCRIBE_CODE;
                header.size = strlen(buffer) + 1;

                /*
                    If the command is a valid one, then send the received message
                    from STDIN
                */
                if(check_command_format(aux, 2) == true) {
                    return_value = send(socket_fd, &header, sizeof(struct Send_Header), 0);
                    DIE(return_value < 0, "Error in sending header.");

                    return_value = send(socket_fd, buffer, header.size, 0);
                    DIE(return_value < 0, "Error in sending packet.");

                    cout << "Unsubscribed from topic." << endl;
                } else {
                    cerr << "Invalid Unsubscribe command!" << endl;
                }
                continue;
            }
        }

        if(FD_ISSET(socket_fd, &tmp_fds)) {
            /*
                Receive a packet from the server.
                Obtain the Header and then the message
            */
            struct Send_Header header;
            return_value = recv(socket_fd, &header, sizeof(struct Send_Header), 0);

            char command[BUFLEN];
            command[0] = '\0';
            return_value = recv(socket_fd, command, header.size, 0);

            /*
                Break into the cases: Exit, ID-in-use error and subscription
                message.
            */
            if(header.operation == EXIT_CODE) {
                break;
            } else if(header.operation == ID_IN_USE_CODE) {
                break;
            } else {
                cout << command << endl;
                continue;
            }
        }
    }
    close(socket_fd);
    return 0;

}