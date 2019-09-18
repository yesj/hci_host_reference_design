#include <stdio.h> 
#include <stdlib.h> 
#include <stdint.h>
#include <sys/time.h>
#include <unistd.h>  //Header file for sleep(). man 3 sleep for details.
#include <stdbool.h> 

#include "npe_gem_hci_library_interface.h"

int main() 
{ 
    bool waitForPingResponse = true;
    npe_gem_hci_library_interface_init();

    while(waitForPingResponse) 
    {
        sleep(1);
        printf("Send Ping\n");
        
        int res = npe_hci_library_send_ping();
        if(res == 0)
            waitForPingResponse = false;        
        fflush(stdout); 
    }

    // // Set Bluetooth name. 
    // utf8_data_t* bluetoothName = "GEM Reference Design"; 
    // wf_gem_hci_manager_send_command_bluetooth_config_set_device_name(bluetoothName);
    // waitForResponse(WF_GEM_HCI_MSG_CLASS_BT_CONFIG, WF_GEM_HCI_COMMAND_ID_BT_CONFIG_SET_DEVICE_NAME);

    // wf_gem_hci_manager_gymconnect_set_fe_type(WF_GEM_HCI_GYMCONNECT_FITNESS_EQUIPMENT_TYPE_TREADMILL);
    // waitForResponse(WF_GEM_HCI_MSG_CLASS_GYM_CONNECT, WF_GEM_HCI_COMMAND_ID_GYM_CONNECT_SET_FE_TYPE);
    // fflush(stdout);

    // pthread_join(thread_id, NULL); 
    // printf("After Thread\n"); 
    exit(0); 
}