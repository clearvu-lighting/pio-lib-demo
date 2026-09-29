#pragma once
#include <Arduino.h>
#include "qqqDALI.h"

#ifdef WS_DALI_TX_PIN
#define TX_PIN WS_DALI_TX_PIN
#else
#define TX_PIN 17
#endif
#ifdef WS_DALI_RX_PIN
#define RX_PIN WS_DALI_RX_PIN
#else
#define RX_PIN 14
#endif

extern uint8_t DALI_Addr[64];
extern uint8_t DALI_NUM;



void DALI_Init();                                                         // Example Initialize the DALI bus
void InitTimer(void);

void Blinking_ALL();                                                      // All lights on the bus flash
void Lighten_ALL();                                                       // Light all lamps on the bus
void Extinguish_ALL();                                                    // Turn off all lights on the bus
void Luminaire_Brightness(uint8_t Light, uint8_t addr);                   // Luminaires with addresses addr (0 to 63) on the bus are set to Light(0 to 100)%
void Scan_DALI_addr_ALL();                                                // Scan all devices on the bus
void Scan_DALI_addr_ALL_DT6();
void Delete_DALI_addr_ALL();                                              // Delete the addresses of all devices on the bus   
void Assign_new_address_ALL();                                            // Reassign addresses to all devices on the bus
