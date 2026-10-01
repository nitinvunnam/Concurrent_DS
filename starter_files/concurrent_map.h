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
#include <memory> //for unique_ptr 

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
        struct alignas(Padded ? CACHE_LINE : alignof(Lock)) Shard { 
            std::map<K, V> map;
            mutable Lock lock;
        };

        static_assert(!Padded || sizeof(Shard) % CACHE_LINE == 0); //error if cache behavior is wrong

        std::unique_ptr<Shard[]> shards_; //array implementation to avoid resizing
        std::size_t nshards_;
    public:
        explicit ShardedMap(std::size_t nshards)
            : shards_(std::make_unique<Shard[]>(nshards)),
            nshards_(nshards)
        {
        }
        std::size_t shard_count() const{
            return nshards_;
        };
        bool insert(const K& key, const V& value){
            std::size_t index = std::hash<K>{}(key) % nshards_;
            std::lock_guard<Lock> guard(shards_[index].lock);
            auto [it, inserted] = shards_[index].map.insert({key, value});
            it->second = value;
            return inserted;
        };// true if key was new

        bool find  (const K& key, V& out) const{
            std::size_t index = std::hash<K>{}(key) % nshards_;
            ReadGuard<Lock> guard(shards_[index].lock);

            auto it = shards_[index].map.find(key);

            if(it == shards_[index].map.end()){
                return false;
            }

            out = it->second;
            return true;
        };     // copy; false if absent

        bool erase (const K& key){
            std::size_t index = std::hash<K>{}(key) % nshards_;
            std::lock_guard<Lock> guard(shards_[index].lock);

            if (shards_[index].map.erase(key)){
                return true;
            }
            return false;
        };                   // true if was present

        std::size_t size() const {
            std::vector<std::unique_lock<Lock>> guards;
            guards.reserve(nshards_);

            for (std::size_t i = 0; i < nshards_; ++i) {
                guards.emplace_back(shards_[i].lock);
            }

            std::size_t total = 0;

            for (std::size_t i = 0; i < nshards_; ++i) {
                total += shards_[i].map.size();
            }

            return total;
        }      
};// Part 4

#endif /* CONCURRENT_MAP_H */
