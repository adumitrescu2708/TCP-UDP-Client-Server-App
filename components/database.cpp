/**
 * @file database.cpp
 * @author Dumitrescu Alexandra
 * @version 0.1
 * @date 2022-05-06
 * 
 * Database - contains 3 hashmaps (unordered_tables for time efficiency)
 *          1. subscription :: <string, vector<subscriber>> - Select a list of subscribers to a
 *             specific topic 
 *          2. locations :: <int, subscriber> - Select a subscriber at a specific location
 *          3. online :: <string, subscriber> - Select a subscriber from a given ID
 * 
 * The Database is only created once at the start of the server.
 * This file provides common opperations on the database.
 * 
 * @copyright Copyright (c) Dumitrescu Alexandra 2022
 * 
 */

#include "../include/database.h"
#include "../include/subscriber.h"
#include "../include/constants.h"
#include "../include/subscriber.h"
#include "../include/helpers.h"
#include "../include/command_parser.h"
#include <cstring>
#include <sstream>
#include <vector>
#include <iomanip>

using namespace std;

/**
 * @brief Function that adds a new client to the Database
 * 
 * @param socket - socket of the new client
 * @param adress - address of the client
 * @param database - database
 * @return true - The operation ends up with success
 * @return false - The operation fails (There is already a client connected with the given ID
 * in the database)
 */
bool add_new_client(int socket, struct sockaddr_in adress, struct Database *database)
{
   /*
        In order for the server to know the client's ID, we send the server a packet from
        the client with its assigned ID. The server then follows the steps:

        1. Check if the given ID is already in the Database
            1.1 If Yes - check if the ID is corresponding to an already connected user
            in which case we send Fail and send an ID_IN_USE packet back to the client.

            1.2 If Yes - and the ID is corresponding to a not connected client, then
            connect the client and send back to the client the Posts in the queue.

        2. Add the new client to the local Database
   */

    char ID[BUFLEN];
    ID[0] = '\0';
    struct Send_Header received_header;
    int return_value = recv(socket, &received_header, sizeof(struct Send_Header), 0);
    DIE(return_value < 0, "Error in receiving header");
    return_value = recv(socket, ID, received_header.size, 0);

    /* (1.1) */
    if(ID_already_in_database(ID, database) == true) {
        cout << "Client " << ID << " already connected." << endl;
        return false;
    }

    cout << "New client " << ID << " connected from " << inet_ntoa(adress.sin_addr);
    cout << ":" <<  socket << "." << endl;
    
    /* (1.2) */
    auto find_user = (*database).online.find(ID);
    if(find_user != (*database).online.end()) {
        /*
            Send the enqueued posts
        */
        while(!find_user->second.SF_queue.empty()) {
            struct Send_Post pkt = find_user->second.SF_queue.front();
            find_user->second.SF_queue.pop();

            struct Send_Header header;
            header.operation = SUBSCRIPTION_SEND;
            char aux[BUFLEN];
            strcpy(aux, pkt.content);
            header.size = strlen(aux) + 1;

            return_value = send(socket, &header, sizeof(struct Send_Header), 0);
            return_value = send(socket, aux, header.size, 0);

        }
    }

    /* (2) */
    struct Subscriber new_subscriber;
    strcpy(new_subscriber.ID, ID);
    new_subscriber.socket_fd    = socket;
    new_subscriber.online       = true;

    (*database).locations[socket] = new_subscriber;
    (*database).online[ID] = new_subscriber;

    return true;
}

/**
 * @brief Function that checks if a given ID is already in the database
 * 
 * @param ID - searched ID
 * @param database - database
 * @return true - a client is already connected with the same ID
 * @return false - no client has this ID
 */

bool ID_already_in_database(char ID[BUFLEN], struct Database* database) {
    for(auto user_data : (*database).locations) {
        struct Subscriber user = user_data.second;
        if(strcmp(user.ID, ID) == 0 && user.online == true) {
            return true;
        }
    }
    return false;
}

/**
 * @brief Breaks the command received from the client into 3 pieces
 * and adds the received topic to the subscriber's hashmap of
 * subscribed topics and SF and stores the subscriber in the
 * hashmap of the database.
 * 
 * @param database - database
 * @param socket_fd - client 
 * @param buffer - command
 */
void add_subscription(struct Database* database, int socket_fd, char buffer[BUFLEN]) {
    struct Subscriber subscriber2 = (*database).locations[socket_fd];
    auto subscriber = (*database).online.find(subscriber2.ID);

    char topic[TOPIC_LEN];
    char SF[2];

    /*
        Obtain the SF char
    */
    SF[0] = buffer[strlen(buffer) - 2];
    SF[1] = '\0';

    /*
        Obtain the topic
    */
    obtain_nth_argument(buffer, 2, topic);

    /*
        Check SF validation and update client's topics hashmap
    */
    if(SF[0] == '1') {
        subscriber->second.subscription_types[topic] = true;
    } else if(SF[0] == '0'){
        subscriber->second.subscription_types[topic] = false;
    } else {
        DIE(SF[0] != '0' && SF[0] == '1', "Command Error");
    }

    /*
        Add the client to the subscription map of the database
    */
    (*database).subscription[topic].push_back(subscriber->second);
}

/**
 * @brief Removes the client from the map of subscriptions in the local database
 * and updates client's map of subscribed topics.
 * 
 * @param database database
 * @param socket_fd client
 * @param buffer command
 */
void remove_subscription(struct Database* database, int socket_fd, char buffer[BUFLEN]) {
    struct Subscriber subscriber2 = (*database).locations[socket_fd];
    auto subscriber = (*database).online.find(subscriber2.ID);

    char topic[TOPIC_LEN];
    obtain_nth_argument(buffer, 2, topic);

    /*
        Remove subscription from map of the client
    */
    subscriber->second.subscription_types.erase(topic);
    
    int position = -1;

    for(unsigned int i = 0; i < (*database).subscription[topic].size(); i++) {
        if(strcmp((*database).subscription[topic][i].ID, subscriber2.ID) == 0) {
            position = i;
            break;
        }
    }
    if(position != -1) {
        (*database).subscription[topic].erase((*database).subscription[topic].begin() + position);
    }
}

/**
 * @brief Update client as being offline
 * 
 * @param database database
 * @param socket_fd socket for user
 */
void disconnect_client(struct Database* database, int socket_fd) {
    struct Subscriber user = (*database).locations[socket_fd];
    user.online = false;
    (*database).locations[socket_fd] = user;
}

/**
 * @brief From the received new_post this method parses the information in a transform packet.
 * Depending on the received data type, we distinguish 4 cases:
 *      1. INT
 *      2. SHORT_REAL
 *      3. FLOAT
 *      4. STRING
 * 
 * @param new_post - new post from UDP server
 * @param transform - transform paket
 * @param IP - IP of the client
 * @param server_address - address of the server
 */
void receive_post (struct Subscription_Post* new_post, struct Send_Post* transform, char IP[16], struct sockaddr_in server_address) {
    switch(new_post->data_type) {
        case 0: {
            /*
                INT - check signed number, convert with ntohl and compute the result message 
            */
            char *p = new_post->content;
            char sign = *(uint8_t *)p;
            uint32_t number;
            memcpy(&number, new_post->content + 1, sizeof(u_int32_t));
            number = ntohl(number);
            if(sign == 1) {
                number = number * (-1);
            }
            sprintf(transform->content, "%s:%d - %s - %s - %d", IP, (server_address.sin_port) ,new_post->topic, "INT", number);
            break;
        }
        case 1: {
            /*
                SHORT_REAL - obtain the number from the content, divide it by 100 with 2 decimal precision
                and compute the result message
            */
            stringstream str;
            char body[1600];
            body[0] = '\0';
            char *p = new_post->content;
            uint16_t number = (*((uint16_t *)p));
            number = ntohs(number);
            str << fixed << showpoint << setprecision(2) << (float)((float) (1.0 * (float) number) / (float) 100.0);
            str >> body;
            sprintf(transform->content, "%s:%d - %s - %s - %s", IP, (server_address.sin_port), new_post->topic, "SHORT_REAL", body);
            break;
        }
        case 2: {
            /*
                FLOAT - obtain the sign, the power and the exponent and compute the resul message
            */
            stringstream str;
            char *p = new_post->content;
            char sign = *(uint8_t *)p;
            char body[1600];
            char aux[1600];
            body[0] = '\0';
            if(sign == 1) {
                body[0] = '-';
                body[1] = '\0';
            }
            p = p + 1;
            uint32_t number;
            memcpy(&number, new_post->content + 1, sizeof(uint32_t));
            number = ntohl(number);
            p = p + sizeof(uint32_t);
            uint8_t power = (*((uint8_t *)(p)));
            str << fixed << showpoint << setprecision(power) << (float) ((float)(1.0 * number) / (pow(10, power)));
            str >> aux;
            strcat(body, aux);
            sprintf(transform->content, "%s:%d - %s - %s - %s", IP, (server_address.sin_port), new_post->topic, "FLOAT", body);
            break;
        }
        case 3 : {
            /*
                STRING - directly compute the result message
            */
            sprintf(transform->content, "%s:%d - %s - %s - %s", IP, (server_address.sin_port), new_post->topic, "STRING", new_post->content);
            break;
        }
    }
}