/* ThreadBase */
#pragma once 

#include "Logger.hpp"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <vector>

namespace CVL
{

class ThreadBase : public Logger
{
    /* Base your threads on this class to use ESP's FreeRTOS threading.
     *
     * Call "start()" from your program to start this class as it's own thread.
     * You can only do this once per instance of this class.
     * 
     * Internally, start will register the static function "run" as the thread callback.
     * Subclasses should leave the "run" function alone. The "run" funcion will call the virtual 
     * instance function "main". You must define your own "main" function. 
     * 
     * In your logic, use "delay" and "suspend" instead of using the FreeRTOS calls directly
     * so that this class can manage the state bits for you.
     * 
     * The "pause" call is for other threads to use to pause your thread. 
    
     * If you override "delay", "suspend", or "pause", be sure to call the super or you might 
     * break the state bit logic.
     */
	public:
    static const uint8_t PAUSE_BIT = 0x80;          // Set when a pause has been requested.
    static const uint8_t SUSPENDED_BIT = 0x40;      // Set when the thread is actually suspended. Internal use only!!!
    static const uint8_t DELAY_BIT = 0x20;          // Set when the thread is suspended due to a timer.
    static const uint8_t END_BIT = 0x10;            // Set when the thread is finished and should end.
    static const uint8_t SUSPEND_LOGIC_MASK = 0xF0; // Internal use only!!!

    enum one_time_flag_t 
    {
        CPU_INFO_DONE = 1
    };
    typedef std::vector<ThreadBase*> thread_list_t;

    protected:
	static const char *       default_tag; 
    static unsigned           one_time_flag;
    static thread_list_t      thread_list;
    static BaseType_t         default_core;
    size_t                    stack_size;
    BaseType_t                core;
    TaskHandle_t              handle;           // This is the thread's handled from xTaskCreate().
	uint8_t                   pause_state;      // This track suspend/sleep logic. 

    public:
    ThreadBase( const char *tag = NULL );
    ThreadBase( const std::string &tag );

    static void dump_all_thread_info(void);
    virtual bool start(void);
    virtual void pause(void);
    virtual void resume(void);
    inline bool paused(void) { return ( pause_state & ( PAUSE_BIT | SUSPENDED_BIT ) ) != 0; }

    inline size_t get_stack_size( void ) { return stack_size; }
    virtual bool set_stack_size( const size_t new_stack_size );

    inline BaseType_t get_core( void ) { return core; }
    virtual bool set_core( const BaseType_t new_core );

    protected:
    static void run( void *param_p );
    virtual void main(void) = 0; 
    virtual void delay( TickType_t delay_millisec );
    static void dump_cpu_info(void);
    void dump_thread_info(unsigned thread_number);
    virtual void suspend(void);
};

template< typename Telemetry >
class TelemetryThreadBase : public ThreadBase
{
    private:
        SemaphoreHandle_t mutex;
        Telemetry data;
    public:
        TelemetryThreadBase( const char *tag = NULL )
        :   ThreadBase( tag )
        ,   mutex( NULL )
        {
            mutex = xSemaphoreCreateMutex();
            assert( mutex );
        }
        TelemetryThreadBase( const std::string &tag )
        :   ThreadBase( tag )
        ,   mutex( NULL )
        {
            mutex = xSemaphoreCreateMutex();
            assert( mutex );
        }
        Telemetry getTelemetry( bool wait = true )
        {
            Telemetry rv = {};
            if( xSemaphoreTake(mutex,wait?portMAX_DELAY:0) )
            {
                rv = data;
                xSemaphoreGive(mutex);
            }
            return rv;
        }
        void setTelemetry( const Telemetry &t, bool wait = true )
        {
            if( xSemaphoreTake(mutex,wait?portMAX_DELAY:0) )
            {
                data = t;
                xSemaphoreGive(mutex);
            }
        }
};

}