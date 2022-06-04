/**
 * @file server.cpp
 * 
 * @author Dumitrescu Alexandra
 * 
 * @brief Implemented the basics of a TCP / UDP server-client communication
 * server, where clients can subscribe to topics for later news coming from
 * the UDP server
 * 
 * @version 0.1
 * @date 2022-05-05
 * 
 * @copyright Copyright (c) Dumitrescu Alexandra 2022
 * 
 */



#include "include/helpers.h"
#include "include/database.h"
#include "include/constants.h"

using namespace std;

void usage(char *file)
{
    /*
        ./server <PORT>
    */
	fprintf(stderr, "Usage: %s server_port\n", file);
	exit(0);
}
/*
    Server

    The server is responsible for receiving UDP messages from clients, store
    them into the Database and send them to the subscribed clients. It also
    manages the TCP clients, keeping track of their status: online/offline
    and their subscriptions.
    
    For these operations, we send string messages.

    Protocol:
    1. Send the length of the required message into a header.
    2. Send <length> bytes of message.

*/
int main(int argc, char* argv[])
{
    setvbuf(stdout, NULL, _IONBF, BUFSIZ);
    /*
        Unique Database
    */
    struct Database database;

    int socket_fd_TCP, new_socket_fd_TCP, port_number, socket_fd_UDP;
    char buffer[BUFLEN];

    struct sockaddr_in server_address, client_address;
    int return_value;
    socklen_t socket_length = sizeof(struct sockaddr_in);

    if(argc < 2) {
        usage(argv[0]);
    }
    int enable = 1;

    /* 
        Create a socket for TCP.
    */
    socket_fd_TCP = socket(AF_INET, SOCK_STREAM, 0);
    DIE(socket_fd_TCP < 0, "Cannot open socket file descriptor for TCP.");

    DIE(setsockopt(socket_fd_TCP, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0, "Error in neagle");

    int neagle = 1;
    return_value = setsockopt(socket_fd_TCP, IPPROTO_TCP, TCP_NODELAY, &neagle, sizeof(int));
    DIE(return_value < 0, "Error in neagle");

    /*
        Create a socket for UDP 
    */
    socket_fd_UDP = socket(AF_INET, SOCK_DGRAM, 0);
    DIE(socket_fd_UDP < 0, "Cannot open socket file descriptor for UDP.");

    /*
        Obtain the address and the socket for the server
    */
    port_number = atoi(argv[1]);
    DIE(port_number == 0, "Error in parsing port number.");

    memset((char *) &server_address, 0, sizeof(server_address));
    server_address.sin_family       = AF_INET;
    server_address.sin_port         = htons((uint16_t)port_number);
    server_address.sin_addr.s_addr  = INADDR_ANY;

    /*
        Connect the sockets for UDP and TCP to server's address
    */
    return_value = bind(socket_fd_TCP, (struct sockaddr *) &server_address, sizeof(struct sockaddr));
    DIE(return_value < 0, "Error in binding TCP socket.");

    return_value = listen(socket_fd_TCP, 5);
    DIE(return_value < 0, "Error in listen process.");

    return_value = bind(socket_fd_UDP, (struct sockaddr *)&server_address, sizeof(server_address));
    DIE(return_value == -1, "Error in binding UDP socket.");

    /* 
        Create the sets for file descriptors for multiplexing.
        Add UDP and TCP sockets and STDIN to the set.
    */
    fd_set read_fds;
    fd_set tmp_fds;
    FD_ZERO(&read_fds);
    FD_ZERO(&tmp_fds);
    FD_SET(STDIN, &read_fds);
    FD_SET(socket_fd_TCP, &read_fds);
    FD_SET(socket_fd_UDP, &read_fds);
    int max_fds;
    max_fds = max(socket_fd_TCP, socket_fd_UDP);

    while(1) {
        tmp_fds = read_fds;

        /*
            Multiplexing process
        */
        return_value = select(max_fds + 1, &tmp_fds, NULL, NULL, NULL);
        DIE(return_value < 0, "Error in select process.");

        memset(buffer, 0, BUFLEN);

        /* 
            Check if the socket for STDIN is set.
            In this case, we treat only the Exit command.
        */
        if(FD_ISSET(STDIN, &tmp_fds)) {
            /* 
                Read the input command. 
            */
            fgets(buffer, BUFLEN - 1, stdin);

            if(strcmp(buffer, EXIT_REQUEST) == 0) {
                memset(buffer, 0, BUFLEN);
                /*  
                    Prepare the sending packet for EXIT request
                    Set the corresponding parameters. 
                    The content will be the exit command and the operation
                    is set to the given exit code.
                */
               char aux[BUFLEN];
               strcpy(aux, EXIT_REQUEST);
               struct Send_Header header;
               header.operation = EXIT_CODE;
               header.size = strlen(aux) + 1;

                /*
                    Send the exit-request packet to all connected
                    users and close sockets.
                */
                for(auto user_data : database.locations) {
                    struct Subscriber user = user_data.second;
                    if(user.online == true) {
                        return_value = send(user.socket_fd, &header, sizeof(struct Send_Header), 0);
                        DIE(return_value < 0, "Error in sending header");

                        return_value = send(user.socket_fd, aux, header.size, 0);
                        DIE(return_value < 0, "Error in sending message");
                        close(user.socket_fd);                
                    }
                }
                break;
            }
        }

        for (int i = 1; i <= max_fds; i ++) {
            /* UDP - Receive message */
            if(FD_ISSET(i, &tmp_fds)) {
                if(i == socket_fd_UDP) {
                    /*
                        If the socket for UDP is set, then a new subscription packet is received.

                        | Subscription Post - |Topic | Data Type | Content |
                        |                     |______|___________|_________|
                        |
                        | Send Post         - | Code | Content |
                        |                     |______|_________|
                        
                        Receive the subscription post via recvfrom command on the UDP socket. Transform
                        the received packet in a sending packet. For the sending packet's content we send
                        the following format: IP:PORT - Topic - Data Type - Content and set the Code to
                        the corresponding code for UDP (See: Constants)
                    */

                    struct Subscription_Post *new_post = (Subscription_Post *)malloc(sizeof(struct Subscription_Post));
                    new_post->content[0] = '\0';
                    memset(new_post, 0, sizeof(struct Subscription_Post));
                    
                    struct Send_Post *transform = (Send_Post *)malloc(sizeof(struct Send_Post));
                    memset(transform, 0, sizeof(struct Send_Post));
                    transform->operation = SUBSCRIPTION_SEND;
                    transform->content[0] = '\0';

                    return_value = recvfrom(socket_fd_UDP, new_post, sizeof(struct Subscription_Post), 0, (struct sockaddr *) &server_address, &socket_length);

                    char UDP_IP[IP_LEN];
                    inet_ntop(AF_INET, &(server_address.sin_addr), UDP_IP, IP_LEN);

                    DIE(return_value < 0, "error");
                    receive_post (new_post, transform, UDP_IP, server_address);

                    /*
                        Send the packet to all connected users in the databse subscribed to the received topic.
                        For the disconnected users, store the post in their local queue of posts, that will be
                        emptied once they restore thei connection.
                    */
                    for(auto user_data : database.subscription[new_post->topic]) {
                        auto test = database.online.find(user_data.ID);
                        if(test->second.online == true) {
                            struct Send_Header header;
                            header.operation = SUBSCRIPTION_SEND;
                            header.size = strlen(transform->content) + 1;

                            return_value = send(test->second.socket_fd, &header, sizeof(struct Send_Header), 0);
                            DIE(return_value < 0, "Error in sending header.");

                            return_value = send(test->second.socket_fd, transform->content, header.size, 0);
                            DIE(return_value < 0, "Error in sending mssage.");

                        } else if (test->second.online == false && test->second.subscription_types[new_post->topic] == true) {
                            (test->second).SF_queue.push(*transform);
                        }      
                    }
                } else if(i == socket_fd_TCP) {
                    /*
                        TCP - Receive new client on the TCP socket of the server

                        First add the client in the Database and distinguish 3 main cases:
                        1. The user is a new user, in which case we add the socket to the set of file descriptors
                        2. The user reconnects, in which case we set its -online- parameter to true
                        3. The user is already connected in the Database, in which case we send an ID_IN_USE packet
                        error.
                    
                    */

                    new_socket_fd_TCP = accept(socket_fd_TCP, (struct sockaddr *) &client_address, (socklen_t *) &socket_length);
                    DIE(new_socket_fd_TCP < 0, "Accept error in receiving TCP.");
                    int neagle3 = 1;
                    setsockopt(socket_fd_TCP, IPPROTO_TCP, TCP_NODELAY, &neagle3, sizeof(int));

                    if(add_new_client(new_socket_fd_TCP, client_address, &database) == true) {
                        FD_SET(new_socket_fd_TCP, &read_fds);
                        if(new_socket_fd_TCP > max_fds) {
                            max_fds = new_socket_fd_TCP;
                        }
                    } else {
                        /* (3) */
                        struct Send_Header header;
                        header.operation = ID_IN_USE_CODE;
                        char aux[BUFLEN];
                        strcpy(aux, ID_IN_USE);
                        header.size = strlen(aux) + 1;

                        return_value = send(new_socket_fd_TCP, &header, sizeof(struct Send_Header), 0);
                        return_value = send(new_socket_fd_TCP, aux, header.size, 0);

                        close(new_socket_fd_TCP);
                    }
                } else {
                    /*
                        Receive new message on one of the sockets of the clients. Distinguish 3 cases:
                        1. The client disconnected, in which case we set the -online- tag to false
                        in both maps and clear the socket from the sets of sockets.
                        2. The client sends a Subscribe request
                        3. The client sends an Unsubscribe request
                    
                    */

                    /* (1) */
                    struct Send_Header header;

                    return_value = recv(i, &header, sizeof(struct Send_Header), 0);
                    DIE(return_value < 0, "Error in receiving TCP.");

                    char aux2[BUFLEN];
                    return_value = recv(i, aux2, header.size, 0);

                    if(return_value == 0) {
                        struct Subscriber subscriber = database.locations[i];
                        cout << "Client " << subscriber.ID << " disconnected." << endl;

                        auto user_data_locations = database.locations.find(i);
                        user_data_locations->second.online = false;

                        auto user_data_online = database.online.find(subscriber.ID);
                        user_data_online->second.online = false;

                        FD_CLR(i, &read_fds);
                        FD_CLR(i, &tmp_fds);
                        close(i);
                    }

                    if(header.operation == SUBSCRIBE_CODE) {    /* (2) */
                        add_subscription(&database, i, aux2); 
                    } else if(header.operation == UNSUBSCRIBE_CODE) { /* (3) */
                        remove_subscription(&database, i, aux2);
                    }
                }
            }
        }
    }
    /*
        Close the sockets.
    */
    close(socket_fd_TCP);
    close(socket_fd_UDP);

    return 0;
}

