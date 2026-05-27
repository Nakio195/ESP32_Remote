/*
 * MainController.h
 *
 *  Created on: 31 janv. 2025
 *      Author: To
 */

#ifndef MAINCONTROLLER_H_
#define MAINCONTROLLER_H_

#include "Arduino.h"
#include "AMT232.h"

#define THROTTLE_PIN 15
#define AN_BRAKE_PIN 2

#define BRAKE_PIN 0
#define PARKBRAKE_PIN 16
#define TURNLIGHT_L_PIN 34
#define TURNLIGHT_R_PIN 35
#define LIGHT_PIN 4
#define WARNLIGHT_PIN 17
#define HORN_PIN 23
#define REVERSE_PIN 14

#define STICK_DEADZONE 2

enum {DATA_THROTTLE=0, DATA_BRAKE=1, DATA_STEERING_H=2, DATA_STEERING_L=3, DATA_FLAGS=4};

struct ControllerData
{
        //Analog inputs
		uint16_t Throttle; 
		uint16_t Brake; 
		int16_t Steering;
        int16_t rawSteering;
		
		// Digital inputs
		bool ParkBrake;
		bool BrakeSwitch;
		bool TurnLight_L;
		bool TurnLight_R;
		bool WarnLight;
		bool Light;
		bool Horn;
        bool Reverse;
};

class MainController {
	public:
		MainController();
		virtual ~MainController();
		void update();
		ControllerData getControllerData();
		uint8_t* getControllerStatus();

	public:
	
        ControllerData Data;

        AMT232 SteeringEncoder;
    
    private:
		uint8_t ControllerStatus[6];
};

extern MainController Controller;

#endif /* MAINCONTROLLER_H_ */
