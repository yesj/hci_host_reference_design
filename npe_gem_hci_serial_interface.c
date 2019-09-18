
#include <stdio.h>
#include <stdint.h>
#include <sys/time.h>
#include <libserialport.h>
#include "wf_gem_hci_comms.h"
#include "wf_gem_hci_manager_gymconnect.h"
#include "npe_gem_hci_serial_interface.h"
#include <pthread.h> 


// Thread Defines
static pthread_mutex_t rxMutex;
static pthread_cond_t rxCond;

static struct sp_port *port;

static parse_bytes_t m_got_data_cb;

void npe_serial_interface_list_ports(void) {
    int i;
    struct sp_port **ports;
    
    int error = sp_list_ports(&ports);
    if (error == SP_OK) {
        for (i = 0; ports[i]; i++) 
        {
            printf("Found port: '%s'\n", sp_get_port_name(ports[i]));
        }
        sp_free_port_list(ports);
    } 
    else 
    {
        printf("No serial devices detected\n");
    }
    printf("\n");
}

// A normal C function that is executed as a thread  
// when its name is specified in pthread_create() 
static void* npe_serial_interface_receive_thread(void *vargp) 
{ 
    uint8_t byte_buff[512];
    struct sp_event_set *eventSet;
    int err = sp_new_event_set(&eventSet);
    if(err != SP_OK)
    {
        printf("Unable to create new event set\n");
        return NULL;
    }

    err = sp_add_port_events(eventSet, port, SP_EVENT_RX_READY);
    if(err != SP_OK)
    {
        printf("Unable to add port events.\n");
        return NULL;
    }

    while(1) 
    {
        sp_wait(eventSet, 1);
        int byte_num = sp_nonblocking_read(port,byte_buff,512);
        if(byte_num > 0)
        {
            if(m_got_data_cb)
                m_got_data_cb(byte_buff,byte_num);
        }   
    } 
    return NULL; 
} 

static void npe_serial_interface_connect_to_port(const char* p_port)
{
    printf("Opening port.");
    int err = sp_get_port_by_name(p_port, &port);
    if(err == SP_OK)
    {
        printf("Open Port.\n");
        err = sp_open(port, SP_MODE_READ_WRITE);

        if(err == SP_OK)
        {
            printf("Set Baud Rate.\n");
            sp_set_baudrate(port,115200);
        }
        else
        {
            printf("Unable to open port\n");
        }   
    }
    else
    {
        printf("Error finding serial device.\n");
    }
    fflush(stdout);
}

void npe_serial_interface_signal_response(handle_recieved_message_t handle_recieved_message_cb, void* p_arg, uint32_t size)
{
    //printf("Recieved message Event Flags %d, Class Id %d, Message Id %d, Length %d\n", message->message_event_flag,
    //    message->message_class_id, message->message_id, message->data_length);
    //wf_gem_hci_manager_process_recevied_message(message);
    pthread_mutex_lock(&rxMutex);

    if(handle_recieved_message_cb)
        handle_recieved_message_cb(p_arg, size);

    pthread_cond_signal(&rxCond);
    //printf("Signal Response\n");
    pthread_mutex_unlock(&rxMutex);
}


int npe_serial_interface_wait_for_response(check_if_wait_condition_met_cb_t check_if_wait_condition_met_cb)
{
    uint8_t waitCount = 10; // Wait a maximum 10 times to get the correct response. 
    int timeInMs = 500;
    struct timeval tv;
    struct timespec ts;
    int res = 1;

    gettimeofday(&tv, NULL);
    ts.tv_sec = time(NULL) + timeInMs / 1000;
    ts.tv_nsec = tv.tv_usec * 1000 + 1000 * 1000 * (timeInMs % 1000);
    ts.tv_sec += ts.tv_nsec / (1000 * 1000 * 1000);
    ts.tv_nsec %= (1000 * 1000 * 1000);

   
        // TODO: Time out.
    pthread_mutex_lock(&rxMutex);
    printf("Wait\n");
    // Check if this is the response we are watining for 

    while(waitCount > 0)
    {
        res = pthread_cond_timedwait(&rxCond, &rxMutex, &ts);
        if(check_if_wait_condition_met_cb)
            if(check_if_wait_condition_met_cb(res))
                break;
        printf("Waiting %d", res);
        fflush(stdout);


        // if (res == 0)
        // {
        //     if(receivedMessage.message_class_id == classId && receivedMessage.message_id == mesgId)
        //     {
        //         break;
        //     }
        //     else
        //     {
        //         waitCount--;       
        //     }
            
        //     printf("Got response\n");
        // }
        // else if (res == ETIMEDOUT)
        // {
        //     printf("Timeout");
        //     break;
        // }    
    }
    pthread_mutex_unlock(&rxMutex);
    return res;
}

void npe_serial_interface_send_byte(uint8_t tx_byte)
{
    uint8_t buffer[1];
    buffer[0] = tx_byte;

    int bytesWrittenOrError = sp_blocking_write(port, buffer, 1, 0);
    if(bytesWrittenOrError > 0)
    {
        //printf("Sent byte %x\n", tx_byte);
    }
	else
    {
        printf("Could not send byte Err: %d byte: %x\n", bytesWrittenOrError, tx_byte);
    }
    
}
   
void npe_serial_interface_init(const char* p_port, parse_bytes_t parse_bytes_cb)
{
    pthread_t thread_id; 
    if(parse_bytes_cb)
    {
        m_got_data_cb = parse_bytes_cb;
    } 

    npe_serial_interface_connect_to_port(p_port);
    
    pthread_create(&thread_id, NULL, npe_serial_interface_receive_thread, NULL);
    pthread_mutex_init(&rxMutex, NULL);
    pthread_cond_init (&rxCond, NULL);
    //pthread_join(thread_id, NULL);
}

   


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