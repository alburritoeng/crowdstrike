#ifndef MUTEXLOCKGUARD_H
#define MUTEXLOCKGUARD_H

#include <string>

namespace utilities
{
    class mutex_lock_guard
    {        
        pthread_mutex_t _mutex;
        public:
            mutex_lock_guard(pthread_mutex_t& mutex)
            {
                _mutex = mutex;
                pthread_mutex_unlock(&_mutex);
            };

            ~mutex_lock_guard()
            {
                pthread_mutex_unlock(&_mutex);
            }
    };
}

#endif