#pragma once
#include <stdio.h>
#include "memory.h"

/* Example line:
 * E | file.c:21@main() | Test message 123.456
 */
#define _LOGF(type, fmt, ...) fprintf(stderr, type " | " __FILE__ ":%u@%s() | " fmt "\n", __LINE__,  __func__, ##__VA_ARGS__)
#define ERRORF(fmt, ...) _LOGF("E", fmt, ##__VA_ARGS__)
#ifndef NDEBUG
#define DEBUGF(fmt, ...) _LOGF("D", fmt, ##__VA_ARGS__)
#else
#define DEBUGF(fmt, ...) (void)(0)
#endif

#define _HAS_VA_ARGS(...) (sizeof( (char[]){#__VA_ARGS__} ) > 1)
#define FAILURE_IF(condition, return_value, ...) \
	do \
		if (condition) \
		{ \
			if (_HAS_VA_ARGS(__VA_ARGS__)) \
				ERRORF(__VA_ARGS__); \
			return return_value; \
		} \
	while (0)

#define FAILURE_GOTO(condition, label, ...) \
	do \
		if (condition) \
		{ \
			if (_HAS_VA_ARGS(__VA_ARGS__)) \
				ERRORF(__VA_ARGS__); \
			goto label; \
	} \
	while (0)

#define FAILURE_ALLOC(object) FAILURE_IF(object == NULL, NULL, "Allocation failure")

