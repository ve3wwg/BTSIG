//////////////////////////////////////////////////////////////////////
// mutex.hpp -- Mutex Support for TFFFT
// Date: Sat Jul  5 07:48:49 2014   (C) Warren Gay ve3wwg
///////////////////////////////////////////////////////////////////////

#ifndef MUTEX_HPP
#define MUTEX_HPP

#include <pthread.h>
                                                                                                                               

class Mutex {
	pthread_mutex_t		mutex;

public:	Mutex();
	~Mutex();

	bool trylock();

	void lock();
	void unlock();
};

#endif // MUTEX_HPP

// End mutex.hpp
