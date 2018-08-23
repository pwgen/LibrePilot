#include <pios.h>

#include <uavobjectmanager.h>
#include <openpilot.h>

#include <radiosettings.h>
#include <radiostatus.h>
#include <radio.h>
#include <taskinfo.h>

#include <pios_rfm22b.h>
#include <pios_board_info.h>


// Private constants
#define SYSTEM_UPDATE_PERIOD_MS 1000

#if defined(PIOS_SYSTEM_STACK_SIZE)
#define STACK_SIZE_BYTES        PIOS_SYSTEM_STACK_SIZE
#else
#define STACK_SIZE_BYTES        924
#endif

#define TASK_PRIORITY           (tskIDLE_PRIORITY + 2)
#define MAX_QUEUE_SIZE 20
// Private types

// Private variables
static xTaskHandle radiosystemTaskHandle;
//static xQueueHandle queue;
static bool stackOverflow;
static bool mallocFailed;
volatile int initRadioTaskDone = 0;
//static uint8_t previousStat;

// Private functions
static void radiosystemTask(void *parameters);
static void radioSettingsUpdatedCb(UAVObjEvent *ev);

/**
 * Create the module task.
 * \returns 0 on success or -1 if initialization failed
 */
int32_t RadioModStart(void)
{
    // Initialize vars
    stackOverflow = false;
    mallocFailed  = false;
//    queue = xQueueCreate(MAX_QUEUE_SIZE, sizeof(UAVObjEvent));
    // Create oplink system task
    xTaskCreate(radiosystemTask, "Radio", STACK_SIZE_BYTES / 4, NULL, TASK_PRIORITY, &radiosystemTaskHandle);
    // Register task
    PIOS_TASK_MONITOR_RegisterTask(TASKINFO_RUNNING_SYSTEM, radiosystemTaskHandle);

    return 0;
}

/**
 * Initialize the module, called on startup.
 * \returns 0 on success or -1 if initialization failed
 */
int32_t RadioModInitialize(void)
{
    // Must registers objects here for system thread because ObjectManager started in OpenPilotInit

    // Call the module start function.

         //
         //PIOS_LED_Toggle(1);
   RadioModStart();

    return 0;
}

//MODULE_INITCALL(RadioModInitialize, RadioModStart);
MODULE_INITCALL(RadioModInitialize, 0);

/**
 * System task, periodically executes every SYSTEM_UPDATE_PERIOD_MS
 */
static void radiosystemTask(__attribute__((unused)) void *parameters)
{
    portTickType lastSysTime;
/*    uint16_t prev_tx_count = 0;
    uint16_t prev_rx_count = 0;
    uint16_t prev_tx_seq   = 0;
    uint16_t prev_rx_seq   = 0; */
 //   bool first_time = true;
    RadioStatusData radioStat;

    /*while (!initRadioTaskDone) {
        vTaskDelay(10);
    }*/




#ifndef PIOS_INCLUDE_WDG
// if no watchdog is enabled, don't reset watchdog in MODULE_TASKCREATE_ALL loop
#define PIOS_WDG_Clear()
#endif
    /* create all modules thread */
    MODULE_TASKCREATE_ALL;

    /* start the delayed callback scheduler */
    PIOS_CALLBACKSCHEDULER_Start();

//    if (mallocFailed) {
        /* We failed to malloc during task creation,
         * system behaviour is undefined.  Reset and let
         * the BootFault code recover for us.
         */
  //      PIOS_SYS_Reset();
   // }
    PIOS_LED_Toggle(3);

    // Initialize previousRFXtalCap used by callback
   // RadioSettingsStatGet(&previousStat);
    RadioSettingsConnectCallback(radioSettingsUpdatedCb);

    // Initialize vars
 //   lastSysTime = xTaskGetTickCount();

   // if (first_time) {
   //                 first_time = false;
    //                }

    // Main system loop
    while (1) {
         //      LINK_LED_TOGGLE;
         PIOS_LED_Toggle(3);
         vTaskDelay(200);
          }

    while (1) {
        // Update the OPLinkStatus UAVO

        RadioStatusGet(&radioStat);

        // Get the stats from the radio device
        //struct rfm22b_stats radio_stats;
        //PIOS_RFM22B_GetStats(pios_rfm22b_id, &radio_stats);
        //radioStat.HeapRemaining = xPortGetFreeHeapSize();
        radioStat.Stat = radioStat.Stat +1;

        // Update the object
        RadioStatusSet(&radioStat);

        // Wait until next period
        vTaskDelayUntil(&lastSysTime, SYSTEM_UPDATE_PERIOD_MS / portTICK_RATE_MS);
    }
}

/**
 * Called by the RTOS when the CPU is idle, used to measure the CPU idle time.
 */

/**
 * Called whenever OPLink settings changed
 */

static void radioSettingsUpdatedCb(__attribute__((unused)) UAVObjEvent *ev)
{
   uint8_t currentStat;

    RadioSettingsStatGet(&currentStat);

    // Check if RFXtalCap value changed
    //if (currentStat != previousStat) {
     //   PIOS_RFM22B_SetXtalCap(pios_rfm22b_id, currentStat);
       // PIOS_RFM22B_Reinit(pios_rfm22b_id);
       // previousStat = currentStat;
    //}

}


// Private constants


/**
 * Create the module task.
 * \returns 0 on success or -1 if initialization failed
int32_t RadioModStart(void)
{
  //    RadioModPeriodicInitialize();
  RadioModThreadInitialize();
  return 0;
}

int32_t RadioModInitialize(void)
{
    // Must registers objects here for system thread because ObjectManager started in OpenPilotInit

    // Call the module start function.
  //    RadioModStart();

    return 0;
}

MODULE_INITCALL(RadioModInitialize, RadioModStart);

*/
