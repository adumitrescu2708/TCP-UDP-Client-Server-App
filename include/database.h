/**
 * @file database.h
 * @author Dumitrescu Alexandra 323CA
 * @brief Header for the Database structure and methods
 * @version 0.1
 * @date 2022-05-07
 * 
 * @copyright Copyright (c) Dumitrescu Alexandra 2022
 * 
 */
#ifndef _DATABASE_H
#define _DATABASE_H


#include "helpers.h"
#include "subscriber.h"
#include "constants.h"
#include "post.h"

using namespace std;

/*
    subscription<string, vector<>>  ::  topic -> subscribed clients
    online<string, subscriber>      ::  ID    -> client
    locations<int, subscriber>      ::  socket-> client
*/
struct Database {
    unordered_map<string, vector<struct Subscriber>> subscription;
    unordered_map<string, struct Subscriber> online;
    unordered_map<int, struct Subscriber> locations;
};

/**
 * @brief Adds a new client in the database
 */
bool add_new_client(int socket, struct sockaddr_in adress, struct Database* database);

/**
 * @brief Adds a new subscription in the database having the command buffer
 * and the socket_fd is the socket of the client
 * 
 */
void add_subscription(struct Database* database, int socket_fd,
                     char buffer[BUFLEN]);

/**
 * @brief sets the client having the port socket_fd in the database to offline
 * 
 */
void disconnect_client(struct Database* database, int socket_fd);

/**
 * @brief Checks if an ID is already in the database to a connected
 * client.
 * 
 */
bool ID_already_in_database(char ID[BUFLEN], struct Database* database);

/**
 * @brief  Transforms the received message from the UDP clients <new_post>
 * into a new post of format Send_Post <transform> and creates the text
 * message that needs to be send to the subscriber.
 * 
 */
void receive_post(struct Subscription_Post* new_post,
                 struct Send_Post* transform, char IP[16],
                 struct sockaddr_in server_address);

/**
 * @brief removes the topic <buffer> from the map of subscriptions of
 * the client at <socket_fd> port stored in the <database>.
 * 
 */
void remove_subscription(struct Database* database, int socket_fd,
                         char buffer[BUFLEN]);

#endif