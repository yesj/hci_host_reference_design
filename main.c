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
#include "npe_gem_hci_data_manager.h"

typedef struct 
{
    uint16_t workout_time_in_seconds;
    uint16_t speed_centi_kph;
    uint16_t deci_grade;
    uint16_t kcal_total;
    uint16_t rate_kcal_per_hour;
    uint32_t distance_meters;


    /* data */
} treadmill_workout_data_t;

static treadmill_workout_data_t m_workout_data;
static wf_gem_hci_gymconnect_fitness_equipment_state_e m_equipment_state = WF_GEM_HCI_GYMCONNECT_FITNESS_EQUIPMENT_STATE_IDLE;


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
    }
}


int main() 
{ 
    uint32_t err;
    bool run = true;

    bool waitForPingResponse = true;
    err = npe_gem_hci_library_interface_init();
    assert(err == NPE_GEM_RESPONSE_OK);

    // Keep sending Ping message until we get a response. 
    while(waitForPingResponse) 
    {
        sleep(1);
        printf("Send Ping\n");
        
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
    err = npe_hci_library_send_command_bluetooth_config_set_device_name("GEM Reference Design");
    assert(err == NPE_GEM_RESPONSE_OK);

    err = npe_hci_library_send_command_bluetooth_info_set_manufacturer_name("North Pole Engineering");
    assert(err == NPE_GEM_RESPONSE_OK);

    err = npe_hci_library_send_command_bluetooth_info_set_model_number("1");
    assert(err == NPE_GEM_RESPONSE_OK);

    err = npe_hci_library_send_command_bluetooth_info_set_serial_number("1234");
    assert(err == NPE_GEM_RESPONSE_OK);

    err = npe_hci_library_send_command_bluetooth_info_set_hardware_rev("1");
    assert(err == NPE_GEM_RESPONSE_OK);

    err = npe_hci_library_send_command_bluetooth_info_set_firmware_rev("2");
    assert(err == NPE_GEM_RESPONSE_OK);

    err = npe_hci_library_send_command_bluetooth_info_set_battery_included(WF_GEM_HCI_BLUETOOTH_BATTERY_SERVICE_NOT_INCLUDE);
    assert(err == NPE_GEM_RESPONSE_OK);

    err = npe_hci_library_send_command_gymconnect_set_supported_equipment_control_features(0);
    assert(err == NPE_GEM_RESPONSE_OK);

    err = npe_hci_library_send_command_ant_config_set_hardware_version(1);
    assert(err == NPE_GEM_RESPONSE_OK);

    err = npe_hci_library_send_command_ant_config_set_model_number(15000);
    assert(err == NPE_GEM_RESPONSE_OK);

    err = npe_hci_library_send_command_ant_config_set_software_version(1, 1);
    assert(err == NPE_GEM_RESPONSE_OK);

    err = npe_hci_library_send_command_ant_config_set_serial_number(1234);
    assert(err == NPE_GEM_RESPONSE_OK);

    err = npe_hci_library_send_command_gymconnect_set_fe_type(WF_GEM_HCI_GYMCONNECT_FITNESS_EQUIPMENT_TYPE_TREADMILL);
    assert(err == NPE_GEM_RESPONSE_OK);

    standard_response_t error_code;
    err = npe_hci_library_send_command_gymconnect_set_fe_state(WF_GEM_HCI_GYMCONNECT_FITNESS_EQUIPMENT_STATE_IDLE, &error_code);
    assert(err == NPE_GEM_RESPONSE_OK);

    
    err = npe_hci_library_send_command_bluetooth_control_start_advertising(&error_code);
    assert(err == NPE_GEM_RESPONSE_OK);

    // Start sending data @ 1Hz to the GEM
    err = npe_gem_hci_data_manager_init(send_data_to_gem);
    assert(err == NPE_GEM_RESPONSE_OK);

    send_data_to_gem();

    print_help();
    fflush(stdout);

   

    while(run)
    {
        uint8_t input_char;
        //char input_buffer[1024];
        //fgets(input_buffer, sizeof(input_buffer), stdin);
        //sscanf(input_buffer, "%c", &input_char);
        input_char = getchar();
        switch(input_char)
        {
            case 'a':
            case 'A':
            {
                // Start advertising
                err = npe_hci_library_send_command_bluetooth_control_start_advertising(&error_code);
                assert(err == NPE_GEM_RESPONSE_OK);
                printf("Advertising Response Error Code: %s\n", NPE_GEM_HCI_LIB_GET_BT_CONTRL_ADV_START_ERROR_CODE_STR(error_code.error_code));
                break;            
            }
            case 's':
            case 'S':
            {
                // Stop advertising
                err = npe_hci_library_send_command_bluetooth_control_stop_advertising(&error_code);
                assert(err == NPE_GEM_RESPONSE_OK);
                printf("Advertising Response Error Code: %s\n", NPE_GEM_HCI_LIB_GET_BT_CONTRL_ADV_STOP_ERROR_CODE_STR(error_code.error_code));
                break;            
            }
            case 'i':
            case 'I':
            {
                // Go to IDLE state
                err = npe_hci_library_send_command_gymconnect_set_fe_state(WF_GEM_HCI_GYMCONNECT_FITNESS_EQUIPMENT_STATE_IDLE, &error_code);
                assert(err == NPE_GEM_RESPONSE_OK);
                m_equipment_state = WF_GEM_HCI_GYMCONNECT_FITNESS_EQUIPMENT_STATE_IDLE;
                break;
            }
            case 'u':
            case 'U':
            {
                // Go to IN-USE state
                err = npe_hci_library_send_command_gymconnect_set_fe_state(WF_GEM_HCI_GYMCONNECT_FITNESS_EQUIPMENT_STATE_IN_USE, &error_code);
                assert(err == NPE_GEM_RESPONSE_OK);
                m_equipment_state = WF_GEM_HCI_GYMCONNECT_FITNESS_EQUIPMENT_STATE_IN_USE;
                break;

            }
            case 'p':
            case 'P':
            {
                // Go to PAUSE state
                err = npe_hci_library_send_command_gymconnect_set_fe_state(WF_GEM_HCI_GYMCONNECT_FITNESS_EQUIPMENT_STATE_PAUSED, &error_code);
                assert(err == NPE_GEM_RESPONSE_OK);
                m_equipment_state = WF_GEM_HCI_GYMCONNECT_FITNESS_EQUIPMENT_STATE_PAUSED;
                break;
            }
            case 'f':
            case 'F':
            {
                // Go to FINISH state
                err = npe_hci_library_send_command_gymconnect_set_fe_state(WF_GEM_HCI_GYMCONNECT_FITNESS_EQUIPMENT_STATE_FINISHED, &error_code);
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