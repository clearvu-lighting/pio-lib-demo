/*
 * This will read a potentiometer as a dial and return a value
 * frmo 0 to 10 on the dial.
 */

#include "ThreadBase.hpp"
#include <cstring>
#include <algorithm>
#include <esp_system.h>

using namespace std;
namespace CVL
{

#ifndef CONFIG_ESP_MAIN_TASK_STACK_SIZE
#define CONFIG_ESP_MAIN_TASK_STACK_SIZE 3096
#endif

// static:
const char *ThreadBase::default_tag = "ThreadBase";
unsigned ThreadBase::one_time_flag = 0;
ThreadBase::thread_list_t ThreadBase::thread_list;
BaseType_t ThreadBase::default_core = tskNO_AFFINITY;

ThreadBase::ThreadBase(const char *tag_)
    : Logger(tag_)
    , stack_size(CONFIG_ESP_MAIN_TASK_STACK_SIZE)
    , core(default_core)
    , handle(NULL)
    , pause_state(0)
{
}

ThreadBase::ThreadBase(const std::string &tag_)
    : Logger(tag_), stack_size(CONFIG_ESP_MAIN_TASK_STACK_SIZE), core(default_core), handle(NULL), pause_state(0)
{
}

bool ThreadBase::set_core(const BaseType_t new_core)
{
    if (handle != NULL)
        return false;
    core = new_core;
    return true;
}

bool ThreadBase::set_stack_size(const size_t new_stack_size)
{
    if (handle != NULL)
        return false;
    stack_size = new_stack_size;
    return true;
}

void ThreadBase::delay(TickType_t delay_millisec)
{
    // LOGD("Delay called in state %x.", pause_state);
    if ((pause_state & PAUSE_BIT) == 0)
    {
        //LOGD("Delaying for %lu milliseconds.", delay_millisec);
        pause_state |= DELAY_BIT;
        vTaskDelay(delay_millisec / portTICK_PERIOD_MS);
        //LOGD("State after delay is %x.", pause_state);
        pause_state &= ~DELAY_BIT;
        //LOGD("State after mask with %x is %x.", ~DELAY_BIT, pause_state);
    }
    if ((pause_state & PAUSE_BIT) != 0)
        suspend();
}

void ThreadBase::pause(void)
{
    pause_state |= PAUSE_BIT;
    LOGI("Pause requsted. New state is %x.", pause_state);
}

void ThreadBase::resume(void)
{
    //LOGD("Resume requsted with state=%x.", pause_state);
    if ((pause_state & SUSPENDED_BIT) != 0)
    {
        //LOGD("Calling vTaskResume.");
        vTaskResume(handle);
    }
    pause_state &= ~(PAUSE_BIT | SUSPENDED_BIT);
}

#include "esp_chip_info.h"
#include "esp_flash.h"

// static public:
void ThreadBase::dump_all_thread_info(void)
{
    const char *tag = default_tag;
    LOGI( "[%s] There are %u threads.", default_tag, thread_list.size());
    // for( ThreadBase *t : thread_list )
    for (unsigned t = 0; t < thread_list.size(); ++t)
    {
        thread_list[t]->dump_thread_info(t + 1);
    }
}

// static protected
void ThreadBase::dump_cpu_info(void)
{
    const char *tag = default_tag;
    if (one_time_flag & CPU_INFO_DONE)
        return;

    /* Print chip information */
    esp_chip_info_t chip_info;
    uint32_t flash_size;
    esp_chip_info(&chip_info);
    LOGI("This is %s chip with %d CPU core(s), %s%s%s%s, ",
             CONFIG_IDF_TARGET,
             chip_info.cores,
             (chip_info.features & CHIP_FEATURE_WIFI_BGN) ? "WiFi/" : "",
             (chip_info.features & CHIP_FEATURE_BT) ? "BT" : "",
             (chip_info.features & CHIP_FEATURE_BLE) ? "BLE" : "",
             (chip_info.features & CHIP_FEATURE_IEEE802154) ? ", 802.15.4 (Zigbee/Thread)" : "");

    unsigned major_rev = chip_info.revision / 100;
    unsigned minor_rev = chip_info.revision % 100;
    LOGI("silicon revision v%d.%d, ", major_rev, minor_rev);
    if (esp_flash_get_size(NULL, &flash_size) != ESP_OK)
    {
        LOGI("Get flash size failed");
        return;
    }
    LOGI("%luMB %s flash", flash_size / (uint32_t)(1024 * 1024),
             (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");

    LOGI("Minimum free heap size: %lu bytes", esp_get_minimum_free_heap_size());

    one_time_flag |= CPU_INFO_DONE;
}

void ThreadBase::dump_thread_info(unsigned thread_number)
{
    TaskStatus_t taskDetails;
#if CONFIG_FREERTOS_USE_TRACE_FACILITY == 1
    vTaskGetInfo(handle, &taskDetails, pdTRUE, eInvalid);
#else
    taskDetails.pxStackBase = 0;
    taskDetails.usStackHighWaterMark = 0;
    taskDetails.uxCurrentPriority = 0;
    taskDetails.pcTaskName = "NONAME";
#endif
    LOGI(
        "Task #%u: name=%s handle=%#lx size=%u base=%#lx highwater=%#lx priority=%u",
        thread_number,
        taskDetails.pcTaskName,
        (unsigned long)handle,
        stack_size,
        (unsigned long)taskDetails.pxStackBase,
        (unsigned long)taskDetails.usStackHighWaterMark,
        (unsigned)taskDetails.uxCurrentPriority);
}

extern "C" void ThreadBase::run(void *param_p)
{
    const char *tag = default_tag;
    ThreadBase *t = (ThreadBase *)param_p;
    LOGI("Thread %s started. Calling main()", t->tag);
    t->dump_thread_info(0);
    t->main();
    LOGI("Thread %s ending. Returned from main().", t->tag);
    t->dump_thread_info(0);
    thread_list_t::iterator i;
    i = find(thread_list.begin(), thread_list.end(), t);
    assert(i != thread_list.end());
    thread_list.erase(i);
    LOGI("Thread count now %u", thread_list.size());
    vTaskDelete(NULL);
}

bool ThreadBase::start(void)
{
    dump_cpu_info();
    UBaseType_t priority = uxTaskPriorityGet(NULL);
    LOGI("Calling xTaskCreatePinnedToCore: name=%s priority=%d stack_size=%u core=%d", tag, priority, stack_size, (int)core);
    if (xTaskCreatePinnedToCore(run, tag, stack_size, this, priority, &handle, core) == pdPASS)
    {
        thread_list.push_back(this);
        LOGI("Task created. count=%u.", thread_list.size());
        return true;
    }
    return false;
}

void ThreadBase::suspend(void)
{
    //LOGD("Suspending thread.");
    pause_state &= ~PAUSE_BIT;
    pause_state |= SUSPENDED_BIT;
    vTaskSuspend(handle);
    pause_state &= ~(SUSPENDED_BIT | PAUSE_BIT);
    //LOGD("Thread resuming from suspend state.");
}

}