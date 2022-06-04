/**
 * @file post.h
 * @author Dumitrescu Alexandra 323CA
 * @brief Header for Post and Header structures
 * @version 0.1
 * @date 2022-05-07
 * 
 * @copyright Copyright (c) Dumitrescu Alexandra 2022
 * 
 */

#ifndef _POST_H
#define _POST_H

#include "helpers.h"

/*
    Subscription Post from UDP clients

    | TOPIC   | TYPE | CONTENT |
    |_________|______|_________|

    This is the format of the messages received from the
    UDP clients. Whenever a new message is received, the
    server parses its arguments and creates the message for
    the clients in the database subscribed to <TOPIC>.

    See Also: <constants.h>
    
*/
struct Subscription_Post {
    char topic[TOPIC_LEN];
    char data_type;
    char content[CONTENT_LEN];
};

/*
    Send Post for TCP clients

    | OPERATION | CONTENT |
    |___________|_________|

    Whenever a client is offline and subscribed to
    a specific topic and the server receives a new
    post having the topic, we store in the queue of
    posts of the client the post having this format.

*/

struct Send_Post {
    int operation;
    char content[BUFLEN];   
};

/*
    Header for the TCP clients

    | SIZE  | OPERATION |
    |_______|___________|

    In order to communicate between the server and the
    TCP clients via text - messages, we send a header 
    specifying the lenght of the incoming message and
    the operation code (defined in the constants). This
    way the receiver knows how many bytes to receive.

    See Also: <constants.h>

*/

struct Send_Header {
    int size;
    int operation;
};

#endif