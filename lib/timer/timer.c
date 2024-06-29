#include "timer.h"

void timerDisable()
{
	// Disable Timer1 interrupt
	TIMSK1 &= ~(1 << OCIE1A);

	// Stop Timer1
	TCCR1B &= ~((1 << CS12) | (1 << CS10));
}

void timerInit(uint16_t timeout_ms)
{
	TCCR1B |= (1 << WGM12) | (1 << CS12) | (1 << CS10);
	OCR1A = (F_CPU / 1000 / 1024) * timeout_ms - 1;
	TIMSK1 |= (1 << OCIE1A);
	sei();
}

void timerReset()
{
	TCNT1 = 0;
}

void timerInit1s()
{
	// Set Timer1 to CTC mode (Clear Timer on Compare Match)
	TCCR1B |= (1 << WGM12);

	// Set compare value for 1 second interrupt
	OCR1A = 15624;

	// Enable Timer1 compare interrupt
	TIMSK1 |= (1 << OCIE1A);

	// Start Timer1 with prescaler 1024
	TCCR1B |= (1 << CS12) | (1 << CS10);

	sei(); // Enable global interrupts
}

void timerInit2s()
{
	// Set Timer1 to CTC mode (Clear Timer on Compare Match)
	TCCR1B |= (1 << WGM12);

	// Set compare value for 2 second interrupt
	OCR1A = 31249;

	// Enable Timer1 compare interrupt
	TIMSK1 |= (1 << OCIE1A);

	// Start Timer1 with prescaler 1024
	TCCR1B |= (1 << CS12) | (1 << CS10);

	sei(); // Enable global interrupts
}