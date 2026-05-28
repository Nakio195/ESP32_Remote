#include <Arduino.h>
#include <driver/twai.h>
#include <PS3Controller.h>

#include "RemoteController.h"

void checkTwaiAlerts();
void checkTwaiStatus();
void printError(esp_err_t err);
void heartbeat();
void sendControllerData(ControllerData controllerData);

bool TransmissionEnable = false;

enum ControllerType {Local, Remote};
ControllerType CurrentController = Local;

int player = 0;
int battery = 0;

twai_general_config_t g_config;
twai_timing_config_t t_config;
twai_filter_config_t f_config;

unsigned long previousTime = millis();
unsigned long controllerDataTimer = 0;
unsigned long controllerStatusTimer = 0;
unsigned long heartbeatTimer = 0;
unsigned long controllerUpdateTimer = 0;

void setup() {
  Serial.begin(115200);
  
  // TWAI Configuration
  g_config = TWAI_GENERAL_CONFIG_DEFAULT(GPIO_NUM_21, GPIO_NUM_22, TWAI_MODE_NORMAL);
  t_config = TWAI_TIMING_CONFIG_500KBITS();
  f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();
  g_config.alerts_enabled = TWAI_ALERT_ALL; 

  // Install TWAI driver
  if (twai_driver_install(&g_config, &t_config, &f_config) == ESP_OK) {
    Serial.println("TWAI driver installed.");
  } else {
    Serial.println("Failed to install TWAI driver.");
    return;
  }

  // Start the TWAI driver
  if (twai_start() == ESP_OK)
    Serial.println("TWAI driver started.");
  else 
    Serial.println("Failed to start TWAI driver.");

  PS3Controller.begin();

  Serial.println("Ready.");
  
}

void loop() {
	unsigned long dt = millis() - previousTime;
	previousTime = millis();

	controllerDataTimer += dt;
	controllerStatusTimer += dt;
    heartbeatTimer += dt;
    controllerUpdateTimer += dt;

    // Update Controller (hit if TransmissionEnable == false)
    if(controllerUpdateTimer >= 50)
    {
        PS3Controller.update();
        controllerUpdateTimer = 0;
    }

    //Check RemoteController connection status
    PS3Controller.checkConnection(millis());
    
    // Heartbeat Generation
    if(heartbeatTimer >= 1000) 
    {
        heartbeat();
        heartbeatTimer = 0;
    }

    // Controller data transmission
    if(controllerDataTimer >= 20 && TransmissionEnable)
    {
        // Update controller if TransmissionEnable and ready to send
        PS3Controller.update();
        controllerUpdateTimer = 0;

        if(PS3Controller.isConnected && CurrentController)
            sendControllerData(PS3Controller.getControllerData());
        controllerDataTimer = 0;
    	delay(1);
    }

    // Controller status transmission
    if((controllerStatusTimer > 1500 || PS3Controller.StatusChanged) && TransmissionEnable)
    {
        PS3Controller.StatusChanged = false;

        uint8_t* controllerStatus = PS3Controller.getControllerStatus();
        twai_message_t statusFrame = { 0 };
        statusFrame.identifier = 0x11;
        statusFrame.extd = 1;
        statusFrame.data_length_code = 4;
        statusFrame.data[0] = controllerStatus[0];
        statusFrame.data[1] = controllerStatus[1];
        statusFrame.data[2] = controllerStatus[2];
        statusFrame.data[3] = controllerStatus[3];  

        twai_transmit(&statusFrame, 10);
    	controllerStatusTimer = 0;
    	delay(1);
    }

    // CAN RX messages
    twai_message_t RX_Frame;
    if (twai_receive(&RX_Frame, pdMS_TO_TICKS(1)) == ESP_OK)
    {
        //RemoteController Settings message
        if (RX_Frame.identifier == 0x19)
        {
            if(RX_Frame.data[0] & 0x01)
            {
                Serial.println("Global Transmission enable");
                TransmissionEnable = true;
            }

            else if(!(RX_Frame.data[0] & 0x01))
            {
                Serial.println("Global Transmission disabled");
                TransmissionEnable = false;
            }
        }   
    }

    checkTwaiStatus();

}

// ################## Utilities functions ##################

void sendControllerData(ControllerData controllerData)
{
    twai_message_t dataFrame = { 0 };
    dataFrame.identifier = 0x10;
    dataFrame.extd = 1;
    dataFrame.data_length_code = 4;
    dataFrame.data[0] = controllerData.Throttle;
    dataFrame.data[1] = controllerData.Brake;
    dataFrame.data[2] = controllerData.Steering;
    dataFrame.data[3] = controllerData.ParkBrake << 7 |
                        controllerData.BrakeSwitch << 6 |
                        controllerData.TurnLight_L << 5 |
                        controllerData.TurnLight_R << 4 |
                        controllerData.WarnLight << 3 |
                        controllerData.Light << 2 |
                        controllerData.Horn << 1 |
                        controllerData.Reverse;

    twai_transmit(&dataFrame, 10);
}   

void heartbeat()
{
    unsigned long timestamp = millis();
    twai_message_t dataFrame = { 0 };
    dataFrame.identifier = 0x18;
    dataFrame.extd = 1;
    dataFrame.data_length_code = 4;
    dataFrame.data[0] = timestamp >> 24;
    dataFrame.data[1] = timestamp >> 16;
    dataFrame.data[2] = timestamp >> 8;
    dataFrame.data[3] = timestamp;

    twai_transmit(&dataFrame, 10);
}


void printError(esp_err_t err) {
    switch (err) {
        case ESP_ERR_INVALID_ARG:
            Serial.println("Invalid Argument");
            break;
        case ESP_ERR_TIMEOUT:
            Serial.println("TX Queue Full (Timeout)");
            break;
        case ESP_FAIL:
            Serial.println("TWAI Controller in Bus-Off or Stopped");
            break;
        case ESP_ERR_INVALID_STATE:
            Serial.println("TWAI Driver Not Installed or Not Running");
            break;
        default:
            Serial.printf("Unknown Error: 0x%X\n", err);
            break;
    }
}

void checkTwaiStatus() {
    twai_status_info_t status;
    twai_get_status_info(&status);

    if(status.tx_error_counter > 10)
      Serial.printf("TX Error Counter: %d", status.tx_error_counter);
    if(status.rx_error_counter > 10)
      Serial.printf("RX Error Counter: %d", status.rx_error_counter);

    if(status.tx_error_counter > 30)
    {
      Serial.printf("[CRITICAL] TX Error - Stopping transmission %d", status.tx_error_counter);
      TransmissionEnable = false;
      twai_stop();
      twai_driver_uninstall();
      delay(100);  // Small delay before reinitializing

      twai_driver_install(&g_config, &t_config, &f_config);
      twai_start();
      Serial.println("TWAI restarted.");
    }
    if(status.rx_error_counter > 30)
      Serial.printf("RX Error Counter: %d", status.rx_error_counter);

    switch (status.state) {
        case TWAI_STATE_STOPPED:
            Serial.println("Stopped");
            break;
        case TWAI_STATE_RUNNING:

            break;
        case TWAI_STATE_BUS_OFF:
            Serial.println("Bus-Off (Critical Error)");
            break;
        default:
            Serial.println("Unknown State");
            break;
    }
}

void checkTwaiAlerts() {
    uint32_t alerts;
    if (twai_read_alerts(&alerts, pdMS_TO_TICKS(10)) == ESP_OK) {
        Serial.print("TWAI Alerts: ");

        if (alerts & TWAI_ALERT_TX_FAILED) Serial.print("[TX Failed] ");
        if (alerts & TWAI_ALERT_TX_SUCCESS) Serial.print("[TX Success] ");
        if (alerts & TWAI_ALERT_RX_QUEUE_FULL) Serial.print("[RX Queue Full] ");
        if (alerts & TWAI_ALERT_BUS_ERROR) Serial.print("[Bus Error] ");
        if (alerts & TWAI_ALERT_BUS_OFF) Serial.print("[Bus-Off] ");
        if (alerts & TWAI_ALERT_RX_FIFO_OVERRUN) Serial.print("[RX FIFO Overrun] ");

        Serial.println();
    }
}