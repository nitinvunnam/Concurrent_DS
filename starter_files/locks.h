// locks.h -- Lab 1: Part 5 and Part 6.  YOU WRITE THIS FILE.
//
// What the harness and tests expect from this header:
//
//   Part 5 (BasicLock):     TASLock  TTASLock  TicketLock  ParkingLock
//   Part 6 (SharedLock):    RWLock   RWLockWP
//
// Every lock is a class with a default constructor and
//
//   void lock();
//   void unlock();
//
// and the two reader-writer locks additionally
//
//   void lock_shared();
//   void unlock_shared();
//
// so that std::lock_guard, std::unique_lock, std::shared_lock, and
// the lab's ReadGuard all work on them.  A lock must not be copyable
// or movable (std::atomic already sees to that).
//
// Everything in here is built from std::atomic.  No std::mutex, no
// std::shared_mutex, no OS primitives except the ones behind
// std::atomic::wait / notify_one, which ParkingLock uses.
//
// Flip HAVE_LOCKS (Part 5) and HAVE_RW (Part 6) in parts.h as each
// set compiles.

#ifndef LOCKS_H
#define LOCKS_H

#include <atomic>
#include <cstdint>
#include <thread>

#include "interface.h"

class TASLock{
    public:
        void lock(){
            while(locked_.exchange(true)){
                continue;
            }
        };
        void unlock(){
            locked_.store(false);
        };
    private:
        std::atomic<bool> locked_{false};
};

class TTASLock{
    public:
    void lock(){
        int backOff = 1;
        while(true){
            while(locked_.load()){
                __builtin_ia32_pause();
            }
            bool result = locked_.exchange(true);

            if(!result){
                break;
            }
            
            for(int i = 0; i < backOff; i++){
                __builtin_ia32_pause();
            }

            if(backOff < MAX_BACKOFF){
                backOff*=2;
            };
        }
    };
    void unlock(){
        locked_.store(false);
    };

    private:
        std::atomic <bool> locked_{false};
        const int MAX_BACKOFF = 64;
};


#endif /* LOCKS_H */
