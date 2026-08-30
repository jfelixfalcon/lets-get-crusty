/* =========================================================================
    Unity Project - A Test Framework for C
    Copyright (c) 2007-21 Mike Karlesky, Mark VanderVoord, Greg Williams
    [Released under MIT License. Please refer to license.txt for details]
============================================================================ */

#ifndef UNITY_FRAMEWORK_H
#define UNITY_FRAMEWORK_H

#include "unity_internals.h"

#ifdef __cplusplus
extern "C"
{
#endif

void UnityBegin(const char* filename);
int  UnityEnd(void);
void setUp(void);
void tearDown(void);

#define TEST_PROTECT() (setjmp(Unity.AbortFrame) == 0)
#define TEST_ABORT() longjmp(Unity.AbortFrame, 1)

#define TEST_LINE_NUM (Unity.CurrentTestLineNumber)

#define RUN_TEST(func) \
    UnityDefaultTestRun(func, #func, __LINE__)

#define TEST_ASSERT(condition)                                           UnityAssert( (condition), #condition, __LINE__ )
#define TEST_ASSERT_TRUE(condition)                                      TEST_ASSERT(condition)
#define TEST_ASSERT_FALSE(condition)                                     TEST_ASSERT(!(condition))
#define TEST_ASSERT_NULL(pointer)                                        TEST_ASSERT((pointer) == NULL)
#define TEST_ASSERT_NOT_NULL(pointer)                                    TEST_ASSERT((pointer) != NULL)
#define TEST_ASSERT_EQUAL_INT(expected, actual)                          UnityAssertEqualNumber((UNITY_INT)(expected), (UNITY_INT)(actual), __LINE__)
#define TEST_ASSERT_EQUAL_UINT(expected, actual)                         UnityAssertEqualNumber((UNITY_INT)(expected), (UNITY_INT)(actual), __LINE__)
#define TEST_ASSERT_EQUAL_UINT32(expected, actual)                       UnityAssertEqualNumber((UNITY_INT)(expected), (UNITY_INT)(actual), __LINE__)
#define TEST_ASSERT_EQUAL_UINT64(expected, actual)                       TEST_ASSERT((uint64_t)(expected) == (uint64_t)(actual))
#define TEST_ASSERT_EQUAL_STRING(expected, actual)                       UnityAssertEqualString((expected), (actual), __LINE__)
#define TEST_ASSERT_FLOAT_WITHIN(delta, expected, actual)                UnityAssertFloatWithin((delta), (expected), (actual), __LINE__)
#define TEST_FAIL_MESSAGE(message)                                       UnityAssert(0, (message), __LINE__)
#define TEST_IGNORE()                                                    UnityIgnore(__LINE__)

#ifdef __cplusplus
}
#endif

#endif /* UNITY_FRAMEWORK_H */
