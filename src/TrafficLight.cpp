#include <iostream>
#include <random>
#include "TrafficLight.h"

/* Implementation of class "MessageQueue" */

template <typename T>
T MessageQueue<T>::receive()
{
    // FP.5a : The method receive should use std::unique_lock<std::mutex> and _condition.wait() 
    // to wait for and receive new messages and pull them from the queue using move semantics. 
    // The received object should then be returned by the receive function. 
    std::unique_lock<std::mutex> lk(_mt);
    _cond.wait(lk);
    auto msg = std::move(_queue.front());
    _queue.pop_front();
    return msg;
    
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex> 
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.
    std::lock_guard<std::mutex> lk(_mt);
    _cond.notify_one();
    _queue.emplace_front(msg);
}


/* Implementation of class "TrafficLight" */


TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
}

void TrafficLight::waitForGreen()
{
    // FP.5b : add the implementation of the method waitForGreen, in which an infinite while-loop 
    // runs and repeatedly calls the receive function on the message queue. 
    // Once it receives TrafficLightPhase::green, the method returns.
    while(true)
    {
        if(_mq.receive() == TrafficLightPhase::green)
            return;
    }
}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::simulate()
{
    // FP.2b : Finally, the private method „cycleThroughPhases“ should be started in a thread when the public method „simulate“ is called. To do this, use the thread queue in the base class. 
    threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    // FP.2a : Implement the function with an infinite loop that measures the time between two loop cycles 
    // and toggles the current phase of the traffic light between red and green and sends an update method 
    // to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds. 
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles. 
    
    // init stop watch
    auto cycle = 0;

    std::random_device rd;
    std::mt19937 gen(rd()); // Seed the generator with system entropy
    std::uniform_int_distribution<> dis(4, 6); // Set distribution for 4-6

    // Generate a random number within the range
    int num = dis(gen);

    while(true)
    {
        // compute time
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        cycle++;
        // toggle
        if(cycle == num)
        {        
            if (_currentPhase == TrafficLightPhase::green) {
                _currentPhase = TrafficLightPhase::red;
            }
            else {
                _currentPhase = TrafficLightPhase::green;
            }
        
            // send update
            _mq.send(std::move(_currentPhase));

            // reset stop watch
            cycle = 0;
        }
    }

}

