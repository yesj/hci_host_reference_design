#ifndef __NPE_GEM_HCI_LIBRARY_RESPONSE_STRINGS__
#define __NPE_GEM_HCI_LIBRARY_RESPONSE_STRINGS__
#include <stdint.h>

#define NPE_GEM_HCI_LIB_GET_BT_CONTRL_ADV_START_ERROR_CODE_STR(error_code)  npe_hci_bluetooth_control_start_advertising_response_error_code_string[error_code]
#define NPE_GEM_HCI_LIB_GET_BT_CONTRL_ADV_STOP_ERROR_CODE_STR(error_code)  npe_hci_bluetooth_control_stop_advertising_response_error_code_string[error_code]

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

#endif