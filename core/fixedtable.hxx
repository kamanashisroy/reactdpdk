
#ifndef NGINZ_FIXED_TABLE_HXX
#define NGINZ_FIXED_TABLE_HXX

#include <tuple>
#include <utility>

namespace nginz
{

template<typename KeyType, typename ValType, const size_t CAPACITY=128, const size_t PRIME=39>
struct FixedDict {
    using T = std::pair<KeyType,ValType>;
    using Iterator = T*;

    template<typename... Params>
    Iterator emplace(KeyType key, Params... args)
    {
        auto&self = *this;
        auto pos  = (key*PRIME)%CAPACITY;

        // TODO optimize in worst case , so that we do not search thorugh whole CAPACITY
        for(int i = 0; i < CAPACITY; i++)
        {
            if(self.data[i+pos].has_value())
            {
                if(self.data[i+pos]->first == key)
                {
                    // replace
                    self.data[i+pos].reset();
                    self.data[i+pos].emplace(std::piecewise_construct, std::forward_as_tuple(key), std::forward_as_tuple(args...));
                    return &self.data[i+pos].value();
                }
            }
            else
            {
                self.data[i+pos].emplace(std::piecewise_construct, std::forward_as_tuple(key), std::forward_as_tuple(args...));
                return &self.data[i+pos].value();
            }
        }
        return nullptr;
    }

    template<typename... Params>
    Iterator find(KeyType key)
    {
        auto&self = *this;
        auto pos  = (key*PRIME)%CAPACITY;

        // TODO optimize in worst case , so that we do not search thorugh whole CAPACITY
        for(int i = 0; i < CAPACITY; i++)
        {
            if(self.data[i+pos].has_value())
            {
                if(self.data[i+pos]->first == key)
                {
                    return &self.data[i+pos].value();
                }
            }
        }
        return nullptr;
    }



    void erase(KeyType key)
    {
        auto pos = (key*PRIME)%CAPACITY;

        // TODO optimize in worst case , so that we do not search thorugh whole CAPACITY
        for(int i = 0; i < CAPACITY; i++)
        {
            if(data[i+pos].has_value())
            {
                if(data[i+pos]->first == key)
                {
                    data[i+pos].reset();
                    return ;
                }
            }
        }
    }


    bool empty() const {
        return 0 == cnt;
    }

    std::size_t size() const {
        return cnt;
    }
    std::optional<T> data[CAPACITY];
    std::size_t cnt = 0;
};

}

#endif // NGINZ_FIXED_TABLE_HXX
