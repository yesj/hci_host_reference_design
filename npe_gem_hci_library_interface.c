#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "wf_gem_hci_comms.h"
#include "wf_gem_hci_manager.h"
#include "npe_gem_hci_serial_interface.h"

wf_gem_hci_comms_message_t receivedMessage;
wf_gem_hci_comms_message_t responseWaitingFor;

static void npe_gem_hci_library_interface_parse_bytes(uint8_t* byte_buff, int byte_num)
{
    for(int i = 0; i < byte_num;i++)
        wf_gem_hci_comms_process_rx_byte(byte_buff[i]);
}

static void npe_gem_hci_library_copy_received_msg(void* message, uint32_t size)
{
    memcpy(&receivedMessage, message, size);
}

void npe_gem_hci_library_interface_init(void)
{
    // Initialize the serial interface
    npe_serial_interface_list_ports();
    npe_serial_interface_init("COM6", npe_gem_hci_library_interface_parse_bytes);

}

static bool npe_gem_library_check_if_response_received(int response)
{
    if(receivedMessage.message_class_id == responseWaitingFor.message_class_id && responseWaitingFor.message_id)
        return true;
    return false;
}


int npe_hci_library_send_ping(void)
{
    wf_gem_hci_manager_send_command_sytem_ping();

    responseWaitingFor.message_class_id = WF_GEM_HCI_MSG_CLASS_SYSTEM;
    responseWaitingFor.message_id = WF_GEM_HCI_COMMAND_ID_SYSTEM_PING;
    return(npe_serial_interface_wait_for_response(npe_gem_library_check_if_response_received));
}

void wf_gem_hci_comms_on_send_byte(uint8_t tx_byte)
{
    npe_serial_interface_send_byte(tx_byte);
    
}

void wf_gem_hci_comms_on_message_received(wf_gem_hci_comms_message_t* message)
{
    printf("Recieved message Event Flags %d, Class Id %d, Message Id %d, Length %d\n", message->message_event_flag,
        message->message_class_id, message->message_id, message->data_length);
    
    // Send to the HCI lib to process the received message
    wf_gem_hci_manager_process_recevied_message(message);

    // Signal other threads that the message has been processed. 
    npe_serial_interface_signal_response(npe_gem_hci_library_copy_received_msg, (void*)message, sizeof(wf_gem_hci_comms_message_t));

}
