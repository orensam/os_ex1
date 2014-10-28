/*
 * osm.c
 *
 *  Created on: Feb 25, 2014
 *      Author: orensam
 *
 *  A library that can be used to measure running times of simple operations.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <sys/time.h>
#include <sys/syscall.h>
#include "osm.h"

typedef struct timeval timeval;

// Default number of total osm_iterations
#define DEFAULT_ITERS 50000
// Number of executions in each actual iteration.
// Note that actual number of iterations is calculated in getNumberOfIters().
#define UNROLL_FACTOR 20
// Macro that cleans measure functions body.
// Has to match UNROLL_FACTOR!!!
#define ROLL_LOOP(X) X; X; X; X; X; X; X; X; X; X; X; X; X; X; X; X; X; X; X; X;

/**
 * Check the number of iterations given by the user.
 * If <=0, set it to default.
 */
unsigned int fixOsmIterations(unsigned int osm_iterations)
{
	if (osm_iterations == 0)
	{
		return DEFAULT_ITERS;
	}
	return osm_iterations;
}

/**
 * Returns the number of iteration that needs to be performed
 * when taking loop unrolling into consideration.
 */
unsigned int getNumberOfIters(unsigned int osm_iterations)
{
	// Divide s.t the actual number of iterations is rounded UP.
	return (fixOsmIterations(osm_iterations) + UNROLL_FACTOR - 1) / UNROLL_FACTOR;
}


/**
 * Calculates the time difference between two given timevals.
 */
long calcTimeDiff(timeval* t1, timeval* t2)
{
	timeval res;
	timersub(t2, t1, &res);
	return res.tv_sec * 1000000000 + res.tv_usec * 1000;
//	return (t2->tv_sec - t1->tv_sec) * 1000000000 + (t2->tv_usec - t1->tv_usec) * 1000;
}

/**
 * Returns the time it takes to perform the iterations themselves,
 * in order for this number to be subtracted from the timing calculations.
 * If anything fails, returns the default value 0.
 */
long getLoopOverhead(unsigned int iters)
{
	unsigned int i=0;
	timeval begin;
	timeval end;

	if (gettimeofday(&begin, NULL) != 0)
	{
		return 0;
	}
	for (i = 0; i < iters; ++i)
	{
	}
	if (gettimeofday(&end, NULL) != 0)
	{
		return 0;
	}

	return calcTimeDiff(&begin, &end);
}

/**
 * Time measurement function that runs all the library's timing functions.
 * and returns them in a timeMeasurmentStructure, along with information
 * about number of iterations, hostname, and timing ratios.
 */
timeMeasurmentStructure measureTimes(unsigned int osm_iterations)
{
	timeMeasurmentStructure stats;
	osm_iterations = fixOsmIterations(osm_iterations);

	stats.numberOfIterations = osm_iterations;

	// Allocate memory for hostname string.
	// Freeing this memory is under the user's responsibility.
	char * hostname = (char *) malloc(HOST_NAME_MAX);
	if (!hostname || gethostname(hostname, HOST_NAME_MAX) != 0)
	{
		stats.machineName = NULL;
	}
	else
	{
		stats.machineName = hostname;
	}

	// Calculate timings
	stats.functionTimeNanoSecond = osm_function_time(osm_iterations);
	stats.instructionTimeNanoSecond = osm_operation_time(osm_iterations);
	stats.trapTimeNanoSecond = osm_syscall_time(osm_iterations);

	// Calculate rations
	stats.functionInstructionRatio = stats.functionTimeNanoSecond / stats.instructionTimeNanoSecond;
	stats.trapInstructionRatio = stats.trapTimeNanoSecond / stats.instructionTimeNanoSecond;

	return stats;
}

/**
 * Initialization function that the user must call
 * before running any other library function.
 * Returns 0 uppon success and -1 on failure
 */
int osm_init()
{
	return 0;
}

/**
 * An empty function which is used to measure
 * function call time.
 */
void __attribute__ ((noinline)) emptyFunc()
{
}

/**
 * Time measurement function for an empty function call.
 * returns time in nano-seconds upon success,
 * and -1 upon failure.
 */
double osm_function_time(unsigned int osm_iterations)
{
	timeval begin;
	timeval end;

	unsigned int i;
	osm_iterations = fixOsmIterations(osm_iterations);
	unsigned int iters = getNumberOfIters(osm_iterations);
	long loopOverhead = getLoopOverhead(iters);

	if (gettimeofday(&begin, NULL) != 0)
	{
		return -1;
	}

	for (i=0; i < iters; ++i)
	{
		ROLL_LOOP(emptyFunc())
	}

	if (gettimeofday(&end, NULL) != 0)
	{
		return -1;
	}
	return (calcTimeDiff(&begin, &end) - loopOverhead) / (double) (UNROLL_FACTOR * iters);
}

/**
 * Time measurement function for an empty trap into the operating system.
 * returns time in nano-seconds upon success,
 * and -1 upon failure.
 */
double osm_syscall_time(unsigned int osm_iterations)
{
	timeval begin;
	timeval end;

	unsigned int i;
	osm_iterations = fixOsmIterations(osm_iterations);
	int iters = getNumberOfIters(osm_iterations);
	long loopOverhead = getLoopOverhead(iters);

	if (gettimeofday(&begin, NULL) != 0)
	{
		return -1;
	}
	for (i=0; i < iters; ++i)
	{
		ROLL_LOOP(OSM_NULLSYSCALL)
	}
	if (gettimeofday(&end, NULL) != 0)
	{
		return -1;
	}
	return (calcTimeDiff(&begin, &end) - loopOverhead) / (double) (UNROLL_FACTOR * iters);
}

/**
 * Returns the time it takes to perform the iterations with one assignment operation
 * and one access to a variable, in order for this number to be subtracted from
 * the timing calculation of a basic instruction.
 * If anything fails, returns the default value 0.
 */
long getSpecialLoopOverhead(unsigned int iters)
{
	unsigned int i=0;
	timeval begin;
	timeval end;

	int j;

	if (gettimeofday(&begin, NULL) != 0)
	{
		return 0;
	}
	for (i = 0; i < iters; ++i)
	{
		// Just assignment
		j = i;
	}
	if (gettimeofday(&end, NULL) != 0)
	{
		return 0;
	}
	i=j; // To make sure j's value will be calculated

	return calcTimeDiff(&begin, &end);
}

/**
 * Time measurement function for a simple arithmetic operation.
 * returns time in nano-seconds upon success,
 * and -1 upon failure.
 */
double osm_operation_time(unsigned int osm_iterations)
{
	timeval begin;
	timeval end;

	unsigned int i, j = 0;
	osm_iterations = fixOsmIterations(osm_iterations);
	int iters = getNumberOfIters(osm_iterations);
	long loopOverhead = getSpecialLoopOverhead(iters);

	if (gettimeofday(&begin, NULL) != 0)
	{
		return -1;
	}
	for (i=0; i < iters; ++i)
	{
		// access variable, perform instruction, perform assignment
		ROLL_LOOP(j = i + 1);
	}
	if (gettimeofday(&end, NULL) != 0)
	{
		return -1;
	}
	i = j; // To make sure j's value will be calculated
	return (calcTimeDiff(&begin, &end) - loopOverhead) / (double) (UNROLL_FACTOR * iters);
}
