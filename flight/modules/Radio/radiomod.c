
#include <pios.h>
#include <uavobjectmanager.h>
#include <openpilot.h>
#include <radiostatus.h>
#include <taskinfo.h>
#include <pios_rfm22b.h>
#include <pios_board_info.h>
#include <pios_board.h>
#include <radiomodthread.h>

// Private constants
#define SYSTEM_UPDATE_PERIOD_MS 1000

#if defined(PIOS_SYSTEM_STACK_SIZE)
#define STACK_SIZE_BYTES        PIOS_SYSTEM_STACK_SIZE
#else
#define STACK_SIZE_BYTES        924
#endif

#define TASK_PRIORITY           (tskIDLE_PRIORITY + 2)
/**
 * Create the module task.
 * \returns 0 on success or -1 if initialization failed
 */
int32_t RadioModStart(void)
{
  //    RadioModPeriodicInitialize();
  RadioModThreadInitialize();
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
  //    RadioModStart();

    return 0;
}

MODULE_INITCALL(RadioModInitialize, RadioModStart);

