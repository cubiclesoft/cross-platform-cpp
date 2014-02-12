// Cross-platform test suite.
// (C) 2013 CubicleSoft.  All Rights Reserved.

#define _CRT_SECURE_NO_WARNINGS
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>

#include "test.h"

#include "convert/convert_int.h"
#include "security/security_csprng.h"
#include "sync/sync_event.h"
#include "sync/sync_mutex.h"
#include "sync/sync_readwritelock.h"
#include "sync/sync_semaphore.h"
#include "sync/sync_tls.h"
#include "sync/sync_util.h"
#include "templates/cache.h"
#include "templates/detachable_list.h"
#include "templates/detachable_queue.h"
#include "templates/static_vector.h"
#include "utf8/utf8_util.h"
#include "utf8/utf8_file_dir.h"

// Test global instantiations for crash bugs.
CubicleSoft::Security::CSPRNG GxSecurityCSPRNG(false);
CubicleSoft::Sync::Event GxSyncEvent;
CubicleSoft::Sync::Mutex GxSyncMutex;
CubicleSoft::Sync::Semaphore GxSyncSemaphore;
CubicleSoft::Sync::ReadWriteLock GxSyncReadWriteLock;
CubicleSoft::Sync::TLS GxSyncTLS;
CubicleSoft::Cache<int, int> GxCache(11);
CubicleSoft::List<int> GxList;
CubicleSoft::Queue<int> GxQueue;
CubicleSoft::StaticVector<int> GxStaticVector(10);
CubicleSoft::UTF8::File GxFile;
CubicleSoft::UTF8::Dir GxDir;

int Test_Convert_Int(FILE *Testfp)
{
	TEST_START(Test_Convert_Int);

	char Data[50];
	bool x;

	x = CubicleSoft::Convert::Int::ToString(Data, 50, (std::uint64_t)0);
	TEST_COMPARE(x, 1);

//printf("\t%s\n", Data);

	x = CubicleSoft::Convert::Int::ToString(Data, 50, (std::uint64_t)1048576);
	TEST_COMPARE(x, 1);

//printf("\t%s\n", Data);

	x = CubicleSoft::Convert::Int::ToString(Data, 50, (std::uint64_t)0, ',');
	TEST_COMPARE(x, 1);

//printf("\t%s\n", Data);

	x = CubicleSoft::Convert::Int::ToString(Data, 50, (std::uint64_t)1048576, ',');
	TEST_COMPARE(x, 1);

//printf("\t%s\n", Data);


	x = CubicleSoft::Convert::Int::ToString(Data, 50, (std::int64_t)0);
	TEST_COMPARE(x, 1);

//printf("\t%s\n", Data);

	x = CubicleSoft::Convert::Int::ToString(Data, 50, (std::int64_t)1048576);
	TEST_COMPARE(x, 1);

//printf("\t%s\n", Data);

	x = CubicleSoft::Convert::Int::ToString(Data, 50, (std::int64_t)-1048576);
	TEST_COMPARE(x, 1);

//printf("\t%s\n", Data);

	x = CubicleSoft::Convert::Int::ToString(Data, 50, (std::int64_t)0, ',');
	TEST_COMPARE(x, 1);

//printf("\t%s\n", Data);

	x = CubicleSoft::Convert::Int::ToString(Data, 50, (std::int64_t)1048576, ',');
	TEST_COMPARE(x, 1);

//printf("\t%s\n", Data);

	x = CubicleSoft::Convert::Int::ToString(Data, 50, (std::int64_t)-1048576, ',');
	TEST_COMPARE(x, 1);

//printf("\t%s\n", Data);


	x = CubicleSoft::Convert::Int::ToFilesizeString(Data, 50, (std::uint64_t)0);
	TEST_COMPARE(x, 1);

//printf("\t%s\n", Data);

	x = CubicleSoft::Convert::Int::ToFilesizeString(Data, 50, (std::uint64_t)1023999);
	TEST_COMPARE(x, 1);

//printf("\t%s\n", Data);

	x = CubicleSoft::Convert::Int::ToFilesizeString(Data, 50, (std::uint64_t)1024000);
	TEST_COMPARE(x, 1);

//printf("\t%s\n", Data);

	x = CubicleSoft::Convert::Int::ToFilesizeString(Data, 50, (std::uint64_t)1023999, 0);
	TEST_COMPARE(x, 1);

//printf("\t%s\n", Data);

	x = CubicleSoft::Convert::Int::ToFilesizeString(Data, 50, (std::uint64_t)1024000, 0);
	TEST_COMPARE(x, 1);

//printf("\t%s\n", Data);

	x = CubicleSoft::Convert::Int::ToFilesizeString(Data, 50, (std::uint64_t)1023999, 0, ',');
	TEST_COMPARE(x, 1);

//printf("\t%s\n", Data);

	x = CubicleSoft::Convert::Int::ToFilesizeString(Data, 50, (std::uint64_t)1024000, 0, ',');
	TEST_COMPARE(x, 1);

//printf("\t%s\n", Data);

	TEST_SUMMARY();

	TEST_RETURN();
}

int Test_Security_CSPRNG(FILE *Testfp)
{
	TEST_START(Test_Security_CSPRNG);

	CubicleSoft::Security::CSPRNG TestCSPRNG(false);
	std::uint8_t Data[4096];
	bool x;
	size_t x2;
	std::uint64_t RandNum;

	x = TestCSPRNG.GetBytes(Data, 4096);
	TEST_COMPARE(x, 1);

	for (x2 = 0; x && x2 < 1000; x2++)
	{
		x = (TestCSPRNG.GetInteger(RandNum, 1, 10) && RandNum >= 1 && RandNum <= 10);
//		printf("%d ", (unsigned int)RandNum);
	}
//	printf("\n");
	TEST_COMPARE(x, 1);

	TEST_SUMMARY();

	TEST_RETURN();
}

int Test_Sync_Event(FILE *Testfp)
{
	TEST_START(Test_Sync_Event);

	CubicleSoft::Sync::Event TestEvent;
	bool x;

	// Unnamed, automatic event.
	x = TestEvent.Create();
	TEST_COMPARE(x, 1);

	x = TestEvent.Wait(0);
	TEST_COMPARE(x, 0);

	x = TestEvent.Fire();
	TEST_COMPARE(x, 1);

	x = TestEvent.Wait(0);
	TEST_COMPARE(x, 1);

	x = TestEvent.Wait(0);
	TEST_COMPARE(x, 0);


	// Named, automatic event.
	x = TestEvent.Create("test_suite");
	TEST_COMPARE(x, 1);

	x = TestEvent.Wait(0);
	TEST_COMPARE(x, 0);

	x = TestEvent.Fire();
	TEST_COMPARE(x, 1);

	x = TestEvent.Wait(0);
	TEST_COMPARE(x, 1);

	x = TestEvent.Wait(0);
	TEST_COMPARE(x, 0);


	// Unnamed, manual event.
	x = TestEvent.Create(NULL, true);
	TEST_COMPARE(x, 1);

	x = TestEvent.Wait(0);
	TEST_COMPARE(x, 0);

	x = TestEvent.Fire();
	TEST_COMPARE(x, 1);

	x = TestEvent.Wait(0);
	TEST_COMPARE(x, 1);

	x = TestEvent.Wait(0);
	TEST_COMPARE(x, 1);

	x = TestEvent.Reset();
	TEST_COMPARE(x, 1);

	x = TestEvent.Wait(0);
	TEST_COMPARE(x, 0);


	// Named, manual event.
	x = TestEvent.Create("test_suite", true);
	TEST_COMPARE(x, 1);

	x = TestEvent.Wait(0);
	TEST_COMPARE(x, 0);

	x = TestEvent.Fire();
	TEST_COMPARE(x, 1);

	x = TestEvent.Wait(0);
	TEST_COMPARE(x, 1);

	x = TestEvent.Wait(0);
	TEST_COMPARE(x, 1);

	x = TestEvent.Reset();
	TEST_COMPARE(x, 1);

	x = TestEvent.Wait(0);
	TEST_COMPARE(x, 0);

	TEST_SUMMARY();

	TEST_RETURN();
}

int Test_Sync_Mutex(FILE *Testfp)
{
	TEST_START(Test_Sync_Mutex);

	CubicleSoft::Sync::Mutex TestMutex;
	bool x;

	// Unnamed mutex.
	x = TestMutex.Create();
	TEST_COMPARE(x, 1);

	x = TestMutex.Lock(0);
	TEST_COMPARE(x, 1);

	x = TestMutex.Lock(0);
	TEST_COMPARE(x, 1);

	x = TestMutex.Unlock();
	TEST_COMPARE(x, 1);

	x = TestMutex.Unlock();
	TEST_COMPARE(x, 1);

	x = TestMutex.Unlock();
	TEST_COMPARE(x, 0);

	x = TestMutex.Lock(0);
	TEST_COMPARE(x, 1);

	{
		CubicleSoft::Sync::Mutex::AutoUnlock TempLock(&TestMutex);
	}

	x = TestMutex.Unlock();
	TEST_COMPARE(x, 0);


	// Named mutex.
	x = TestMutex.Create("test_suite");
	TEST_COMPARE(x, 1);

	x = TestMutex.Lock(0);
	TEST_COMPARE(x, 1);

	x = TestMutex.Lock(0);
	TEST_COMPARE(x, 1);

	x = TestMutex.Unlock();
	TEST_COMPARE(x, 1);

	x = TestMutex.Unlock();
	TEST_COMPARE(x, 1);

	x = TestMutex.Unlock();
	TEST_COMPARE(x, 0);

	x = TestMutex.Lock(0);
	TEST_COMPARE(x, 1);

	{
		CubicleSoft::Sync::Mutex::AutoUnlock TempLock(&TestMutex);
	}

	x = TestMutex.Unlock();
	TEST_COMPARE(x, 0);

	TEST_SUMMARY();

	TEST_RETURN();
}


int Test_Sync_Semaphore(FILE *Testfp)
{
	TEST_START(Test_Sync_Semaphore);

	CubicleSoft::Sync::Semaphore TestSemaphore;
	bool x;
	int y;

	// Unnamed semaphore.
	x = TestSemaphore.Create(NULL, 2);
	TEST_COMPARE(x, 1);

	x = TestSemaphore.Lock(0);
	TEST_COMPARE(x, 1);

	x = TestSemaphore.Lock(0);
	TEST_COMPARE(x, 1);

	x = TestSemaphore.Lock(0);
	TEST_COMPARE(x, 0);

	{
		CubicleSoft::Sync::Semaphore::AutoUnlock TempLock(&TestSemaphore);
	}

	x = TestSemaphore.Unlock(&y);
	TEST_COMPARE(x, 1);

	x = (y == 1);
	TEST_COMPARE(x, 1);


	// Named semaphore.
	x = TestSemaphore.Create("test_suite", 2);
	TEST_COMPARE(x, 1);

	x = TestSemaphore.Lock(0);
	TEST_COMPARE(x, 1);

	x = TestSemaphore.Lock(0);
	TEST_COMPARE(x, 1);

	x = TestSemaphore.Lock(0);
	TEST_COMPARE(x, 0);

	{
		CubicleSoft::Sync::Semaphore::AutoUnlock TempLock(&TestSemaphore);
	}

	x = TestSemaphore.Unlock(&y);
	TEST_COMPARE(x, 1);

	x = (y == 1);
	TEST_COMPARE(x, 1);

	TEST_SUMMARY();

	TEST_RETURN();
}

int Test_Sync_ReadWriteLock(FILE *Testfp)
{
	TEST_START(Test_Sync_ReadWriteLock);

	CubicleSoft::Sync::ReadWriteLock TestReadWrite;
	bool x;

	// Unnamed reader-writer lock.
	x = TestReadWrite.Create();
	TEST_COMPARE(x, 1);

	x = TestReadWrite.ReadLock(0);
	TEST_COMPARE(x, 1);

	x = TestReadWrite.ReadLock(0);
	TEST_COMPARE(x, 1);

	x = TestReadWrite.ReadUnlock();
	TEST_COMPARE(x, 1);

	x = TestReadWrite.WriteLock(0);
	TEST_COMPARE(x, 0);

	{
		CubicleSoft::Sync::ReadWriteLock::AutoReadUnlock TempLock(&TestReadWrite);
	}

	// Don't rely on the behavior of returning false when read unlocks exceeding the number of read locks.
	// Use AutoReadUnlock instead for every successful ReadLock().
	x = TestReadWrite.ReadUnlock();
	TEST_COMPARE(x, 0);

	x = TestReadWrite.WriteLock(0);
	TEST_COMPARE(x, 1);

	x = TestReadWrite.WriteLock(0);
	TEST_COMPARE(x, 0);

	x = TestReadWrite.ReadLock(0);
	TEST_COMPARE(x, 0);

	{
		CubicleSoft::Sync::ReadWriteLock::AutoWriteUnlock TempLock(&TestReadWrite);
	}


	// Named reader-writer lock.
	x = TestReadWrite.Create("test_suite");
	TEST_COMPARE(x, 1);

	x = TestReadWrite.ReadLock(0);
	TEST_COMPARE(x, 1);

	x = TestReadWrite.ReadLock(0);
	TEST_COMPARE(x, 1);

	x = TestReadWrite.ReadUnlock();
	TEST_COMPARE(x, 1);

	x = TestReadWrite.WriteLock(0);
	TEST_COMPARE(x, 0);

	{
		CubicleSoft::Sync::ReadWriteLock::AutoReadUnlock TempLock(&TestReadWrite);
	}

	// Don't rely on the behavior of returning false when read unlocks exceeding the number of read locks.
	// Use AutoReadUnlock instead for every successful ReadLock().
	x = TestReadWrite.ReadUnlock();
	TEST_COMPARE(x, 0);

	x = TestReadWrite.WriteLock(0);
	TEST_COMPARE(x, 1);

	x = TestReadWrite.WriteLock(0);
	TEST_COMPARE(x, 0);

	x = TestReadWrite.ReadLock(0);
	TEST_COMPARE(x, 0);

	{
		CubicleSoft::Sync::ReadWriteLock::AutoWriteUnlock TempLock(&TestReadWrite);
	}

	TEST_SUMMARY();

	TEST_RETURN();
}

int Test_Templates_Cache(FILE *Testfp)
{
	TEST_START(Test_Templates_Cache);

	CubicleSoft::Cache<size_t, int> TestCache(11);
	bool x;
	size_t x2;
	int y;

	for (x2 = 0; x2 < 100; x2++)
	{
		TestCache.Insert(x2, x2, (int)(x2 + 1));
	}

	x = TestCache.Exists(99, 99);
	TEST_COMPARE(x, 1);

	x = TestCache.Remove(99, 99);
	TEST_COMPARE(x, 1);

	x = TestCache.Exists(99, 99);
	TEST_COMPARE(x, 0);

	x = TestCache.Find(y, 0, 0);
	TEST_COMPARE(x, 0);

	TEST_SUMMARY();

	TEST_RETURN();
}

int Test_Templates_List(FILE *Testfp)
{
	TEST_START(Test_Templates_List);

	CubicleSoft::List<int> TestList, TestList2;
	CubicleSoft::ListNode<int> *Node;
	bool x;
	int x2;

	for (x2 = 0; x2 < 100; x2++)  TestList += x2;

	x = (TestList.GetSize() == 100);
	TEST_COMPARE(x, 1);

	while ((Node = TestList.Shift()) != NULL)  TestList2 += Node;

	x = (TestList.GetSize() == 0);
	TEST_COMPARE(x, 1);

	x = (TestList2.GetSize() == 100);
	TEST_COMPARE(x, 1);

	TestList.DetachAllAndAppend(TestList2);

	x = (TestList.GetSize() == 100);
	TEST_COMPARE(x, 1);

	x = (TestList2.GetSize() == 0);
	TEST_COMPARE(x, 1);

	TEST_SUMMARY();

	TEST_RETURN();
}

int Test_Templates_Queue(FILE *Testfp)
{
	TEST_START(Test_Templates_Queue);

	CubicleSoft::Queue<int> TestQueue, TestQueue2;
	CubicleSoft::QueueNode<int> *Node;
	bool x;
	int x2;

	for (x2 = 0; x2 < 100; x2++)  TestQueue += x2;

	x = (TestQueue.GetSize() == 100);
	TEST_COMPARE(x, 1);

	while ((Node = TestQueue.Shift()) != NULL)  TestQueue2 += Node;

	x = (TestQueue.GetSize() == 0);
	TEST_COMPARE(x, 1);

	x = (TestQueue2.GetSize() == 100);
	TEST_COMPARE(x, 1);

	TestQueue.DetachAllAndAppend(TestQueue2);

	x = (TestQueue.GetSize() == 100);
	TEST_COMPARE(x, 1);

	x = (TestQueue2.GetSize() == 0);
	TEST_COMPARE(x, 1);

	TEST_SUMMARY();

	TEST_RETURN();
}

int Test_Templates_StaticVector(FILE *Testfp)
{
	TEST_START(Test_Templates_StaticVector);

	CubicleSoft::StaticVector<int> TestVector(10);
	bool x;
	int x2;

	for (x2 = 0; x2 < 10; x2++)  TestVector[x2] = x2;

	x = (TestVector.GetSize() == 10);
	TEST_COMPARE(x, 1);

	x = (TestVector[1] == 1);
	TEST_COMPARE(x, 1);

	TEST_SUMMARY();

	TEST_RETURN();
}

int Test_Sync_TLS(FILE *Testfp)
{
	TEST_START(Test_Sync_TLS);

	bool x;

	x = GxSyncTLS.ThreadInit();
	TEST_COMPARE(x, 1);

	if (x)
	{
		char *Data = (char *)GxSyncTLS.malloc(100);
		x = (Data != NULL);
		TEST_COMPARE(x, 1);

		Data = (char *)GxSyncTLS.realloc(Data, 1000);
		x = (Data != NULL);
		TEST_COMPARE(x, 1);

		GxSyncTLS.free(Data);

		x = GxSyncTLS.ThreadEnd();
		TEST_COMPARE(x, 1);
	}

	TEST_SUMMARY();

	TEST_RETURN();
}

int Test_UTF8_File(FILE *Testfp)
{
	TEST_START(Test_UTF8_File);

	const char *Str = "Donate to CubicleSoft.  It covers costs.\n";
	char Data[4096], Data2[4096];
	CubicleSoft::UTF8::File TestFile;
	bool x;
	size_t y;

	// Cleanup previous attempts.
	CubicleSoft::UTF8::File::Delete("test.txt");
	CubicleSoft::UTF8::File::Delete("test.txt.dat");

	time_t t1 = time(NULL);

	x = TestFile.Open("test.txt", O_CREAT | O_WRONLY | O_TRUNC);
	TEST_COMPARE(x, 1);

	x = TestFile.Write(Str, y);
	TEST_COMPARE(x, 1);

	x = (y == strlen(Str));
	TEST_COMPARE(x, 1);

	x = (y == TestFile.GetCurrPos());
	TEST_COMPARE(x, 1);

	x = (y == TestFile.GetMaxPos());
	TEST_COMPARE(x, 1);

	x = TestFile.Close();
	TEST_COMPARE(x, 1);

	x = CubicleSoft::UTF8::File::GetAbsoluteFilename(Data, 4096, NULL, "test.txt");
	TEST_COMPARE(x, 1);

	x = CubicleSoft::UTF8::File::Exists(Data);
	TEST_COMPARE(x, 1);

	x = CubicleSoft::UTF8::File::Realpath(Data2, 4096, Data);
	TEST_COMPARE(x, 1);

	CubicleSoft::UTF8::File::FileStat TestStat;
	x = CubicleSoft::UTF8::File::Stat(TestStat, Data2, false);
	TEST_COMPARE(x, 1);

	x = (TestStat.st_ctime > t1 - 5 && TestStat.st_ctime < t1 + 5);
	TEST_COMPARE(x, 1);
	x = (TestStat.st_atime > t1 - 5 && TestStat.st_atime < t1 + 5);
	TEST_COMPARE(x, 1);
	x = (TestStat.st_mtime > t1 - 5 && TestStat.st_mtime < t1 + 5);
	TEST_COMPARE(x, 1);

	x = CubicleSoft::UTF8::File::Chmod(Data, 0444);
	TEST_COMPARE(x, 1);

	strcat(Data2, ".dat");

	x = CubicleSoft::UTF8::File::Copy(Data, Data2);
	TEST_COMPARE(x, 1);

	x = CubicleSoft::UTF8::File::Delete(Data);
	TEST_COMPARE(x, 1);

	x = CubicleSoft::UTF8::File::Move(Data2, Data);
	TEST_COMPARE(x, 1);

	x = CubicleSoft::UTF8::File::Chmod(Data, 0666);
	TEST_COMPARE(x, 1);

	std::uint64_t TempTime = (std::uint64_t)(t1 - (24 * 60 * 60)) * (std::uint64_t)1000000;
	x = CubicleSoft::UTF8::File::SetFileTimes(Data, NULL, &TempTime, &TempTime);
	TEST_COMPARE(x, 1);

	x = CubicleSoft::UTF8::File::Delete(Data);
	TEST_COMPARE(x, 1);

	TEST_SUMMARY();

	TEST_RETURN();
}

int Test_UTF8_Dir(FILE *Testfp)
{
	TEST_START(Test_UTF8_Dir);

	char Filename[4096];
	CubicleSoft::UTF8::Dir TestDir;
	bool x;
	size_t y;

	x = CubicleSoft::UTF8::Dir::Getcwd(Filename, 4096);
	TEST_COMPARE(x, 1);

	x = CubicleSoft::UTF8::Dir::Mkdir("test/awesome/path", 0777, false);
	TEST_COMPARE(x, 0);

	x = CubicleSoft::UTF8::Dir::Mkdir("test/awesome/path", 0777, true);
	TEST_COMPARE(x, 1);

	x = TestDir.Open("test");
	TEST_COMPARE(x, 1);

	y = 0;
	while ((x = TestDir.Read(Filename, 4096)))
	{
//		printf("\t%s\n", Filename);
		y++;
	}
	x = (y == 3);
	TEST_COMPARE(x, 1);

	x = TestDir.Close();
	TEST_COMPARE(x, 0);

	x = CubicleSoft::UTF8::Dir::Rmdir("test", false);
	TEST_COMPARE(x, 0);

	x = CubicleSoft::UTF8::Dir::Rmdir("test", true);
	TEST_COMPARE(x, 1);

	TEST_SUMMARY();

	TEST_RETURN();
}


int main(void)
{
	Test_Convert_Int(stdout);
	Test_Security_CSPRNG(stdout);
	Test_Sync_Event(stdout);
	Test_Sync_Mutex(stdout);
	Test_Sync_Semaphore(stdout);
	Test_Sync_ReadWriteLock(stdout);
	Test_Templates_Cache(stdout);
	Test_Templates_List(stdout);
	Test_Templates_Queue(stdout);
	Test_Templates_StaticVector(stdout);
	Test_Sync_TLS(stdout);
	Test_UTF8_File(stdout);
	Test_UTF8_Dir(stdout);

	{
		if (GxSyncTLS.ThreadInit())
		{
			char BytesUsed[100];
			char *Data[100];
			std::uint32_t x;
			size_t y, y2;

			printf("Running random Sync::TLS speed test...");
			x = 0;
			y = 0;
			srand(0);
			time_t t1 = time(NULL);
			while (time(NULL) == t1)  {}
			t1 = time(NULL) + 3;
			while (t1 > time(NULL))
			{
				Data[y++] = (char *)GxSyncTLS.malloc((rand() & 0x7FFF) + 1);
				if (y == 100)
				{
					while (y)  GxSyncTLS.free(Data[--y]);
				}

				x++;
			}

			printf("done.\n");
			CubicleSoft::Convert::Int::ToString(BytesUsed, 100, (std::uint64_t)(x / 3), ',');
			printf("\tRate:  %s allocation/free pairs/sec\n\n", BytesUsed);

			while (y)  GxSyncTLS.free(Data[--y]);

			// Dump bucket stats.
			x = 0;
			while (GxSyncTLS.GetBucketInfo(x++, y, y2))
			{
				CubicleSoft::Convert::Int::ToString(BytesUsed, 100, (std::uint64_t)y2, ',');
				printf("\tBucket #%u:  %u nodes (%s bytes)\n", x, (unsigned int)y, BytesUsed);
			}
			printf("\n");

			GxSyncTLS.ThreadEnd();
			GxSyncTLS.ThreadInit();

			printf("Running bucket Sync::TLS speed test...");
			x = 0;
			y = 0;
			y2 = 0;
			t1 = time(NULL);
			while (time(NULL) == t1)  {}
			t1 = time(NULL) + 3;
			while (t1 > time(NULL))
			{
				Data[y++] = (char *)GxSyncTLS.malloc(1 << y2);
				if (y == 100)
				{
					while (y)  GxSyncTLS.free(Data[--y]);
					y2++;
					if (y2 == 15)  y2 = 0;
				}

				x++;
			}

			printf("done.\n");
			CubicleSoft::Convert::Int::ToString(BytesUsed, 100, (std::uint64_t)(x / 3), ',');
			printf("\tRate:  %s allocation/free pairs/sec\n\n", BytesUsed);

			while (y)  GxSyncTLS.free(Data[--y]);

			// Dump bucket stats.
			x = 0;
			while (GxSyncTLS.GetBucketInfo(x++, y, y2))
			{
				CubicleSoft::Convert::Int::ToString(BytesUsed, 100, (std::uint64_t)y2, ',');
				printf("\tBucket #%u:  %u nodes (%s bytes)\n", x, (unsigned int)y, BytesUsed);
			}
			printf("\n");

			GxSyncTLS.ThreadEnd();


			// Reset and run with pure malloc/free.
			// Doing these tests AFTER Sync::TLS gives malloc/free the benefit of the doubt.
			// That is, the system is already primed, therefore malloc/free get an advantage over Sync::TLS.
			printf("Running random malloc/free speed test...");
			x = 0;
			y = 0;
			srand(0);
			t1 = time(NULL);
			while (time(NULL) == t1)  {}
			t1 = time(NULL) + 3;
			while (t1 > time(NULL))
			{
				Data[y++] = (char *)malloc((rand() & 0x7FFF) + 1);
				if (y == 100)
				{
					while (y)  free(Data[--y]);
				}

				x++;
			}

			printf("done.\n");
			CubicleSoft::Convert::Int::ToString(BytesUsed, 100, (std::uint64_t)(x / 3), ',');
			printf("\tRate:  %s allocation/free pairs/sec\n\n", BytesUsed);

			while (y)  free(Data[--y]);


			printf("Running bucket malloc/free speed test...");
			x = 0;
			y = 0;
			y2 = 0;
			t1 = time(NULL);
			while (time(NULL) == t1)  {}
			t1 = time(NULL) + 3;
			while (t1 > time(NULL))
			{
				Data[y++] = (char *)malloc(1 << y2);
				if (y == 100)
				{
					while (y)  free(Data[--y]);
					y2++;
					if (y2 == 15)  y2 = 0;
				}

				x++;
			}

			printf("done.\n");
			CubicleSoft::Convert::Int::ToString(BytesUsed, 100, (std::uint64_t)(x / 3), ',');
			printf("\tRate:  %s allocation/free pairs/sec\n\n", BytesUsed);

			while (y)  free(Data[--y]);
		}
	}

	return 0;
}
