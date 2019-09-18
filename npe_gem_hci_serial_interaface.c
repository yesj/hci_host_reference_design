
#include <stdio.h>
#include <stdint.h>
#include "wf_gem_hci_comms.h"
#include "wf_gem_hci_manager_gymconnect.h"

void wf_gem_hci_manager_on_command_response_system_ping(void)
{
    printf("wf_gem_hci_manager_on_command_response_system_ping\n");
}

void wf_gem_hci_manager_on_command_response_system_shutdown(uint8_t error_code)
{
printf("wf_gem_hci_manager_on_command_response_system_shutdown\n");
}



void wf_gem_hci_manager_on_command_response_system_get_gem_module_version_info(wf_gem_hci_system_gem_module_version_info_t *version_info)
{
    printf("wf_gem_hci_manager_on_command_response_system_get_gem_module_version_info\n");
}


void wf_gem_hci_manager_on_command_response_system_reset(uint8_t error_code)
{
    printf("wf_gem_hci_manager_on_command_response_system_reset\n");
}

void wf_gem_hci_manager_on_event_system_powerup(void)
{
    printf("wf_gem_hci_manager_on_event_system_powerup\n");
}


void wf_gem_hci_manager_on_event_system_shutdown(void)
{
    printf("wf_gem_hci_manager_on_event_system_shutdown\n");
}

// +~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+
// 		Bluetooth Control Command Responses, Events
// +~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+
void wf_gem_hci_manager_on_command_response_bluetooth_control_start_advertising(uint8_t error_code)
{   
    printf("wf_gem_hci_manager_on_command_response_bluetooth_control_start_advertising\n");
}
void wf_gem_hci_manager_on_command_response_bluetooth_control_stop_advertising(uint8_t error_code)
{
    printf("wf_gem_hci_manager_on_command_response_bluetooth_control_stop_advertising\n");
}

void wf_gem_hci_manager_on_command_response_bluetooth_control_get_bluetooth_state(wf_gem_hci_bluetooth_state_e bluetooth_state)
{
    printf("wf_gem_hci_manager_on_command_response_bluetooth_control_get_bluetooth_state\n");
}

void wf_gem_hci_manager_on_event_bluetooth_control_advertising_timed_out(void)
{
    printf("wf_gem_hci_manager_on_event_bluetooth_control_advertising_timed_out\n");
}
void wf_gem_hci_manager_on_event_bluetooth_control_connected(void)
{
    printf("wf_gem_hci_manager_on_event_bluetooth_control_connected\n");
}
void wf_gem_hci_manager_on_event_bluetooth_control_disconnected(bool peripheral_solicited, bool central_solicited)
{
    printf("wf_gem_hci_manager_on_event_bluetooth_control_disconnected\n");
}



// +~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+
// Bluetooth Configuration Command Responses
// +~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+
void wf_gem_hci_manager_on_command_response_bluetooth_config_get_device_name(utf8_data_t* device_name)
{
    printf("wf_gem_hci_manager_on_command_response_bluetooth_config_get_device_name\n");
}
void wf_gem_hci_manager_on_command_response_bluetooth_config_set_device_name(uint8_t error_code)
{
    printf("wf_gem_hci_manager_on_command_response_bluetooth_config_set_device_name\n");
}



void wf_gem_hci_manager_on_begin_retry_timer(uint16_t cmd_timeout_ms)
{
    printf("wf_gem_hci_manager_on_begin_retry_timer\n");
}

// cancel/stop the timer started in wf_gem_hci_manager_on_begin_retry_timer()
void wf_gem_hci_manager_on_cancel_retry_timer(void)
{
    printf("wf_gem_hci_manwf_gem_hci_manager_on_cancel_retry_timerager_gymconnect_on_command_send_failure\n");
}


// This callback is called if a command message can not be sent and all retries have failed.
void wf_gem_hci_manager_on_command_send_failure(wf_gem_hci_comms_message_t* message)
{
    printf("wf_gem_hci_manager_on_command_send_failure\n");
}



void wf_gem_hci_manager_gymconnect_on_command_send_failure(wf_gem_hci_comms_message_t *message)
{
    printf("wf_gem_hci_manager_gymconnect_on_command_send_failure\n");
}

void wf_gem_hci_manager_gymconnect_on_command_response_get_fe_type(wf_gem_hci_gymconnect_fitness_equipment_type_e fe_type)
{
    printf("wf_gem_hci_manager_gymconnect_on_command_response_get_fe_type\n");
}
void wf_gem_hci_manager_gymconnect_on_command_response_set_fe_type(uint8_t error_code)
{
    printf("wf_gem_hci_manager_gymconnect_on_command_response_set_fe_type\n");
}

void wf_gem_hci_manager_gymconnect_on_command_response_get_fe_state(wf_gem_hci_gymconnect_fitness_equipment_state_e fe_state)
{
    printf("wf_gem_hci_manager_gymconnect_on_command_response_get_fe_state\n");
}
void wf_gem_hci_manager_gymconnect_on_command_response_set_fe_state(uint8_t error_code)
{
    printf("wf_gem_hci_manager_gymconnect_on_command_response_set_fe_state\n");
}

void wf_gem_hci_manager_gymconnect_on_command_response_get_fe_program_name(utf8_data_t *program_name)
{
    printf("wf_gem_hci_manager_gymconnect_on_command_response_get_fe_program_name\n");
}
void wf_gem_hci_manager_gymconnect_on_command_response_set_fe_program_name(uint8_t error_code)
{
    printf("wf_gem_hci_manager_gymconnect_on_command_response_set_fe_program_name\n");
}

void wf_gem_hci_manager_gymconnect_on_workout_data_update_complete(uint8_t error_code)
{
    printf("wf_gem_hci_manager_gymconnect_on_workout_data_update_complete\n");
}

void wf_gem_hci_manager_gymconnecton_event_heart_rate_value_received(uint16_t heart_rate_value)
{
    printf("wf_gem_hci_manager_gymconnecton_event_heart_rate_value_received\n");
}
void wf_gem_hci_manager_gymconnecton_event_cadence_value_received(uint16_t cadence_value)
{
    printf("wf_gem_hci_manager_gymconnecton_event_cadence_value_received\n");
}