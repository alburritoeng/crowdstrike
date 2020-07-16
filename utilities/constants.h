// constants for methods
#ifndef CONSTANTS_H
#define CONSTANTS_H

constexpr unsigned int  buffer_size               = 1024;               // the max size of any shared buffer
constexpr int           invalid_input_count       = -1;                 // input count is not valid
constexpr int           invalid_input_value       = -2;                 // input flags are invalid
constexpr int           input_file_does_not_exist = -3;                 // input file does not exist
constexpr int           permissions               = 0666;               // permissions everywhere
constexpr int           initial_empty_semaphore_value   = 0;            // input file does not exist
constexpr const char*   mutex_mmap_name           = "cwpp_mutex";       // the name of the file to map for mutex shared memory
constexpr const char*   ftok_file_name            = "cwpp";             // to be used for FTOK token generation
constexpr const char*   var_temp                  = "/var/tmp/";        // path to use for writing file for FTOK generation rather than use a "." 
constexpr const char*   dot_txt                   = ".txt";             // making a file name 
constexpr const char*   separator                 = "\r\n";             // for finding lines in our shared buffer
constexpr const char*   full_semaphore_name       = "full";              // full semaphore name
constexpr const char*   empty_semaphore_name       = "empty";              // full semaphore name
#endif