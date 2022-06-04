/**
 * @file command_parser.h
 * @author Dumitrescu Alexandra 323CA
 * @brief Header for basic commands to check the
 *        validation of the received commands.
 * @version 0.1
 * @date 2022-05-07
 * 
 * @copyright Copyright (c) Dumitrescu Alexandra 2022
 * 
 */
#ifndef _COMMAND_PARSER_H
#define _COMMAND_PARSER_H

#include "helpers.h"
#include "constants.h"

/**
 * @brief Function that checks if the command has the
 *        specified number of arguments.
 * 
 * @param buffer - command
 * @param correct_arguments - correct number of arguments
 * @return true - the command is valid
 * @return false - the command is invalid
 */

bool check_command_format(char buffer[BUFLEN], int correct_arguments);



/**
 * @brief Retrieves in the result parameter n-th argument
 *        from the received command.
 * 
 * @param buffer - command
 * @param n - index of the argument
 * @param result - copy of n-th argument in the buffer command
 */

void obtain_nth_argument(char buffer[BUFLEN], int n, char result[TOPIC_LEN]);

#endif