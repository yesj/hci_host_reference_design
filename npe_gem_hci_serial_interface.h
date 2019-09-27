#include <stdint.h>
#ifndef __NPE_GEM_HCI_SERIAL_INTERFACE__
#define __NPE_GEM_HCI_SERIAL_INTERFACE__




typedef void (*handle_recieved_message_t)(void*, uint32_t);     // Handles a complete message on rx thread.
typedef bool (*check_if_wait_condition_met_cb_t)(int);          // Processes a recieved message after it has been receive and signaled by rx thread
typedef void (*parse_bytes_t)(uint8_t*, int);                   // Process bytes received on rx thread. 
typedef void (*transmit_message_t)(void);                       // Transmit message in tx thread
typedef void (*timeout_t)(void);                                // 1 Hz timeout on tx thread

void npe_serial_interface_list_ports(void);
uint32_t npe_serial_interface_init(const char* p_port, parse_bytes_t parse_bytes_cb, transmit_message_t tx_msg_cb, timeout_t timeout_cb);
uint32_t npe_serial_interface_send_byte(uint8_t tx_byte);
uint32_t npe_serial_interface_wait_for_response(check_if_wait_condition_met_cb_t process_recieved_message_cb);
void npe_serial_interface_signal_response(handle_recieved_message_t handle_recieved_message_cb, void* p_arg, uint32_t size);
void npe_serial_transmit_message_and_unlock(void);
bool npe_serial_transmit_lock(void);

#endif