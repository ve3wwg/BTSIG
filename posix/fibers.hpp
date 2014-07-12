//////////////////////////////////////////////////////////////////////
// fibers.hpp -- Teensy Framework For Fibers Testing (TFFFT)
// Date: Sat Jul  5 07:46:13 2014   (C) Warren Gay ve3wwg
///////////////////////////////////////////////////////////////////////

#ifndef FIBERS_HPP
#define FIBERS_HPP

#include <stdint.h>
#include "mutex.hpp"

#include <pthread.h>

extern "C" {
	// Coroutine function prototype
	typedef void (*fiber_func_t)(void *arg);
}

enum FiberState {
	FiberCreated,			// Fiber created, but not yet started
	FiberExecuting,			// Fiber is executing
	FiberReturned,			// Fiber has terminated
	FiberInvalid			// Invalid fiberx value
};

struct fiber_t {
	FiberState		state;			// State of the fiber
	fiber_func_t		func;
	void			*arg;
	pthread_t		thread;			// Underlying thread
	Mutex			*mutex;			// Thread mutex
	uint32_t		stack_size;
	void			*fibers;		// Template parent object
};

template <unsigned max_fibers=16>
class Fibers {
	fiber_t			fibers[max_fibers];	// Collection of fibers
	volatile uint16_t	n_fibers;		// # of active fibers
	volatile uint16_t	cur_crx;		// Index to currently executing coroutine

public:	Fibers(uint32_t main_stack=8192,bool instrument=false,uint32_t pattern_override=0);

	inline uint32_t size()	{ return n_fibers; }
	inline unsigned current() { return cur_crx; }

	unsigned create(fiber_func_t func,void *arg,uint32_t stack_size);
	void yield();

	FiberState state(uint32_t fiberx);
	FiberState join(uint32_t fiberx);
	FiberState restart(uint32_t fiberx,fiber_func_t func,void *arg);

	static void *launch(void *arg);
};

template <unsigned max_fibers>
Fibers<max_fibers>::Fibers(uint32_t main_stack,bool instrument,uint32_t pattern_override) {

	(void)pattern_override;				// Ignored (here for compatibility with Teensy lib)
	(void)instrument;				// ditto

	n_fibers = 1;
	cur_crx = 0;
	
	fibers[0].state = FiberExecuting;
	fibers[0].thread = pthread_self();
	fibers[0].mutex = new Mutex;
	fibers[0].stack_size = main_stack;		// Not used
	fibers[0].mutex->lock();
	fibers[0].fibers = (void *)this;
}

template <unsigned max_fibers>
unsigned
Fibers<max_fibers>::create(fiber_func_t func,void *arg,uint32_t stack_size) {
	int rc;

	// Round stack size to a word multiple
	stack_size = ( stack_size + sizeof (uint32_t) ) / sizeof (uint32_t) * sizeof (uint32_t);
	
	if ( n_fibers >= max_fibers )
		return 0;		// Failed: too many fibers
	
	fiber_t &new_cr = fibers[n_fibers++];
	
	new_cr.func = func;
	new_cr.arg = arg;
	new_cr.stack_size = stack_size;
	new_cr.mutex = new Mutex;
	new_cr.mutex->lock();		// Create it locked
	new_cr.state = FiberCreated;
	new_cr.fibers = fibers[0].fibers;

	rc = pthread_create(&new_cr.thread,0,launch,&new_cr);
	assert(!rc);
	(void)rc;

	return n_fibers - 1;		// Return zero-based coroutine # (main == 0)
}

template <unsigned max_fibers>
void
Fibers<max_fibers>::yield() {

	if ( n_fibers <= 1 )
		return;			// There is only the main context running
	
	uint16_t nextx = cur_crx + 1;	// Compute the coroutine # of the next fiber
	if ( nextx >= n_fibers )
		nextx = 0;		// Wrap around to main context at the end
	
	volatile fiber_t& last_cr = fibers[cur_crx];	// Current context
	volatile fiber_t& next_cr = fibers[nextx];	// Next context
	cur_crx = nextx;			// Change the context no.

	next_cr.mutex->unlock();
	last_cr.mutex->lock();
}

template <unsigned max_fibers>
FiberState
Fibers<max_fibers>::state(uint32_t fiberx) {

	if ( fiberx >= n_fibers )
		return FiberInvalid;			// Invalid fiberx value given

	volatile fiber_t& crentry = fibers[fiberx];	// Access requested coroutine
	return crentry.state;				// Return its state
}

template <unsigned max_fibers>
FiberState
Fibers<max_fibers>::join(uint32_t fiberx) {
	FiberState s;

	if ( fiberx >= n_fibers )
		return FiberInvalid;			// Invalid fiberx: Nothing to join with

	while ( (s = state(fiberx)) != FiberReturned )
		this->yield();				// Keep waiting

	return s;
}

template <unsigned max_fibers>
FiberState
Fibers<max_fibers>::restart(uint32_t fiberx,fiber_func_t func,void *arg) {
	int rc;

	if ( fiberx >= n_fibers )
		return FiberInvalid;			// Invalid fiberx: Nothing to join with

	fiber_t& fiber = fibers[fiberx];
	assert(fiber.state == FiberReturned);
	rc = pthread_cancel(fiber.thread);
	assert(!rc);
	rc = pthread_detach(fiber.thread);
	assert(!rc);
	(void)rc;

	rc = pthread_create(&fiber.thread,0,launch,&fiber);
	return fiber.state;
}

template <unsigned max_fibers>
void *
Fibers<max_fibers>::launch(void *arg) {
	fiber_t& fiber = *static_cast<fiber_t *>(arg);
	Fibers<max_fibers> *pool = static_cast<Fibers<max_fibers> *>(fiber.fibers);

	fiber.mutex->lock();
	fiber.state = FiberExecuting;
	fiber.func(fiber.arg);
	fiber.state = FiberReturned;
	for (;;) {
		pool->yield();
	}
	return 0;
}

#endif // FIBERS_HPP

// End fibers.hpp
