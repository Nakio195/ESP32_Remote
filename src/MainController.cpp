/*
 * MainController.cpp
 *
 *  Created on: 31 janv. 2025
 *      Author: To
 */

#include "MainController.h"

MainController Controller;

MainController::MainController() {
	// TODO Auto-generated constructor stub
	Data.Throttle = 0;
	Data.Steering = 123;
	Data.Brake = 0;
	Data.ParkBrake = false;
	Data.TurnLight_L = false;
	Data.TurnLight_R = false;
	Data.Light = false;
	Data.WarnLight = false;
	Data.Horn = false;
	Data.Reverse = false;

	pinMode(THROTTLE_PIN, INPUT);
	pinMode(AN_BRAKE_PIN, INPUT);
	pinMode(BRAKE_PIN, INPUT_PULLUP);
	pinMode(PARKBRAKE_PIN, INPUT);
	pinMode(TURNLIGHT_L_PIN, INPUT);
	pinMode(TURNLIGHT_R_PIN, INPUT);
	pinMode(LIGHT_PIN, INPUT);
	pinMode(WARNLIGHT_PIN, INPUT);
	pinMode(HORN_PIN, INPUT);
	pinMode(REVERSE_PIN, INPUT);
}

MainController::~MainController() {
	// TODO Auto-generated destructor stub
}

ControllerData MainController::getControllerData()
{
	return Data;
}

uint8_t* MainController::getControllerStatus()
{
	uint32_t timestamp = millis();
	ControllerStatus[0] = timestamp >> 24 & 0xFF;
	ControllerStatus[1] = timestamp >> 16 & 0xFF;
	ControllerStatus[2] = timestamp >> 8 & 0xFF;
	ControllerStatus[3] = timestamp & 0xFF;

	return ControllerStatus;
}

void MainController::update()
{
	Data.Throttle = (analogRead(THROTTLE_PIN) >> 4);
	Data.Brake = (analogRead(AN_BRAKE_PIN) >> 4);


    if(Data.Throttle < 40)
        Data.Throttle = 0;
    else
        Data.Throttle -= 40;

    if(Data.Brake < 40)
        Data.Brake = 0;
    else
        Data.Brake -= 40;

    Data.Throttle *= 1.70;
    Data.Brake *= 1.53;

    Data.Throttle = constrain(Data.Throttle, 0, 255);
    Data.Brake = constrain(Data.Brake, 0, 255);

	Data.BrakeSwitch = !digitalRead(BRAKE_PIN);
	Data.ParkBrake = digitalRead(PARKBRAKE_PIN);
	Data.TurnLight_L = digitalRead(TURNLIGHT_L_PIN);
	Data.TurnLight_R = digitalRead(TURNLIGHT_R_PIN);
	Data.Light = digitalRead(LIGHT_PIN);
	Data.WarnLight = digitalRead(WARNLIGHT_PIN);
	Data.Horn = digitalRead(HORN_PIN);
	Data.Reverse = digitalRead(REVERSE_PIN);

    bool readError = false;
    SteeringEncoder.read(SS_PIN, &readError);

    if(!readError)
    {
        Data.Steering = SteeringEncoder.getPosition();

        if(Data.Steering < STICK_DEADZONE && Data.Steering > -STICK_DEADZONE)
            Data.Steering = 0;
        else
        {
            if(Data.Steering > 0)
                Data.Steering -= STICK_DEADZONE;
            else
                Data.Steering += STICK_DEADZONE;
        }
        
    }
}
