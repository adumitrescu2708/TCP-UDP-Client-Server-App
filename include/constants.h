/**
 * @file constants.h
 * @author Dumitrescu Alexandra 323CA
 * @brief Header for constants used in the
 *        clients, server and database.
 * @version 0.1
 * @date 2022-05-07
 * 
 * @copyright Copyright (c) Dumitrescu Alexandra 2022
 * 
 */


#ifndef _CONSTANTS_H

#define EXIT_REQUEST            "exit\n"
#define SUBSCRIBE_REQUEST       "subscribe"
#define UNSUBSCRIBE_REQUEST     "unsubscribe"
#define ID_IN_USE               "id_in_use"

#define STDIN                   0
#define BUFLEN                  1600
#define MAX_CLIENTS             5
#define MAX_POSTS               256
#define IP_LEN                  16
#define TOPIC_LEN               50
#define DATA_TYPE_LEN           11
#define CONTENT_LEN             1500
#define TOTAL_LEN               1551
#define SUBSCRIPTION_SEND       3
#define EXIT_CODE               1
#define ID_IN_USE_CODE          2
#define UNSUBSCRIBE_CODE        13
#define SUBSCRIBE_CODE          12
#define ID_CODE                 11

#endif