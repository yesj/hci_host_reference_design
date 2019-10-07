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

/** @brief Format of respnse from GEM 
 *
 */
typedef union 
{
    standard_response_t response;
} npe_hci_response_t;

/** @brief Union of GEM messages
 *
 */
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

/** @brief Describes a GEM message in concrete format 
 *
 */
typedef struct 
{
    uint8_t message_class_id;
    uint8_t message_id;
    npe_inc_function_args args;
} npe_hci_function_info_type;


static npe_hci_response_t m_last_response;          // Last response received from GEM
static wf_gem_hci_comms_message_t receivedMessage;  // Last received message from GEM
static npe_hci_function_info_type messageToSend;    // Message to send to GEM
static one_second_timeout_t m_timeout_cb;           // Function to call on 1 second timeout. 

/** @brief Called by RX THREAD. Parses received bytes from GEM. 
 *
 */
static void npe_gem_hci_library_interface_parse_bytes(uint8_t* byte_buff, int byte_num)
{
    for(int i = 0; i < byte_num;i++)
    {
        wf_gem_hci_comms_process_rx_byte(byte_buff[i]);
    }
}

      
/** @brief Called by TX THREAD. Sends a HCI ANT Config Message to GEM. 
 *
 */
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
            uint8_t main_version = messageToSend.args.ant_config_set_version.main;
            uint8_t supplemental = messageToSend.args.ant_config_set_version.supplemental;
            wf_gem_hci_manager_send_command_ant_config_set_software_version(main_version, supplemental);
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

/** @brief Called by TX THREAD. Sends a HCI Gymconnect Message to GEM. 
 *
 */
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

/** @brief Called by TX THREAD. Sends a HCI Bluetooth Control Message to GEM. 
 *
 */
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

/** @brief Called by TX THREAD. Sends a HCI Bluetooth Device Info Message to GEM. 
 *
 */
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

/** @brief Called by TX THREAD. Sends a HCI Bluetooth Config Message to GEM. 
 *
 */
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

/** @brief Called by TX THREAD. Sends a HCI System Message to GEM. 
 *
 */
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



/** @brief Called by TX THREAD. Sends a HCI message to GEM. This should be 
 * called only once local struct messageToSend has been populated. 
 *
 */
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

/** @brief CALLED BY 1 SECOND TIMER thread and calls application func
 *
 * @return  ::true if response we are waiting on has been recieved
 * 
 */
static void npe_hci_library_timeout(void)
{
    if(m_timeout_cb)
    {
        m_timeout_cb();
    }
}

/** @brief CALLED BY RX THREAD to copy received message to local structure.
 *
 * @param[in] message is a pointer to the received message.
 * @param[in] size is a 4-byte unsigned value denoting the size of the recieved message in bytes.
 */
static void npe_gem_hci_library_process_received_msg(void* message, uint32_t size)
{
    // TO DO - ASSERT SIZE DOES NOT EXCEED LOCAL BUFFER
    memcpy(&receivedMessage, message, size);
}

/** @brief CALLED BY RX THREAD to check if RX condition has been met.
 *
 * @return  ::true if response we are waiting on has been recieved
 * 
 */
static bool npe_gem_library_check_if_response_received(void)
{
    if(receivedMessage.message_class_id == messageToSend.message_class_id && receivedMessage.message_id == messageToSend.message_id)
    {
        return true;
    }
    
    return false;
}

/** @brief Initializes the NPE GEM HCI library and serial interface.
 *
 * @param[in] p_comport is a string denoting the name of the port to open.
 * @param[in] one_second_timeout_cb is an unsigned char (1 octet) denoting the channel number to query.
 *
 * @return  ::NPE_GEM_RESPONSE_OK
 *          ::NPE_GEM_RESPONSE_SERIAL_NO_COMPORT
 *          ::NPE_GEM_RESPONSE_SERIAL_OPEN_FAIL
 *          ::NPE_GEM_RESPONSE_SERIAL_CONFIG_FAIL
 */
uint32_t npe_gem_hci_library_interface_init(const char* p_comport, one_second_timeout_t one_second_timout_cb)
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
    return(npe_serial_interface_init(p_comport, &tx_callbacks));

}

/** @brief Send Ping command to GEM.
 *
 * @return  ::NPE_GEM_RESPONSE_OK
 *          ::NPE_GEM_RESPONSE_RETRIES_EXHAUSTED
 *          ::NPE_GEM_RESPONSE_TIMEOUT_OUT
 */
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

/** @brief Send the Bluetooth Device Name to the GEM.
 *
 * @param[in] bluetooth_name is a string denoting the bluetooth device name.
 * @param[out] p_set_device_name_response is the response code recieved from the GEM
 *
 * @return  ::NPE_GEM_RESPONSE_OK
 *          ::NPE_GEM_RESPONSE_RETRIES_EXHAUSTED
 *          ::NPE_GEM_RESPONSE_TIMEOUT_OUT
 */
uint32_t npe_hci_library_send_command_bluetooth_config_set_device_name(utf8_data_t* bluetooth_name, standard_response_t* p_set_device_name_response)
{
    bool locked = npe_serial_transmit_lock(); 
    messageToSend.message_class_id = WF_GEM_HCI_MSG_CLASS_BT_CONFIG;
    messageToSend.message_id = WF_GEM_HCI_COMMAND_ID_BT_CONFIG_SET_DEVICE_NAME;
    messageToSend.args.bt_config_set_device_name.bluetooth_name = bluetooth_name;
    
    if(locked)
        npe_serial_transmit_message_and_unlock();
    else
        wf_gem_hci_manager_send_command_bluetooth_config_set_device_name(messageToSend.args.bt_config_set_device_name.bluetooth_name);
 
    
    uint32_t res = npe_serial_interface_wait_for_response(npe_gem_library_check_if_response_received);

    assert(receivedMessage.message_class_id == WF_GEM_HCI_MSG_CLASS_BT_CONFIG);
    assert(receivedMessage.message_id == WF_GEM_HCI_COMMAND_ID_BT_CONFIG_SET_DEVICE_NAME);

    p_set_device_name_response->error_code = m_last_response.response.error_code;

    return(res);
}

/** @brief Send the Bluetooth Manufacturer Name to the GEM.
 *
 * @param[in] manufacturer_name is a string denoting the manufacturer name.
 * @param[out] p_set_manufacturer_name_response is the response code recieved from the GEM
 *
 * @return  ::NPE_GEM_RESPONSE_OK
 *          ::NPE_GEM_RESPONSE_RETRIES_EXHAUSTED
 *          ::NPE_GEM_RESPONSE_TIMEOUT_OUT
 */
uint32_t npe_hci_library_send_command_bluetooth_info_set_manufacturer_name(utf8_data_t* manufacturer_name, standard_response_t* p_set_manufacturer_name_response)
{
    bool locked = npe_serial_transmit_lock();
    messageToSend.message_class_id = WF_GEM_HCI_MSG_CLASS_BT_DEVICE_INFO;
    messageToSend.message_id = WF_GEM_HCI_COMMAND_ID_BT_DEVICE_INFO_SET_MANU_NAME;
    messageToSend.args.bt_device_info_set_manufacturer_name.manufacturer_name = manufacturer_name;
    
    if(locked)
        npe_serial_transmit_message_and_unlock();
    else
        wf_gem_hci_manager_send_command_bluetooth_info_set_manufacturer_name(messageToSend.args.bt_device_info_set_manufacturer_name.manufacturer_name);
    
    uint32_t res = npe_serial_interface_wait_for_response(npe_gem_library_check_if_response_received);
    
    assert(receivedMessage.message_class_id == WF_GEM_HCI_MSG_CLASS_BT_DEVICE_INFO);
    assert(receivedMessage.message_id == WF_GEM_HCI_COMMAND_ID_BT_DEVICE_INFO_SET_MANU_NAME);

    p_set_manufacturer_name_response->error_code = m_last_response.response.error_code;
    return(res);
}

/** @brief Send the Bluetooth Model Number to the GEM.
 *
 * @param[in] model_number is a string denoting the model number.
 * @param[out] p_set_manufacturer_name_response is the response code recieved from the GEM
 *
 * @return  ::NPE_GEM_RESPONSE_OK
 *          ::NPE_GEM_RESPONSE_RETRIES_EXHAUSTED
 *          ::NPE_GEM_RESPONSE_TIMEOUT_OUT
 */
uint32_t npe_hci_library_send_command_bluetooth_info_set_model_number(utf8_data_t* model_number, standard_response_t* p_set_model_number_response)
{
    bool locked = npe_serial_transmit_lock();
    messageToSend.message_class_id = WF_GEM_HCI_MSG_CLASS_BT_DEVICE_INFO;
    messageToSend.message_id = WF_GEM_HCI_COMMAND_ID_BT_DEVICE_INFO_SET_MODEL_NUM;
    messageToSend.args.bt_device_info_set_model_number.model_number = model_number;
    if(locked)
        npe_serial_transmit_message_and_unlock();
    else
        wf_gem_hci_manager_send_command_bluetooth_info_set_model_number(messageToSend.args.bt_device_info_set_model_number.model_number);
 
    uint32_t res = npe_serial_interface_wait_for_response(npe_gem_library_check_if_response_received);
    
    assert(receivedMessage.message_class_id == WF_GEM_HCI_MSG_CLASS_BT_DEVICE_INFO);
    assert(receivedMessage.message_id == WF_GEM_HCI_COMMAND_ID_BT_DEVICE_INFO_SET_MODEL_NUM);

    p_set_model_number_response->error_code = m_last_response.response.error_code;
    return(res);
}

/** @brief Send the Bluetooth Serial Number to the GEM.
 *
 * @param[in] serial_number is a string denoting the serial number.
 * @param[out] p_response_error_code is the response code recieved from the GEM
 *
 * @return  ::NPE_GEM_RESPONSE_OK
 *          ::NPE_GEM_RESPONSE_RETRIES_EXHAUSTED
 *          ::NPE_GEM_RESPONSE_TIMEOUT_OUT
 */
uint32_t npe_hci_library_send_command_bluetooth_info_set_serial_number(utf8_data_t* serial_number, standard_response_t* p_set_serial_number_response)
{
    bool locked = npe_serial_transmit_lock();
    messageToSend.message_class_id = WF_GEM_HCI_MSG_CLASS_BT_DEVICE_INFO;
    messageToSend.message_id = WF_GEM_HCI_COMMAND_ID_BT_DEVICE_INFO_SET_SERIAL_NUM;
    messageToSend.args.bt_device_info_set_serial_number.serial_number = serial_number;
    if(locked)
        npe_serial_transmit_message_and_unlock();
    else
        wf_gem_hci_manager_send_command_bluetooth_info_set_serial_number(messageToSend.args.bt_device_info_set_serial_number.serial_number);
 
    uint32_t res = npe_serial_interface_wait_for_response(npe_gem_library_check_if_response_received);
    
    assert(receivedMessage.message_class_id == WF_GEM_HCI_MSG_CLASS_BT_DEVICE_INFO);
    assert(receivedMessage.message_id == WF_GEM_HCI_COMMAND_ID_BT_DEVICE_INFO_SET_SERIAL_NUM);

    p_set_serial_number_response->error_code = m_last_response.response.error_code;
    return(res);
}

/** @brief Send the Bluetooth Hardware Revision to the GEM.
 *
 * @param[in] hardware_revision is a string denoting the hardware revision.
 * @param[out] p_response is the error code recieved from the GEM
 *
 * @return  ::NPE_GEM_RESPONSE_OK
 *          ::NPE_GEM_RESPONSE_RETRIES_EXHAUSTED
 *          ::NPE_GEM_RESPONSE_TIMEOUT_OUT
 */
uint32_t npe_hci_library_send_command_bluetooth_info_set_hardware_rev(utf8_data_t* hardware_revision, standard_response_t* p_response)
{
    bool locked = npe_serial_transmit_lock();
    messageToSend.message_class_id = WF_GEM_HCI_MSG_CLASS_BT_DEVICE_INFO;
    messageToSend.message_id = WF_GEM_HCI_COMMAND_ID_BT_DEVICE_INFO_SET_HW_REV;
    messageToSend.args.bt_device_info_set_hardware_revision.hardware_revision = hardware_revision;
    
    if(locked)
        npe_serial_transmit_message_and_unlock();
    else 
        wf_gem_hci_manager_send_command_bluetooth_info_set_hardware_rev(messageToSend.args.bt_device_info_set_hardware_revision.hardware_revision);
    
    uint32_t res = npe_serial_interface_wait_for_response(npe_gem_library_check_if_response_received);
    
    assert(receivedMessage.message_class_id == WF_GEM_HCI_MSG_CLASS_BT_DEVICE_INFO);
    assert(receivedMessage.message_id == WF_GEM_HCI_COMMAND_ID_BT_DEVICE_INFO_SET_HW_REV);

    p_response->error_code = m_last_response.response.error_code;
    return(res);
}

/** @brief Send the Bluetooth Firmware Revision to the GEM.
 *
 * @param[in] firmware_revision is a string denoting the firmware revision number.
 * @param[out] p_response is the error code recieved from the GEM
 *
 * @return  ::NPE_GEM_RESPONSE_OK
 *          ::NPE_GEM_RESPONSE_RETRIES_EXHAUSTED
 *          ::NPE_GEM_RESPONSE_TIMEOUT_OUT
 */
uint32_t npe_hci_library_send_command_bluetooth_info_set_firmware_rev(utf8_data_t* firmware_revision, standard_response_t* p_response)
{
    bool locked = npe_serial_transmit_lock();
    messageToSend.message_class_id = WF_GEM_HCI_MSG_CLASS_BT_DEVICE_INFO;
    messageToSend.message_id = WF_GEM_HCI_COMMAND_ID_BT_DEVICE_INFO_SET_FW_REV;
    messageToSend.args.bt_device_info_set_firmware_revision.firmware_revision = firmware_revision;
    
    if(locked)
        npe_serial_transmit_message_and_unlock();
    else 
        wf_gem_hci_manager_send_command_bluetooth_info_set_firmware_rev(messageToSend.args.bt_device_info_set_firmware_revision.firmware_revision);
    
    uint32_t res = npe_serial_interface_wait_for_response(npe_gem_library_check_if_response_received);
    
    assert(receivedMessage.message_class_id == WF_GEM_HCI_MSG_CLASS_BT_DEVICE_INFO);
    assert(receivedMessage.message_id == WF_GEM_HCI_COMMAND_ID_BT_DEVICE_INFO_SET_FW_REV);

    p_response->error_code = m_last_response.response.error_code;
    return(res);
}

/** @brief Send whether battery service should be included to the GEM.
 *
 * @param[in] battery_included 1 byte unsigned value denoting whether to enable battery service.
 *            WF_GEM_HCI_BLUETOOTH_BATTERY_SERVICE_INCLUDE (0x01)
 *            WF_GEM_HCI_BLUETOOTH_BATTERY_SERVICE_NOT_INCLUDE (0x00)
 * @param[out] p_response is the error code recieved from the GEM
 *
 * @return  ::NPE_GEM_RESPONSE_OK
 *          ::NPE_GEM_RESPONSE_RETRIES_EXHAUSTED
 *          ::NPE_GEM_RESPONSE_TIMEOUT_OUT
 *          ::NPE_GEM_RESPONSE_INVALID_PARAMETER
 */
uint32_t npe_hci_library_send_command_bluetooth_info_set_battery_included(uint8_t battery_included, standard_response_t* p_response)
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
  
    uint32_t res = npe_serial_interface_wait_for_response(npe_gem_library_check_if_response_received);
    
    assert(receivedMessage.message_class_id == WF_GEM_HCI_MSG_CLASS_BT_DEVICE_INFO);
    assert(receivedMessage.message_id == WF_GEM_HCI_COMMAND_ID_BT_DEVICE_INFO_SET_BATT_SERV_INC);

    p_response->error_code = m_last_response.response.error_code;
    return(res);
}

/** @brief Starts bluetooth advertising on the GEM
 *
 * @param[out] p_advertising_start_response is the response code recieved from the GEM
 *
 * @return  ::NPE_GEM_RESPONSE_OK
 *          ::NPE_GEM_RESPONSE_RETRIES_EXHAUSTED
 *          ::NPE_GEM_RESPONSE_TIMEOUT_OUT
 */
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

/** @brief Stops bluetooth advertising on the GEM
 *
 * @param[out] p_advertising_stop_response is the response code recieved from the GEM
 *
 * @return  ::NPE_GEM_RESPONSE_OK
 *          ::NPE_GEM_RESPONSE_RETRIES_EXHAUSTED
 *          ::NPE_GEM_RESPONSE_TIMEOUT_OUT
 */
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

/** @brief Send the ANT hardware revision to GEM 
 *
 * @param[in] hardware_version 1 byte unsigned value denoting ANT hardware revision.
 * @param[out] p_response is the error code recieved from the GEM
 *
 * @return  ::NPE_GEM_RESPONSE_OK
 *          ::NPE_GEM_RESPONSE_RETRIES_EXHAUSTED
 *          ::NPE_GEM_RESPONSE_TIMEOUT_OUT
 */
uint32_t npe_hci_library_send_command_ant_config_set_hardware_version(uint8_t hardware_version, standard_response_t* p_response)
{
    bool locked = npe_serial_transmit_lock(); 
    messageToSend.message_class_id = WF_GEM_HCI_MSG_CLASS_ANT_CONFIG;
    messageToSend.message_id = WF_GEM_HCI_COMMAND_ID_ANT_CONFIG_SET_HW_VER;
    messageToSend.args.ant_config_set_hardware_version.hardware_version = hardware_version;
    
    if(locked)
        npe_serial_transmit_message_and_unlock();
    else
        wf_gem_hci_manager_send_command_ant_config_set_hardware_version(messageToSend.args.ant_config_set_hardware_version.hardware_version);
    
    uint32_t res = npe_serial_interface_wait_for_response(npe_gem_library_check_if_response_received);

    assert(receivedMessage.message_class_id == WF_GEM_HCI_MSG_CLASS_ANT_CONFIG);
    assert(receivedMessage.message_id == WF_GEM_HCI_COMMAND_ID_ANT_CONFIG_SET_HW_VER);

    p_response->error_code = m_last_response.response.error_code;

    return(res);
}

/** @brief Send the ANT model number to GEM 
 *
 * @param[in] model_number 2 byte unsigned value denoting ANT model number.
 * @param[out] p_response is the error code recieved from the GEM
 * 
 * @return  ::NPE_GEM_RESPONSE_OK
 *          ::NPE_GEM_RESPONSE_RETRIES_EXHAUSTED
 *          ::NPE_GEM_RESPONSE_TIMEOUT_OUT
 */
uint32_t npe_hci_library_send_command_ant_config_set_model_number(uint16_t model_number, standard_response_t* p_response)
{
    bool locked = npe_serial_transmit_lock();
    messageToSend.message_class_id = WF_GEM_HCI_MSG_CLASS_ANT_CONFIG;
    messageToSend.message_id = WF_GEM_HCI_COMMAND_ID_ANT_CONFIG_SET_MODEL_NUM;
    messageToSend.args.ant_config_set_model_number.model_number = model_number;
    if(locked)
        npe_serial_transmit_message_and_unlock();
    else
        wf_gem_hci_manager_send_command_ant_config_set_model_number(messageToSend.args.ant_config_set_model_number.model_number);
  
    uint32_t res = npe_serial_interface_wait_for_response(npe_gem_library_check_if_response_received);

    assert(receivedMessage.message_class_id == WF_GEM_HCI_MSG_CLASS_ANT_CONFIG);
    assert(receivedMessage.message_id == WF_GEM_HCI_COMMAND_ID_ANT_CONFIG_SET_MODEL_NUM);

    p_response->error_code = m_last_response.response.error_code;

    return(res);
}

/** @brief Send the ANT software version to GEM 
 *
 * @param[in] main 1 byte unsigned value denoting the main software version.
 * @param[in] main 1 byte unsigned value denoting the supplemental software version.
 * @param[out] p_response is the error code recieved from the GEM
 *
 * @return  ::NPE_GEM_RESPONSE_OK
 *          ::NPE_GEM_RESPONSE_RETRIES_EXHAUSTED
 *          ::NPE_GEM_RESPONSE_TIMEOUT_OUT
 */
uint32_t npe_hci_library_send_command_ant_config_set_software_version(uint8_t main, uint8_t supplemental, standard_response_t* p_response)
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
 
    uint32_t res = npe_serial_interface_wait_for_response(npe_gem_library_check_if_response_received);

    assert(receivedMessage.message_class_id == WF_GEM_HCI_MSG_CLASS_ANT_CONFIG);
    assert(receivedMessage.message_id == WF_GEM_HCI_COMMAND_ID_ANT_CONFIG_SET_SW_VER);

    p_response->error_code = m_last_response.response.error_code;

    return(res);
}

/** @brief Send the ANT serial number to GEM 
 *
 * @param[in] serial_number 4 byte unsigned value denoting ANT serial number.
 * @param[out] p_response is the error code recieved from the GEM
 *
 * @return  ::NPE_GEM_RESPONSE_OK
 *          ::NPE_GEM_RESPONSE_RETRIES_EXHAUSTED
 *          ::NPE_GEM_RESPONSE_TIMEOUT_OUT
 */
uint32_t npe_hci_library_send_command_ant_config_set_serial_number(uint32_t serial_number, standard_response_t* p_response)
{
    bool locked = npe_serial_transmit_lock();

    messageToSend.message_class_id = WF_GEM_HCI_MSG_CLASS_ANT_CONFIG;
    messageToSend.message_id = WF_GEM_HCI_COMMAND_ID_ANT_CONFIG_SET_SERIAL_NUMBER;
    messageToSend.args.ant_config_set_serial_number.serial_number = serial_number;
    if(locked)
        npe_serial_transmit_message_and_unlock();
    else
        wf_gem_hci_manager_send_command_ant_config_set_serial_number(messageToSend.args.ant_config_set_serial_number.serial_number);
          
    uint32_t res = npe_serial_interface_wait_for_response(npe_gem_library_check_if_response_received);

    assert(receivedMessage.message_class_id == WF_GEM_HCI_MSG_CLASS_ANT_CONFIG);
    assert(receivedMessage.message_id == WF_GEM_HCI_COMMAND_ID_ANT_CONFIG_SET_SERIAL_NUMBER);

    p_response->error_code = m_last_response.response.error_code;

    return(res);
}

/** @brief Set GEM controllable features
 *
 * @param[in] equipment_control_field_identifier 4 byte unsigned bitfield denoting denoting contollable feautures.
 * @param[out] p_response is the error code recieved from the GEM
 * 
 * @return  ::NPE_GEM_RESPONSE_OK
 *          ::NPE_GEM_RESPONSE_RETRIES_EXHAUSTED
 *          ::NPE_GEM_RESPONSE_TIMEOUT_OUT
 */
uint32_t npe_hci_library_send_command_gymconnect_set_supported_equipment_control_features(uint32_t equipment_control_field_identifier, standard_response_t* p_response)
{
    bool locked = npe_serial_transmit_lock(); 
    messageToSend.message_class_id = WF_GEM_HCI_MSG_CLASS_GYM_CONNECT;
    messageToSend.message_id = WF_GEM_HCI_COMMAND_ID_GYM_CONNECT_SET_FE_CONTROL_FEATURES;
    messageToSend.args.gymconnect_set_supported_equipment_control_features.equipment_control_field_identifier = equipment_control_field_identifier;
    
    if(locked)
        npe_serial_transmit_message_and_unlock();
    else 
        wf_gem_hci_manager_gymconnect_set_supported_equipment_control_features(messageToSend.args.gymconnect_set_supported_equipment_control_features.equipment_control_field_identifier);
    
    uint32_t res = npe_serial_interface_wait_for_response(npe_gem_library_check_if_response_received);

    assert(receivedMessage.message_class_id == WF_GEM_HCI_MSG_CLASS_GYM_CONNECT);
    assert(receivedMessage.message_id == WF_GEM_HCI_COMMAND_ID_GYM_CONNECT_SET_FE_CONTROL_FEATURES);

    p_response->error_code = m_last_response.response.error_code;

    return(res);
}

/** @brief Send the fitness equipment type to GEM 
 *
 * @param[in] fe_type is an enum denoting the fitness equipment type.
 * @param[out] p_response is the error code recieved from the GEM
 *
 * @return  ::NPE_GEM_RESPONSE_OK
 *          ::NPE_GEM_RESPONSE_RETRIES_EXHAUSTED
 *          ::NPE_GEM_RESPONSE_TIMEOUT_OUT
 */
uint32_t npe_hci_library_send_command_gymconnect_set_fe_type(wf_gem_hci_gymconnect_fitness_equipment_type_e fe_type, standard_response_t* p_response)
{
    bool locked = npe_serial_transmit_lock();

    messageToSend.message_class_id = WF_GEM_HCI_MSG_CLASS_GYM_CONNECT;
    messageToSend.message_id = WF_GEM_HCI_COMMAND_ID_GYM_CONNECT_SET_FE_TYPE;
    messageToSend.args.gymconnect_set_fe_type.fe_type = fe_type;
    
    if(locked)
        npe_serial_transmit_message_and_unlock();
    else
        wf_gem_hci_manager_gymconnect_set_fe_type(messageToSend.args.gymconnect_set_fe_type.fe_type);
            
    uint32_t res = npe_serial_interface_wait_for_response(npe_gem_library_check_if_response_received);

    assert(receivedMessage.message_class_id == WF_GEM_HCI_MSG_CLASS_GYM_CONNECT);
    assert(receivedMessage.message_id == WF_GEM_HCI_COMMAND_ID_GYM_CONNECT_SET_FE_TYPE);

    p_response->error_code = m_last_response.response.error_code;

    return(res);
}

/** @brief Send the fitness equipment state to GEM 
 *
 * @param[in] fe_state is an enum denoting the fitness equipment state.
 * @param[out] p_fe_state_response is the response code recieved from the GEM
 *
 * @return  ::NPE_GEM_RESPONSE_OK
 *          ::NPE_GEM_RESPONSE_RETRIES_EXHAUSTED
 *          ::NPE_GEM_RESPONSE_TIMEOUT_OUT
 */
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
/** @brief Sends set fitness equipment data to the GEM
 *
 * @param[out] p_update_response is the response code recieved from the GEM
 *
 * @return  ::NPE_GEM_RESPONSE_OK
 *          ::NPE_GEM_RESPONSE_RETRIES_EXHAUSTED
 *          ::NPE_GEM_RESPONSE_TIMEOUT_OUT
 */
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

/******************** WF functions defined ************************************************************************************************/


void wf_gem_hci_comms_on_send_byte(uint8_t tx_byte)
{
    uint32_t err = npe_serial_interface_send_byte(tx_byte);
    assert(err == NPE_GEM_RESPONSE_OK);

}
// Called on RX thread.
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
    fflush(stdout);
    assert(false);
}




// ****** Process Command Responses. *************************************************************** //
void wf_gem_hci_manager_on_command_response_generic(uint8_t error_code)
{
    m_last_response.response.error_code = error_code;
}


void wf_gem_hci_manager_on_command_response_bluetooth_control_start_advertising(uint8_t error_code)
{   
    m_last_response.response.error_code = error_code;
}

void wf_gem_hci_manager_on_command_response_bluetooth_control_stop_advertising(uint8_t error_code)
{
    
}

void wf_gem_hci_manager_on_command_response_bluetooth_config_set_device_name(uint8_t error_code)
{
    m_last_response.response.error_code = error_code;
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
void wf_gem_hci_manager_on_command_response_bluetooth_control_get_bluetooth_state(wf_gem_hci_bluetooth_state_e bluetooth_state)
{
    printf("wf_gem_hci_manager_on_command_response_bluetooth_control_get_bluetooth_state\n");
}

void wf_gem_hci_manager_on_event_bluetooth_control_advertising_timed_out(void)
{
    printf("wf_gem_hci_manager_on_event_bluetooth_control_advertising_timed_out\n");
    fflush(stdout);
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
    //printf("wf_gem_hci_manager_gymconnect_on_workout_data_update_complete\n");
}

void wf_gem_hci_manager_gymconnecton_event_heart_rate_value_received(uint16_t heart_rate_value)
{
    printf("wf_gem_hci_manager_gymconnecton_event_heart_rate_value_received\n");
}
void wf_gem_hci_manager_gymconnecton_event_cadence_value_received(uint16_t cadence_value)
{
    printf("wf_gem_hci_manager_gymconnecton_event_cadence_value_received\n");
}