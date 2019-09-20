#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "npe_error_code.h"
#include "wf_gem_hci_comms.h"
#include "wf_gem_hci_manager.h"
#include "wf_gem_hci_manager_gymconnect.h"
#include "npe_gem_hci_serial_interface.h"

wf_gem_hci_comms_message_t receivedMessage;
wf_gem_hci_comms_message_t responseWaitingFor;

static void npe_gem_hci_library_interface_parse_bytes(uint8_t* byte_buff, int byte_num)
{
    for(int i = 0; i < byte_num;i++)
        wf_gem_hci_comms_process_rx_byte(byte_buff[i]);
}

static void npe_gem_hci_library_process_received_msg(void* message, uint32_t size)
{
    memcpy(&receivedMessage, message, size);
    
}

uint32_t npe_gem_hci_library_interface_init(void)
{
    // Initialize the serial interface
    npe_serial_interface_list_ports();
    return(npe_serial_interface_init("COM6", npe_gem_hci_library_interface_parse_bytes));

}

static bool npe_gem_library_check_if_response_received(int response)
{
    // Send to the HCI lib to process the received message
    wf_gem_hci_manager_process_recevied_message(&receivedMessage);
    if(receivedMessage.message_class_id == responseWaitingFor.message_class_id && responseWaitingFor.message_id)
        return true;
    return false;
}


uint32_t npe_hci_library_send_ping(void)
{
    wf_gem_hci_manager_send_command_sytem_ping();

    responseWaitingFor.message_class_id = WF_GEM_HCI_MSG_CLASS_SYSTEM;
    responseWaitingFor.message_id = WF_GEM_HCI_COMMAND_ID_SYSTEM_PING;
    return(npe_serial_interface_wait_for_response(npe_gem_library_check_if_response_received));
}
uint32_t npe_hci_library_send_command_bluetooth_config_set_device_name(utf8_data_t* bluetoothName)
{
    wf_gem_hci_manager_send_command_bluetooth_config_set_device_name(bluetoothName);

    responseWaitingFor.message_class_id = WF_GEM_HCI_MSG_CLASS_BT_CONFIG;
    responseWaitingFor.message_id = WF_GEM_HCI_COMMAND_ID_BT_CONFIG_SET_DEVICE_NAME;
    return(npe_serial_interface_wait_for_response(npe_gem_library_check_if_response_received));
}

uint32_t npe_hci_library_send_command_bluetooth_info_set_manufacturer_name(utf8_data_t* manufacturer_name)
{
    wf_gem_hci_manager_send_command_bluetooth_info_set_manufacturer_name(manufacturer_name);

    responseWaitingFor.message_class_id = WF_GEM_HCI_MSG_CLASS_BT_DEVICE_INFO;
    responseWaitingFor.message_id = WF_GEM_HCI_COMMAND_ID_BT_DEVICE_INFO_SET_MANU_NAME;
    return(npe_serial_interface_wait_for_response(npe_gem_library_check_if_response_received));
}



uint32_t npe_hci_library_send_command_bluetooth_info_set_model_number(utf8_data_t* model_number)
{
    wf_gem_hci_manager_send_command_bluetooth_info_set_model_number(model_number);

    responseWaitingFor.message_class_id = WF_GEM_HCI_MSG_CLASS_BT_DEVICE_INFO;
    responseWaitingFor.message_id = WF_GEM_HCI_COMMAND_ID_BT_DEVICE_INFO_SET_MODEL_NUM;
    return(npe_serial_interface_wait_for_response(npe_gem_library_check_if_response_received));
}

uint32_t npe_hci_library_send_command_bluetooth_info_set_serial_number(utf8_data_t* serial_number)
{
    wf_gem_hci_manager_send_command_bluetooth_info_set_serial_number(serial_number);

    responseWaitingFor.message_class_id = WF_GEM_HCI_MSG_CLASS_BT_DEVICE_INFO;
    responseWaitingFor.message_id = WF_GEM_HCI_COMMAND_ID_BT_DEVICE_INFO_SET_SERIAL_NUM;
    return(npe_serial_interface_wait_for_response(npe_gem_library_check_if_response_received));
}

uint32_t npe_hci_library_send_command_bluetooth_info_set_hardware_rev(utf8_data_t* hardware_revision)
{
    wf_gem_hci_manager_send_command_bluetooth_info_set_hardware_rev(hardware_revision);

    responseWaitingFor.message_class_id = WF_GEM_HCI_MSG_CLASS_BT_DEVICE_INFO;
    responseWaitingFor.message_id = WF_GEM_HCI_COMMAND_ID_BT_DEVICE_INFO_SET_HW_REV;
    return(npe_serial_interface_wait_for_response(npe_gem_library_check_if_response_received));
}
uint32_t npe_hci_library_send_command_bluetooth_info_set_firmware_rev(utf8_data_t* firmware_revision)
{
    wf_gem_hci_manager_send_command_bluetooth_info_set_firmware_rev(firmware_revision);

    responseWaitingFor.message_class_id = WF_GEM_HCI_MSG_CLASS_BT_DEVICE_INFO;
    responseWaitingFor.message_id = WF_GEM_HCI_COMMAND_ID_BT_DEVICE_INFO_SET_FW_REV;
    return(npe_serial_interface_wait_for_response(npe_gem_library_check_if_response_received));
}

uint32_t npe_hci_library_send_command_bluetooth_info_set_battery_included(uint8_t battery_included)
{
    wf_gem_hci_manager_send_command_bluetooth_info_set_battery_included(battery_included);

    responseWaitingFor.message_class_id = WF_GEM_HCI_MSG_CLASS_BT_DEVICE_INFO;
    responseWaitingFor.message_id = WF_GEM_HCI_COMMAND_ID_BT_DEVICE_INFO_SET_BATT_SERV_INC;
    return(npe_serial_interface_wait_for_response(npe_gem_library_check_if_response_received));
}

uint32_t npe_hci_library_send_command_gymconnect_set_supported_equipment_control_features(uint32_t equipment_control_field_identifier)
{
    wf_gem_hci_manager_gymconnect_set_supported_equipment_control_features(equipment_control_field_identifier);

    responseWaitingFor.message_class_id = WF_GEM_HCI_MSG_CLASS_GYM_CONNECT;
    responseWaitingFor.message_id = WF_GEM_HCI_COMMAND_ID_GYM_CONNECT_SET_FE_CONTROL_FEATURES;
    return(npe_serial_interface_wait_for_response(npe_gem_library_check_if_response_received));
}

void wf_gem_hci_comms_on_send_byte(uint8_t tx_byte)
{
    uint32_t err = npe_serial_interface_send_byte(tx_byte);
    assert(err == NPE_GEM_RESPONSE_OK);

}

void wf_gem_hci_comms_on_message_received(wf_gem_hci_comms_message_t* message)
{
    printf("Recieved message Event Flags %d, Class Id %d, Message Id %d, Length %d\n", message->message_event_flag,
        message->message_class_id, message->message_id, message->data_length);
    


    // Signal other threads that the message has been processed. 
    npe_serial_interface_signal_response(npe_gem_hci_library_process_received_msg, (void*)message, sizeof(wf_gem_hci_comms_message_t));

}

