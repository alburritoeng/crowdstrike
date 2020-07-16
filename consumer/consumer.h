#include <map>
#include <semaphore.h>
#include <memory>
#include "../utilities/memorymapping.h"

namespace cwpp
{
    class consumer
    {        
        std::unique_ptr<sharedmemory::memorymapping> memoryMap;        
        std::string substring;
        uint numberOfSharedBuffers;
        sem_t* semaphore_empty;
        sem_t* semaphore_full;
        std::map<uint, pthread_mutex_t*> mutex_map;

        std::map<uint, pthread_mutex_t*> initalize_mutex(int numSharedBuffers);

        public:
            consumer(uint shared_buffers, std::string const& sub);
            ~consumer();
            void start_consuming();
    };
}