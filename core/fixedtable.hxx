/*
fixedtable.hxx file is part of reactdpdk.
reactdpdk is a practice example of dpdk based project.
Copyright (C) 2025  Kamanashis Roy
reactdpdk is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
reactdpdk is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
You should have received a copy of the GNU General Public License
along with reactdpdk.  If not, see <https://www.gnu.org/licenses/>.
*/

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
        auto numInserted = self.data[pos].numInserted;

        // try replace first
        for(size_t i = 0,j = 0; i < CAPACITY and j < numInserted; i++)
        {
            auto&cur = self.data[(i+pos)%CAPACITY];
            if(cur.content.has_value())
            {
                auto key2 = cur.content->first;
                if(key2 == key)
                {
                    // replace
                    cur.content.reset();
                    cur.content.emplace(std::piecewise_construct, std::forward_as_tuple(key), std::forward_as_tuple(args...));
                    return &cur.content.value();
                }
                auto pos2 = self.calcPos_(key2);
                if(pos2 == pos)
                {
                    j++; // number of element processed
                }
            }
        }
        for(int i = 0; i < CAPACITY; i++)
        {
            auto&cur = self.data[(i+pos)%CAPACITY];
            if(not cur.content.has_value())
            {
                cur.content.emplace(std::piecewise_construct, std::forward_as_tuple(key), std::forward_as_tuple(args...));
                self.data[pos].numInserted ++;
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
            auto&cur = self.data[(i+pos)%CAPACITY];
            if(cur.content.has_value())
            {
                auto key2 = cur.content->first;
                if(key2 == key)
                {
                    return &cur.content.value();
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
            auto&cur = self.data[(i+pos)%CAPACITY];
            if(cur.content.has_value())
            {
                auto key2 = cur.content->first;
                if(key2 == key)
                {
                    cur.content.reset();
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

    std::array<TContent,CAPACITY> data;

    std::size_t cnt = 0;
};

}

#endif // NGINZ_FIXED_TABLE_HXX
