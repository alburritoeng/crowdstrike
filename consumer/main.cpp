#include <iostream>
#include "../utilities/constants.h"
#include "consumer.h"
#include <memory>

void print_usage(int error)
{
    std::cout << "ERROR code" << error << std::endl;
    std::cerr << "Usage: " << "[N - number of shared buffers] [S - substring to search for]" << std::endl;    
}

int main(int argc, char* argv[])
{
    if (argc != 3)
    {
        print_usage(invalid_input_count);
        return invalid_input_count;
    }

    uint numberOfSharedBuffers = atoi(argv[1]);
    if (numberOfSharedBuffers <= 0)
    {
        print_usage(invalid_input_value); 
        return invalid_input_value;
    }

    std::string substring = argv[2];        
    if(substring.length()==0)
    {
        print_usage(invalid_input_value);
        return invalid_input_value;
    }

    std::unique_ptr<cwpp::consumer> consumerObj(new cwpp::consumer(numberOfSharedBuffers, substring));    
    consumerObj->start_consuming();
    return 0;
}