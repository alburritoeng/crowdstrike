#include "../utilities/utilities.h"
#include <stdlib.h>
#include <map>
#include <semaphore.h>
#include "../utilities/constants.h"
#include "../utilities/memorymapping.h"
#include <memory>

namespace cwpp
{
    class producer
    {   
        std::unique_ptr<sharedmemory::memorymapping> memoryMap;
        sem_t* semaphore_empty;
        sem_t* semaphore_full;
        
        uint numberOfSharedBuffers = 0;
        uint currentSharedMemIndex = 0;
        std::string file_name;      
        std::map<uint, pthread_mutex_t*> mutex_map;
        std::map<uint, pthread_mutex_t*> initalize_mutex(int numSharedBuffers);
        
        void write_to_shared_memory(std::string& str);
        void initialize_shared_buffers(uint sharedBuffersCount);
        void initialize_semaphores(uint num);
        void clean_up_shared_buffers();
        void clean_up_semaphores();        

        public:
            producer(std::string const& file, uint shared_buffers);
            ~producer();
            void start_producing();

    };
}