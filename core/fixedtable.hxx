
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

    struct TContent {
        std::size_t numInserted = 0;
        std::optional<T> content;
    };

    inline std::size_t calcPos_(KeyType key)
    {
        return (key*PRIME)%CAPACITY;
    }

    template<typename... Params>
    Iterator emplace(KeyType key, Params... args)
    {
        auto&self = *this;
        if (self.cnt >= CAPACITY)
        {
            return nullptr;
        }
        auto pos  = self.calcPos_(key);

        for(int i = 0; i < CAPACITY; i++)
        {
            if(self.data[i+pos].content.has_value())
            {
                if(self.data[i+pos].content->first == key)
                {
                    // replace
                    self.data[i+pos].content.reset();
                    self.data[i+pos].content.emplace(std::piecewise_construct, std::forward_as_tuple(key), std::forward_as_tuple(args...));
                    return &self.data[i+pos].content.value();
                }
            }
            else
            {
                self.data[i+pos].content.emplace(std::piecewise_construct, std::forward_as_tuple(key), std::forward_as_tuple(args...));
                self.data[i+pos].numInserted ++;
                self.cnt ++;
                assert(self.cnt < CAPACITY);
                return &self.data[i+pos].content.value();
            }
        }
        return nullptr;
    }

    template<typename... Params>
    Iterator find(KeyType key)
    {
        auto&self = *this;
        auto pos  = self.calcPos_(key);

        auto numInserted = self.data[pos].numInserted;

        for(size_t i = 0,j = 0; i < CAPACITY and j < numInserted; i++)
        {
            if(self.data[i+pos].content.has_value())
            {
                auto key2 = self.data[i+pos].content->first;
                if(key2 == key)
                {
                    return &self.data[i+pos].content.value();
                }
                auto pos2 = self.calcPos_(key2);
                if(pos2 == pos)
                {
                    j++; // number of element processed
                }
            }
        }
        return nullptr;
    }



    bool erase(KeyType key)
    {
        auto&self = *this;
        auto pos  = self.calcPos_(key);

        auto numInserted = self.data[pos].numInserted;

        for(size_t i = 0,j = 0; i < CAPACITY and j < numInserted; i++)
        {
            if(self.data[i+pos].content.has_value())
            {
                auto key2 = self.data[i+pos].content->first;
                if(key2 == key)
                {
                    self.data[i+pos].content.reset();
                    self.data[pos].numInserted--;
                    self.cnt--;
                    return true;
                }
                auto pos2 = self.calcPos_(key2);
                if(pos2 == pos)
                {
                    j++; // number of element processed
                }
            }
        }
        return false;
    }


    bool empty() const {
        return 0 == cnt;
    }

    bool full() const {
        return CAPACITY == cnt;
    }


    std::size_t size() const {
        return cnt;
    }
    TContent data[CAPACITY];
    std::size_t cnt = 0;
};

}

#endif // NGINZ_FIXED_TABLE_HXX
