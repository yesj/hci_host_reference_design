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

#ifndef __NPE_GEM_HCI_LIBRARY_RESPONSE_STRINGS__
#define __NPE_GEM_HCI_LIBRARY_RESPONSE_STRINGS__
#include <stdint.h>

#define NPE_GEM_HCI_COMMON_ERROR_BASE                   ((uint8_t) 200)
#define NPE_GEM_HCI_COMMON_UNKNOWN_ERROR_BASE           ((uint8_t) 254)

#define NPE_GEM_HCI_LIB_PRINT_IF_ERROR(response_code, string_to_print)  \
        do                                                              \
        {                                                               \
                if(response_code != 0) printf("%s", string_to_print);   \
                fflush(stdout);                                         \
        } while(0)


//TO DO!! - check that index fits into specific array, and other array boudaries. 
#define NPE_GEM_HCI_LIB_PROCESS_ERROR_CODE(specific_error_array, error_code) \
        error_code < NPE_GEM_HCI_COMMON_ERROR_BASE ? \
                specific_error_array[error_code] \
                : \
                error_code < NPE_GEM_HCI_COMMON_UNKNOWN_ERROR_BASE ? \
                npe_hci_common_error_code_string[error_code] \
                : \
                npe_hci_common_unknown_error_code_string[error_code]

#define NPE_GEM_HCI_LIB_HW_SET_PINS_ERROR_CODE_STR(error_code)  \
        NPE_GEM_HCI_LIB_PROCESS_ERROR_CODE(npe_hci_hardware_set_pins_error_code_string, error_code)

#define NPE_GEM_HCI_LIB_GET_BT_CONTRL_ADV_START_ERROR_CODE_STR(error_code)  \
        NPE_GEM_HCI_LIB_PROCESS_ERROR_CODE(npe_hci_bluetooth_control_start_advertising_response_error_code_string, error_code)
        
#define NPE_GEM_HCI_LIB_GET_BT_CONTRL_ADV_STOP_ERROR_CODE_STR(error_code)  \
        NPE_GEM_HCI_LIB_PROCESS_ERROR_CODE(npe_hci_bluetooth_control_stop_advertising_response_error_code_string, error_code)

#define NPE_GEM_HCI_LIB_GET_BT_CONFIG_SET_DEVICE_NAME_ERROR_CODE_STR(error_code)  \
        NPE_GEM_HCI_LIB_PROCESS_ERROR_CODE(npe_hci_string_response_error_code_string, error_code)

#define NPE_GEM_HCI_LIB_GET_BT_INFO_SET_MANUFACTURER_NAME_ERROR_CODE_STR(error_code)  \
        NPE_GEM_HCI_LIB_PROCESS_ERROR_CODE(npe_hci_string_response_error_code_string, error_code)

#define NPE_GEM_HCI_LIB_GET_BT_INFO_SET_MODEL_NUMBER_ERROR_CODE_STR(error_code)  \
        NPE_GEM_HCI_LIB_PROCESS_ERROR_CODE(npe_hci_generic_response_error_code_string, error_code)

#define NPE_GEM_HCI_LIB_GET_BT_INFO_SET_SERIAL_NUMBER_ERROR_CODE_STR(error_code)  \
        NPE_GEM_HCI_LIB_PROCESS_ERROR_CODE(npe_hci_string_response_error_code_string, error_code)

#define NPE_GEM_HCI_LIB_GET_BT_INFO_SET_FIRMWARE_REVISION_ERROR_CODE_STR(error_code)  \
        NPE_GEM_HCI_LIB_PROCESS_ERROR_CODE(npe_hci_string_response_error_code_string, error_code)

#define NPE_GEM_HCI_LIB_GET_BT_INFO_SET_HARDWARE_REVISION_ERROR_CODE_STR(error_code)  \
        NPE_GEM_HCI_LIB_PROCESS_ERROR_CODE(npe_hci_string_response_error_code_string, error_code)

#define NPE_GEM_HCI_LIB_GET_BT_INFO_SET_BATTERY_SERVICE_INCLUDED_ERROR_CODE_STR(error_code)  \
        NPE_GEM_HCI_LIB_PROCESS_ERROR_CODE(npe_hci_generic_response_error_code_string, error_code)

#define NPE_GEM_HCI_LIB_GET_ANT_CONFIG_SET_HARDWARE_VERSION_ERROR_CODE_STR(error_code)  \
        NPE_GEM_HCI_LIB_PROCESS_ERROR_CODE(npe_hci_generic_response_error_code_string, error_code)

#define NPE_GEM_HCI_LIB_GET_ANT_CONFIG_SET_MODEL_NUMBER_ERROR_CODE_STR(error_code)  \
        NPE_GEM_HCI_LIB_PROCESS_ERROR_CODE(npe_hci_generic_response_error_code_string, error_code)

#define NPE_GEM_HCI_LIB_GET_ANT_CONFIG_SET_SOFTWARE_VERSION_ERROR_CODE_STR(error_code)  \
        NPE_GEM_HCI_LIB_PROCESS_ERROR_CODE(npe_hci_generic_response_error_code_string, error_code)

#define NPE_GEM_HCI_LIB_GET_ANT_CONFIG_SET_SERIAL_NUMBER_ERROR_CODE_STR(error_code)  \
        NPE_GEM_HCI_LIB_PROCESS_ERROR_CODE(npe_hci_generic_response_error_code_string, error_code)

#define NPE_GEM_HCI_LIB_GET_GYMCONNECT_SET_FITNESS_EQUIPMENT_TYPE_ERROR_CODE_STR(error_code)  \
        NPE_GEM_HCI_LIB_PROCESS_ERROR_CODE(npe_hci_generic_response_error_code_string, error_code)

#define NPE_GEM_HCI_LIB_GET_GYMCONNECT_SET_FITNESS_EQUIPMENT_STATE_ERROR_CODE_STR(error_code)  \
        NPE_GEM_HCI_LIB_PROCESS_ERROR_CODE(npe_hci_generic_response_error_code_string, error_code)

#define NPE_GEM_HCI_LIB_GET_GYMCONNECT_SET_SUPPORTED_EQUIPMENT_CONTROL_ERROR_CODE_STR(error_code)  \
        NPE_GEM_HCI_LIB_PROCESS_ERROR_CODE(npe_hci_gymconnect_set_equipment_control_error_code_string, error_code)

#define NPE_GEM_HCI_LIB_GET_GYMCONNECT_WORKOUT_DATA_UPDATE_ERROR_CODE_STR(error_code)  \
        NPE_GEM_HCI_LIB_PROCESS_ERROR_CODE(npe_hci_generic_response_error_code_string, error_code)

#define NPE_GEM_HCI_LIB_ANT_RECEIVER_START_DISCOVERY_ERROR_CODE_STR(error_code)  \
        NPE_GEM_HCI_LIB_PROCESS_ERROR_CODE(npe_hci_ant_receiver_start_discovery_error_code_string, error_code)
       


static const char* npe_hci_generic_response_error_code_string[] = {  
        "Success"                                                                            
        }; 

static const char* npe_hci_string_response_error_code_string[] = {  
        "Success",                                                                             
        "Full Device Name will not fit, it has been truncated"
        }; 


static const char* npe_hci_hardware_set_pins_error_code_string[] = {  
        "Success",                                                                             
        "One or more invalid pin numbers were supplied",
        "One or more invalid Pin IO Modes were supplied"
        }; 

static const char* npe_hci_bluetooth_control_start_advertising_response_error_code_string[] = {  
        "Success",                                                                             
        "Error - Already Advertising",                                                          
        "Error - Already Connected"
        }; 

static const char* npe_hci_bluetooth_control_stop_advertising_response_error_code_string[] = {  
        "Success",                                                                             
        "Error - Was Not Advertising",                                                          
        "Error - Already Connected"
        }; 

static const char* npe_hci_common_error_code_string[] = {  
        "Invalid Data Length",          // 200                                                                     
        "Invalid Parameter Value",      // 201                                             
        "Malformed UTF-8 string",       // 202
        "Incorrect State",              // 203
        "Not Supported",                // 204
        }; 


static const char* npe_hci_common_unknown_error_code_string[] = {  
        "Unknown Message Type",         // 254                                                                     
        "Unknown Error"                 // 255                                             
        }; 
                
static const char* npe_hci_gymconnect_set_equipment_control_error_code_string[] = {  
        "Success",                                                                             
        "Error - BLE Advertising",                                                          
        "Error - BLE Connection Established"
        }; 

static const char* npe_hci_ant_receiver_start_discovery_error_code_string[] = {  
        "Success",                                                                             
        "Invalid/Unsupported ANT+ Receiver Profile specified",
        "UNKNOWN 2",              
        "UNKNOWN 3",                                                   
        "UNKNOWN 4",  
        "UNKNOWN 5",  
        "UNKNOWN 6",
        "UNKNOWN 7",
        "UNKNOWN 8",
        "UNKNOWN 9",
        "UNKNOWN 10",
        "UNKNOWN 11",
        "UNKNOWN 12",
        "UNKNOWN 13",
        "UNKNOWN 14",
        "UNKNOWN 15",
        "UNKNOWN 16",
        "UNKNOWN 17",
        "UNKNOWN 18",
        "UNKNOWN 19",
        "Unable to begin discovery process"
        }; 

#endif