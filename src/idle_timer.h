#ifndef __IDLE_TIMER_H__
#define __IDLE_TIMER_H__

#include <chrono>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <thread>

class IdleTimer {
    public: 
        /**
         * Default constructor.
         * Takes an `idleTask` that should be run when the timer expires.
         */
        IdleTimer(std::function<void()> idleTask);

        /**
         * Starts the idle timer that will run the task function on timeout after `IDLE_PERIOD` 
         * elapsed.
         */
        void start();

        /**
         * Stops the idle timer if it's running.
         * The task function WON'T be run.
         */
        void cancel();
        
        /**
         * Default destructor.
         */
        ~IdleTimer();

    private:
        /**
         * Idle period: Period that has to elapse for the task provided to be run.
         */
        static const std::chrono::minutes IDLE_PERIOD;

        std::function<void()> task;
        std::condition_variable timer;
        std::mutex timerMutex;
        std::atomic<bool> stopSignal;
        std::thread currentThread;
};

#endif /* __IDLE_TIMER_H__ */
