//////////////////////////////////////////////////////////////////////
// mutex.cpp -- Mutex Support for TFFFT
// Date: Sat Jul  5 07:50:07 2014  (C) Warren Gay VE3WWG
///////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <assert.h>

#include "mutex.hpp"

Mutex::Mutex() {
	int rc;

	rc = pthread_mutex_init(&mutex,0);
	assert(!rc);
	(void)rc;
}

Mutex::~Mutex() {
	int rc;

	rc = pthread_mutex_destroy(&mutex);
	assert(!rc);
	(void)rc;
}

void
Mutex::lock() {
	int rc;

	rc = pthread_mutex_lock(&mutex);
	assert(!rc);
	(void)rc;
}

bool
Mutex::trylock() {
	int rc;

	rc = pthread_mutex_trylock(&mutex);
	assert(rc == 0 || rc == EBUSY);
	return !!rc;
}

void
Mutex::unlock() {
	int rc;

	rc = pthread_mutex_unlock(&mutex);
	assert(!rc);
	(void)rc;
}

// End mutex.cpp
