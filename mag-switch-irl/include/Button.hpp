#pragma once

#include "freertos/queue.h"
#include "driver/gpio.h"
#include "ThreadBase.hpp"

namespace CVL
{

class Button : public ThreadBase
{
	protected:
    gpio_num_t pin;
    int64_t debounce_ms;
	QueueHandle_t gpio_evt_queue;
    int64_t press_time;
    int64_t release_time;
    uint8_t flags;

    public:
    static const int64_t DEFAULT_DEBOUNCE_MS = 100ll;
    static const uint8_t INVERTED_FLAG = 1;
    static const uint8_t NEXT_FLAG = 2;

    Button( gpio_num_t pin, uint8_t flags_ = 0 );
    inline uint32_t gpio_pin_sel(void) { return 1ULL<<pin; }
    inline bool is_inverted(void) { return (bool)( flags & INVERTED_FLAG ); }
    inline int get_level( void ) { return gpio_get_level( pin ) ^ ( flags & INVERTED_FLAG ); }

    struct Event
    {
        int64_t event_time;
    };

    protected:
    virtual void init_pin(void);
    virtual void init_queue(void);
    virtual void init_isr(void);
	static void isr_handler(void* arg);
	virtual void loop(void);
    virtual void main(void);
    virtual void handle_button_press( int64_t delta );
    virtual void handle_button_event( Event e );
    virtual void handle_button_timeout( void );
};

class ToggleButton : public Button
{
    public:
    static const uint8_t PAUSE_FLAG = Button::NEXT_FLAG;
    static const uint8_t NEXT_FLAG = PAUSE_FLAG << 1;
    ToggleButton( gpio_num_t pin, uint8_t flags = 0 );
    inline bool get_state( void ) { return (bool)( flags & ToggleButton::PAUSE_FLAG ); }

    protected:
    virtual void handle_button_press( int64_t delta );
};

}