#include <string.h>                     // strcmp()
#include <iostream>                     // atoi()
#include <thread>                       // std::thread and join()
#include <chrono>                       // std::chrono::high_resolution_clock::now();
#include <vector>                       // std::vector<>
#include <mutex>                        // std::mutex
#include <atomic>                       // std::atomic<int> and fetch_add()

std::mutex sharedCounter_mtx;
std::atomic<bool> start(false);         // to ensure threads run in parallel
std::vector<int> localCounterVector;


/*
 * incrementiTimesRaceCondition will run the command '++sharedCounter' i times
 * once start is set to true
 *
 * Input Arguments:
 * sharedCounter - reference to variable to increment i times
 * i - reference to the number of times to increment sharedCounter
 *
 * Return Values:
 * None
 */
void incrementiTimesRaceCondition(int& sharedCounter, int& i) {
    while (!start.load());
    for(int incrementCounter = 0; incrementCounter < i; ++incrementCounter) {
        ++sharedCounter;
    }
}

/*
 * incrementiTimesMutexLock will lock a mutex, run the commnad '++sharedCounter'
 * i times, then unlock the mutex once start is set to true
 *
 * Input Arguments:
 * sharedCounter - reference to variable to lock, increment i times, and unlock
 * i - reference to the number of times to increment sharedCounter
 *
 * Return Values:
 * None
 */
void incrementiTimesMutexLock(int& sharedCounter, int& i) {
    while (!start.load());
    sharedCounter_mtx.lock();
    for(int incrementCounter = 0; incrementCounter < i; ++incrementCounter) {
        ++sharedCounter;
    }
    sharedCounter_mtx.unlock();
}

/*
 * incrementiTimesLockGuard will lock a mutex with a lock guard then run the command
 * '++sharedCounter' i times once start is set to true.  The lock will automatically
 * be released when the lock guard goes out of scope
 *
 * Input Arguments:
 * sharedCounter - reference to variable to lock with a lock guard then increment i times
 * i - reference to the number of times to increment sharedCounter
 *
 * Return Values:
 * None
 */
void incrementiTimesLockGuard(int& sharedCounter, int& i) {
    while (!start.load());
    std::lock_guard<std::mutex> lock(sharedCounter_mtx);
    for(int incrementCounter = 0; incrementCounter < i; ++incrementCounter) {
        ++sharedCounter;
    }
}

/*
 * incrementiTimesAtomic will run the command 'sharedCounterAtomic.fetch_add(1, std::memory_order_relaxed)'
 * i times once start is set to true
 *
 * Input Arguments:
 * sharedCounterAtomic - reference to atomic variable to increment i times
 * i - reference to the number of times to increment sharedCounterAtomic
 *
 * Return Values:
 * None
 */
void incrementiTimesAtomic(std::atomic<int>& sharedCounterAtomic, int& i) {
    while (!start.load());
    for(int incrementCounter = 0; incrementCounter < i; ++incrementCounter) {
        sharedCounterAtomic.fetch_add(1, std::memory_order_relaxed);
    }
}

/*
 * incrementiTimesLocalCounter will run the command '++localCounterVector[iterator]'
 * i times once start is set to true
 *
 * Input Arguments:
 * iterator - value of index in localCounterVector to increment i times
 * i - reference to the number of times to increment sharedCounterAtomic
 */
void incrementiTimesLocalCounter(int iterator, int& i) {
    while (!start.load());
    for(int incrementCounter = 0; incrementCounter < i; ++incrementCounter) {
        ++localCounterVector[iterator];
    }
}

int main(int argc, char *argv[]) {

    // Default values for t and i are 4 and 10000, respectively
    int t = 4;
    int i = 10000;
    int sharedCounter = 0;
    std::atomic<int> sharedCounterAtomic;
    sharedCounterAtomic = 0;
    std::vector<std::thread> threadVector;

    /*
     * Parsing command line arguments...
     * Argument directly following "-t" (if any) will be t
     * Argument directly following "-i" (if any) will be i
     * If multiple "-t" or multiple "-i" flags are found, the last one will be used
     * If "-t" or "-i" is the last command line argument, it will be ignored
     */
    int lastIndexToCheck = argc-1;
    for(int argcIterator = 1; argcIterator < lastIndexToCheck; ++argcIterator) {
        if(strcmp(argv[argcIterator], "-t") == 0) {
            argcIterator += 1;
            t = atoi(argv[argcIterator]);
        }
        else if (strcmp(argv[argcIterator], "-i") == 0) {
            argcIterator += 1;
            i = atoi(argv[argcIterator]);
        }
    }
    
    std::cout << "Function Name\tFinal Couter Value\tThreads\tIncrements/Millisecond\tSeconds\n";
    
    /*
     * t threads each increment sharedCounter i times in parallel with race condition
     * Incorrect value of sharedCounter will result due to race condition
     *
     * t, Increments/Millisecond, and seconds will be printed, threadVector, sharedCounter
     * and start will be reset
     */
    for(int iterator = 0; iterator < t; ++iterator) {
        threadVector.push_back(std::thread(incrementiTimesRaceCondition, std::ref(sharedCounter), std::ref(i)));
    }
    auto t1 = std::chrono::high_resolution_clock::now();
    start = true;
    for(auto& t : threadVector) {
        t.join();
    }
    auto t2 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> tDelta = t2-t1;
    auto seconds = tDelta.count();
    std::cout << "incrementiTimesRaceCondition\t" << sharedCounter<< "\t" << t << "\t" << sharedCounter*1000/seconds << "\t" << seconds << "\n";
    sharedCounter = 0;
    threadVector.clear();
    start = false;
    
    /*
     * t threads each increment sharedCounter i in parallel times using a mutex
     * sharedCounter will be set to i*t
     *
     * t, Increments/Millisecond, and seconds will be printed, threadVector, sharedCounter
     * and start will be reset
     */
    for(int iterator = 0; iterator < t; ++iterator) {
        threadVector.push_back(std::thread(incrementiTimesMutexLock, std::ref(sharedCounter), std::ref(i)));
    }
    t1 = std::chrono::high_resolution_clock::now();
    start = true;
    for(auto& t : threadVector) {
        t.join();
    }
    t2 = std::chrono::high_resolution_clock::now();
    tDelta = t2-t1;
    seconds = tDelta.count();
    std::cout << "incrementiTimesMutexLock\t" << sharedCounter<< "\t" << t << "\t" << sharedCounter*1000/seconds << "\t" << seconds << "\n";
    sharedCounter = 0;
    threadVector.clear();
    start = false;
    
    /*
     * t threads each increment sharedCounter i times in parallel using a lock guard
     * sharedCounter will be set to i*t
     *
     * t, Increments/Millisecond, and seconds will be printed, threadVector, sharedCounter
     * and start will be reset
     */
    for(int iterator = 0; iterator < t; ++iterator) {
        threadVector.push_back(std::thread(incrementiTimesLockGuard, std::ref(sharedCounter), std::ref(i)));
    }
    t1 = std::chrono::high_resolution_clock::now();
    start = true;
    for(auto& t : threadVector) {
        t.join();
    }
    t2 = std::chrono::high_resolution_clock::now();
    tDelta = t2-t1;
    seconds = tDelta.count();
    std::cout << "incrementiTimesLockGuard\t" << sharedCounter<< "\t" << t << "\t" << sharedCounter*1000/seconds << "\t" << seconds << "\n";
    sharedCounter = 0;
    threadVector.clear();
    start = false;
    
    /*
     * t threads each increment atomic int sharedCounterAtomic i times in parallel
     * sharedCounterAtomic will be set to i*t
     *
     * t, Increments/Millisecond, and seconds will be printed, threadVector, sharedCounter
     * and start will be reset
     */
    for(int iterator = 0; iterator < t; ++iterator) {
        threadVector.push_back(std::thread(incrementiTimesAtomic, std::ref(sharedCounterAtomic), std::ref(i)));
    }
    t1 = std::chrono::high_resolution_clock::now();
    start = true;
    for(auto& t : threadVector) {
        t.join();
    }
    t2 = std::chrono::high_resolution_clock::now();
    tDelta = t2-t1;
    seconds = tDelta.count();
    std::cout << "incrementiTimesAtomic\t" << sharedCounterAtomic<< "\t" << t << "\t" << sharedCounterAtomic*1000/seconds << "\t" << seconds << "\n";
    sharedCounterAtomic = 0;
    threadVector.clear();
    start = false;
    
    /*
     * t threads each increment a local variable t times and write the local variable
     * into the global vector localCounterVector.  After all threads have completed,
     * the sum of all elements in localCounterVector is stored in sharedCounter
     *
     * t, Increments/Millisecond, and seconds will be printed, localCounterVector,
     * sharedCounter and start will be reset
     */
    for(int iterator = 0; iterator < t; ++iterator) {
        localCounterVector.push_back(0);
        threadVector.push_back(std::thread(incrementiTimesLocalCounter, iterator, std::ref(i)));
    }
    t1 = std::chrono::high_resolution_clock::now();
    start = true;
    for(auto& t : threadVector) {
        t.join();
    }
    for(auto& localCounter : localCounterVector) {
        sharedCounter += localCounter;
    }
    t2 = std::chrono::high_resolution_clock::now();
    tDelta = t2-t1;
    seconds = tDelta.count();
    std::cout << "incrementiTimesLocalCounter\t" << sharedCounter<< "\t" << t << "\t" << sharedCounter*1000/seconds << "\t" << seconds << "\n";
    sharedCounter = 0;
    threadVector.clear();
    localCounterVector.clear();
    start = false;
    
}