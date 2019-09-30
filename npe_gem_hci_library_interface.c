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

typedef void (*npe_gem_fp)(void); // Generic function pointer


typedef union 
{
    uint8_t message_class_id;
    uint8_t message_id;
    standard_response_t response;
} npe_hci_response_t;

typedef union 
{
    struct {
       utf8_data_t* bluetooth_name;
    } bt_config_set_device_name;
    struct {
        utf8_data_t* manufacturer_name;
    } bt_device_info_set_manufacturer_name;
    struct {
        utf8_data_t* model_number;
    } bt_device_info_set_model_number;
    struct {
        utf8_data_t* serial_number;
    }bt_device_info_set_serial_number;
    struct {
        utf8_data_t* hardware_revision;
    }bt_device_info_set_hardware_revision;
    struct {
        utf8_data_t* firmware_revision;
    }bt_device_info_set_firmware_revision;
    struct {
        uint8_t  battery_included;
    }bt_device_info_set_battery_included; 
    struct {
        uint32_t  equipment_control_field_identifier;
    }gymconnect_set_supported_equipment_control_features; 
    struct {
        uint8_t  hardware_version;
    }ant_config_set_hardware_version;
    struct {
        uint16_t  model_number;
    }ant_config_set_model_number; 
    struct {
        uint8_t  main;
        uint8_t supplemental;
    }ant_config_set_version; 
    struct {
        uint32_t serial_number;
    }ant_config_set_serial_number;
    struct {
        wf_gem_hci_gymconnect_fitness_equipment_type_e fe_type;
    }gymconnect_set_fe_type;
struct {
        wf_gem_hci_gymconnect_fitness_equipment_state_e fe_state;
    }gymconnect_set_fe_state;

} npe_inc_function_args;

typedef struct 
{
    uint8_t message_class_id;
    uint8_t message_id;
    npe_inc_function_args args;
} npe_hci_function_info_type;


static npe_hci_response_t m_last_response;
static wf_gem_hci_comms_message_t receivedMessage;
static npe_hci_function_info_type messageToSend;
static one_second_timeout_t m_timeout_cb;

static void npe_gem_hci_library_interface_parse_bytes(uint8_t* byte_buff, int byte_num)
{
    for(int i = 0; i < byte_num;i++)
        wf_gem_hci_comms_process_rx_byte(byte_buff[i]);
}

      

static void npe_hci_library_send_ant_config_message(uint8_t message_id)
{
    switch(message_id)
    {
        case WF_GEM_HCI_COMMAND_ID_ANT_CONFIG_SET_HW_VER:
        {
            wf_gem_hci_manager_send_command_ant_config_set_hardware_version(messageToSend.args.ant_config_set_hardware_version.hardware_version);
            break;
        }
        case WF_GEM_HCI_COMMAND_ID_ANT_CONFIG_SET_MODEL_NUM:
        {
            wf_gem_hci_manager_send_command_ant_config_set_model_number(messageToSend.args.ant_config_set_model_number.model_number);
            break;
        }
        case WF_GEM_HCI_COMMAND_ID_ANT_CONFIG_SET_SW_VER:
        {
            uint8_t main = messageToSend.args.ant_config_set_version.main;
            uint8_t supplemental = messageToSend.args.ant_config_set_version.supplemental;
            wf_gem_hci_manager_send_command_ant_config_set_software_version(main, supplemental);
            break;
        }
        case WF_GEM_HCI_COMMAND_ID_ANT_CONFIG_SET_SERIAL_NUMBER:
        {
            wf_gem_hci_manager_send_command_ant_config_set_serial_number(messageToSend.args.ant_config_set_serial_number.serial_number);
            break;
        }
        default:
        {
            break;
        }
    }
}

// Called by transmit thread 
static void npe_hci_library_send_gymconnect_message(uint8_t message_id)
{
    switch(message_id)
    {
        case WF_GEM_HCI_COMMAND_ID_GYM_CONNECT_SET_FE_CONTROL_FEATURES:
        {
            wf_gem_hci_manager_gymconnect_set_supported_equipment_control_features(messageToSend.args.gymconnect_set_supported_equipment_control_features.equipment_control_field_identifier);
            break;
        }
        case WF_GEM_HCI_COMMAND_ID_GYM_CONNECT_SET_FE_TYPE:
        {
            wf_gem_hci_manager_gymconnect_set_fe_type(messageToSend.args.gymconnect_set_fe_type.fe_type);
            break;
        }
        case WF_GEM_HCI_COMMAND_ID_GYM_CONNECT_SET_FE_STATE:
        {
            wf_gem_hci_manager_gymconnect_set_fe_state(messageToSend.args.gymconnect_set_fe_state.fe_state);
            break;
        }
        case WF_GEM_HCI_COMMAND_ID_GYM_CONNECT_UPDATE_WORKOUT_DATA:
        {
            wf_gem_hci_manager_gymconnect_perform_workout_data_update();
            break;
        }
        default:
        {
            break;
        }
    }
}

// Called by transmit thread 
static void npe_hci_library_send_bt_control_message(uint8_t message_id)
{
    switch(message_id)
    {
        case WF_GEM_HCI_COMMAND_ID_BT_CONTROL_START_ADV:
        {
            wf_gem_hci_manager_send_command_bluetooth_control_start_advertising();
            break;
        }
        case WF_GEM_HCI_COMMAND_ID_BT_CONTROL_STOP_ADV:
        {
            wf_gem_hci_manager_send_command_bluetooth_control_stop_advertising();
            break;
        }
        default:
        {
            break;
        }
    }
}
static void npe_hci_library_send_bt_device_info_message(uint8_t message_id)
{
    switch(message_id)
    {
        case WF_GEM_HCI_COMMAND_ID_BT_DEVICE_INFO_SET_MANU_NAME:
        {
            wf_gem_hci_manager_send_command_bluetooth_info_set_manufacturer_name(messageToSend.args.bt_device_info_set_manufacturer_name.manufacturer_name);
            break;
        }
        case WF_GEM_HCI_COMMAND_ID_BT_DEVICE_INFO_SET_MODEL_NUM:
        {
            wf_gem_hci_manager_send_command_bluetooth_info_set_model_number(messageToSend.args.bt_device_info_set_model_number.model_number);
            break;
        }
        case WF_GEM_HCI_COMMAND_ID_BT_DEVICE_INFO_SET_SERIAL_NUM:
        {
            wf_gem_hci_manager_send_command_bluetooth_info_set_serial_number(messageToSend.args.bt_device_info_set_serial_number.serial_number);
            break;
        }
        case WF_GEM_HCI_COMMAND_ID_BT_DEVICE_INFO_SET_HW_REV:
        {
            wf_gem_hci_manager_send_command_bluetooth_info_set_hardware_rev(messageToSend.args.bt_device_info_set_hardware_revision.hardware_revision);
            break;
        } 
        case WF_GEM_HCI_COMMAND_ID_BT_DEVICE_INFO_SET_FW_REV:
        {
            wf_gem_hci_manager_send_command_bluetooth_info_set_firmware_rev(messageToSend.args.bt_device_info_set_firmware_revision.firmware_revision);
            break;
        }
        case WF_GEM_HCI_COMMAND_ID_BT_DEVICE_INFO_SET_BATT_SERV_INC:
        {
            wf_gem_hci_manager_send_command_bluetooth_info_set_battery_included(messageToSend.args.bt_device_info_set_battery_included.battery_included);
            break;
        }
    }
}

static void npe_hci_library_send_bt_config_message(uint8_t message_id)
{
    switch(message_id)
    {
        case WF_GEM_HCI_COMMAND_ID_BT_CONFIG_SET_DEVICE_NAME:
        {
            wf_gem_hci_manager_send_command_bluetooth_config_set_device_name(messageToSend.args.bt_config_set_device_name.bluetooth_name);
            break;
        }
    }
}
static void npe_hci_library_send_system_message(uint8_t message_id)
{
    switch(message_id)
    {
        case WF_GEM_HCI_COMMAND_ID_SYSTEM_PING:
        {

            wf_gem_hci_manager_send_command_sytem_ping();
            break;
        }
    }
}




static void npe_hci_library_send_message(void)
{
    switch(messageToSend.message_class_id)
    {
        case WF_GEM_HCI_MSG_CLASS_SYSTEM:
        {
            npe_hci_library_send_system_message(messageToSend.message_id);
            break;
        }
        case WF_GEM_HCI_MSG_CLASS_BT_CONFIG:
        {
            npe_hci_library_send_bt_config_message(messageToSend.message_id);
            break;
        }
        case WF_GEM_HCI_MSG_CLASS_BT_DEVICE_INFO:
        {
            npe_hci_library_send_bt_device_info_message(messageToSend.message_id);
            break;
        }
        case WF_GEM_HCI_MSG_CLASS_BT_CONTORL:
        {
            npe_hci_library_send_bt_control_message(messageToSend.message_id);
            break;
        }
        case WF_GEM_HCI_MSG_CLASS_GYM_CONNECT:
        {
            npe_hci_library_send_gymconnect_message(messageToSend.message_id);

        }
        case WF_GEM_HCI_MSG_CLASS_ANT_CONFIG:
        {
            npe_hci_library_send_ant_config_message(messageToSend.message_id);
        }
        default:
        {
            break;
        }
    }
}

static void npe_gem_hci_library_process_received_msg(void* message, uint32_t size)
{
    memcpy(&receivedMessage, message, size);
}


static void npe_hci_library_timeout(void)
{
    if(m_timeout_cb)
    {
        m_timeout_cb();
    }
}
uint32_t npe_gem_hci_library_interface_init(one_second_timeout_t one_second_timout_cb)
{
    npe_serial_interface_callbacks_t tx_callbacks;
    tx_callbacks.parse_bytes_cb = npe_gem_hci_library_interface_parse_bytes;
    tx_callbacks.transmit_message_cb = npe_hci_library_send_message;
    tx_callbacks.timeout_cb = npe_hci_library_timeout;
    tx_callbacks.retry_timeout_cb = wf_gem_hci_manager_process_command_retry;

    // Initialize the serial interface
    if(one_second_timout_cb)
        m_timeout_cb = one_second_timout_cb;


    npe_serial_interface_list_ports();
    return(npe_serial_interface_init("COM6", &tx_callbacks));

}

static bool npe_gem_library_check_if_response_received(int response)
{
    if(receivedMessage.message_class_id == messageToSend.message_class_id && receivedMessage.message_id == messageToSend.message_id)
    {
        return true;
    }
    return false;
}


uint32_t npe_hci_library_send_ping(void)
{
    bool locked = npe_serial_transmit_lock();
    messageToSend.message_class_id = WF_GEM_HCI_MSG_CLASS_SYSTEM;
    messageToSend.message_id = WF_GEM_HCI_COMMAND_ID_SYSTEM_PING;
    // Start send message then wait for response.
    if(locked) 
        npe_serial_transmit_message_and_unlock();
    else
        wf_gem_hci_manager_send_command_sytem_ping();  
    

    return(npe_serial_interface_wait_for_response(npe_gem_library_check_if_response_received));
}
uint32_t npe_hci_library_send_command_bluetooth_config_set_device_name(utf8_data_t* bluetoothName)
{
    bool locked = npe_serial_transmit_lock(); 
    messageToSend.message_class_id = WF_GEM_HCI_MSG_CLASS_BT_CONFIG;
    messageToSend.message_id = WF_GEM_HCI_COMMAND_ID_BT_CONFIG_SET_DEVICE_NAME;
    messageToSend.args.bt_config_set_device_name.bluetooth_name = bluetoothName;
    
    if(locked)
        npe_serial_transmit_message_and_unlock();
    else
        wf_gem_hci_manager_send_command_bluetooth_config_set_device_name(messageToSend.args.bt_config_set_device_name.bluetooth_name);
 
    return(npe_serial_interface_wait_for_response(npe_gem_library_check_if_response_received));
}

uint32_t npe_hci_library_send_command_bluetooth_info_set_manufacturer_name(utf8_data_t* manufacturer_name)
{
    bool locked = npe_serial_transmit_lock();
    messageToSend.message_class_id = WF_GEM_HCI_MSG_CLASS_BT_DEVICE_INFO;
    messageToSend.message_id = WF_GEM_HCI_COMMAND_ID_BT_DEVICE_INFO_SET_MANU_NAME;
    messageToSend.args.bt_device_info_set_manufacturer_name.manufacturer_name = manufacturer_name;
    
    if(locked)
        npe_serial_transmit_message_and_unlock();
    else
        wf_gem_hci_manager_send_command_bluetooth_info_set_manufacturer_name(messageToSend.args.bt_device_info_set_manufacturer_name.manufacturer_name);
 
    
    return(npe_serial_interface_wait_for_response(npe_gem_library_check_if_response_received));
}



uint32_t npe_hci_library_send_command_bluetooth_info_set_model_number(utf8_data_t* model_number)
{
    bool locked = npe_serial_transmit_lock();
    messageToSend.message_class_id = WF_GEM_HCI_MSG_CLASS_BT_DEVICE_INFO;
    messageToSend.message_id = WF_GEM_HCI_COMMAND_ID_BT_DEVICE_INFO_SET_MODEL_NUM;
    messageToSend.args.bt_device_info_set_model_number.model_number = model_number;
    if(locked)
        npe_serial_transmit_message_and_unlock();
    else
        wf_gem_hci_manager_send_command_bluetooth_info_set_model_number(messageToSend.args.bt_device_info_set_model_number.model_number);
 
    return(npe_serial_interface_wait_for_response(npe_gem_library_check_if_response_received));
}

uint32_t npe_hci_library_send_command_bluetooth_info_set_serial_number(utf8_data_t* serial_number)
{
    bool locked = npe_serial_transmit_lock();
    messageToSend.message_class_id = WF_GEM_HCI_MSG_CLASS_BT_DEVICE_INFO;
    messageToSend.message_id = WF_GEM_HCI_COMMAND_ID_BT_DEVICE_INFO_SET_SERIAL_NUM;
    messageToSend.args.bt_device_info_set_serial_number.serial_number = serial_number;
    if(locked)
        npe_serial_transmit_message_and_unlock();
    else
        wf_gem_hci_manager_send_command_bluetooth_info_set_serial_number(messageToSend.args.bt_device_info_set_serial_number.serial_number);
 
    return(npe_serial_interface_wait_for_response(npe_gem_library_check_if_response_received));
}

uint32_t npe_hci_library_send_command_bluetooth_info_set_hardware_rev(utf8_data_t* hardware_revision)
{
    bool locked = npe_serial_transmit_lock();
    messageToSend.message_class_id = WF_GEM_HCI_MSG_CLASS_BT_DEVICE_INFO;
    messageToSend.message_id = WF_GEM_HCI_COMMAND_ID_BT_DEVICE_INFO_SET_HW_REV;
    messageToSend.args.bt_device_info_set_hardware_revision.hardware_revision = hardware_revision;
    
    if(locked)
        npe_serial_transmit_message_and_unlock();
    else 
        wf_gem_hci_manager_send_command_bluetooth_info_set_hardware_rev(messageToSend.args.bt_device_info_set_hardware_revision.hardware_revision);
 
    return(npe_serial_interface_wait_for_response(npe_gem_library_check_if_response_received));
}
uint32_t npe_hci_library_send_command_bluetooth_info_set_firmware_rev(utf8_data_t* firmware_revision)
{
    bool locked = npe_serial_transmit_lock();
    messageToSend.message_class_id = WF_GEM_HCI_MSG_CLASS_BT_DEVICE_INFO;
    messageToSend.message_id = WF_GEM_HCI_COMMAND_ID_BT_DEVICE_INFO_SET_FW_REV;
    messageToSend.args.bt_device_info_set_firmware_revision.firmware_revision = firmware_revision;
    
    if(locked)
        npe_serial_transmit_message_and_unlock();
    else 
        wf_gem_hci_manager_send_command_bluetooth_info_set_firmware_rev(messageToSend.args.bt_device_info_set_firmware_revision.firmware_revision);
   
    return(npe_serial_interface_wait_for_response(npe_gem_library_check_if_response_received));
}

uint32_t npe_hci_library_send_command_bluetooth_info_set_battery_included(uint8_t battery_included)
{
    if(battery_included != WF_GEM_HCI_BLUETOOTH_BATTERY_SERVICE_NOT_INCLUDE &&
        battery_included != WF_GEM_HCI_BLUETOOTH_BATTERY_SERVICE_INCLUDE)
    {
       return(NPE_GEM_RESPONSE_INVALID_PARAMETER); 
    }

    bool locked = npe_serial_transmit_lock();
    messageToSend.message_class_id = WF_GEM_HCI_MSG_CLASS_BT_DEVICE_INFO;
    messageToSend.message_id = WF_GEM_HCI_COMMAND_ID_BT_DEVICE_INFO_SET_BATT_SERV_INC;
    messageToSend.args.bt_device_info_set_battery_included.battery_included = battery_included;
    
    if(locked)
        npe_serial_transmit_message_and_unlock();
    else
        wf_gem_hci_manager_send_command_bluetooth_info_set_battery_included(messageToSend.args.bt_device_info_set_battery_included.battery_included);
  
    
    return(npe_serial_interface_wait_for_response(npe_gem_library_check_if_response_received));
}

uint32_t npe_hci_library_send_command_bluetooth_control_start_advertising(standard_response_t* p_advertising_start_response)
{
    uint32_t res;
    bool locked = npe_serial_transmit_lock();

    messageToSend.message_class_id = WF_GEM_HCI_MSG_CLASS_BT_CONTORL;
    messageToSend.message_id = WF_GEM_HCI_COMMAND_ID_BT_CONTROL_START_ADV;
    
    if(locked)
        npe_serial_transmit_message_and_unlock();
    else
        wf_gem_hci_manager_send_command_bluetooth_control_start_advertising();
 
    res = npe_serial_interface_wait_for_response(npe_gem_library_check_if_response_received);

    assert(receivedMessage.message_class_id == WF_GEM_HCI_MSG_CLASS_BT_CONTORL);
    assert(receivedMessage.message_id == WF_GEM_HCI_COMMAND_ID_BT_CONTROL_START_ADV);

    p_advertising_start_response->error_code = m_last_response.response.error_code;

    return(res);
}

uint32_t npe_hci_library_send_command_bluetooth_control_stop_advertising(standard_response_t* p_advertising_stop_response)
{
    uint32_t res;
    bool locked = npe_serial_transmit_lock();

    messageToSend.message_class_id = WF_GEM_HCI_MSG_CLASS_BT_CONTORL;
    messageToSend.message_id = WF_GEM_HCI_COMMAND_ID_BT_CONTROL_STOP_ADV;
    
    if(locked)
        npe_serial_transmit_message_and_unlock();
    else
        wf_gem_hci_manager_send_command_bluetooth_control_stop_advertising();

    res = npe_serial_interface_wait_for_response(npe_gem_library_check_if_response_received);

    assert(receivedMessage.message_class_id == WF_GEM_HCI_MSG_CLASS_BT_CONTORL);
    assert(receivedMessage.message_id == WF_GEM_HCI_COMMAND_ID_BT_CONTROL_STOP_ADV);

    p_advertising_stop_response->error_code = m_last_response.response.error_code;

    return(res);
}

uint32_t npe_hci_library_send_command_gymconnect_set_supported_equipment_control_features(uint32_t equipment_control_field_identifier)
{
    bool locked = npe_serial_transmit_lock(); 
    messageToSend.message_class_id = WF_GEM_HCI_MSG_CLASS_GYM_CONNECT;
    messageToSend.message_id = WF_GEM_HCI_COMMAND_ID_GYM_CONNECT_SET_FE_CONTROL_FEATURES;
    messageToSend.args.gymconnect_set_supported_equipment_control_features.equipment_control_field_identifier = equipment_control_field_identifier;
    
    if(locked)
        npe_serial_transmit_message_and_unlock();
    else 
        wf_gem_hci_manager_gymconnect_set_supported_equipment_control_features(messageToSend.args.gymconnect_set_supported_equipment_control_features.equipment_control_field_identifier);
    
    return(npe_serial_interface_wait_for_response(npe_gem_library_check_if_response_received));
}

uint32_t npe_hci_library_send_command_ant_config_set_hardware_version(uint8_t hardware_version)
{
    bool locked = npe_serial_transmit_lock(); 
    messageToSend.message_class_id = WF_GEM_HCI_MSG_CLASS_ANT_CONFIG;
    messageToSend.message_id = WF_GEM_HCI_COMMAND_ID_ANT_CONFIG_SET_HW_VER;
    messageToSend.args.ant_config_set_hardware_version.hardware_version = hardware_version;
    
    if(locked)
        npe_serial_transmit_message_and_unlock();
    else
        wf_gem_hci_manager_send_command_ant_config_set_hardware_version(messageToSend.args.ant_config_set_hardware_version.hardware_version);
     
    return(npe_serial_interface_wait_for_response(npe_gem_library_check_if_response_received));
}

uint32_t npe_hci_library_send_command_ant_config_set_model_number(uint16_t model_number)
{
    bool locked = npe_serial_transmit_lock();
    messageToSend.message_class_id = WF_GEM_HCI_MSG_CLASS_ANT_CONFIG;
    messageToSend.message_id = WF_GEM_HCI_COMMAND_ID_ANT_CONFIG_SET_MODEL_NUM;
    messageToSend.args.ant_config_set_model_number.model_number = model_number;
    if(locked)
        npe_serial_transmit_message_and_unlock();
    else
        wf_gem_hci_manager_send_command_ant_config_set_model_number(messageToSend.args.ant_config_set_model_number.model_number);
  
    return(npe_serial_interface_wait_for_response(npe_gem_library_check_if_response_received));
}

uint32_t npe_hci_library_send_command_ant_config_set_software_version(uint8_t main, uint8_t supplemental)
{
    bool locked = npe_serial_transmit_lock();
 
    messageToSend.message_class_id = WF_GEM_HCI_MSG_CLASS_ANT_CONFIG;
    messageToSend.message_id = WF_GEM_HCI_COMMAND_ID_ANT_CONFIG_SET_SW_VER;
    messageToSend.args.ant_config_set_version.main = main;
    messageToSend.args.ant_config_set_version.supplemental = supplemental;
    
    if(locked)
        npe_serial_transmit_message_and_unlock();
    else
        wf_gem_hci_manager_send_command_ant_config_set_software_version(main, supplemental);
 
    return(npe_serial_interface_wait_for_response(npe_gem_library_check_if_response_received));
}

uint32_t npe_hci_library_send_command_ant_config_set_serial_number(uint32_t serial_number)
{
    bool locked = npe_serial_transmit_lock();

    messageToSend.message_class_id = WF_GEM_HCI_MSG_CLASS_ANT_CONFIG;
    messageToSend.message_id = WF_GEM_HCI_COMMAND_ID_ANT_CONFIG_SET_SERIAL_NUMBER;
    messageToSend.args.ant_config_set_serial_number.serial_number = serial_number;
    if(locked)
        npe_serial_transmit_message_and_unlock();
    else
        wf_gem_hci_manager_send_command_ant_config_set_serial_number(messageToSend.args.ant_config_set_serial_number.serial_number);
          
    return(npe_serial_interface_wait_for_response(npe_gem_library_check_if_response_received));
}

uint32_t npe_hci_library_send_command_gymconnect_set_fe_type(wf_gem_hci_gymconnect_fitness_equipment_type_e fe_type)
{
    bool locked = npe_serial_transmit_lock();

    messageToSend.message_class_id = WF_GEM_HCI_MSG_CLASS_GYM_CONNECT;
    messageToSend.message_id = WF_GEM_HCI_COMMAND_ID_GYM_CONNECT_SET_FE_TYPE;
    messageToSend.args.gymconnect_set_fe_type.fe_type = fe_type;
    
    if(locked)
        npe_serial_transmit_message_and_unlock();
    else
        wf_gem_hci_manager_gymconnect_set_fe_type(messageToSend.args.gymconnect_set_fe_type.fe_type);
            
    return(npe_serial_interface_wait_for_response(npe_gem_library_check_if_response_received));
}

uint32_t npe_hci_library_send_command_gymconnect_set_fe_state(wf_gem_hci_gymconnect_fitness_equipment_state_e fe_state, standard_response_t* p_fe_state_response)
{
    uint32_t res;
    bool locked = npe_serial_transmit_lock();
    messageToSend.message_class_id = WF_GEM_HCI_MSG_CLASS_GYM_CONNECT;
    messageToSend.message_id = WF_GEM_HCI_COMMAND_ID_GYM_CONNECT_SET_FE_STATE;
    messageToSend.args.gymconnect_set_fe_state.fe_state = fe_state;
    
    if(locked)
        npe_serial_transmit_message_and_unlock();
    else
        wf_gem_hci_manager_gymconnect_set_fe_state(messageToSend.args.gymconnect_set_fe_state.fe_state);
            
    res = npe_serial_interface_wait_for_response(npe_gem_library_check_if_response_received);

    assert(receivedMessage.message_class_id == WF_GEM_HCI_MSG_CLASS_GYM_CONNECT);
    assert(receivedMessage.message_id == WF_GEM_HCI_COMMAND_ID_GYM_CONNECT_SET_FE_STATE);

    p_fe_state_response->error_code = m_last_response.response.error_code;
    
    return(res);
}

uint32_t npe_hci_library_send_command_gymconnect_perform_workout_data_update(standard_response_t* p_update_response)
{
    uint32_t res;
    bool locked = npe_serial_transmit_lock(); // Check if on same thread - lock if not

    messageToSend.message_class_id = WF_GEM_HCI_MSG_CLASS_GYM_CONNECT;
    messageToSend.message_id = WF_GEM_HCI_COMMAND_ID_GYM_CONNECT_UPDATE_WORKOUT_DATA;
    
    // If we are on the same thread, just send the message. 
    // Otherwise need to marshall to the tx thread. 
    if(locked)
        npe_serial_transmit_message_and_unlock();
    else
        wf_gem_hci_manager_gymconnect_perform_workout_data_update();
    
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

void wf_gem_hci_manager_on_begin_retry_timer(uint16_t cmd_timeout_ms)
{
    npe_serial_interface_start_retry_timer(cmd_timeout_ms);
}

// cancel/stop the timer started in wf_gem_hci_manager_on_begin_retry_timer()
void wf_gem_hci_manager_on_cancel_retry_timer(void)
{
    npe_serial_interface_cancel_retry_timer();
}

// This callback is called if a command message can not be sent and all retries have failed.
void wf_gem_hci_manager_on_command_send_failure(wf_gem_hci_comms_message_t* message)
{
    printf("wf_gem_hci_manager_on_command_send_failure\n");
    assert(false);
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
