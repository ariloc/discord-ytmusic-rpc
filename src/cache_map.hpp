#ifndef __CACHE_MAP_HPP__
#define __CACHE_MAP_HPP__

#include <map>
#include <list>

static const int MAX_CACHE = 20;

// key -> songInfo, val -> std::string (url)
template <typename Key, typename Val> class sized_cacheMap {
    struct Wrapper;
    typedef std::map<Key,Wrapper> Map;
    typedef typename Map::const_iterator map_iter;
    typedef std::list< std::pair< Val, map_iter > > List;
    typedef typename List::const_iterator list_iter;
    struct Wrapper {
        list_iter iter;
    };

    Map map;
    List list;

    public: 
        int max_cache = MAX_CACHE;
        
        // Position increments cronologically. The highest position represents the newest entry (list.rbegin()).
        void update() {
            while ((int)list.size() > max_cache) {
                map.erase((*list.begin()).second); // erase the map element pointed by the iterator
                list.pop_front(); // erase the oldest list element
            }
        }
        void insert (Key k, Val v) {
            auto it = map.find(k); // does the song exist in the cache?
            if (it != map.end()) {
                list.splice(list.end(), list, (*it).second.iter); // if it does, move the entry to the end
                list.rbegin()->first = v; // and change the url to the new one (if equal, it would be overwritten)
            }
            else {
                list.push_back({v,map.end()}); // if it doesn't, first insert the url with dummy iterator
                auto newIt = ( map.insert({ k, {prev(list.end())} }) ).first; // then insert the key and list iterator into the map, and get the element iterator
                list.rbegin()->second = newIt; // change the iterator in the list with the one of the map we got
            }
            update();
        }
        list_iter find (Key k) {
            auto it = map.find(k);
            if (it == map.end()) return list.end();
            return (*it).second.iter;
        }
        list_iter end() {return list.end();}
        list_iter begin() {return list.begin();}
};

#endif /* __CACHE_MAP_HPP__ */