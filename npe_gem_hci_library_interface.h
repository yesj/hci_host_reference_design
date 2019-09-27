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
#ifndef __NPE_GEM_HCI_LIBRARY_INTERFACE__
#define __NPE_GEM_HCI_LIBRARY_INTERFACE__
#include <stdint.h>

#include "wf_gem_hci_manager_gymconnect.h"

// Callback function - call @ 1Hz to allow application
// to provide fitness equipment data to the GEM
typedef void (*one_second_timeout_t)(void);

// Standard response to GEM HCI messages. 
typedef struct npe_gem_hci_library_interface
{
    uint8_t error_code;
}standard_response_t;


/** @brief Initializes the NPE GEM HCI library and serial interface.
 *
 * @param[in] one_second_timeout_cb is an unsigned char (1 octet) denoting the channel number to query.
 *
 * @return  ::NPE_GEM_RESPONSE_OK
 *          ::NPE_GEM_RESPONSE_SERIAL_OPEN_FAIL
 */
uint32_t npe_gem_hci_library_interface_init(one_second_timeout_t one_second_timeout_cb);

/** @brief Send Ping command to GEM.
 *
 * @return  ::NPE_GEM_RESPONSE_OK
 *          ::NPE_GEM_RESPONSE_RETRIES_EXHAUSTED
 *          ::NPE_GEM_RESPONSE_TIMEOUT_OUT
 */
uint32_t npe_hci_library_send_ping(void);

/** @brief Send the Bluetooth Device Name to the GEM.
 *
 * @param[in] bluetooth_name is a string denoting the bluetooth device name.
 *
 * @return  ::NPE_GEM_RESPONSE_OK
 *          ::NPE_GEM_RESPONSE_RETRIES_EXHAUSTED
 *          ::NPE_GEM_RESPONSE_TIMEOUT_OUT
 */
uint32_t npe_hci_library_send_command_bluetooth_config_set_device_name(utf8_data_t* bluetooth_name);

/** @brief Send the Bluetooth Manufacturer Name to the GEM.
 *
 * @param[in] manufacturer_name is a string denoting the manufacturer name.
 *
 * @return  ::NPE_GEM_RESPONSE_OK
 *          ::NPE_GEM_RESPONSE_RETRIES_EXHAUSTED
 *          ::NPE_GEM_RESPONSE_TIMEOUT_OUT
 */
uint32_t npe_hci_library_send_command_bluetooth_info_set_manufacturer_name(utf8_data_t* manufacturer_name);

/** @brief Send the Bluetooth Model Number to the GEM.
 *
 * @param[in] model_number is a string denoting the model number.
 *
 * @return  ::NPE_GEM_RESPONSE_OK
 *          ::NPE_GEM_RESPONSE_RETRIES_EXHAUSTED
 *          ::NPE_GEM_RESPONSE_TIMEOUT_OUT
 */
uint32_t npe_hci_library_send_command_bluetooth_info_set_model_number(utf8_data_t* model_number);

/** @brief Send the Bluetooth Serial Number to the GEM.
 *
 * @param[in] serial_number is a string denoting the serial number.
 *
 * @return  ::NPE_GEM_RESPONSE_OK
 *          ::NPE_GEM_RESPONSE_RETRIES_EXHAUSTED
 *          ::NPE_GEM_RESPONSE_TIMEOUT_OUT
 */
uint32_t npe_hci_library_send_command_bluetooth_info_set_serial_number(utf8_data_t* serial_number);

/** @brief Send the Bluetooth Hardware Revision to the GEM.
 *
 * @param[in] hardware_revision is a string denoting the hardware revision.
 *
 * @return  ::NPE_GEM_RESPONSE_OK
 *          ::NPE_GEM_RESPONSE_RETRIES_EXHAUSTED
 *          ::NPE_GEM_RESPONSE_TIMEOUT_OUT
 */
uint32_t npe_hci_library_send_command_bluetooth_info_set_hardware_rev(utf8_data_t* hardware_revision);

/** @brief Send the Bluetooth Firmware Revision to the GEM.
 *
 * @param[in] firmware_revision is a string denoting the firmware revision number.
 *
 * @return  ::NPE_GEM_RESPONSE_OK
 *          ::NPE_GEM_RESPONSE_RETRIES_EXHAUSTED
 *          ::NPE_GEM_RESPONSE_TIMEOUT_OUT
 */
uint32_t npe_hci_library_send_command_bluetooth_info_set_firmware_rev(utf8_data_t* firmware_revision);

/** @brief Send whether battery service should be included to the GEM.
 *
 * @param[in] battery_included 1 byte unsigned value denoting whether to enable battery service.
 *            WF_GEM_HCI_BLUETOOTH_BATTERY_SERVICE_INCLUDE (0x01)
 *            WF_GEM_HCI_BLUETOOTH_BATTERY_SERVICE_NOT_INCLUDE (0x00)
 *
 * @return  ::NPE_GEM_RESPONSE_OK
 *          ::NPE_GEM_RESPONSE_RETRIES_EXHAUSTED
 *          ::NPE_GEM_RESPONSE_TIMEOUT_OUT
 *          ::NPE_GEM_RESPONSE_INVALID_PARAMETER
 */
uint32_t npe_hci_library_send_command_bluetooth_info_set_battery_included(uint8_t battery_included);

/** @brief Set GEM controllable features
 *
 * @param[in] equipment_control_field_identifier 4 byte unsigned bitfield denoting denoting contollable feautures.
 *
 * @return  ::NPE_GEM_RESPONSE_OK
 *          ::NPE_GEM_RESPONSE_RETRIES_EXHAUSTED
 *          ::NPE_GEM_RESPONSE_TIMEOUT_OUT
 */
uint32_t npe_hci_library_send_command_gymconnect_set_supported_equipment_control_features(uint32_t equipment_control_field_identifier);

/** @brief Send the ANT hardware revision to GEM 
 *
 * @param[in] hardware_version 1 byte unsigned value denoting ANT hardware revision.
 *
 * @return  ::NPE_GEM_RESPONSE_OK
 *          ::NPE_GEM_RESPONSE_RETRIES_EXHAUSTED
 *          ::NPE_GEM_RESPONSE_TIMEOUT_OUT
 */
uint32_t npe_hci_library_send_command_ant_config_set_hardware_version(uint8_t hardware_version);

/** @brief Send the ANT model number to GEM 
 *
 * @param[in] model_number 2 byte unsigned value denoting ANT model number.
 *
 * @return  ::NPE_GEM_RESPONSE_OK
 *          ::NPE_GEM_RESPONSE_RETRIES_EXHAUSTED
 *          ::NPE_GEM_RESPONSE_TIMEOUT_OUT
 */
uint32_t npe_hci_library_send_command_ant_config_set_model_number(uint16_t model_number);

/** @brief Send the ANT software version to GEM 
 *
 * @param[in] main 1 byte unsigned value denoting the main software version.
 * @param[in] main 1 byte unsigned value denoting the supplemental software version.
 *
 * @return  ::NPE_GEM_RESPONSE_OK
 *          ::NPE_GEM_RESPONSE_RETRIES_EXHAUSTED
 *          ::NPE_GEM_RESPONSE_TIMEOUT_OUT
 */
uint32_t npe_hci_library_send_command_ant_config_set_software_version(uint8_t main, uint8_t supplemental);

/** @brief Send the ANT serial number to GEM 
 *
 * @param[in] serial_number 4 byte unsigned value denoting ANT serial number.
 *
 * @return  ::NPE_GEM_RESPONSE_OK
 *          ::NPE_GEM_RESPONSE_RETRIES_EXHAUSTED
 *          ::NPE_GEM_RESPONSE_TIMEOUT_OUT
 */
uint32_t npe_hci_library_send_command_ant_config_set_serial_number(uint32_t serial_number);

/** @brief Send the fitness equipment type to GEM 
 *
 * @param[in] fe_type is an enum denoting the fitness equipment type.
 *
 * @return  ::NPE_GEM_RESPONSE_OK
 *          ::NPE_GEM_RESPONSE_RETRIES_EXHAUSTED
 *          ::NPE_GEM_RESPONSE_TIMEOUT_OUT
 */
uint32_t npe_hci_library_send_command_gymconnect_set_fe_type(wf_gem_hci_gymconnect_fitness_equipment_type_e fe_type);

/** @brief Send the fitness equipment state to GEM 
 *
 * @param[in] fe_state is an enum denoting the fitness equipment state.
 * @param[out] p_fe_state_response is the response code recieved from the GEM
 *
 * @return  ::NPE_GEM_RESPONSE_OK
 *          ::NPE_GEM_RESPONSE_RETRIES_EXHAUSTED
 *          ::NPE_GEM_RESPONSE_TIMEOUT_OUT
 */
uint32_t npe_hci_library_send_command_gymconnect_set_fe_state(wf_gem_hci_gymconnect_fitness_equipment_state_e fe_state, standard_response_t* p_fe_state_response);

/** @brief Starts bluetooth advertising on the GEM
 *
 * @param[out] p_advertising_start_response is the response code recieved from the GEM
 *
 * @return  ::NPE_GEM_RESPONSE_OK
 *          ::NPE_GEM_RESPONSE_RETRIES_EXHAUSTED
 *          ::NPE_GEM_RESPONSE_TIMEOUT_OUT
 */
uint32_t npe_hci_library_send_command_bluetooth_control_start_advertising(standard_response_t* p_advertising_start_response);

/** @brief Stops bluetooth advertising on the GEM
 *
 * @param[out] p_advertising_stop_response is the response code recieved from the GEM
 *
 * @return  ::NPE_GEM_RESPONSE_OK
 *          ::NPE_GEM_RESPONSE_RETRIES_EXHAUSTED
 *          ::NPE_GEM_RESPONSE_TIMEOUT_OUT
 */
uint32_t npe_hci_library_send_command_bluetooth_control_stop_advertising(standard_response_t* p_advertising_stop_response);

/** @brief Sends set fitness equipment data to the GEM
 *
 * @param[out] p_update_response is the response code recieved from the GEM
 *
 * @return  ::NPE_GEM_RESPONSE_OK
 *          ::NPE_GEM_RESPONSE_RETRIES_EXHAUSTED
 *          ::NPE_GEM_RESPONSE_TIMEOUT_OUT
 */
uint32_t npe_hci_library_send_command_gymconnect_perform_workout_data_update(standard_response_t* p_update_response);

#endif