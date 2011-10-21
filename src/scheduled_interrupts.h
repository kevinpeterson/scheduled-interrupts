/*
 ============================================================================
 Name        : scheduled_interrupts.c
 Author      : Kevin Peterson
 Version     : 0.5.0
 Copyright   : GPL
 Description : Main Header File
 ============================================================================
 */
#define ZERO 0b1
#define ONE 0b10
#define TWO 0b100
#define THREE 0b1000
#define FOUR 0b10000
#define FIVE 0b100000
#define ALL_PERIODS 0x0F
#define ALL_FRAMES 0x1F

#define INT8 uint8_t
#define INT16 uint16_t

typedef volatile struct scheduled_interrupt {
	INT16 schedule;
	void(*interrupt_function)(void);
} scheduled_interrupt;

void on_interrupt(volatile INT8* time_counter);

void sort_interrupts();

void set_interrupt_schedule_time(
		volatile scheduled_interrupt* interrupt, INT8 time);
void set_interrupt_schedule_frame(
		volatile scheduled_interrupt* interrupt, INT16 frame);
void set_interrupt_schedule_period(
		volatile scheduled_interrupt* interrupt, INT16 period);

void init_interrupt_scheduler();

void register_interrupt(
		INT8 time,
		INT16 frame,
		INT16 period,
		void(*interrupt_function)(void),
		scheduled_interrupt* interrupt);
