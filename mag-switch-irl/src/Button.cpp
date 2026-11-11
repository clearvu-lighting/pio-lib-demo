#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
#include "driver/gpio.h"
#ifndef CLEAR_VU_MESHTASTIC
#include "driver/gpio_filter.h"
#undef NOGLITCH
#else
#define NOGLITCH
#endif
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"
#include "Button.hpp"

#define ESP_INTR_FLAG_DEFAULT 0

namespace CVL
{

Button::Button( gpio_num_t pin_, uint8_t flags_ )
	: ThreadBase( "BUTTON" )
    , pin( pin_ )
    , debounce_ms( 100ll )
    , gpio_evt_queue( NULL )
    , press_time( 0ll )
    , release_time( 0ll )
    , flags( flags_ ) 
{
    char tag_[ TAG_WIDTH+1 ];
    size_t len_ = snprintf( tag_, TAG_WIDTH, "[BUTTON] (%u)", pin_ );
    tag_[len_] = 0;
    set_tag( tag_ );
}

extern "C"
void IRAM_ATTR Button::isr_handler(void* arg)
{
    Button *me = (Button*)arg;
    gpio_intr_disable(me->pin);
    Event e;
    e.event_time = esp_timer_get_time();
    xQueueSendFromISR( me->gpio_evt_queue, &e, NULL );
    gpio_intr_enable(me->pin);
}

void Button::handle_button_press( int64_t delta )
{
    LOGI( "%s Button pressed for %f seconds", tag, 1.0 * delta / 1000000 );
    press_time = release_time = 0ll;
}

void Button::handle_button_event( Event e )
{
    uint64_t now = esp_timer_get_time();
    int button_level = get_level();
    LOGD( "%s Button level=%d, event_time=%lld, now=%lld, press_time=%lld, release_time=%lld", tag, button_level, e.event_time, now, press_time, release_time );
    if( button_level == 1 ) // Button press event
    {
        if( press_time == 0ll ) { press_time = now; LOGD( "%s New button press", tag ); }
        else LOGD( "%s Continue as same press. Delta = %f", tag, 1.0 * ( now - press_time ) / 1000000 );
    }
    else if ( press_time ) { release_time = now; LOGD( "%s Early release.", tag ); }
    else LOGD( "%s Stray tigger event", tag );
}

void Button::handle_button_timeout( void )
{
    uint64_t now = esp_timer_get_time();
    int button_level = get_level();
    int64_t delta;
    if( button_level == 1 ) { delta = now - press_time; LOGD( "%s Button held for %f seconds so far", tag, 1.0 * delta / 1000000 ); }
    else if( press_time )
    {
        if( release_time == 0ll ) { release_time = now; LOGD( "%s Wait one more timout, just in case.", tag ); }
        else
        {
            delta = release_time - press_time;
            handle_button_press( delta );
        }
    }
}

void Button::loop(void)
{
    Event e;
    while(1)
    {
        if( xQueueReceive( gpio_evt_queue, &e, debounce_ms / portTICK_PERIOD_MS ) )
            handle_button_event( e );
        else
            handle_button_timeout();
    }
}

void Button::init_pin(void)
{
    LOGI( "%s flags=%x", tag, flags );
    esp_err_t e = ESP_OK;

    #ifndef NOGLITCH
    gpio_pin_glitch_filter_config_t config = { .clk_src = GLITCH_FILTER_CLK_SRC_DEFAULT, .gpio_num = pin };
    gpio_glitch_filter_handle_t filter;
    e = gpio_new_pin_glitch_filter(&config,&filter);
    ESP_ERROR_CHECK(e);
    e = gpio_glitch_filter_enable(filter);
    ESP_ERROR_CHECK(e);
    #endif

    gpio_config_t io_conf = {};
    io_conf.intr_type = GPIO_INTR_POSEDGE;
    io_conf.pin_bit_mask = 1ULL << (uint32_t)pin;
    LOGD( "%s bit_mask=%llx", tag, io_conf.pin_bit_mask );
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    io_conf.pull_down_en = GPIO_PULLDOWN_ENABLE;
    e = gpio_config(&io_conf);
    ESP_ERROR_CHECK(e);
}

void Button::init_queue(void)
{
    //create a queue to handle gpio event from isr
    gpio_evt_queue = xQueueCreate( 10, sizeof(Event) );
}

void Button::init_isr(void)
{
    //install gpio isr service
    gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);

    //hook isr handler for specific gpio pin
    gpio_isr_handler_add( pin, isr_handler, (void*)this );
}

//now wait for messages from the interrupt
void Button::main(void)
{
    init_pin();
    init_queue();
    init_isr();
    loop();
}

ToggleButton::ToggleButton( gpio_num_t pin, uint8_t flags )
    : Button( pin, flags )
{
}

void ToggleButton::handle_button_press( int64_t delta )
{
    Button::handle_button_press( delta );
    LOGI( "%s pressed.", tag );
    flags ^= PAUSE_FLAG;
}

}