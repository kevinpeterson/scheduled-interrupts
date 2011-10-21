scheduled-interupts - AVR Interrupt Scheduler
======================

scheduled-interupts is an AVR interrupt scheduling tool used
schedule multiple events using a single AVR timer routine.

How it works
==================================
* Create an arbitrary function.
* Register to fire with a given schedule within a timer.
* One interrupt "schedule" has four "periods" consisting of 5 "frames",
and within each frame a time of 0-126. Your exact interrupt timing will
depend on your actual interrupt timing


Example
==============================
	init_interrupt_scheduler();
	
	register_interrupt(
			10,
			ONE | THREE,
			ONE,
			your_interrupt_definition,
			&function_to_fire);
		
			
Hook it up to your interrupt like this, adjusting
for your particular AVR Timer semantics.

	ISR(TIMER0_OVF_vect) {
		on_interrupt(&TCNT0L);
	}