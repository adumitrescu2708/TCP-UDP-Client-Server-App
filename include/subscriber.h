/**
 * @file subscriber.h
 * @author Dumitrescu Alexandra 323CA
 * @brief Header for Subscriber structure
 * @version 0.1
 * @date 2022-05-07
 * 
 * @copyright Copyright (c) Dumitrescu Alexandra 2022
 */


#ifndef _SUBSCRIBER_H
#define _SUBSCRIBER_H

#define ID_MAX_LEN 32
#define IP_MAX_LEN 32

#include "helpers.h"
#include <queue>

using namespace std;

/*
    | SOCKET | ID | ONLINE | QUEUE | SUBSCRIPTIONS |
    |________|____|________|_______|_______________|

    The structure for Subscriber.
    <socket>    = file descriptor in the server
    <ID>        = ID in the database
    <online>    = online/offline
    <queue>     = queue of Posts received from
    |             the UDP clients of the subscription
    |             topics whenever the online tag is set
    |             to offline
    <subscriptions> = map of subscriptio type - bool
    |                 true/false depending on the
    |                 SF character received

*/
struct Subscriber {
    int socket_fd;
    char ID[ID_MAX_LEN];
    bool online;
    queue<struct Send_Post> SF_queue;
    unordered_map<string, bool> subscription_types;
};


#endif