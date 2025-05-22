/*
 * Copyright (c) 2006-2024, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-04-08     RT-Thread    first version
 */

#include <rtthread.h>
#include <sys/select.h>

// Define logging system (ULOG or SYSLOG)
#ifndef ULOG_USING_SYSLOG
#define LOG_TAG              "Tick"      // Log tag for this module
#define LOG_LVL              LOG_LVL_DBG // Log level: Debug
#include <ulog.h>
#else
#include <syslog.h>
#endif /* ULOG_USING_SYSLOG */

// Thread configuration parameters
#define THREAD_STACK_SIZE   512 // Thread stack size in bytes
#define THREAD_PRIORITY     20  // Thread priority (lower number = higher priority)

// Global counter variable
static int count = 0;

/**
 * @brief Thread entry function
 * @param parameter Thread parameter (unused in this case)
 * 
 * This thread increments a counter every second and logs the value.
 */
static void thread_entry(void *parameter) {
    while (1) {
        count++;
        LOG_D("Tick - %d",count); // Log current tick count
        
        // Example of conditional execution (commented out)
        if (count == 1) {
           // ulog_example(); // Example ULOG function call
        }
        
        rt_thread_mdelay(1000); // Delay for 1 second (1000ms)
    }
}

/**
 * @brief Main application entry point
 * @return Always returns 0
 * 
 * Currently empty as initialization is done in tick_init()
 */
int main(void) {
    return 0;
}

/**
 * @brief Initialize the tick counter thread
 * 
 * Creates and starts a thread that increments a counter every second.
 * The thread is registered as a command in the RT-Thread shell.
 */
void tick_init() {
    rt_thread_t tid; // Thread handle

    // Create thread with specified parameters
    tid = rt_thread_create("local",       // Thread name
                         thread_entry,   // Entry function
                         RT_NULL,        // Parameter
                         THREAD_STACK_SIZE, // Stack size
                         THREAD_PRIORITY,   // Priority
                         20);               // Time slice

    // Start thread if creation was successful
    if (tid != RT_NULL) {
        rt_thread_startup(tid);
    }
}
// Register tick_init as a shell command test
MSH_CMD_EXPORT(tick_init, "Run tick counter example with ULOG output")


