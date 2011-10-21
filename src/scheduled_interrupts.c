/*
 ============================================================================
 Name        : scheduled_interrupts.c
 Author      : Kevin Peterson
 Version     : 0.5.0
 Copyright   : GPL
 Description : Main Implementation
 ============================================================================
 */
#include <stdlib.h>
#include <stdio.h>
#include <avr/interrupt.h>
#include "scheduled_interrupts.h"

#ifndef COUNTER_OVERFLOW
#define COUNTER_OVERFLOW 256
#endif

/*
Timer bits
0000           00000        0000000
12-15-Period   7-11-Frame   0-6-Time
*/
#define NUM_OF_FRAMES 5
#define NUM_OF_PERIODS 4
#define FRAME_OFFSET 7
#define PERIOD_OFFSET 12
#define TIME_MASK 0x7F
#define FRAME_MASK 0xF80
#define PERIOD_MASK 0xF000

void on_frame_interrupt();
#ifndef __AVR__
void on_motor_pwm_interrupt();
void on_servo_pwm_interrupt();
#endif

volatile scheduled_interrupt* get_next_interrupt();
INT8 compare_interrupts(scheduled_interrupt* a,
		scheduled_interrupt* b);
void sort_interrupts();
void clearArray(scheduled_interrupt* array[]);
void copyArray(scheduled_interrupt* arraySource[], scheduled_interrupt* arrayTarget[]);

#ifndef NUM_OF_INTERRUPTS
#define NUM_OF_INTERRUPTS 10
#endif

scheduled_interrupt frame_interrupt = {0,on_frame_interrupt };

volatile scheduled_interrupt* scheduled_interrupts[NUM_OF_INTERRUPTS];
volatile scheduled_interrupt* queued_scheduled_interrupts[NUM_OF_INTERRUPTS];
scheduled_interrupt* queued_interrupt = NULL;

volatile INT8 current_frame = 0;
volatile INT8 current_period = 0;
volatile INT8 current_interrupt_pos = 0;
volatile INT8 total_frame_time = 0;

void on_frame_interrupt(){

	if(current_frame < (NUM_OF_FRAMES - 1)){
		current_frame++;
	} else {
		current_frame = 0;

		if(current_period < (NUM_OF_PERIODS - 1)){
			current_period++;
		} else {
			current_period = 0;
		}
	}
	current_interrupt_pos = 0;
	total_frame_time = 0;
}

void init_interrupt_scheduler(){
	clearArray(scheduled_interrupts);
	clearArray(queued_scheduled_interrupts);

	queued_scheduled_interrupts[0] = &frame_interrupt;

	set_interrupt_schedule_frame(
			&frame_interrupt,
			ALL_FRAMES);

	set_interrupt_schedule_period(
			&frame_interrupt,
			ALL_PERIODS);

	set_interrupt_schedule_time(
			&frame_interrupt,
			127);
}

void clearArray(scheduled_interrupt* array[]){
	INT8 i;
	for(i=0;i<NUM_OF_INTERRUPTS;i++){
		array[i] = NULL;
	}
}

void copyArray(scheduled_interrupt* arraySource[], scheduled_interrupt* arrayTarget[]){
	clearArray(arrayTarget);

	INT8 i;
	for(i=0;i<NUM_OF_INTERRUPTS;i++){
		arrayTarget[i] = arraySource[i];
	}
}


void register_interrupt(
		INT8 time,
		INT16 frame,
		INT16 period,
		void(*interrupt_function)(void),
		scheduled_interrupt* interrupt){

	set_interrupt_schedule_time(interrupt, time);
	set_interrupt_schedule_frame(interrupt, frame);
	set_interrupt_schedule_period(interrupt, period);

	interrupt->interrupt_function = interrupt_function;

	INT8 i;
	for(i=0;i<NUM_OF_INTERRUPTS;i++){
		if(queued_scheduled_interrupts[i] == NULL){
			queued_scheduled_interrupts[i] = interrupt;
			break;
		}
	}
}

void on_interrupt(volatile INT8* time_counter){

	if(queued_interrupt != NULL){
		queued_interrupt->interrupt_function();
	}

	scheduled_interrupt* interrupt = get_next_interrupt();

	INT8 time = ((interrupt->schedule & TIME_MASK) << 1);

	*time_counter = (255 - (time - total_frame_time));

	total_frame_time = time;

	queued_interrupt = interrupt;
}

void set_interrupt_schedule_time(
		volatile scheduled_interrupt* interrupt, INT8 time){
	interrupt->schedule =
			((interrupt->schedule & ~TIME_MASK) | time);
}

void set_interrupt_schedule_frame(
		volatile scheduled_interrupt* interrupt, INT16 frame){
	interrupt->schedule =
			((interrupt->schedule & ~FRAME_MASK) | (frame << FRAME_OFFSET));
}

void set_interrupt_schedule_period(
		volatile scheduled_interrupt* interrupt, INT16 period){
	interrupt->schedule =
			((interrupt->schedule & ~PERIOD_MASK) | (period << PERIOD_OFFSET));
}

volatile scheduled_interrupt* get_next_interrupt(){

	for(;current_interrupt_pos<NUM_OF_INTERRUPTS;
			current_interrupt_pos++){

		if(
				((1 << (current_period + PERIOD_OFFSET))
						&
						(scheduled_interrupts[current_interrupt_pos]->schedule))
						&&
				((1 << (current_frame + FRAME_OFFSET))
						&
						(scheduled_interrupts[current_interrupt_pos]->schedule)) ){

			return
				scheduled_interrupts[current_interrupt_pos++];
		}
	}

   return NULL;
}

void sort_interrupts() {
	INT8 k;
	for (k = 1; k < NUM_OF_INTERRUPTS; ++k) {
		volatile scheduled_interrupt* key = queued_scheduled_interrupts[k];
		if(key == NULL){
			break;
		}
		INT8 i = k - 1;
		while ((i >= 0) && (
				(key->schedule & TIME_MASK) <
				(queued_scheduled_interrupts[i]->schedule & TIME_MASK))) {
			queued_scheduled_interrupts[i + 1] = queued_scheduled_interrupts[i];
			--i;
		}
		queued_scheduled_interrupts[i + 1] = key;
	}

	cli();
	copyArray(queued_scheduled_interrupts, scheduled_interrupts);
	sei();
}
