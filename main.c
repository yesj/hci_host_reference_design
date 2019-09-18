#include <stdio.h> 
#include <stdlib.h> 
#include <stdint.h>
#include <sys/time.h>
#include <unistd.h>  //Header file for sleep(). man 3 sleep for details. 
#include <pthread.h> 
#include <libserialport.h>
#include "wf_gem_hci_manager.h"
#include "wf_gem_hci_manager_gymconnect.h"

// Threads
pthread_mutex_t rxMutex;
pthread_cond_t rxCond;
wf_gem_hci_comms_message_t receivedMessage;

const char* desired_port = "COM6";

struct sp_port *port;

void list_ports() {
  int i;
  struct sp_port **ports;

  int error = sp_list_ports(&ports);
  if (error == SP_OK) {
    for (i = 0; ports[i]; i++) {
      printf("Found port: '%s'\n", sp_get_port_name(ports[i]));
    }
    sp_free_port_list(ports);
  } else {
    printf("No serial devices detected\n");
  }
  printf("\n");
}

void parse_serial(uint8_t *byte_buff, int byte_num) {
  for(int i = 0; i < byte_num;i++){
    wf_gem_hci_comms_process_rx_byte(byte_buff[i]);
  }
  printf("\n");
}

// "Callbacks!"
void wf_gem_hci_comms_on_send_byte(uint8_t tx_byte)
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

void wf_gem_hci_comms_on_message_received(wf_gem_hci_comms_message_t* message)
{
    printf("Recieved message Event Flags %d, Class Id %d, Message Id %d, Length %d\n", message->message_event_flag,
        message->message_class_id, message->message_id, message->data_length);
    wf_gem_hci_manager_process_recevied_message(message);
    pthread_mutex_lock(&rxMutex);
    memcpy(&receivedMessage, message, sizeof(wf_gem_hci_comms_message_t));
    pthread_cond_signal(&rxCond);
    printf("Signal Response\n");
    pthread_mutex_unlock(&rxMutex);
}



// A normal C function that is executed as a thread  
// when its name is specified in pthread_create() 
void *receiveThread(void *vargp) 
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
            parse_serial(byte_buff,byte_num);
        }   
    } 
    return NULL; 
} 

static void connectToPort(void)
{
    printf("Opening port.");
    int err = sp_get_port_by_name("COM6", &port);
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

}




uint8_t waitForResponse(uint8_t classId, uint8_t mesgId)
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
        if (res == 0)
        {
            if(receivedMessage.message_class_id == classId && receivedMessage.message_id == mesgId)
            {
                break;
            }
            else
            {
                waitCount--;       
            }
            
            printf("Got response\n");
        }
        else if (res == ETIMEDOUT)
        {
            printf("Timeout");
            break;
        }    
    }
    pthread_mutex_unlock(&rxMutex);
    return res;
}
   
int main() 
{ 
    bool waitForPingResponse = true;
    pthread_t thread_id; 
    list_ports();
    connectToPort();
  
    printf("Before Thread\n"); 
    pthread_create(&thread_id, NULL, receiveThread, NULL);
    pthread_mutex_init(&rxMutex, NULL);
    pthread_cond_init (&rxCond, NULL);
    
    while(waitForPingResponse) 
    {
        sleep(1);
        printf("Send Ping\n");
        wf_gem_hci_manager_send_command_sytem_ping(); 
        int res = waitForResponse(WF_GEM_HCI_MSG_CLASS_SYSTEM, WF_GEM_HCI_COMMAND_ID_SYSTEM_PING);

        if(res == 0)
            waitForPingResponse = false; 
        //if(waitForResponse(0,0) != ETIMEDOUT)
        //    break;

        



        fflush(stdout); 
    }

    // Set Bluetooth name. 
    utf8_data_t* bluetoothName = "GEM Reference Design"; 
    wf_gem_hci_manager_send_command_bluetooth_config_set_device_name(bluetoothName);
    waitForResponse(WF_GEM_HCI_MSG_CLASS_BT_CONFIG, WF_GEM_HCI_COMMAND_ID_BT_CONFIG_SET_DEVICE_NAME);

    wf_gem_hci_manager_gymconnect_set_fe_type(WF_GEM_HCI_GYMCONNECT_FITNESS_EQUIPMENT_TYPE_TREADMILL);
    waitForResponse(WF_GEM_HCI_MSG_CLASS_GYM_CONNECT, WF_GEM_HCI_COMMAND_ID_GYM_CONNECT_SET_FE_TYPE);
    fflush(stdout);

    pthread_join(thread_id, NULL); 
    printf("After Thread\n"); 
    exit(0); 
}