#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "npe_error_code.h"

#include "wf_gem_hci_comms.h"
#include "wf_gem_hci_manager.h"
#include "wf_gem_hci_manager_gymconnect.h"
#include "npe_gem_hci_serial_interface.h"
#include "npe_gem_hci_library_interface.h"



typedef union 
{
    uint8_t message_class_id;
    uint8_t message_id;
    standard_response_t response;
} npe_hci_response_t;



static npe_hci_response_t m_last_response;
static wf_gem_hci_comms_message_t receivedMessage;
static wf_gem_hci_comms_message_t responseWaitingFor;

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
    if(receivedMessage.message_class_id == responseWaitingFor.message_class_id && responseWaitingFor.message_id)
    {
        return true;
    }
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

uint32_t npe_hci_library_send_command_bluetooth_control_start_advertising(standard_response_t* p_advertising_start_response)
{
    uint32_t res;
    wf_gem_hci_manager_send_command_bluetooth_control_start_advertising();

    responseWaitingFor.message_class_id = WF_GEM_HCI_MSG_CLASS_BT_CONTORL;
    responseWaitingFor.message_id = WF_GEM_HCI_COMMAND_ID_BT_CONTROL_START_ADV;
    res = npe_serial_interface_wait_for_response(npe_gem_library_check_if_response_received);

    assert(receivedMessage.message_class_id == WF_GEM_HCI_MSG_CLASS_BT_CONTORL);
    assert(receivedMessage.message_id == WF_GEM_HCI_COMMAND_ID_BT_CONTROL_START_ADV);

    p_advertising_start_response->error_code = m_last_response.response.error_code;

    return(res);
}

uint32_t npe_hci_library_send_command_bluetooth_control_stop_advertising(standard_response_t* p_advertising_stop_response)
{
    uint32_t res;
    wf_gem_hci_manager_send_command_bluetooth_control_stop_advertising();

    responseWaitingFor.message_class_id = WF_GEM_HCI_MSG_CLASS_BT_CONTORL;
    responseWaitingFor.message_id = WF_GEM_HCI_COMMAND_ID_BT_CONTROL_STOP_ADV;
    res = npe_serial_interface_wait_for_response(npe_gem_library_check_if_response_received);

    assert(receivedMessage.message_class_id == WF_GEM_HCI_MSG_CLASS_BT_CONTORL);
    assert(receivedMessage.message_id == WF_GEM_HCI_COMMAND_ID_BT_CONTROL_STOP_ADV);

    p_advertising_stop_response->error_code = m_last_response.response.error_code;

    return(res);
}

uint32_t npe_hci_library_send_command_gymconnect_set_supported_equipment_control_features(uint32_t equipment_control_field_identifier)
{
    wf_gem_hci_manager_gymconnect_set_supported_equipment_control_features(equipment_control_field_identifier);

    responseWaitingFor.message_class_id = WF_GEM_HCI_MSG_CLASS_GYM_CONNECT;
    responseWaitingFor.message_id = WF_GEM_HCI_COMMAND_ID_GYM_CONNECT_SET_FE_CONTROL_FEATURES;
    return(npe_serial_interface_wait_for_response(npe_gem_library_check_if_response_received));
}

uint32_t npe_hci_library_send_command_ant_config_set_hardware_version(uint8_t hardware_version)
{
    wf_gem_hci_manager_send_command_ant_config_set_hardware_version(hardware_version);

    responseWaitingFor.message_class_id = WF_GEM_HCI_MSG_CLASS_ANT_CONFIG;
    responseWaitingFor.message_id = WF_GEM_HCI_COMMAND_ID_ANT_CONFIG_SET_HW_VER;
    return(npe_serial_interface_wait_for_response(npe_gem_library_check_if_response_received));
}

uint32_t npe_hci_library_send_command_ant_config_set_model_number(uint16_t model_number)
{
    wf_gem_hci_manager_send_command_ant_config_set_model_number(model_number);

    responseWaitingFor.message_class_id = WF_GEM_HCI_MSG_CLASS_ANT_CONFIG;
    responseWaitingFor.message_id = WF_GEM_HCI_COMMAND_ID_ANT_CONFIG_SET_MODEL_NUM;
    return(npe_serial_interface_wait_for_response(npe_gem_library_check_if_response_received));
}

uint32_t npe_hci_library_send_command_ant_config_set_software_version(uint8_t main, uint8_t supplemental)
{
    wf_gem_hci_manager_send_command_ant_config_set_software_version(main, supplemental);

    responseWaitingFor.message_class_id = WF_GEM_HCI_MSG_CLASS_ANT_CONFIG;
    responseWaitingFor.message_id = WF_GEM_HCI_COMMAND_ID_ANT_CONFIG_SET_SW_VER;
    return(npe_serial_interface_wait_for_response(npe_gem_library_check_if_response_received));
}

uint32_t npe_hci_library_send_command_ant_config_set_serial_number(uint32_t serial_number)
{
    wf_gem_hci_manager_send_command_ant_config_set_serial_number(serial_number);

    responseWaitingFor.message_class_id = WF_GEM_HCI_MSG_CLASS_ANT_CONFIG;
    responseWaitingFor.message_id = WF_GEM_HCI_COMMAND_ID_ANT_CONFIG_SET_SERIAL_NUMBER;
    return(npe_serial_interface_wait_for_response(npe_gem_library_check_if_response_received));
}

uint32_t npe_hci_library_send_command_gymconnect_set_fe_type(wf_gem_hci_gymconnect_fitness_equipment_type_e fe_type)
{
    wf_gem_hci_manager_gymconnect_set_fe_type(fe_type);

    responseWaitingFor.message_class_id = WF_GEM_HCI_MSG_CLASS_GYM_CONNECT;
    responseWaitingFor.message_id = WF_GEM_HCI_COMMAND_ID_GYM_CONNECT_SET_FE_TYPE;
    return(npe_serial_interface_wait_for_response(npe_gem_library_check_if_response_received));
}

uint32_t npe_hci_library_send_command_gymconnect_set_fe_state(wf_gem_hci_gymconnect_fitness_equipment_state_e fe_state, standard_response_t* p_fe_state_response)
{
    uint32_t res;
    wf_gem_hci_manager_gymconnect_set_fe_state(fe_state);

    responseWaitingFor.message_class_id = WF_GEM_HCI_MSG_CLASS_GYM_CONNECT;
    responseWaitingFor.message_id = WF_GEM_HCI_COMMAND_ID_GYM_CONNECT_SET_FE_STATE;
    res = npe_serial_interface_wait_for_response(npe_gem_library_check_if_response_received);

    assert(receivedMessage.message_class_id == WF_GEM_HCI_MSG_CLASS_GYM_CONNECT);
    assert(receivedMessage.message_id == WF_GEM_HCI_COMMAND_ID_GYM_CONNECT_SET_FE_STATE);

    p_fe_state_response->error_code = m_last_response.response.error_code;
    
    return(res);
}

uint32_t npe_hci_library_send_command_gymconnect_perform_workout_data_update(standard_response_t* p_update_response)
{
    uint32_t res;
    wf_gem_hci_manager_gymconnect_perform_workout_data_update();

    responseWaitingFor.message_class_id = WF_GEM_HCI_MSG_CLASS_GYM_CONNECT;
    responseWaitingFor.message_id = WF_GEM_HCI_COMMAND_ID_GYM_CONNECT_UPDATE_WORKOUT_DATA;
    res = npe_serial_interface_wait_for_response(npe_gem_library_check_if_response_received);

    assert(receivedMessage.message_class_id == WF_GEM_HCI_MSG_CLASS_GYM_CONNECT);
    assert(receivedMessage.message_id == WF_GEM_HCI_COMMAND_ID_GYM_CONNECT_UPDATE_WORKOUT_DATA);

    p_update_response->error_code = m_last_response.response.error_code;
    
    return(res);
}

void wf_gem_hci_comms_on_send_byte(uint8_t tx_byte)
{
    uint32_t err = npe_serial_interface_send_byte(tx_byte);
    assert(err == NPE_GEM_RESPONSE_OK);

}

void wf_gem_hci_comms_on_message_received(wf_gem_hci_comms_message_t* message)
{
    // Send to the HCI lib to process the received message
    wf_gem_hci_manager_process_recevied_message(message);
    
    // Signal other threads that the message has been processed. 
    npe_serial_interface_signal_response(npe_gem_hci_library_process_received_msg, (void*)message, sizeof(wf_gem_hci_comms_message_t));

}





// ****** Process Command Responses. *************************************************************** //
void wf_gem_hci_manager_on_command_response_bluetooth_control_start_advertising(uint8_t error_code)
{   
    m_last_response.response.error_code = error_code;
}

void wf_gem_hci_manager_on_command_response_bluetooth_control_stop_advertising(uint8_t error_code)
{
    m_last_response.response.error_code = error_code;
}
