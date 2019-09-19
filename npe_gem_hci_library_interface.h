#ifndef __NPE_GEM_HCI_SERIAL_INTERFACE__
#define __NPE_GEM_HCI_SERIAL_INTERFACE__
#include <stdint.h>

uint32_t npe_gem_hci_library_interface_init(void);
uint32_t npe_hci_library_send_ping(void);
uint32_t npe_hci_library_send_command_bluetooth_config_set_device_name(utf8_data_t* bluetoothName);
uint32_t npe_hci_library_send_command_bluetooth_info_set_manufacturer_name(utf8_data_t* manufacturer_name);
uint32_t npe_hci_library_send_command_bluetooth_info_set_model_number(utf8_data_t* model_number);
#endif