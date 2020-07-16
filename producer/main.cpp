#include <iostream>
#include "../utilities/utilities.h"
#include "../utilities/constants.h"
#include "producer.h"
#include <memory>
#include <semaphore.h>
#include <string.h>
#include <fcntl.h>
#include <iostream>


void print_usage(int error)
{
    std::cout << "ERROR code" << error << std::endl;
    std::cerr << "Usage: " << "[path to file] [N - number of shared buffers]" << std::endl;    
}

int main(int argc, char* argv[])
{
    if (argc != 3)
    {           
        print_usage(invalid_input_count);
        return invalid_input_count;
    }

    std::string file_name(argv[1]);
    if (!utilities::helpers::file_exists(file_name.c_str()))
    {
        print_usage(input_file_does_not_exist);
        return input_file_does_not_exist;
    }

    uint numberOfSharedBuffers = atoi(argv[2]);
    if (numberOfSharedBuffers <= 0)
    {
        print_usage(invalid_input_value);
        return invalid_input_value;
    }

    std::shared_ptr<cwpp::producer> producerObj( new cwpp::producer(file_name, numberOfSharedBuffers));
    producerObj->start_producing();
    
    return 0;
}
