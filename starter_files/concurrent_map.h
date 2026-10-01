// concurrent_map.h -- Lab 1: Part 1 and Part 4.  YOU WRITE THIS FILE.
//
// Nothing compiles until CoarseMap exists, so it is the first thing
// to write.  What the harness and tests expect from this header:
//
//   template <typename K, typename V>
//   class CoarseMap;                               // Part 1
//
//   template <typename K, typename V,
//             class Lock = std::mutex, bool Padded = true>
//       requires BasicLock<Lock>
//   class ShardedMap;                              // Part 4
//       explicit ShardedMap(std::size_t nshards);
//       std::size_t shard_count() const;
//
// Both must satisfy ConcurrentMap<M, K, V> from interface.h:
//
//   bool insert(const K& key, const V& value);   // true if key was new
//   bool find  (const K& key, V& out) const;     // copy; false if absent
//   bool erase (const K& key);                   // true if was present
//   std::size_t size() const;
//
// Values come out as COPIES, never references or iterators into the
// container.  The report asks why.
//
// Flip HAVE_SHARDED in parts.h when ShardedMap compiles.

#ifndef CONCURRENT_MAP_H
#define CONCURRENT_MAP_H

#include <cstddef>
#include <functional>
#include <map>
#include <mutex>
#include <vector>

#include "interface.h"

template <typename K, typename V>
class CoarseMap{
    private:
        std::map<K,V> map_;
        mutable std::mutex mutex_;
    
    public:
        bool insert(const K& key, const V& value){
            std::lock_guard<std::mutex> guard(mutex_);
            auto [it, inserted] = map_.insert({key, value});
            it->second = value;
            return inserted;
        };   // true if key was new

        bool find(const K& key, V& out) const{
            std::lock_guard<std::mutex> guard(mutex_);

            auto it = map_.find(key);

            if(it == map_.end()){
                return false;
            }

            out = it->second;
            return true;

        };     // copy out; false if absent
        bool erase (const K& key){
            std::lock_guard<std::mutex> guard(mutex_);

            if (map_.erase(key)){
                return true;
            }
            return false;

        };                   // true if it was present
        std::size_t size() const{
            std::lock_guard<std::mutex> guard(mutex_);
            return map_.size();
        };
};

template <typename K, typename V, class Lock = std::mutex, bool Padded = true>
    requires BasicLock<Lock>
class ShardedMap{
    private:
        struct Shard{
            std::map<K,V> map;
            mutable Lock lock;
        };
        std::vector<Shard> shards;
    public:
        explicit ShardedMap(std::size_t nshards)
            :shards(nshards)
        {
        };
        std::size_t shard_count() const{
            return shards.size();
        };
        bool insert(const K& key, const V& value){
            std::size_t index = std::hash<K>{}(key) % shards.size();
            std::lock_guard<Lock> guard(shards[index].lock);
            auto [it, inserted] = shards[index].map.insert({key, value});
            it->second = value;
            return inserted;
        };// true if key was new

        bool find  (const K& key, V& out) const{
            std::size_t index = std::hash<K>{}(key) % shards.size();
            std::lock_guard<Lock> guard(shards[index].lock);
            auto it = shards[index].map.find(key);

            if(it == shards[index].map.end()){
                return false;
            }

            out = it->second;
            return true;
        };     // copy; false if absent

        bool erase (const K& key){
            std::size_t index = std::hash<K>{}(key) % shards.size();
            std::lock_guard<Lock> guard(shards[index].lock);

            if (shards[index].map.erase(key)){
                return true;
            }
            return false;
        };                   // true if was present
        std::size_t size() const{
            std::size_t index = std::hash<K>{}(key) % shards.size();
            std::lock_guard<Lock> guard(shards[index].lock);

            return shards[index].map.size();
        };        
};// Part 4

#endif /* CONCURRENT_MAP_H */
