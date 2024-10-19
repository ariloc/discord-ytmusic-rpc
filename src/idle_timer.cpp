#include "idle_timer.h"

const std::chrono::minutes IdleTimer::IDLE_PERIOD = std::chrono::minutes(15);

IdleTimer::IdleTimer(std::function<void()> idleTask) {
    task = idleTask;
    stopSignal = false;
}

void IdleTimer::start() {
    cancel();
    currentThread = std::thread([this]() {
        std::unique_lock<std::mutex> lock(timerMutex);
        bool stoppedEarly = timer.wait_for(lock, IDLE_PERIOD, [this]{ return stopSignal == true; });
        if (!stoppedEarly)
            task();
    });
}

void IdleTimer::cancel() {
    {
        std::lock_guard<std::mutex> lock(timerMutex);
        stopSignal = true;
        timer.notify_all();
    }
    if (currentThread.joinable())
        currentThread.join();
    {
        std::lock_guard<std::mutex> lock(timerMutex);
        stopSignal = false;
    }
}

IdleTimer::~IdleTimer() { cancel(); }
