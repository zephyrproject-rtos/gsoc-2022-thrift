#include <thrift/concurrency/Mutex.h>

namespace apache
{
    namespace thrift
    {
        namespace concurrency
        {

            Mutex::Mutex() {}

            void Mutex::lock() const {}

            bool Mutex::trylock() const
            {
                return false;
            }

            bool Mutex::timedlock(int64_t milliseconds) const
            {
                return false;
            }

            void Mutex::unlock() const
            {
            }

            void *Mutex::getUnderlyingImpl() const
            {
                return nullptr;
            }
        }
    }
} // apache::thrift::concurrency