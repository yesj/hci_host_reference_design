#include <stdio.h> 
#include <stdlib.h> 
#include <stdint.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdbool.h> 
#include "wf_gem_hci_manager.h"
#include "npe_error_code.h"
#include "npe_gem_hci_library_interface.h"

int main() 
{ 
    uint32_t err;
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
    utf8_data_t* bluetoothName = "GEM Reference Design"; 
    err = npe_hci_library_send_command_bluetooth_config_set_device_name(bluetoothName);
    assert(err == NPE_GEM_RESPONSE_OK);

    err = npe_hci_library_send_command_bluetooth_info_set_manufacturer_name("North Pole Engineering");
    assert(err == NPE_GEM_RESPONSE_OK);

    // wf_gem_hci_manager_gymconnect_set_fe_type(WF_GEM_HCI_GYMCONNECT_FITNESS_EQUIPMENT_TYPE_TREADMILL);
    // waitForResponse(WF_GEM_HCI_MSG_CLASS_GYM_CONNECT, WF_GEM_HCI_COMMAND_ID_GYM_CONNECT_SET_FE_TYPE);
    // fflush(stdout);

    // pthread_join(thread_id, NULL); 
    // printf("After Thread\n"); 
    exit(0); 
}