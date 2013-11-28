// C++ test suite macros.
// (C) 2013 CubicleSoft.  All Rights Reserved.

const char *Global__StartTestStr = "%s (%lu) test started.\n";
const char *Global__PassedTestStr = "%s (%lu) test %u passed.\n";
const char *Global__FailedTestStr = "%s (%lu) test %u failed.\n";
const char *Global__SummaryTestsStr = "%s summary:\n  %u tests performed.\n  %u tests failed.\n\n";
const char *Global__SummaryTestsSuccessStr = "%s summary:\n  %u tests performed.\n  All tests passed.\n\n";
const char *Global__CustomTestStartStr = "%s (%lu) custom test started.\n";
const char *Global__CustomTestEndStr = "%s (%lu) custom test ended.\n";

#define TEST_FAILED    0
#define TEST_SUCCESS   1
#define TEST_START(arg)           const char *Local__FuncName = #arg; \
                                  int Local__TestCounter = 0; \
                                  int Local__TestsFailed = 0; \
                                  fprintf(Testfp, Global__StartTestStr, Local__FuncName, __LINE__); \
                                  fflush(Testfp)
#define TEST_COMPARE(arg, arg2)   do { \
                                    Local__TestCounter++; \
                                    if ((arg) == (arg2))  fprintf(Testfp, Global__PassedTestStr, Local__FuncName, __LINE__, Local__TestCounter); \
                                    else \
                                    { \
                                      fprintf(Testfp, Global__FailedTestStr, Local__FuncName, __LINE__, Local__TestCounter); \
                                      Local__TestsFailed++; \
                                    } \
                                    fflush(Testfp); \
                                  } while (0)
#define TEST_RESULT(arg)          do { \
                                    Local__TestCounter++; \
                                    if ((arg) == TEST_SUCCESS)  fprintf(Testfp, Global__PassedTestStr, Local__FuncName, __LINE__, Local__TestCounter); \
                                    else \
                                    { \
                                      fprintf(Testfp, Global__FailedTestStr, Local__FuncName, __LINE__, Local__TestCounter); \
                                      Local__TestsFailed++; \
                                    } \
                                    fflush(Testfp); \
                                  } while (0)
#define TEST_CUSTOM_START()       do { \
                                    fprintf(Testfp, Global__CustomTestStartStr, Local__FuncName, __LINE__); \
                                    fflush(Testfp); \
                                  } while (0)
#define TEST_CUSTOM_END()         do { \
                                    Local__TestCounter++; \
                                    fprintf(Testfp, Global__CustomTestEndStr, Local__FuncName, __LINE__); \
                                    fflush(Testfp); \
                                  } while (0)
#define TEST_SUMMARY()            do { \
									if (Local__TestsFailed)  fprintf(Testfp, Global__SummaryTestsStr, Local__FuncName, Local__TestCounter, Local__TestsFailed); \
									else  fprintf(Testfp, Global__SummaryTestsSuccessStr, Local__FuncName, Local__TestCounter); \
                                    fflush(Testfp); \
                                  } while (0)
#define TEST_RETURN()             return !Local__TestsFailed
