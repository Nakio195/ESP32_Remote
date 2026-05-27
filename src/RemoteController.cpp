/*
 * RemoteController.cpp
 *
 *  Created on: 31 janv. 2025
 *      Author: To
 */

#include "RemoteController.h"
#include <Ps3Controller.h>
#include "esp_log.h"
#include "esp_bt.h"
#include "esp_gap_bt_api.h"

RemoteController PS3Controller;

RemoteController::RemoteController() 
{    
	L_Stick_x = 0;
	L_Stick_y = 0;
	R_Stick_x = 0;
	R_Stick_y = 0;
	// Analog Triggers
	L2_Trigger = 0;
	R2_Trigger = 0;
	// Symbols buttons
	Cross = 0;
	Square = 0;
	Triangle = 0;
	Circle = 0;
	// DPAD buttons
	DPAD_L = 0;
	DPAD_U = 0;
	DPAD_R = 0;
	DPAD_D = 0;
	//LR Buttons
	L1 = 0;
	R1 = 0;
	L3 = 0;
	R3 = 0;
	// Control buttons
	Start = 0;
	Select = 0;
	PlayStation = 0;
	//Status information
	isConnected = 0;
	BatteryLevel = 0;
	LedControl = 0;

	// Analog DeadZones
	L_AnalogDeadZone = 0;
	R_AnalogDeadZone = 0;
	L_TriggerDeadZone = 0;
	R_TriggerDeadZone = 0;

    // Heartbeat timer
    lastReportTime = 0;

}

RemoteController::~RemoteController() {
	// TODO Auto-generated destructor stub
}


void RemoteController::begin()
{
    Ps3.attach(PS3Controller.update);
    Ps3.attachOnConnect(PS3Controller.onConnect);
    Ps3.attachOnDisconnect(PS3Controller.onDisconnect);
    Ps3.begin("8c:7c:b5:2f:a1:ee"); // Seb
    //Ps3.begin("28:0d:fc:0f:22:4b"); // EA
}

ControllerData RemoteController::getControllerData()
{
    ControllerData data;

    data.Throttle = (uint8_t)(R2_Trigger)/4;
    data.Brake = L2_Trigger;
    data.Steering = L_Stick_x;

    data.ParkBrake = Start;
    data.BrakeSwitch = R2_Trigger > 10;
    data.TurnLight_L = L1;
    data.TurnLight_R = R1;
    data.WarnLight = Square;
    data.Light = Circle;
    data.Horn = Triangle;
    data.Reverse = Square;

	return data;
}

void RemoteController::onConnect()
{
    Serial.println("Controller Connected");
	PS3Controller.isConnected = true;
    PS3Controller.lastReportTime = millis();
}

void RemoteController::onDisconnect()
{
    Serial.println("Controller Disconnected");
	PS3Controller.isConnected = false;
}

void RemoteController::tick()
{
    lastReportTime = millis();;
}

void RemoteController::checkConnection(uint32_t currentTime)
{
    // If no report received for more than 50 ms, consider disconnected
    if(isConnected && (currentTime - lastReportTime) > 50)
        onDisconnect();
}

void RemoteController::update()
{
    uint32_t currentTime = millis();
    // Heartbeat every 100ms
    PS3Controller.tick();
    //--- Digital cross/square/triangle/circle button events ---
    if( Ps3.event.button_down.cross )
        PS3Controller.Cross = true;
    if( Ps3.event.button_up.cross )
    	PS3Controller.Cross = false;

    if( Ps3.event.button_down.square )
    	PS3Controller.Square = true;
    if( Ps3.event.button_up.square )
    	PS3Controller.Square = false;

    if( Ps3.event.button_down.triangle )
    	PS3Controller.Triangle = true;
    if( Ps3.event.button_up.triangle )
    	PS3Controller.Triangle = false;

    if( Ps3.event.button_down.circle )
    	PS3Controller.Circle = true;
    if( Ps3.event.button_up.circle )
    	PS3Controller.Circle = false;

    //--------------- Digital D-pad button events --------------
    if( Ps3.event.button_down.up )
    	PS3Controller.DPAD_U = true;
    if( Ps3.event.button_up.up )
    	PS3Controller.DPAD_U = false;

    if( Ps3.event.button_down.right )
    	PS3Controller.DPAD_R = true;
    if( Ps3.event.button_up.right )
    	PS3Controller.DPAD_R = false;

    if( Ps3.event.button_down.down )
    	PS3Controller.DPAD_D = true;
    if( Ps3.event.button_up.down )
    	PS3Controller.DPAD_D = false;

    if( Ps3.event.button_down.left )
    	PS3Controller.DPAD_L = true;
    if( Ps3.event.button_up.left )
    	PS3Controller.DPAD_L = false;

    //------------- Digital shoulder button events -------------
    if( Ps3.event.button_down.l1 )
    	PS3Controller.L1 = true;
    if( Ps3.event.button_up.l1 )
    	PS3Controller.L1 = false;

    if( Ps3.event.button_down.r1 )
    	PS3Controller.R1 = true;
    if( Ps3.event.button_up.r1 )
    	PS3Controller.R1 = false;

    //--------------- Digital stick button events --------------
    if( Ps3.event.button_down.l3 )
    	PS3Controller.L3 = true;
    if( Ps3.event.button_up.l3 )
    	PS3Controller.L3 = false;

    if( Ps3.event.button_down.r3 )
    	PS3Controller.R3 = true;
    if( Ps3.event.button_up.r3 )
    	PS3Controller.R3 = false;

    //---------- Digital select/start/ps button events ---------
    if( Ps3.event.button_down.select )
    	PS3Controller.Select = true;
    if( Ps3.event.button_up.select )
    	PS3Controller.Select = false;

    if( Ps3.event.button_down.start )
    	PS3Controller.Start = true;
    if( Ps3.event.button_up.start )
    	PS3Controller.Start = false;

    if( Ps3.event.button_down.ps )
    	PS3Controller.PlayStation = true;
    if( Ps3.event.button_up.ps )
    	PS3Controller.PlayStation = false;


    //---------------- Analog stick value events ---------------
   if( abs(Ps3.event.analog_changed.stick.lx) || abs(Ps3.event.analog_changed.stick.ly))
   {
	   if(abs(Ps3.data.analog.stick.lx) > STICK_DEADZONE || abs(Ps3.data.analog.stick.ly) > STICK_DEADZONE)
	   {
		   PS3Controller.L_Stick_x = Ps3.data.analog.stick.lx;
		   PS3Controller.L_Stick_y = Ps3.data.analog.stick.ly * -1;
		   PS3Controller.L_AnalogDeadZone = false;
	   }

	   else
	   {
		   if(!PS3Controller.L_AnalogDeadZone)
		   {
			   PS3Controller.L_Stick_x = 0;
			   PS3Controller.L_Stick_y = 0;
		   }

		   PS3Controller.L_AnalogDeadZone = true;
	   }
   }

   if( abs(Ps3.event.analog_changed.stick.rx) || abs(Ps3.event.analog_changed.stick.ry))
   {
	   if(abs(Ps3.data.analog.stick.rx) > STICK_DEADZONE || abs(Ps3.data.analog.stick.ry) > STICK_DEADZONE)
	   {
		   PS3Controller.R_Stick_x = Ps3.data.analog.stick.rx;
		   PS3Controller.R_Stick_y = Ps3.data.analog.stick.ry * -1;
		   PS3Controller.R_AnalogDeadZone = false;
	   }

	   else
	   {
		   if(!PS3Controller.R_AnalogDeadZone)
		   {
			   PS3Controller.R_Stick_x = 0;
			   PS3Controller.R_Stick_y = 0;
		   }

		   PS3Controller.R_AnalogDeadZone = true;
	   }
   }

   //---------------------- Triggers events --------------------

   if(abs(Ps3.event.analog_changed.button.l2))
   {
	   if(abs(Ps3.data.analog.button.l2) > STICK_DEADZONE || abs(Ps3.data.analog.button.l2) > STICK_DEADZONE)
	   {
		   PS3Controller.L2_Trigger = Ps3.data.analog.button.l2;
		   PS3Controller.L_TriggerDeadZone = false;
	   }

	   else
	   {
		   if(!PS3Controller.L_TriggerDeadZone)
			   PS3Controller.L2_Trigger = 0;

		   PS3Controller.L_TriggerDeadZone = true;
	   }
   }


   if(abs(Ps3.event.analog_changed.button.r2))
   {
	   if(abs(Ps3.data.analog.button.r2) > STICK_DEADZONE || abs(Ps3.data.analog.button.r2) > STICK_DEADZONE)
	   {
		   PS3Controller.R2_Trigger = Ps3.data.analog.button.r2;
		   PS3Controller.R_TriggerDeadZone = false;
	   }

	   else
	   {
		   if(!PS3Controller.R_TriggerDeadZone)
			   PS3Controller.R2_Trigger = 0;

		   PS3Controller.R_TriggerDeadZone = true;
	   }
   }

   //---------------------- Battery events ---------------------
   int battery = PS3Controller.BatteryLevel;
    if( battery != Ps3.data.status.battery ){
        battery = Ps3.data.status.battery;
        if( battery == ps3_status_battery_charging )      PS3Controller.BatteryLevel = RemoteController::Charging;
        else if( battery == ps3_status_battery_full )     PS3Controller.BatteryLevel = RemoteController::Full;
        else if( battery == ps3_status_battery_high )     PS3Controller.BatteryLevel = RemoteController::High;
        else if( battery == ps3_status_battery_low)       PS3Controller.BatteryLevel = RemoteController::Low;
        else if( battery == ps3_status_battery_dying )    PS3Controller.BatteryLevel = RemoteController::Dying;
        else if( battery == ps3_status_battery_shutdown ) PS3Controller.BatteryLevel = RemoteController::Shutdown;
        else PS3Controller.BatteryLevel = RemoteController::Undefined;
    }

}
