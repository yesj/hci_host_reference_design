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
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/time.h>
#include <pthread.h> 
#include <stdbool.h>
#include <libserialport.h>
#include <errno.h>
#include "npe_error_code.h"
#include "wf_gem_hci_comms.h"
#include "wf_gem_hci_manager_gymconnect.h"
#include "npe_gem_hci_serial_interface.h"

// TX thread events. 
#define NPE_HCI_TX_FLAG_SEND_MSG    ((uint32_t) 0x00000001) // Send a message to GEM
#define NPE_HCI_TX_FLAG_TIMEOUT     ((uint32_t) 0x00000002) // Timeout occurred. 
#define NPE_HCI_TX_FLAG_RETRY       ((uint32_t) 0x00000004) // Retry message.

// Threading local variables. 
static pthread_mutex_t rxMutex, txMutex;
static pthread_cond_t rxCond, txCond;
static pthread_t rx_thread_id, tx_thread_id, timer_thread_id, retry_timer_thread_id; 

static uint32_t m_tx_event;     // Events to process on TX thread.

// Libserialport local variables.
static struct sp_port *port;

// static callbacks 
static npe_serial_interface_callbacks_t m_callback;



/** @brief Executes Retry Timer Thread. Timer events pushed to TX thread. 
 *
 * @param[in] vargp pointer to variable args - not used. 
 *
 * @return  NULL
 * 
 */
static void* npe_serial_interface_retry_timer(void* vargp)
{
    struct timespec ts;
    int res;
    assert(vargp != NULL);

    long msec = *(uint16_t*)vargp;

    ts.tv_sec = msec / 1000;
    ts.tv_nsec = (msec % 1000) * 1000000;

    res = nanosleep(&ts, &ts);

    if(res == 0)
    {
        pthread_mutex_lock(&txMutex);

        m_tx_event |= NPE_HCI_TX_FLAG_RETRY;

        pthread_cond_signal(&txCond);
        pthread_mutex_unlock(&txMutex);
    }

    return NULL;

}

/** @brief Executes 1Hz Timer Thread. Timer events pushed to TX thread. 
 * Never returns.
 *
 * @param[in] vargp pointer to variable args - not used. 
 *
 * @return  NULL
 * 
 */
static void* npe_serial_interface_timer_thread(void *vargp)
{
    while(1)
    {
        sleep(1);
        
        pthread_mutex_lock(&txMutex);

        m_tx_event |= NPE_HCI_TX_FLAG_TIMEOUT;

        pthread_cond_signal(&txCond);
        pthread_mutex_unlock(&txMutex);

    }
    return NULL;
}

/** @brief Executes TX Thread. Never returns.
 *
 * @param[in] vargp pointer to variable args - not used. 
 *
 * @return  NULL
 * 
 */
static void* npe_serial_interface_transmit_thread(void *vargp) 
{   

    while(1)
    {
        pthread_mutex_lock(&txMutex);
        // Check if this is the response we are watining for 
        pthread_cond_wait(&txCond, &txMutex);

        
        while(m_tx_event)
        {
            if(m_tx_event & NPE_HCI_TX_FLAG_SEND_MSG)
            {
                m_tx_event &= ~NPE_HCI_TX_FLAG_SEND_MSG;
                
                if(m_callback.transmit_message_cb)
                    m_callback.transmit_message_cb();
            }
            if(m_tx_event & NPE_HCI_TX_FLAG_TIMEOUT)
            {
                m_tx_event &= ~NPE_HCI_TX_FLAG_TIMEOUT;

                if(m_callback.timeout_cb)
                    m_callback.timeout_cb();
            }
            if(m_tx_event & NPE_HCI_TX_FLAG_RETRY)
            {
                m_tx_event &= ~NPE_HCI_TX_FLAG_RETRY;
                if(m_callback.retry_timeout_cb)
                    m_callback.retry_timeout_cb();
                
                pthread_join(retry_timer_thread_id, NULL);
            }
        }
        pthread_mutex_unlock(&txMutex);

       
    }
        
    return NULL;
}

/** @brief Executes RX Thread. Never returns.
 *
 * @param[in] vargp pointer to variable args - not used. 
 *
 * @return  NULL
 * 
 */
static void* npe_serial_interface_receive_thread(void *vargp) 
{ 
    uint8_t byte_buff[512];
    struct sp_event_set *eventSet;
    int err = sp_new_event_set(&eventSet);
    if(err != SP_OK)
    {
        printf("Unable to create new event set\n");
        return NULL;
    }

    err = sp_add_port_events(eventSet, port, SP_EVENT_RX_READY);
    if(err != SP_OK)
    {
        printf("Unable to add port events.\n");
        return NULL;
    }

    while(1) 
    {
        sp_wait(eventSet, 0);
        
        int byte_num = sp_nonblocking_read(port,byte_buff,512);
        if(byte_num > 0)
        {
            if(m_callback.parse_bytes_cb)
                m_callback.parse_bytes_cb(byte_buff,byte_num);
        }   
    } 
    return NULL; 
} 

/** @brief Connect to, open and configure COMPORT. 
 *
 * @param[in] p_port is a string that specifies the comport (e.g. "COM6").
 *
 * @return  ::NPE_GEM_RESPONSE_OK
 *          ::NPE_GEM_RESPONSE_SERIAL_NO_COMPORT
 *          ::NPE_GEM_RESPONSE_SERIAL_OPEN_FAIL
 *          ::NPE_GEM_RESPONSE_SERIAL_CONFIG_FAIL
 */
static uint32_t npe_serial_interface_connect_to_port(const char* p_port)
{
    uint32_t err_code = NPE_GEM_RESPONSE_OK;

    int err = sp_get_port_by_name(p_port, &port);
    if(err == SP_OK)
    {
        
        err = sp_open(port, SP_MODE_READ_WRITE);

        if(err == SP_OK)
        {
            // Configure comport - 
            // Baud: 115200
            // Data Bits: 8
            // Stop Bits: 1
            // Parity: None
            // Flow Control: None
            err = sp_set_baudrate(port,115200);
            if(err != SP_OK) return NPE_GEM_RESPONSE_SERIAL_CONFIG_FAIL;

            err = sp_set_flowcontrol (port, SP_FLOWCONTROL_NONE);
            if(err != SP_OK) return NPE_GEM_RESPONSE_SERIAL_CONFIG_FAIL;

            err = sp_set_bits(port, 8);
            if(err != SP_OK) return NPE_GEM_RESPONSE_SERIAL_CONFIG_FAIL;

            err = sp_set_stopbits(port, 1);
            if(err != SP_OK) return NPE_GEM_RESPONSE_SERIAL_CONFIG_FAIL;

            err = sp_set_parity(port, SP_FLOWCONTROL_NONE);
            if(err != SP_OK) return NPE_GEM_RESPONSE_SERIAL_CONFIG_FAIL;

            err = sp_set_dtr(port, SP_DTR_ON);
            if(err != SP_OK) return NPE_GEM_RESPONSE_SERIAL_CONFIG_FAIL;

            


        }
        else
        {
            err_code = NPE_GEM_RESPONSE_SERIAL_OPEN_FAIL;
        }   
    }
    else
    {
        err_code = NPE_GEM_RESPONSE_SERIAL_NO_COMPORT;
    }
    return (err_code);
}

/** @brief Initialize and open the comport. Defaults to 115200, 8 bits, 1 stop, no parity.
 *
 * @param[in] p_port is a string that specifies the comport (e.g. "COM6").
 * @param[in] p_callbacks is a struct of all callbacks.
 *
 * @return  ::NPE_GEM_RESPONSE_OK
 *          ::NPE_GEM_RESPONSE_SERIAL_NO_COMPORT
 *          ::NPE_GEM_RESPONSE_SERIAL_OPEN_FAIL
 *          ::NPE_GEM_RESPONSE_SERIAL_CONFIG_FAIL
 */
uint32_t npe_serial_interface_init(const char* p_port, npe_serial_interface_callbacks_t* p_callback)
{
    uint32_t err_code;

    assert(p_callback != NULL);

    if(p_callback->parse_bytes_cb)
    {
        m_callback.parse_bytes_cb = p_callback->parse_bytes_cb;
    } 

    if(p_callback->transmit_message_cb)
    {
        m_callback.transmit_message_cb = p_callback->transmit_message_cb;
    }
    if(p_callback->timeout_cb)
    {
        m_callback.timeout_cb = p_callback->timeout_cb;
    }
    if(p_callback->retry_timeout_cb)
    {
        m_callback.retry_timeout_cb = p_callback->retry_timeout_cb;
    }

    

    err_code = npe_serial_interface_connect_to_port(p_port);
    if(err_code == NPE_GEM_RESPONSE_OK)
    {
        // Receive thread setup
        int res = pthread_create(&rx_thread_id, NULL, npe_serial_interface_receive_thread, NULL);
        assert(res == 0);
        res = pthread_mutex_init(&rxMutex, NULL);
        assert(res == 0);
        res = pthread_cond_init (&rxCond, NULL);
        assert(res == 0);

        // Transmit thread
        res = pthread_create(&tx_thread_id, NULL, npe_serial_interface_transmit_thread, NULL);
        assert(res == 0);
        res = pthread_mutex_init(&txMutex, NULL);
        assert(res == 0);
        res = pthread_cond_init (&txCond, NULL);
        assert(res == 0);

        // Timer thread (for 1Hz timer)
        res = pthread_create(&timer_thread_id, NULL, npe_serial_interface_timer_thread, NULL);
        assert(res == 0);

    }

    return(err_code);
    //pthread_join(thread_id, NULL);
}

/** @brief Lists all ports available. Probably does not work on Android.
 * 
 */
void npe_serial_interface_list_ports(void) {
    int i;
    struct sp_port **ports;
    
    int error = sp_list_ports(&ports);
    if (error == SP_OK) {
        for (i = 0; ports[i]; i++) 
        {
            printf("Found port: '%s'\n", sp_get_port_name(ports[i]));
        }
        sp_free_port_list(ports);
    } 
    else 
    {
        printf("No serial devices detected\n");
    }
    printf("\n");
}

/** @brief Sends a single byte to the comport. Should be called in the TX thread context.
 *
 * @param[in] tx_byte is a a byte that will be sent to the comport. 
 *
 * @return  ::NPE_GEM_RESPONSE_OK
 *          ::NPE_GEM_RESPONSE_UNABLE_TO_WRITE
 */
uint32_t npe_serial_interface_send_byte(uint8_t tx_byte)
{
    uint32_t error_code = NPE_GEM_RESPONSE_OK;
    uint8_t buffer[1];
    buffer[0] = tx_byte;

    int bytesWrittenOrError = sp_blocking_write(port, buffer, 1, 0);
    if(bytesWrittenOrError <= 0)
    {
        error_code = NPE_GEM_RESPONSE_UNABLE_TO_WRITE;
    }
	return(error_code);
    
}

/** @brief Locks the TX thread if called from a context that is not the TX thread. If called from TX thread
 * nothing is locked and the function returns false. This function should be called before setting up 
 * a message to be transmitted to the serial port. 
 *
 * @return: true if called from thread not TX thread. False is called from TX thread. 
 * 
 */
bool npe_serial_transmit_lock(void)
{
    // Check if the calling function is
    // on the same thread as we are. If yes
    // then no need to put it on the thread. 
    if(tx_thread_id == pthread_self())
    {
        //pthread_mutex_unlock(&txMutex);
         return(false);
    }
    pthread_mutex_lock(&txMutex);
    return true;
}

/** @brief If the TX thread was locked (npe_serial_transmit_lock returns true) - call this 
 * function to transmit message and unlock the thread.  
 *
 */
void npe_serial_transmit_message_and_unlock(void)
{
    m_tx_event |= NPE_HCI_TX_FLAG_SEND_MSG;
    pthread_cond_signal(&txCond);
    pthread_mutex_unlock(&txMutex);
}

/** @brief Once a message is received on the RX thread, this function should be called
 * to signal any other threads waiting for this message. This function should be called in 
 * the context of the RX thread. 
 *
 * @param[in] handle_recieved_message_cb is a callback called before signaling that the message
 * was recieved. Allows shared data to be set in a protected way.
 * @param[in] p_arg isa pointer to the arguments for the function.
 * @param[in] size is that size of the arguments. 
 * 
 */
void npe_serial_interface_signal_response(handle_recieved_message_t handle_recieved_message_cb, void* p_arg, uint32_t size)
{
    pthread_mutex_lock(&rxMutex);

    if(handle_recieved_message_cb)
        handle_recieved_message_cb(p_arg, size);

    pthread_cond_signal(&rxCond);
    pthread_mutex_unlock(&rxMutex);
}

/** @brief Waits to receive a correctly formatted response from GEM. If a response is  
 * recieved then it will call the supplied callback, which can decide if the desired 
 * response was recieved. If the callback returns false the wait is retried up to 10 times
 * The wait for the response is WF_GEM_HCI_DEFAULT_MAX_COMMAND_RETRY_ATTEMPTS+1) * 
 * WF_GEM_HCI_DEFAULT_COMMAND_TIMEOUT_MS + 50.
 *
 * @param[in] check_if_wait_condition_met_cb is a callback called when a response recieved. 
 * It should return true is the desired response has been received.  
 *
 * @return  ::NPE_GEM_RESPONSE_OK
 *          ::NPE_GEM_RESPONSE_RETRIES_EXHAUSTED -> got responses but not the one we were looking for
 *          ::NPE_GEM_RESPONSE_TIMEOUT_OUT -> No response received within the timeout
 */
uint32_t npe_serial_interface_wait_for_response(check_if_wait_condition_met_cb_t check_if_wait_condition_met_cb)
{

    uint32_t error_code = NPE_GEM_RESPONSE_RETRIES_EXHAUSTED;
    uint8_t waitCount = 10; // Wait a maximum 10 times to get the correct response. 
    int timeInMs = (WF_GEM_HCI_DEFAULT_MAX_COMMAND_RETRY_ATTEMPTS+1) * WF_GEM_HCI_DEFAULT_COMMAND_TIMEOUT_MS + 50;
    struct timeval tv;
    struct timespec ts;
    int res;

    

    // TODO: Time out.
    pthread_mutex_lock(&rxMutex);
    
    // Check if this is the response we are watining for 
    while(waitCount-- > 0)
    {
        gettimeofday(&tv, NULL);
        ts.tv_sec = time(NULL) + timeInMs / 1000;
        ts.tv_nsec = tv.tv_usec * 1000 + 1000 * 1000 * (timeInMs % 1000);
        ts.tv_sec += ts.tv_nsec / (1000 * 1000 * 1000);
        ts.tv_nsec %= (1000 * 1000 * 1000);

        res = pthread_cond_timedwait(&rxCond, &rxMutex, &ts);

        if(res == 0)
        {
            error_code = NPE_GEM_RESPONSE_OK;
        }
        else if(res == ETIMEDOUT)
        {
            error_code = NPE_GEM_RESPONSE_TIMEOUT_OUT;
        }           
        else
        {
            // Otherwise something more serious went wrong. 
            assert(false);
        }
    
        if(check_if_wait_condition_met_cb)
        {
            if(check_if_wait_condition_met_cb())
            {       
                break;
            }
        }
            
    }
    pthread_mutex_unlock(&rxMutex);

    return error_code;
}

 /** @brief Start the retry timer.
 *
 * @param[in] msec is a 16 bit unsigned int specifying the timeout in millseconds. 
 * 
 */  
void npe_serial_interface_start_retry_timer(uint16_t msec)
{

    // Create thread for one-shot timer.
    int res = pthread_create(&retry_timer_thread_id, NULL, npe_serial_interface_retry_timer, (void*)&msec);
    assert(res == 0);
}

/** @brief Cancels the retry timer.
 * 
 */
void npe_serial_interface_cancel_retry_timer(void)
{
    int res = pthread_cancel(retry_timer_thread_id);
    assert(res == 0);
    pthread_join(retry_timer_thread_id, NULL);

}
   
