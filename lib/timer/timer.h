#ifndef __TIMER_H_
#define __TIMER_H_
#include <stdint.h>
#include <avr/boot.h>
#include <avr/interrupt.h>

void timerDisable(void);
void timerInit(uint16_t timeout_ms);
void timerInit1s(void);
void timerReset(void);
#endif