/**
 * @file command_parser.cpp
 * @author Dumitrescu Alexandra 323CA
 * @brief Functions that check the validation of
 * input commands from server/client.
 * @version 0.1
 * @date 2022-05-07
 * 
 * @copyright Copyright (c) Dumitrescu Alexandra 2022
 * 
 */
#include "../include/command_parser.h"

/**
 * @brief The function checks if the command has the expeccted
 * number of arguments. Using strtok we break the command in
 * pieces using " " delimiter and count the total char* 
 * received.
 * 
 * @param buffer - received command
 * @param correct_arguments - correct number of arguments
 * @return true - valid command
 * @return false - invalid command
 */
bool check_command_format(char buffer[BUFLEN], int correct_arguments) {
    char tokens[] = " ";
    char* p = strtok(buffer, tokens);
    int counter = 0;

    while(p != NULL) {
        counter ++;
        p = strtok(NULL, tokens);
    }
    
    return (counter == correct_arguments);
}


/**
 * @brief The funcion obtains the n-th argument in the given command.
 * Using strtok with " " delimiter we retrieve the n-th char* found
 * 
 * @param buffer - command
 * @param n - index of the argument
 * @param result - n-th argument in the command
 */
void obtain_nth_argument(char buffer[BUFLEN], int n, char result[TOPIC_LEN]) {
    buffer[strlen(buffer) - 1] = '\0';
    char tokens[] = " ";
    char* p = strtok(buffer, tokens);
    int counter = 0;

    while(p != NULL) {
        counter ++;
        if(counter == n) {
            strcpy(result, p);
            return;
        }
        p = strtok(NULL, tokens);
    }
}