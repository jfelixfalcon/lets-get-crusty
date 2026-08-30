/* =========================================================================
    Unity Project - A Test Framework for C
    Copyright (c) 2007-21 Mike Karlesky, Mark VanderVoord, Greg Williams
    [Released under MIT License. Please refer to license.txt for details]
============================================================================ */

#include "unity.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

struct UNITY_STORAGE_T Unity;

void UnityBegin(const char* filename)
{
    Unity.TestFile = filename;
    Unity.CurrentTestName = NULL;
    Unity.CurrentTestLineNumber = 0;
    Unity.NumberOfTests = 0;
    Unity.TestFailures = 0;
    Unity.TestIgnores = 0;
    Unity.CurrentTestFailed = 0;
    Unity.CurrentTestIgnored = 0;
}

int UnityEnd(void)
{
    printf("-----------------------\n");
    printf("%u Tests %u Failures %u Ignored\n",
           Unity.NumberOfTests, Unity.TestFailures, Unity.TestIgnores);

    if (Unity.TestFailures == 0)
    {
        printf("OK\n");
        return 0;
    }

    printf("FAIL\n");
    return (int)Unity.TestFailures;
}

void UnityAssert(int condition, const char* message, int lineNumber)
{
    if (!condition)
    {
        Unity.CurrentTestFailed = 1;
        printf("%s:%d:%s:FAIL: %s\n",
               Unity.TestFile, lineNumber,
               Unity.CurrentTestName ? Unity.CurrentTestName : "",
               message ? message : "Expression evaluated to false");
        TEST_ABORT();
    }
}

void UnityAssertEqualNumber(UNITY_INT expected, UNITY_INT actual, int lineNumber)
{
    if (expected != actual)
    {
        Unity.CurrentTestFailed = 1;
        printf("%s:%d:%s:FAIL: Expected %d Was %d\n",
               Unity.TestFile, lineNumber,
               Unity.CurrentTestName ? Unity.CurrentTestName : "",
               expected, actual);
        TEST_ABORT();
    }
}

void UnityAssertEqualString(const char* expected, const char* actual, int lineNumber)
{
    if ((expected == NULL) && (actual == NULL))
    {
        return;
    }
    if ((expected == NULL) || (actual == NULL) || (strcmp(expected, actual) != 0))
    {
        Unity.CurrentTestFailed = 1;
        printf("%s:%d:%s:FAIL: Expected '%s' Was '%s'\n",
               Unity.TestFile, lineNumber,
               Unity.CurrentTestName ? Unity.CurrentTestName : "",
               expected ? expected : "NULL",
               actual ? actual : "NULL");
        TEST_ABORT();
    }
}

void UnityAssertFloatWithin(float delta, float expected, float actual, int lineNumber)
{
    if (fabsf(expected - actual) > delta)
    {
        Unity.CurrentTestFailed = 1;
        printf("%s:%d:%s:FAIL: Expected %f Was %f (diff %f > delta %f)\n",
               Unity.TestFile, lineNumber,
               Unity.CurrentTestName ? Unity.CurrentTestName : "",
               (double)expected, (double)actual, (double)fabsf(expected - actual), (double)delta);
        TEST_ABORT();
    }
}

void UnityIgnore(int lineNumber)
{
    Unity.CurrentTestIgnored = 1;
    printf("%s:%d:%s:IGNORE\n",
           Unity.TestFile, lineNumber,
           Unity.CurrentTestName ? Unity.CurrentTestName : "");
    TEST_ABORT();
}

void UnityDefaultTestRun(UnityTestFunction Func, const char* FuncName, int FuncLineNum)
{
    Unity.CurrentTestName = FuncName;
    Unity.CurrentTestLineNumber = (UNITY_UINT)FuncLineNum;
    Unity.NumberOfTests++;
    Unity.CurrentTestFailed = 0;
    Unity.CurrentTestIgnored = 0;

    if (TEST_PROTECT())
    {
        setUp();
        Func();
    }

    if (TEST_PROTECT())
    {
        tearDown();
    }

    if (Unity.CurrentTestIgnored)
    {
        Unity.TestIgnores++;
    }
    else if (Unity.CurrentTestFailed)
    {
        Unity.TestFailures++;
    }
    else
    {
        printf("%s:%d:%s:PASS\n", Unity.TestFile, FuncLineNum, FuncName);
    }
}
