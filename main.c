//  Copyright (c) 2017-2019 by North Pole Engineering, Inc.  All rights reserved.
//
//  Printed in the United States of America.  Except as permitted under the United States
//  Copyright Act of 1976, no part of this software may be reproduced or distributed in
//  any form or by any means, without the prior written permission of North Pole
//  Engineering, Inc., unless such copying is expressly permitted by federal copyright law.
//
//  Address copying inquires to:
//  North Pole Engineering, Inc.
//  npe@npe-inc.com
//  221 North First St. Ste. 310
//  Minneapolis, Minnesota 55401
//
//  Information contained in this software has been created or obtained by North Pole Engineering,
//  Inc. from sources believed to be reliable.  However, North Pole Engineering, Inc. does not
//  guarantee the accuracy or completeness of the information published herein nor shall
//  North Pole Engineering, Inc. be liable for any errors, omissions, or damages arising
//  from the use of this software.

#include <stdio.h> 
#include <stdlib.h> 
#include <stdint.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdbool.h> 
#include "wf_gem_hci_manager.h"
#include "npe_error_code.h"
#include "npe_gem_hci_library_interface.h"
#include "npe_gem_hci_library_response_strings.h"


/** @brief Treadmill data struct.  */
typedef struct 
{
    uint16_t workout_time_in_seconds;
    uint16_t speed_centi_kph;
    uint16_t deci_grade;
    uint16_t kcal_total;
    uint16_t rate_kcal_per_hour;
    uint32_t distance_meters;
} treadmill_workout_data_t;

/** @brief Local variables.  */
static treadmill_workout_data_t m_workout_data;
static wf_gem_hci_gymconnect_fitness_equipment_state_e m_equipment_state = WF_GEM_HCI_GYMCONNECT_FITNESS_EQUIPMENT_STATE_IDLE;

/** @brief Prints instructions to screen in CLI UI.
 */
void print_help(void)
{
    printf("\n\n******* NPE GEM Host Reference Design ********** \n\n\
'h' - help (this menu)\n\
'a' - start advertising\n\
's' - stop advertising\n\
'i' - goto IDLE\n\
'u' - goto IN-USE\n\
'p' - goto PAUSED\n\
'f' - goto FINISHED\n\
'+' - increase grade\n\
'-' - decrease grade\n\
'q' - quit\n\
\n************************************************* \n");
}

/** @brief Updates simulation data and sends to GEM. This function can 
 * be called in any context, e.g. 1s timer thread and main. All tx call
 * are marshalled to the tx thread. 
 *
 */
void send_data_to_gem(void)
{
    if(m_equipment_state == WF_GEM_HCI_GYMCONNECT_FITNESS_EQUIPMENT_STATE_IN_USE) {
        m_workout_data.speed_centi_kph = 2400; // 24 km/hour
        m_workout_data.distance_meters += 7; // 7 m/s
        wf_gem_hci_manager_gymconnect_set_workout_data_elapsed_workout_time(m_workout_data.workout_time_in_seconds++);
        wf_gem_hci_manager_gymconnect_set_workout_data_speed(m_workout_data.speed_centi_kph);

        wf_gem_hci_manager_gymconnect_set_workout_data_grade(m_workout_data.deci_grade);
        wf_gem_hci_manager_gymconnect_set_workout_data_cumulative_horizontal_distance(m_workout_data.distance_meters);
        wf_gem_hci_manager_gymconnect_set_workout_data_cumulative_energy(m_workout_data.kcal_total);
        wf_gem_hci_manager_gymconnect_set_workout_data_energy_rate(m_workout_data.rate_kcal_per_hour);
        
        standard_response_t response;
        uint32_t res = npe_hci_library_send_command_gymconnect_perform_workout_data_update(&response);
        assert(res == NPE_GEM_RESPONSE_OK);
        NPE_GEM_HCI_LIB_PRINT_IF_ERROR(response.error_code, NPE_GEM_HCI_LIB_GET_GYMCONNECT_WORKOUT_DATA_UPDATE_ERROR_CODE_STR(response.error_code));

    }
}


int main(int argc, char *argv[]) 
{ 
    char* p_port = "COM6";
    if(argc > 1)
        p_port = argv[1];

    standard_response_t response_code;
    uint32_t err;
    bool run = true;

    bool waitForPingResponse = true;
    err = npe_gem_hci_library_interface_init(p_port, send_data_to_gem);

    switch(err)
    {
        case NPE_GEM_RESPONSE_SERIAL_NO_COMPORT:
        {
            printf("Comport %s does not exist.", p_port);
            exit(1);
            break;
        }
        case NPE_GEM_RESPONSE_SERIAL_OPEN_FAIL:
        {
            printf("Unable to open port %s.", p_port);
            exit(1);
            break;
        }
        case NPE_GEM_RESPONSE_SERIAL_CONFIG_FAIL:
        {
            printf("Port %s found and opened but unable to configure.", p_port);
            exit(1);
            break;
        }
        default:
        {
            assert(err == NPE_GEM_RESPONSE_OK);
            break;
        }
    }
    

    // Keep sending Ping message until we get a response. 
    while(waitForPingResponse) 
    {
        sleep(1);
       
        err = npe_hci_library_send_ping();

        if(err == NPE_GEM_RESPONSE_OK)
            waitForPingResponse = false;  
        else if(err == NPE_GEM_RESPONSE_RETRIES_EXHAUSTED)
            printf("Retries waiting for response exhausted\n");
        else if(err == NPE_GEM_RESPONSE_TIMEOUT_OUT)
            printf("Wait for response timed out");      
        fflush(stdout); 
    }

    // Set Bluetooth name. 
    err = npe_hci_library_send_command_bluetooth_config_set_device_name("GEM Reference Design", &response_code);
    assert(err == NPE_GEM_RESPONSE_OK);
    NPE_GEM_HCI_LIB_PRINT_IF_ERROR(response_code.error_code, NPE_GEM_HCI_LIB_GET_BT_CONFIG_SET_DEVICE_NAME_ERROR_CODE_STR(response_code.error_code));
            
    err = npe_hci_library_send_command_bluetooth_info_set_manufacturer_name("North Pole Engineering", &response_code);
    assert(err == NPE_GEM_RESPONSE_OK);
    NPE_GEM_HCI_LIB_PRINT_IF_ERROR(response_code.error_code, NPE_GEM_HCI_LIB_GET_BT_INFO_SET_MANUFACTURER_NAME_ERROR_CODE_STR(response_code.error_code));

    err = npe_hci_library_send_command_bluetooth_info_set_model_number("1", &response_code);
    assert(err == NPE_GEM_RESPONSE_OK);
    NPE_GEM_HCI_LIB_PRINT_IF_ERROR(response_code.error_code, NPE_GEM_HCI_LIB_GET_BT_INFO_SET_MODEL_NUMBER_ERROR_CODE_STR(response_code.error_code));

    err = npe_hci_library_send_command_bluetooth_info_set_serial_number("1234", &response_code);
    assert(err == NPE_GEM_RESPONSE_OK);
    NPE_GEM_HCI_LIB_PRINT_IF_ERROR(response_code.error_code, NPE_GEM_HCI_LIB_GET_BT_INFO_SET_SERIAL_NUMBER_ERROR_CODE_STR(response_code.error_code));

    err = npe_hci_library_send_command_bluetooth_info_set_hardware_rev("1", &response_code);
    assert(err == NPE_GEM_RESPONSE_OK);
    NPE_GEM_HCI_LIB_PRINT_IF_ERROR(response_code.error_code, NPE_GEM_HCI_LIB_GET_BT_INFO_SET_HARDWARE_REVISION_ERROR_CODE_STR(response_code.error_code));

    err = npe_hci_library_send_command_bluetooth_info_set_firmware_rev("2", &response_code);
    assert(err == NPE_GEM_RESPONSE_OK);
    NPE_GEM_HCI_LIB_PRINT_IF_ERROR(response_code.error_code, NPE_GEM_HCI_LIB_GET_BT_INFO_SET_FIRMWARE_REVISION_ERROR_CODE_STR(response_code.error_code));

    err = npe_hci_library_send_command_bluetooth_info_set_battery_included(WF_GEM_HCI_BLUETOOTH_BATTERY_SERVICE_NOT_INCLUDE, &response_code);
    assert(err == NPE_GEM_RESPONSE_OK);
    NPE_GEM_HCI_LIB_PRINT_IF_ERROR(response_code.error_code, NPE_GEM_HCI_LIB_GET_BT_INFO_SET_BATTERY_SERVICE_INCLUDED_ERROR_CODE_STR(response_code.error_code));

    err = npe_hci_library_send_command_gymconnect_set_supported_equipment_control_features(0, &response_code);
    assert(err == NPE_GEM_RESPONSE_OK);
    NPE_GEM_HCI_LIB_PRINT_IF_ERROR(response_code.error_code, NPE_GEM_HCI_LIB_GET_GYMCONNECT_SET_SUPPORTED_EQUIPMENT_CONTROL_ERROR_CODE_STR(response_code.error_code));

    err = npe_hci_library_send_command_ant_config_set_hardware_version(1, &response_code);
    assert(err == NPE_GEM_RESPONSE_OK);
    NPE_GEM_HCI_LIB_PRINT_IF_ERROR(response_code.error_code, NPE_GEM_HCI_LIB_GET_ANT_CONFIG_SET_HARDWARE_VERSION_ERROR_CODE_STR(response_code.error_code));

    err = npe_hci_library_send_command_ant_config_set_model_number(15000, &response_code);
    assert(err == NPE_GEM_RESPONSE_OK);
    NPE_GEM_HCI_LIB_PRINT_IF_ERROR(response_code.error_code, NPE_GEM_HCI_LIB_GET_ANT_CONFIG_SET_MODEL_NUMBER_ERROR_CODE_STR(response_code.error_code));

    err = npe_hci_library_send_command_ant_config_set_software_version(1, 1, &response_code);
    assert(err == NPE_GEM_RESPONSE_OK);
    NPE_GEM_HCI_LIB_PRINT_IF_ERROR(response_code.error_code, NPE_GEM_HCI_LIB_GET_ANT_CONFIG_SET_SOFTWARE_VERSION_ERROR_CODE_STR(response_code.error_code));

    err = npe_hci_library_send_command_ant_config_set_serial_number(1234, &response_code);
    assert(err == NPE_GEM_RESPONSE_OK);
    NPE_GEM_HCI_LIB_PRINT_IF_ERROR(response_code.error_code, NPE_GEM_HCI_LIB_GET_ANT_CONFIG_SET_SERIAL_NUMBER_ERROR_CODE_STR(response_code.error_code));

    err = npe_hci_library_send_command_gymconnect_set_fe_type(WF_GEM_HCI_GYMCONNECT_FITNESS_EQUIPMENT_TYPE_TREADMILL, &response_code);
    assert(err == NPE_GEM_RESPONSE_OK);
    NPE_GEM_HCI_LIB_PRINT_IF_ERROR(response_code.error_code, NPE_GEM_HCI_LIB_GET_GYMCONNECT_SET_FITNESS_EQUIPMENT_TYPE_ERROR_CODE_STR(response_code.error_code));

    err = npe_hci_library_send_command_gymconnect_set_fe_state(WF_GEM_HCI_GYMCONNECT_FITNESS_EQUIPMENT_STATE_IDLE, &response_code);
    assert(err == NPE_GEM_RESPONSE_OK);
    NPE_GEM_HCI_LIB_PRINT_IF_ERROR(response_code.error_code, NPE_GEM_HCI_LIB_GET_GYMCONNECT_SET_FITNESS_EQUIPMENT_STATE_ERROR_CODE_STR(response_code.error_code));

    err = npe_hci_library_send_command_bluetooth_control_start_advertising(&response_code);
    assert(err == NPE_GEM_RESPONSE_OK);
    NPE_GEM_HCI_LIB_PRINT_IF_ERROR(response_code.error_code, NPE_GEM_HCI_LIB_GET_BT_CONTRL_ADV_START_ERROR_CODE_STR(response_code.error_code));

    // Send initial data
    send_data_to_gem();

    print_help();
    fflush(stdout);

    while(run)
    {
        uint8_t input_char;

        input_char = getchar();
        switch(input_char)
        {
            case 'a':
            case 'A':
            {
                // Start advertising
                err = npe_hci_library_send_command_bluetooth_control_start_advertising(&response_code);
                assert(err == NPE_GEM_RESPONSE_OK);
                NPE_GEM_HCI_LIB_PRINT_IF_ERROR(response_code.error_code, NPE_GEM_HCI_LIB_GET_BT_CONTRL_ADV_START_ERROR_CODE_STR(response_code.error_code));
                break;            
            }
            case 's':
            case 'S':
            {
                // Stop advertising
                err = npe_hci_library_send_command_bluetooth_control_stop_advertising(&response_code);
                assert(err == NPE_GEM_RESPONSE_OK);
                NPE_GEM_HCI_LIB_PRINT_IF_ERROR(response_code.error_code, NPE_GEM_HCI_LIB_GET_BT_CONTRL_ADV_STOP_ERROR_CODE_STR(response_code.error_code));
                break;            
            }
            case 'i':
            case 'I':
            {
                // Go to IDLE state
                err = npe_hci_library_send_command_gymconnect_set_fe_state(WF_GEM_HCI_GYMCONNECT_FITNESS_EQUIPMENT_STATE_IDLE, &response_code);
                assert(err == NPE_GEM_RESPONSE_OK);
                m_equipment_state = WF_GEM_HCI_GYMCONNECT_FITNESS_EQUIPMENT_STATE_IDLE;
                break;
            }
            case 'u':
            case 'U':
            {
                // Go to IN-USE state
                err = npe_hci_library_send_command_gymconnect_set_fe_state(WF_GEM_HCI_GYMCONNECT_FITNESS_EQUIPMENT_STATE_IN_USE, &response_code);
                assert(err == NPE_GEM_RESPONSE_OK);
                m_equipment_state = WF_GEM_HCI_GYMCONNECT_FITNESS_EQUIPMENT_STATE_IN_USE;
                break;

            }
            case 'p':
            case 'P':
            {
                // Go to PAUSE state
                err = npe_hci_library_send_command_gymconnect_set_fe_state(WF_GEM_HCI_GYMCONNECT_FITNESS_EQUIPMENT_STATE_PAUSED, &response_code);
                assert(err == NPE_GEM_RESPONSE_OK);
                m_equipment_state = WF_GEM_HCI_GYMCONNECT_FITNESS_EQUIPMENT_STATE_PAUSED;
                break;
            }
            case 'f':
            case 'F':
            {
                // Go to FINISH state
                err = npe_hci_library_send_command_gymconnect_set_fe_state(WF_GEM_HCI_GYMCONNECT_FITNESS_EQUIPMENT_STATE_FINISHED, &response_code);
                assert(err == NPE_GEM_RESPONSE_OK);
                m_equipment_state = WF_GEM_HCI_GYMCONNECT_FITNESS_EQUIPMENT_STATE_FINISHED;
                break;
            }
            case 'q':
            case 'Q':
            {
                run = false;     
                break;
            }
            case '+':
            {
                // Increase grade      
                m_workout_data.deci_grade += 50;

                if(m_workout_data.deci_grade > 1000)
                    m_workout_data.deci_grade = 1000;

                break;
            }
            case '-':
            {
                // Decrease grade
                m_workout_data.deci_grade -= 50;
                break;
            }
            case 'h':
            case 'H':
            {
                print_help();
                break;
            }
            default:
            {           
                break;
            }
        }
        fflush(stdout);
    }


    


    // wf_gem_hci_manager_gymconnect_set_fe_type(WF_GEM_HCI_GYdMCONNECT_FITNESS_EQUIPMENT_TYPE_TREADMILL);
    // waitForResponse(WF_GEM_HCI_MSG_CLASS_GYM_CONNECT, WF_GEM_HCI_COMMAND_ID_GYM_CONNECT_SET_FE_TYPE);
    // fflush(stdout);

    // pthread_join(thread_id, NULL); 
    // printf("After Thread\n"); 
    exit(0); 
}