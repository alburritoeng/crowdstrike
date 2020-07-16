#include <iostream>
#include <istream>
#include <fstream>
#include <string.h>
#include <stdlib.h>
#include <vector>
#include <unistd.h>
#include <semaphore.h>
#include <fcntl.h>
#include <map>
#include <sstream>
#include <cstring>
#include "../utilities/memorymapping.h"
#include "../utilities/constants.h"
#include "../utilities/messages.h"
#include "../utilities/utilities.h"
#include "consumer.h"
#include "../utilities/mutexlockguard.h"

namespace cwpp
{
    consumer::consumer(uint shared_buffers, std::string const& sub)
    {
        //Initialize               
        numberOfSharedBuffers = shared_buffers;
        substring = std::move(sub);
        memoryMap = std::unique_ptr<sharedmemory::memorymapping>(new sharedmemory::memorymapping());       

        mutex_map = initalize_mutex(shared_buffers); 
    }

    std::map<uint, pthread_mutex_t*> consumer::initalize_mutex(int numSharedBuffers)
    {
        std::map<uint, pthread_mutex_t*> map;
                
        semaphore_empty = sem_open(empty_semaphore_name, O_CREAT);
        semaphore_full  = sem_open(full_semaphore_name,  O_CREAT);
        
        for(int i = 0; i < numSharedBuffers; i++)
        {
            map.insert({i, sharedmemory::memorymapping::init_get_shared_mutex(i)});
        }      

        return std::move(map);
    }

    consumer::~consumer()
    {
        std::cout << "End of Consumer" << std::endl;
    }

    void consumer::start_consuming()
    {   
        uint entry = 0;
        do
        {   
            sem_wait(semaphore_full);           
            
            auto current_mutex = mutex_map.find(entry);
            mapped_data* sharedmemory = nullptr;            
     
            // waiting for mutex             
            utilities::mutex_lock_guard mutex_guard(*current_mutex->second); //release mutex is scoped
            {
                /************critical section************/           

                // read from shared buffer 
                auto shared_memory = memoryMap->get_memory_map(entry);
                
                // sanity checks -
                if(shared_memory == nullptr )
                {
                    if(++entry >= numberOfSharedBuffers)
                    {
                        entry = 0;
                    }          
                    std::cout << error_reading_shared_mem << std::endl;            
                    continue;                
                }          

                // is this shared buffer ready for reading
                if(shared_memory->state == data_rw_state::data_read || shared_memory->state == data_rw_state::data_init)
                {                               
                    
                    if(++entry >= numberOfSharedBuffers)
                    {
                        entry = 0;
                    }                      
                    continue;
                }
                
                std::string str(shared_memory->sentences);                
                if(str.length()!=shared_memory->datalength)
                {
                    std::cout << "length != data_length" << std::endl;
                    continue;
                }

                // yep, we can read from this buffer, lets try
                if(shared_memory->sentences != nullptr)
                {
                    char *line = nullptr;

                    line = strtok(shared_memory->sentences, separator);
                    while (line != nullptr) 
                    {
                        std::string s(line);
                        auto found = s.find(substring);
                        if(found != std::string::npos)
                        {
                            std::cout << line << std::endl;
                        }

                        line = strtok(nullptr, separator); // nullptr means continue from where the previous successful call to strtok ended
                    }
                    
                    // we are done reading from this shared buffer, set the state & then release the mutex below
                    shared_memory->state = data_rw_state::data_read;
                }
                else
                {
                    // unexpected, print it out
                    std::cout << error_reading_shared_mem << std::endl;
                }    

                // set sem_post EMPTY                
            }

            sem_post(semaphore_empty);
            
            // find another shared buffer that has been read to use for reading            
            if(++entry >= numberOfSharedBuffers)
            {
                entry = 0;
            }            

        } while (true); //per instruction, won't terminate     

        std::cout << "Terminate" << std::endl;   // will never hit
    }
}