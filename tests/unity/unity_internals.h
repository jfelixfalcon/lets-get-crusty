/* =========================================================================
    Unity Project - A Test Framework for C
    Copyright (c) 2007-21 Mike Karlesky, Mark VanderVoord, Greg Williams
    [Released under MIT License. Please refer to license.txt for details]
============================================================================ */

#ifndef UNITY_INTERNALS_H
#define UNITY_INTERNALS_H

#include <setjmp.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

typedef int UNITY_INT;
typedef unsigned int UNITY_UINT;

struct UNITY_STORAGE_T
{
    const char* TestFile;
    const char* CurrentTestName;
    UNITY_UINT CurrentTestLineNumber;
    UNITY_UINT NumberOfTests;
    UNITY_UINT TestFailures;
    UNITY_UINT TestIgnores;
    UNITY_UINT CurrentTestFailed;
    UNITY_UINT CurrentTestIgnored;
    jmp_buf AbortFrame;
};

extern struct UNITY_STORAGE_T Unity;

typedef void (*UnityTestFunction)(void);

void UnityAssert(int condition, const char* message, int lineNumber);
void UnityAssertEqualNumber(UNITY_INT expected, UNITY_INT actual, int lineNumber);
void UnityAssertEqualString(const char* expected, const char* actual, int lineNumber);
void UnityAssertFloatWithin(float delta, float expected, float actual, int lineNumber);
void UnityIgnore(int lineNumber);
void UnityDefaultTestRun(UnityTestFunction Func, const char* FuncName, int FuncLineNum);

#ifdef __cplusplus
}
#endif

#endif /* UNITY_INTERNALS_H */
