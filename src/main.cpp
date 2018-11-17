/**
 * \file Main file 
 * \brief It instantiates the vending machine system and the platform-oriented interfaces 
 * 
 * \author André Mattos <andrempmattos@gmail.com>
 * \author Daniel Baron <zdaniz22@gmail.com>
 * 
 * \date 10/28/2017
 * 
 * \defgroup VendingMachineCore
 */

#include "../include/VendingMachine.hpp"
#include "../include/DebugInterface.hpp"
#include "../include/ProjectIncludes.hpp"

using namespace std::chrono;
using namespace VMCore;

//Determine our platform's tic period
const double microsPerClkTic{1.0E6 * system_clock::period::num / system_clock::period::den};

//Our REQUIRED processing period
const milliseconds intervalPeriodMillis{100};

//Initialize the chrono timepoints 
system_clock::time_point currentStartTime{system_clock::now()};
system_clock::time_point nextStartTime{currentStartTime};

//loop counter
int timerCounter;

//Application objects
Interface* interfaceOverride = new DebugInterface();
VendingMachine* machineCore = new VendingMachine(interfaceOverride);
void* pMachineCore = /*(void*)*/machineCore;

/*
int main() {
	while (true) {
		//Get our current "wakeup" time
		currentStartTime = system_clock::now();

		//Determine the point in time at which we want to wakeup for the
		//next pass through the loop.
		nextStartTime = currentStartTime + intervalPeriodMillis;
		
		//Timer for advertising switch
		++timerCounter;
		if (timerCounter%100 == 0) {
			std::cout << "propaganda" << std::endl;
			timerCounter = 0;
		}

		//User inputs
		UserData* user = new UserData();
		//interfaceOverride->getUserInput(user);
		//interfaceOverride->decodeUserInput(user);

		//Sleep till our next period start time
		std::this_thread::sleep_until(nextStartTime);
	}

	delete interfaceOverride;
}
*/

int main() {
	std::atomic<bool> interrupted;
	UserData* user = new UserData();

	while(true) {
		interrupted.store(false);

		// create a new thread that does stuff in the background
		std::thread VMThread([&]() {
			while(!interrupted) {
				//Get our current "wakeup" time
				currentStartTime = system_clock::now();

				//Determine the point in time at which we want to wakeup for the
				//next pass through the loop.
				nextStartTime = currentStartTime + intervalPeriodMillis;
				
				//Timer for advertising switch
				++timerCounter;
				if (timerCounter % 100 == 0) {
					machineCore->timerEvent();
					timerCounter = 0;
				}

				//User input handler
				interfaceOverride->decodeUserInput(user);

				//Sleep till our next period start time
				std::this_thread::sleep_until(nextStartTime);
			}
		});

	//User inputs
	interfaceOverride->getUserInput(user);

	// when input is complete, interrupt thread and wait for it to finish
	interrupted.store(true);
	VMThread.join();

	}

	delete interfaceOverride;
	delete machineCore;
}