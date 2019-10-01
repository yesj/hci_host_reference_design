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
#include <stdint.h>
#ifndef __NPE_GEM_HCI_SERIAL_INTERFACE__
#define __NPE_GEM_HCI_SERIAL_INTERFACE__

typedef void (*handle_recieved_message_t)(void*, uint32_t);     // Handles a complete message on rx thread.
typedef bool (*check_if_wait_condition_met_cb_t)(int);          // Processes a recieved message after it has been receive and signaled by rx thread
typedef void (*parse_bytes_t)(uint8_t*, int);                   // Process bytes received on rx thread. 
typedef void (*transmit_message_t)(void);                       // Transmit message in tx thread
typedef void (*timeout_t)(void);                                // 1 Hz timeout on tx thread
typedef void (*retry_timeout_t)(void);                          // Retry timeout on tx thread

// Callback struct 
typedef struct {
    parse_bytes_t  parse_bytes_cb;              // Parses bytes recieved by RX thread
    transmit_message_t  transmit_message_cb;    // Called by TX thread to start message transfer
    timeout_t timeout_cb;                       // Called by TX thread @ 1Hz.
    retry_timeout_t retry_timeout_cb;           // Called by TX thread on retry timeout
} npe_serial_interface_callbacks_t;

/** @brief Lists all ports available. Probably does not work on Android.
 * 
 */
void npe_serial_interface_list_ports(void);

/** @brief Initialize and open the comport. Defaults to 115200, 8 bits, 1 stop, no parity.
 *
 * @param[in] p_port is a string that specifies the comport (e.g. "COM6").
 * @param[in] p_callbacks is a struct of all callbacks.
 *
 * @return  ::NPE_GEM_RESPONSE_OK
 *          ::NPE_GEM_RESPONSE_SERIAL_OPEN_FAIL
 */
uint32_t npe_serial_interface_init(const char* p_port, npe_serial_interface_callbacks_t* p_callbacks);

/** @brief Sends a single byte to the comport. Should be called in the TX thread context.
 *
 * @param[in] tx_byte is a a byte that will be sent to the comport. 
 *
 * @return  ::NPE_GEM_RESPONSE_OK
 *          ::NPE_GEM_RESPONSE_UNABLE_TO_WRITE
 */
uint32_t npe_serial_interface_send_byte(uint8_t tx_byte);

/** @brief Waits to receive a correctly formatted response from GEM. If a response is  
 * recieved then it will call the supplied callback, which can decide if the desired 
 * response was recieved. If the callback returns false the wait is retried up to 10 times
 * The wait for the response is WF_GEM_HCI_DEFAULT_MAX_COMMAND_RETRY_ATTEMPTS+1) * 
 * WF_GEM_HCI_DEFAULT_COMMAND_TIMEOUT_MS + 50.
 *
 * @param[in] process_recieved_message_cb is a callback called when a response recieved. 
 * It should return true is the desired response has been received.  
 *
 * @return  ::NPE_GEM_RESPONSE_OK
 *          ::NPE_GEM_RESPONSE_RETRIES_EXHAUSTED -> got responses but not the one we were looking for
 *          ::NPE_GEM_RESPONSE_TIMEOUT_OUT -> No response received within the timeout
 */
uint32_t npe_serial_interface_wait_for_response(check_if_wait_condition_met_cb_t process_recieved_message_cb);

/** @brief Once a message is received on the RX thread, this function should be called
 * to signal any other threads waiting for this message. This function should be called in 
 * the context of the RX thread. 
 *
 * @param[in] handle_recieved_message_cb is a callback called before signaling that the message
 * was recieved. 
 * @param[in] p_arg isa pointer to the arguments for the function.
 * @param[in] size is that size of the arguments. 
 * 
 */
void npe_serial_interface_signal_response(handle_recieved_message_t handle_recieved_message_cb, void* p_arg, uint32_t size);
void npe_serial_transmit_message_and_unlock(void);
bool npe_serial_transmit_lock(void);
void npe_serial_interface_start_retry_timer(uint16_t msec);
void npe_serial_interface_cancel_retry_timer(void);

#endif