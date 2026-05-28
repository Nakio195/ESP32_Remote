/*
 * RemoteController.h
 *
 *  Created on: 31 janv. 2025
 *      Author: To
 */

#ifndef RemoteController_H_
#define RemoteController_H_

#include "Arduino.h"

#define STICK_DEADZONE 10

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

class RemoteController 
{
	public:
		RemoteController();
		virtual ~RemoteController();
        void begin();
		static void update();
		static void onConnect();
		static void onDisconnect();
		ControllerData getControllerData();
		uint8_t* getControllerStatus();

        void checkConnection(uint32_t currentTime);

        void tick();

	public:
        bool StatusChanged;

		enum BatteryLevels{Undefined, Shutdown, Dying, Low, High, Full, Charging};
		//Analog stick
		int8_t	L_Stick_x;
		int8_t	L_Stick_y;
		int8_t	R_Stick_x;
		int8_t	R_Stick_y;
		// Analog Triggers
		int8_t L2_Trigger;
		int8_t R2_Trigger;
		// Symbols buttons
		bool Cross;
		bool Square;
		bool Triangle;
		bool Circle;
		// DPAD buttons
		bool DPAD_L;
		bool DPAD_U;
		bool DPAD_R;
		bool DPAD_D;
		//LR Buttons
		bool L1;
		bool R1;
		bool L3;
		bool R3;
		// Control buttons
		bool Start;
		bool Select;
		bool PlayStation;
		//Status information
		bool isConnected;
		uint8_t BatteryLevel;
		uint8_t LedControl;

		// Analog DeadZones
		bool L_AnalogDeadZone;
		bool R_AnalogDeadZone;
		bool L_TriggerDeadZone;
		bool R_TriggerDeadZone;
        uint32_t lastReportTime;
        
		uint8_t ControllerStatus[6];
};

extern RemoteController PS3Controller;

#endif /* RemoteController_H_ */
