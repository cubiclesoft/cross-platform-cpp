// Cross-platform test suite.
// (C) 2015 CubicleSoft.  All Rights Reserved.

#define _CRT_SECURE_NO_WARNINGS
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <cinttypes>

#include "test.h"

// Must be first to avoid Winsock 2 issues.
//#include "network/network_server.h"

#include "convert/convert_int.h"
#include "security/security_csprng.h"
#include "sync/sync_event.h"
#include "sync/sync_mutex.h"
#include "sync/sync_readwritelock.h"
#include "sync/sync_semaphore.h"
#include "sync/sync_sharedmem.h"
#include "sync/sync_tls.h"
#include "sync/sync_util.h"
#include "templates/cache.h"
#include "templates/detachable_list.h"
#include "templates/detachable_ordered_hash.h"
#include "templates/detachable_queue.h"
#include "templates/static_vector.h"
#include "templates/static_2d_array.h"
#include "templates/static_mixed_var.h"
#include "templates/fast_find_replace.h"
#include "templates/packed_ordered_hash.h"
#include "environment/environment_appinfo.h"
#include "utf8/utf8_util.h"
#include "utf8/utf8_appinfo.h"
#include "utf8/utf8_file_dir.h"
#include "utf8/utf8_mixed_var.h"
//#include "network/network_init.h"
//#include "network/network_async_helper.h"

// Test global instantiations for crash bugs.
CubicleSoft::Security::CSPRNG GxSecurityCSPRNG(false);
CubicleSoft::Sync::Event GxSyncEvent;
CubicleSoft::Sync::Mutex GxSyncMutex;
CubicleSoft::Sync::Semaphore GxSyncSemaphore;
CubicleSoft::Sync::ReadWriteLock GxSyncReadWriteLock;
CubicleSoft::Sync::SharedMem GxSyncSharedMem;
CubicleSoft::Sync::TLS GxSyncTLS;
CubicleSoft::Sync::TLS::MixedVar GxSyncTLSMixedVar;
CubicleSoft::Cache<int, int> GxCache(11);
CubicleSoft::List<int> GxList;
CubicleSoft::OrderedHash<int> GxOrderedHash(47);
CubicleSoft::PackedOrderedHash<int> GxPackedOrderedHash(32);
CubicleSoft::Queue<int> GxQueue;
CubicleSoft::StaticVector<int> GxStaticVector(10);
CubicleSoft::Static2DArray<int> GxStatic2DArray(20, 2);
CubicleSoft::StaticMixedVar<char[8192]> GxStaticMixedVar;
CubicleSoft::FastFind<char> GxFastFind;
CubicleSoft::FastReplace<char> GxFastReplace;
CubicleSoft::UTF8::File GxUTF8File;
CubicleSoft::UTF8::Dir GxUTF8Dir;
CubicleSoft::UTF8::UTF8MixedVar<char[8192]> GxUTF8MixedVar;
//CubicleSoft::Network::Init GxNetworkInit;
//CubicleSoft::Network::AsyncHelper GxAsyncHelper;
//CubicleSoft::Network::Server GxServer;

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
	x = TestEvent.Create("test_suite2", true);
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

	y = 0;
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

	// Don't rely on the behavior of returning false when read unlocks exceed the number of read locks.
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

int Test_Sync_SharedMem(FILE *Testfp)
{
	TEST_START(Test_Sync_SharedMem);

	CubicleSoft::Sync::SharedMem TestSharedMem;
	bool x;

	x = TestSharedMem.Create("test_suite", 150);
	TEST_COMPARE(x, 1);

	x = TestSharedMem.First();
	TEST_COMPARE(x, 1);

	x = (TestSharedMem.GetSize() == 150);
	TEST_COMPARE(x, 1);

	strcpy(TestSharedMem.RawData(), "test");

	{
		CubicleSoft::Sync::SharedMem TestSharedMem2;

		x = TestSharedMem2.Create("test_suite", 150);
		TEST_COMPARE(x, 1);

		x = TestSharedMem2.First();
		TEST_COMPARE(x, 0);

		x = (TestSharedMem2.GetSize() == 150);
		TEST_COMPARE(x, 1);

		x = (memcmp(TestSharedMem2.RawData(), "test", 5) == 0);
		TEST_COMPARE(x, 1);
	}

	// Close all handles and then reconnect.  The data written before should not be there since all references are removed.
	x = TestSharedMem.Create("test_suite", 150);
	TEST_COMPARE(x, 1);

	x = TestSharedMem.First();
	TEST_COMPARE(x, 1);

	x = (TestSharedMem.GetSize() == 150);
	TEST_COMPARE(x, 1);

	x = (memcmp(TestSharedMem.RawData(), "test", 5) != 0);
	TEST_COMPARE(x, 1);

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

int Test_Templates_OrderedHash(FILE *Testfp)
{
	TEST_START(Test_Templates_OrderedHash);

	CubicleSoft::OrderedHash<int> TestHash(47);
	CubicleSoft::OrderedHashNode<int> *Node, *Nodes = NULL;
	bool x;
	int x2;

	for (x2 = 0; x2 < 100; x2++)  TestHash.Push(x2, x2);
	TestHash.Push("", 1, x2++);
	TestHash.Push("test", 5, x2++);

	x = (TestHash.GetListSize() == 102);
	TEST_COMPARE(x, 1);

	// Test existence.
	x = true;
	for (x2 = 0; x2 < 100 && x; x2++)
	{
		if (x)  x = (TestHash.Find(x2) != NULL);
	}
	if (x)  x = (TestHash.Find("", 1) != NULL);
	if (x)  x = (TestHash.Find("test", 5) != NULL);
	TEST_COMPARE(x, 1);

	// Test order.
	x = true;
	Node = TestHash.FirstList();
	for (x2 = 0; x2 < 100 && x; x2++)  x = ((Node = Node->NextList()) != NULL);
	if (x)
	{
		x = (Node->GetStrKey() != NULL && Node->GetIntKey() == 1 && Node->Value == x2);
		Node = Node->NextList();
		x2++;
	}
	if (x)
	{
		x = (Node->GetStrKey() != NULL && Node->GetIntKey() == 5 && Node->Value == x2);
		Node = Node->NextList();
		x2++;
	}
	TEST_COMPARE(x, 1);

	while ((Node = TestHash.Shift(false)) != NULL)
	{
		Node->NextListNode = Nodes;
		Nodes = Node;
	}

	x = (TestHash.GetListSize() == 0);
	TEST_COMPARE(x, 1);

	while (Nodes != NULL)
	{
		Node = Nodes->NextListNode;
		TestHash.Unshift(Nodes);
		Nodes = Node;
	}

	x = (TestHash.GetListSize() == 102);
	TEST_COMPARE(x, 1);

	TEST_SUMMARY();

	TEST_RETURN();
}

int Test_Templates_PackedOrderedHash(FILE *Testfp)
{
	TEST_START(Test_Templates_PackedOrderedHash);

	CubicleSoft::PackedOrderedHash<int> TestHash(32);
	CubicleSoft::PackedOrderedHashNode<int> *Node;
	bool x;
	int x2;

	for (x2 = 0; x2 < 100; x2++)  TestHash.Set(x2, x2);
	TestHash.Set("", 1, x2++);
	TestHash.Set("test", 5, x2++);

	x = (TestHash.GetSize() == 102);
	TEST_COMPARE(x, 1);

	// Test existence.
	x = true;
	for (x2 = 0; x2 < 100 && x; x2++)  x = (TestHash.Find(x2) != NULL);
	if (x)  x = (TestHash.Find("", 1) != NULL);
	if (x)  x = (TestHash.Find("test", 5) != NULL);
	TEST_COMPARE(x, 1);

	// Test order.
	x = true;
	size_t Pos = TestHash.GetNextPos();
	for (x2 = 0; x2 < 100 && x; x2++)
	{
		Node = TestHash.Next(Pos);
		x = (Node != NULL && Node->GetStrKey() == NULL && Node->GetIntKey() == x2 && Node->Value == x2);
	}
	if (x)
	{
		Node = TestHash.Next(Pos);
		x = (Node != NULL && Node->GetStrKey() != NULL && Node->GetStrLen() == 1 && Node->Value == x2);
		x2++;
	}
	if (x)
	{
		Node = TestHash.Next(Pos);
		x = (Node != NULL && Node->GetStrKey() != NULL && Node->GetStrLen() == 5 && Node->Value == x2);
		x2++;
	}
	TEST_COMPARE(x, 1);

	// Test unset.
	x = true;
	for (x2 = 0; x2 < 100 && x; x2++)  x = TestHash.Unset(x2);
	TEST_COMPARE(x, 1);

	x = (TestHash.GetSize() == 2);
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

int Test_Templates_Static2DArray(FILE *Testfp)
{
	TEST_START(Test_Templates_Static2DArray);

	CubicleSoft::Static2DArray<int> TestArray(20, 2);
	bool x;
	int x2, y2;

	for (y2 = 0; y2 < 2; y2++)
	{
		for (x2 = 0; x2 < 20; x2++)  TestArray[y2][x2] = (y2 * 20) + x2;
	}

	x = (TestArray.GetWidth() == 20);
	TEST_COMPARE(x, 1);

	x = (TestArray.GetHeight() == 2);
	TEST_COMPARE(x, 1);

	x = (TestArray[1][1] == 21);
	TEST_COMPARE(x, 1);

	TEST_SUMMARY();

	TEST_RETURN();
}

int Test_Templates_StaticMixedVar(FILE *Testfp)
{
	TEST_START(Test_Templates_StaticMixedVar);

	CubicleSoft::StaticMixedVar<char[100]> TestMixedVar;
	bool x;

	x = TestMixedVar.IsNone();
	TEST_COMPARE(x, 1);

	TestMixedVar.SetBool(true);

	x = TestMixedVar.IsBool();
	TEST_COMPARE(x, 1);

	x = TestMixedVar.GetBool();
	TEST_COMPARE(x, 1);

	TestMixedVar.SetBool(false);

	x = TestMixedVar.GetBool();
	TEST_COMPARE(x, 0);

	TestMixedVar.SetInt(-5);

	x = TestMixedVar.IsBool();
	TEST_COMPARE(x, 0);

	x = TestMixedVar.IsInt();
	TEST_COMPARE(x, 1);

	x = TestMixedVar.IsUInt();
	TEST_COMPARE(x, 0);

	// Not a boolean anymore (but uses MxInt).
	x = TestMixedVar.GetBool();
	TEST_COMPARE(x, 1);

	x = (TestMixedVar.GetInt() == -5);
	TEST_COMPARE(x, 1);

	TestMixedVar.SetUInt(15);

	x = TestMixedVar.IsInt();
	TEST_COMPARE(x, 0);

	x = TestMixedVar.IsUInt();
	TEST_COMPARE(x, 1);

	x = (TestMixedVar.GetInt() == 15);
	TEST_COMPARE(x, 1);

	x = (TestMixedVar.GetUInt() == 15);
	TEST_COMPARE(x, 1);

	TestMixedVar.SetDouble(150.7);

	x = TestMixedVar.IsDouble();
	TEST_COMPARE(x, 1);

	// MxInt and MxDouble are two separate things.  Intentionally not a union.
	x = (TestMixedVar.GetInt() == 15);
	TEST_COMPARE(x, 1);

	// Floating point is reliably bad.
	x = (TestMixedVar.GetDouble() > 150.7 - 0.0001 && TestMixedVar.GetDouble() < 150.7 + 0.0001);
	TEST_COMPARE(x, 1);

	TestMixedVar.SetStr("");
	TestMixedVar.SetStr("test");

	x = TestMixedVar.IsStr();
	TEST_COMPARE(x, 1);

	x = (TestMixedVar.GetSize() == 4);
	TEST_COMPARE(x, 1);

	TestMixedVar.SetData("test\0test", 9);

	x = (TestMixedVar.GetSize() == 9);
	TEST_COMPARE(x, 1);

	TestMixedVar.PrependStr("test");

	x = (TestMixedVar.GetSize() == 13);
	TEST_COMPARE(x, 1);

	TestMixedVar.PrependData("test\0test", 9);

	x = (TestMixedVar.GetSize() == 22);
	TEST_COMPARE(x, 1);

	TestMixedVar.AppendStr("test");

	x = (TestMixedVar.GetSize() == 26);
	TEST_COMPARE(x, 1);

	TestMixedVar.AppendData("test\0test", 9);

	x = (TestMixedVar.GetSize() == 35);
	TEST_COMPARE(x, 1);

	TestMixedVar.AppendChar('/');
	TestMixedVar.AppendChar('/');
	TestMixedVar.AppendChar('/');

	x = (TestMixedVar.GetSize() == 38);
	TEST_COMPARE(x, 1);

	TestMixedVar.AppendMissingChar('/');
	TestMixedVar.AppendMissingChar('a');

	x = (TestMixedVar.GetSize() == 39);
	TEST_COMPARE(x, 1);

	TestMixedVar.RemoveTrailingChar('a');

	while (TestMixedVar.RemoveTrailingChar('/'))  {}

	x = (TestMixedVar.GetSize() == 35);
	TEST_COMPARE(x, 1);

	TestMixedVar.SetSize(1000);

	x = (TestMixedVar.GetSize() == 35);
	TEST_COMPARE(x, 1);

	TestMixedVar.SetSize(26);

	x = (TestMixedVar.GetSize() == 26);
	TEST_COMPARE(x, 1);

	x = (memcmp(TestMixedVar.GetStr(), "test\0testtesttest\0testtest", 26) == 0);
	TEST_COMPARE(x, 1);

	TEST_SUMMARY();

	TEST_RETURN();
}

int Test_Templates_UTF8MixedVar(FILE *Testfp)
{
	TEST_START(Test_Templates_UTF8MixedVar);

	CubicleSoft::UTF8::UTF8MixedVar<char[100]> TestMixedVar;
	WCHAR TempData[100];
	bool x;
	size_t y;

	TestMixedVar.SetUTF8(L"test", 4);

	x = (TestMixedVar.GetSize() == 4);
	TEST_COMPARE(x, 1);

	y = sizeof(TempData) / sizeof(WCHAR);
	TestMixedVar.ConvertFromUTF8(TempData, y);

	x = (y == 5);
	TEST_COMPARE(x, 1);

	TestMixedVar.AppendUTF8(L"test");

	x = (TestMixedVar.GetSize() == 8);
	TEST_COMPARE(x, 1);

	TEST_SUMMARY();

	TEST_RETURN();
}

int Test_Templates_FastFindReplace(FILE *Testfp)
{
	TEST_START(Test_Templates_FastFindReplace);

	char *TempStr;
	CubicleSoft::StaticMixedVar<char[100]> TempBuffer, TempBuffer2;
	bool x;
	size_t y;

	TempBuffer.SetStr("This is a test.");

	x = (CubicleSoft::FastReplace<char>::ReplaceAll(TempStr, y, TempBuffer.GetStr(), TempBuffer.GetSize(), "test", 4, "success", 7) == 1);
	TempStr[y] = '\0';
	TEST_COMPARE(x, 1);

	delete[] TempStr;

	y = TempBuffer2.GetMaxSize();
	x = (CubicleSoft::FastReplace<char>::StaticReplaceAll(TempBuffer2.GetStr(), y, TempBuffer.GetStr(), TempBuffer.GetSize(), "test", 4, "success", 7) == 1);
	TempBuffer2.SetSize(y);
	TEST_COMPARE(x, 1);

	TempBuffer.SetStr("This is a test with a @LONGERTOKEN@.");

	y = TempBuffer2.GetMaxSize();
	x = (CubicleSoft::FastReplace<char>::StaticReplaceAll(TempBuffer2.GetStr(), y, TempBuffer.GetStr(), TempBuffer.GetSize(), "@LONGERTOKEN@", (size_t)-1, "long replacement string", (size_t)-1) == 1);
	TempBuffer2.SetSize(y);
	TEST_COMPARE(x, 1);

	TempBuffer.SetStr("This is a test with a @REALLYLONGTOKEN@.");

	y = TempBuffer2.GetMaxSize();
	x = (CubicleSoft::FastReplace<char>::StaticReplaceAll(TempBuffer2.GetStr(), y, TempBuffer.GetStr(), TempBuffer.GetSize(), "@REALLYLONGTOKEN@", (size_t)-1, "long replacement string", (size_t)-1) == 1);
	TempBuffer2.SetSize(y);
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

int Test_Sync_TLSMixedVar(FILE *Testfp)
{
	TEST_START(Test_Sync_TLSMixedVar);

	bool x;

	x = GxSyncTLS.ThreadInit();
	TEST_COMPARE(x, 1);

	if (x)
	{
		CubicleSoft::Sync::TLS::MixedVar TestMixedVar(&GxSyncTLS);

		TestMixedVar.SetStr("");
		TestMixedVar.SetStr("test");

		x = TestMixedVar.IsStr();
		TEST_COMPARE(x, 1);

		x = (TestMixedVar.GetSize() == 4);
		TEST_COMPARE(x, 1);

		TestMixedVar.SetData("test\0test", 9);

		x = (TestMixedVar.GetSize() == 9);
		TEST_COMPARE(x, 1);

		TestMixedVar.PrependStr("test");

		x = (TestMixedVar.GetSize() == 13);
		TEST_COMPARE(x, 1);

		TestMixedVar.PrependData("test\0test", 9);

		x = (TestMixedVar.GetSize() == 22);
		TEST_COMPARE(x, 1);

		TestMixedVar.AppendStr("test");

		x = (TestMixedVar.GetSize() == 26);
		TEST_COMPARE(x, 1);

		TestMixedVar.AppendData("test\0test", 9);

		x = (TestMixedVar.GetSize() == 35);
		TEST_COMPARE(x, 1);

		TestMixedVar.AppendChar('/');
		TestMixedVar.AppendChar('/');
		TestMixedVar.AppendChar('/');

		x = (TestMixedVar.GetSize() == 38);
		TEST_COMPARE(x, 1);

		TestMixedVar.AppendMissingChar('/');
		TestMixedVar.AppendMissingChar('a');

		x = (TestMixedVar.GetSize() == 39);
		TEST_COMPARE(x, 1);

		TestMixedVar.RemoveTrailingChar('a');

		while (TestMixedVar.RemoveTrailingChar('/'))  {}

		x = (TestMixedVar.GetSize() == 35);
		TEST_COMPARE(x, 1);

		TestMixedVar.SetSize(1000);

		x = (TestMixedVar.GetSize() == 1000);
		TEST_COMPARE(x, 1);

		TestMixedVar.SetSize(26);

		x = (TestMixedVar.GetSize() == 26);
		TEST_COMPARE(x, 1);

		x = (memcmp(TestMixedVar.GetStr(), "test\0testtesttest\0testtest", 26) == 0);
		TEST_COMPARE(x, 1);

		x = (TestMixedVar.ReplaceStr("test", "123") == 6);
		TEST_COMPARE(x, 1);

		TestMixedVar.SetStr("This is a test.");

		x = (TestMixedVar.ReplaceStr("test", "success") == 1);
		TEST_COMPARE(x, 1);

		x = GxSyncTLS.ThreadEnd();
		TEST_COMPARE(x, 1);
	}

	TEST_SUMMARY();

	TEST_RETURN();
}

int Test_Environment_AppInfo(FILE *Testfp)
{
	TEST_START(Test_Environment_AppInfo);

	bool x;
	CubicleSoft::Environment::ProcessIDType CurrProcessID;
	CubicleSoft::Environment::ThreadIDType CurrThreadID;
	std::uint64_t CurrMicrosecond;

	CurrProcessID = CubicleSoft::Environment::AppInfo::GetCurrentProcessID();
	printf("\tCurrent process ID:  %u\n", (unsigned int)CurrProcessID);
	x = ((int)CurrProcessID != 0);
	TEST_COMPARE(x, 1);

	CurrThreadID = CubicleSoft::Environment::AppInfo::GetCurrentThreadID();
// Attempting to cast the thread ID fails to compile on platforms where the return value of GetCurrentThreadID() is a pointer instead of an integer.
//	printf("\tCurrent thread ID:  %u\n", (unsigned int)CurrThreadID);
//	x = ((int)CurrThreadID != 0);
//	TEST_COMPARE(x, 1);

	CurrMicrosecond = CubicleSoft::Environment::AppInfo::GetUnixMicrosecondTime();
	printf("\tCurrent UNIX microsecond:  %u\n", (unsigned int)CurrMicrosecond);
	x = (CurrMicrosecond != 0);
	TEST_COMPARE(x, 1);

	TEST_SUMMARY();

	TEST_RETURN();
}

int Test_UTF8_AppInfo(FILE *Testfp, const char *Argv0)
{
	TEST_START(Test_UTF8_AppInfo);

	bool x;
	char TempBuffer[1024];
	size_t y;

	y = sizeof(TempBuffer);
	x = CubicleSoft::UTF8::AppInfo::GetExecutableFilename(TempBuffer, y, Argv0);
	printf("\tCurrent executable filename:\n\t%s\n", TempBuffer);
	TEST_COMPARE(x, 1);

	y = sizeof(TempBuffer);
	x = CubicleSoft::UTF8::AppInfo::GetExecutablePath(TempBuffer, y, Argv0);
	printf("\tCurrent executable path:\n\t%s\n", TempBuffer);
	TEST_COMPARE(x, 1);

	y = sizeof(TempBuffer);
	x = CubicleSoft::UTF8::AppInfo::GetSystemAppStorageDir(TempBuffer, y);
	printf("\tCurrent system application storage path:\n\t%s\n", TempBuffer);
	TEST_COMPARE(x, 1);

	y = sizeof(TempBuffer);
	x = CubicleSoft::UTF8::AppInfo::GetSystemAppStorageDir(TempBuffer, y, "test_suite");
	printf("\tCurrent system application-specific storage path:\n\t%s\n", TempBuffer);
	TEST_COMPARE(x, 1);

	y = sizeof(TempBuffer);
	x = CubicleSoft::UTF8::AppInfo::GetCurrentUserAppStorageDir(TempBuffer, y);
	printf("\tCurrent user application storage path:\n\t%s\n", TempBuffer);
	TEST_COMPARE(x, 1);

	y = sizeof(TempBuffer);
	x = CubicleSoft::UTF8::AppInfo::GetCurrentUserAppStorageDir(TempBuffer, y, "test_suite");
	printf("\tCurrent user application-specific storage path:\n\t%s\n", TempBuffer);
	TEST_COMPARE(x, 1);

	y = sizeof(TempBuffer);
	x = CubicleSoft::UTF8::AppInfo::GetTempStorageDir(TempBuffer, y);
	printf("\tCurrent temporary file storage path:\n\t%s\n", TempBuffer);
	TEST_COMPARE(x, 1);

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

	x = CubicleSoft::UTF8::File::Exists("test/");
	TEST_COMPARE(x, 0);

	x = CubicleSoft::UTF8::Dir::Mkdir("test/awesome/path", 0777, false);
	TEST_COMPARE(x, 0);

	x = CubicleSoft::UTF8::Dir::Mkdir("test/awesome/path", 0777, true);
	TEST_COMPARE(x, 1);

	x = CubicleSoft::UTF8::File::Exists("test/");
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

#pragma optimize("", off)
#pragma GCC push_options
#pragma GCC optimize("O0")

int main(int argc, char **argv)
{
	if (argc == 1)
	{
		Test_Convert_Int(stdout);
		Test_Security_CSPRNG(stdout);
		Test_Sync_Event(stdout);
		Test_Sync_Mutex(stdout);
		Test_Sync_Semaphore(stdout);
		Test_Sync_ReadWriteLock(stdout);
		Test_Sync_SharedMem(stdout);
		Test_Templates_Cache(stdout);
		Test_Templates_List(stdout);
		Test_Templates_OrderedHash(stdout);
		Test_Templates_PackedOrderedHash(stdout);
		Test_Templates_Queue(stdout);
		Test_Templates_StaticVector(stdout);
		Test_Templates_Static2DArray(stdout);
		Test_Templates_StaticMixedVar(stdout);
		Test_Templates_UTF8MixedVar(stdout);
		Test_Templates_FastFindReplace(stdout);
		Test_Sync_TLS(stdout);
		Test_Sync_TLSMixedVar(stdout);
		Test_Environment_AppInfo(stdout);
		Test_UTF8_AppInfo(stdout, argv[0]);
		Test_UTF8_File(stdout);
		Test_UTF8_Dir(stdout);
	}
	else if (!strcmp("loop", argv[1]))
	{
		// NOTE:  Any given benchmark is strongly suspect if it produces nonsensical numbers such as "150 million nodes/sec".
		//        That could simply mean the important code bits were optimized away by the compiler into a while loop that does nothing.
		//        This benchmark can be used to confirm that such an optimization has taken place without having to delve into an assembler.

		printf("Generic while loop\n");
		printf("------------------\n");

		char FinalNum[100];
		std::uint32_t x;

		printf("Empty while loop speed test...");

		{
			// Baseline for failure.
			x = 0;
			time_t t1 = time(NULL);
			while (time(NULL) == t1)  {}
			t1 = time(NULL) + 3;
			while (t1 > time(NULL))
			{
				x++;
			}

			CubicleSoft::Convert::Int::ToString(FinalNum, 100, (std::uint64_t)(x / 3), ',');
			printf("\n\t%s iterations/sec", FinalNum);
		}

		printf("\n\n");
	}
	else if (!strcmp("synctls", argv[1]))
	{
		printf("Sync::TLS comparison benchmark\n");
		printf("------------------------------\n");

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

		printf("\n\n");
	}
	else if (!strcmp("hashkey", argv[1]))
	{
		printf("Hash key comparison benchmark\n");
		printf("-----------------------------\n");

		char HashesDone[100];
		char Data[10001];
		std::uint32_t x;
		size_t y, y2;

		size_t SkipPos[] = {
			1, 2, 3, 4, 5, 6, 7, 8, 9,
			10, 15, 20, 25, 30, 35, 40, 45, 50, 55, 60, 65, 70, 75, 80, 85, 90, 95,
			100, 150, 200, 250, 300, 350, 400, 450, 500, 550, 600, 650, 700, 750, 800, 850, 900, 950,
			1000, 1500, 2000, 2500, 3000, 3500, 4000, 4500, 5000, 5500, 6000, 6500, 7000, 7500, 8000, 8500, 9000, 9500,
			10000
		};

		for (y = 0; y < sizeof(Data); y++)  Data[y] = (char)y;

		printf("Running CacheUtil::GetHashKey() speed tests...");
		for (y = 0; y < sizeof(SkipPos) / sizeof(size_t); y++)
		{
			x = 0;
			time_t t1 = time(NULL);
			while (time(NULL) == t1)  {}
			t1 = time(NULL) + 3;
			y2 = SkipPos[y];
			while (t1 > time(NULL))
			{
				CubicleSoft::CacheUtil::GetHashKey((std::uint8_t *)Data, y2);

				x++;
			}

			CubicleSoft::Convert::Int::ToString(HashesDone, 100, (std::uint64_t)(x / 3), ',');
			printf("\n\t%u - %s hashes/sec", (unsigned int)y2, HashesDone);
		}

		printf("\n\n");

		printf("Running OrderedHashUtil::GetSipHashKey() speed tests...");
		for (y = 0; y < sizeof(SkipPos) / sizeof(size_t); y++)
		{
			x = 0;
			time_t t1 = time(NULL);
			while (time(NULL) == t1)  {}
			t1 = time(NULL) + 3;
			y2 = SkipPos[y];
			while (t1 > time(NULL))
			{
				CubicleSoft::OrderedHashUtil::GetSipHashKey((std::uint8_t *)Data, y2, 0, 0, 2, 4);

				x++;
			}

			CubicleSoft::Convert::Int::ToString(HashesDone, 100, (std::uint64_t)(x / 3), ',');
			printf("\n\t%u - %s hashes/sec", (unsigned int)y2, HashesDone);
		}

		printf("\n\n");
	}
	else if (!strcmp("list", argv[1]))
	{
		printf("List performance benchmark\n");
		printf("--------------------------\n");

		char NumNodes[100];
		std::uint32_t x, y;

		srand((unsigned int)time(NULL));

		printf("Running List speed tests...");

		{
			// Insertion.
			x = 0;
			time_t t1 = time(NULL);
			while (time(NULL) == t1)  {}
			t1 = time(NULL) + 3;
			CubicleSoft::List<std::uint32_t> TempList;
			while (t1 > time(NULL))
			{
				TempList.Push(x);

				x++;
			}

			CubicleSoft::Convert::Int::ToString(NumNodes, 100, (std::uint64_t)(x / 3), ',');
			printf("\n\tInsertion - %s nodes added/sec", NumNodes);
		}

		{
			// Detach/reattach performance.
			CubicleSoft::List<std::uint32_t> TempList;
			CubicleSoft::ListNode<std::uint32_t> *Node;
			for (x = 0; x < 1000000; x++)  TempList.Push(x);

			x = 0;
			time_t t1 = time(NULL);
			while (time(NULL) == t1)  {}
			t1 = time(NULL) + 3;
			while (t1 > time(NULL))
			{
				Node = TempList.Shift();
				TempList.Push(Node);

				x++;
			}

			CubicleSoft::Convert::Int::ToString(NumNodes, 100, (std::uint64_t)(x / 3), ',');
			printf("\n\tDetach/attach performance - %s nodes/sec", NumNodes);
		}

		{
			// Find performance.
			CubicleSoft::List<std::uint32_t> TempList;
			CubicleSoft::ListNode<std::uint32_t> *Node;
			for (x = 0; x < 1000000; x++)  TempList.Push(x);

			x = 0;
			time_t t1 = time(NULL);
			while (time(NULL) == t1)  {}
			t1 = time(NULL) + 3;
			while (t1 > time(NULL))
			{
				y = rand() % 1000000;
				Node = TempList.First();
				while (Node->Value != y)  Node = Node->Next();
				if (Node == NULL)  printf("Unable to find node!\n");

				x++;
			}

			CubicleSoft::Convert::Int::ToString(NumNodes, 100, (std::uint64_t)(x / 3), ',');
			printf("\n\tFind performance (1 million nodes) - %s nodes/sec", NumNodes);
		}

		printf("\n\n");
	}
	else if (!strcmp("hash", argv[1]))
	{
		printf("Hash performance benchmark\n");
		printf("--------------------------\n");

		char NumNodes[100];
		std::uint32_t x;

		printf("Running OrderedHash speed tests...");

		{
			// Integer keys, djb2 hash keys.
			x = 0;
			time_t t1 = time(NULL);
			while (time(NULL) == t1)  {}
			t1 = time(NULL) + 3;
			CubicleSoft::OrderedHash<std::uint32_t> TempHash(3);
			while (t1 > time(NULL))
			{
				TempHash.Push(x, x);

				x++;
			}

			CubicleSoft::Convert::Int::ToString(NumNodes, 100, (std::uint64_t)(x / 3), ',');
			printf("\n\tInteger keys, djb2 hash keys - %s nodes added/sec", NumNodes);
		}

		{
			// String keys (raw bytes of the integer implanted into the hash), djb2 hash keys.
			x = 0;
			time_t t1 = time(NULL);
			while (time(NULL) == t1)  {}
			t1 = time(NULL) + 3;
			char Str[30] = {0};
			CubicleSoft::OrderedHash<std::uint32_t> TempHash(3);
			while (t1 > time(NULL))
			{
				((std::uint32_t *)Str)[5] = x;
				TempHash.Push(Str, 30, x);

				x++;
			}

			CubicleSoft::Convert::Int::ToString(NumNodes, 100, (std::uint64_t)(x / 3), ',');
			printf("\n\tString keys, djb2 hash keys - %s nodes added/sec", NumNodes);
		}

		{
			// Integer keys, SipHash hash keys.
			x = 0;
			time_t t1 = time(NULL);
			while (time(NULL) == t1)  {}
			t1 = time(NULL) + 3;
			CubicleSoft::OrderedHash<std::uint32_t> TempHash(3, 0, 0);
			while (t1 > time(NULL))
			{
				TempHash.Push(x, x);

				x++;
			}

			CubicleSoft::Convert::Int::ToString(NumNodes, 100, (std::uint64_t)(x / 3), ',');
			printf("\n\tInteger keys, SipHash hash keys - %s nodes added/sec", NumNodes);
		}

		{
			// String keys (raw bytes of the integer implanted into the hash), SipHash hash keys.
			x = 0;
			time_t t1 = time(NULL);
			while (time(NULL) == t1)  {}
			t1 = time(NULL) + 3;
			char Str[30] = {0};
			CubicleSoft::OrderedHash<std::uint32_t> TempHash(3, 0, 0);
			while (t1 > time(NULL))
			{
				((std::uint32_t *)Str)[5] = x;
				TempHash.Push(Str, 30, x);

				x++;
			}

			CubicleSoft::Convert::Int::ToString(NumNodes, 100, (std::uint64_t)(x / 3), ',');
			printf("\n\tString keys, SipHash hash keys - %s nodes added/sec", NumNodes);
		}

		{
			// Integer keys, djb2 hash keys, detach/reattach performance.
			CubicleSoft::OrderedHash<std::uint32_t> TempHash(3);
			CubicleSoft::OrderedHashNode<std::uint32_t> *Node;
			for (x = 0; x < 1000000; x++)  TempHash.Push(x, x);

			x = 0;
			time_t t1 = time(NULL);
			while (time(NULL) == t1)  {}
			t1 = time(NULL) + 3;
			while (t1 > time(NULL))
			{
				Node = TempHash.Shift(false);
				TempHash.Push(Node);

				x++;
			}

			CubicleSoft::Convert::Int::ToString(NumNodes, 100, (std::uint64_t)(x / 3), ',');
			printf("\n\tInteger keys, detach/attach performance - %s nodes/sec", NumNodes);
		}

		{
			// String keys, djb2 hash keys, detach/reattach performance.
			CubicleSoft::OrderedHash<std::uint32_t> TempHash(3);
			CubicleSoft::OrderedHashNode<std::uint32_t> *Node;
			char Str[30] = {0};
			for (x = 0; x < 1000000; x++)
			{
				((std::uint32_t *)Str)[5] = x;
				TempHash.Push(Str, 30, x);
			}

			x = 0;
			time_t t1 = time(NULL);
			while (time(NULL) == t1)  {}
			t1 = time(NULL) + 3;
			while (t1 > time(NULL))
			{
				Node = TempHash.Shift(false);
				TempHash.Push(Node);

				x++;
			}

			CubicleSoft::Convert::Int::ToString(NumNodes, 100, (std::uint64_t)(x / 3), ',');
			printf("\n\tString keys, detach/attach performance - %s nodes/sec", NumNodes);
		}

		{
			// Integer keys, djb2 hash keys, find performance.
			CubicleSoft::OrderedHash<std::uint32_t> TempHash(3);
			CubicleSoft::OrderedHashNode<std::uint32_t> *Node;
			for (x = 0; x < 1000000; x++)  TempHash.Push(x, x);

			x = 0;
			time_t t1 = time(NULL);
			while (time(NULL) == t1)  {}
			t1 = time(NULL) + 3;
			while (t1 > time(NULL))
			{
				Node = TempHash.Find(rand() % 1000000);
				if (Node == NULL)  printf("Unable to find node!\n");

				x++;
			}

			CubicleSoft::Convert::Int::ToString(NumNodes, 100, (std::uint64_t)(x / 3), ',');
			printf("\n\tInteger keys, find performance (1 million nodes) - %s nodes/sec", NumNodes);
		}

		{
			// String keys, djb2 hash keys, find performance.
			CubicleSoft::OrderedHash<std::uint32_t> TempHash(3);
			CubicleSoft::OrderedHashNode<std::uint32_t> *Node;
			char Str[30] = {0};
			for (x = 0; x < 1000000; x++)
			{
				((std::uint32_t *)Str)[5] = x;
				TempHash.Push(Str, 30, x);
			}

			x = 0;
			time_t t1 = time(NULL);
			while (time(NULL) == t1)  {}
			t1 = time(NULL) + 3;
			while (t1 > time(NULL))
			{
				((std::uint32_t *)Str)[5] = rand() % 1000000;
				Node = TempHash.Find(Str, 30);
				if (Node == NULL)  printf("Unable to find node!\n");

				x++;
			}

			CubicleSoft::Convert::Int::ToString(NumNodes, 100, (std::uint64_t)(x / 3), ',');
			printf("\n\tString keys, find performance (1 million nodes) - %s nodes/sec", NumNodes);
		}

		printf("\n\n");

		printf("Running PackedOrderedHash speed tests...");

		{
			// Integer keys, djb2 hash keys.
			x = 0;
			time_t t1 = time(NULL);
			while (time(NULL) == t1)  {}
			t1 = time(NULL) + 3;
			CubicleSoft::PackedOrderedHash<std::uint32_t> TempHash(3);
			while (t1 > time(NULL))
			{
				TempHash.Set(x, x);

				x++;
			}

			CubicleSoft::Convert::Int::ToString(NumNodes, 100, (std::uint64_t)(x / 3), ',');
			printf("\n\tInteger keys, djb2 hash keys - %s nodes added/sec", NumNodes);
		}

		{
			// String keys (raw bytes of the integer implanted into the hash), djb2 hash keys.
			x = 0;
			time_t t1 = time(NULL);
			while (time(NULL) == t1)  {}
			t1 = time(NULL) + 3;
			char Str[30] = {0};
			CubicleSoft::PackedOrderedHash<std::uint32_t> TempHash(3);
			while (t1 > time(NULL))
			{
				((std::uint32_t *)Str)[5] = x;
				TempHash.Set(Str, 30, x);

				x++;
			}

			CubicleSoft::Convert::Int::ToString(NumNodes, 100, (std::uint64_t)(x / 3), ',');
			printf("\n\tString keys, djb2 hash keys - %s nodes added/sec", NumNodes);
		}

		{
			// Integer keys, SipHash hash keys.
			x = 0;
			time_t t1 = time(NULL);
			while (time(NULL) == t1)  {}
			t1 = time(NULL) + 3;
			CubicleSoft::PackedOrderedHash<std::uint32_t> TempHash(3, 0, 0);
			while (t1 > time(NULL))
			{
				TempHash.Set(x, x);

				x++;
			}

			CubicleSoft::Convert::Int::ToString(NumNodes, 100, (std::uint64_t)(x / 3), ',');
			printf("\n\tInteger keys, SipHash hash keys - %s nodes added/sec", NumNodes);
		}

		{
			// String keys (raw bytes of the integer implanted into the hash), SipHash hash keys.
			x = 0;
			time_t t1 = time(NULL);
			while (time(NULL) == t1)  {}
			t1 = time(NULL) + 3;
			char Str[30] = {0};
			CubicleSoft::PackedOrderedHash<std::uint32_t> TempHash(3, 0, 0);
			while (t1 > time(NULL))
			{
				((std::uint32_t *)Str)[5] = x;
				TempHash.Set(Str, 30, x);

				x++;
			}

			CubicleSoft::Convert::Int::ToString(NumNodes, 100, (std::uint64_t)(x / 3), ',');
			printf("\n\tString keys, SipHash hash keys - %s nodes added/sec", NumNodes);
		}

		{
			// Integer keys, djb2 hash keys, find performance.
			CubicleSoft::PackedOrderedHash<std::uint32_t> TempHash(3);
			CubicleSoft::PackedOrderedHashNode<std::uint32_t> *Node;
			for (x = 0; x < 1000000; x++)  TempHash.Set(x, x);

			x = 0;
			time_t t1 = time(NULL);
			while (time(NULL) == t1)  {}
			t1 = time(NULL) + 3;
			while (t1 > time(NULL))
			{
				Node = TempHash.Find(rand() % 1000000);
				if (Node == NULL)  printf("Unable to find node!\n");

				x++;
			}

			CubicleSoft::Convert::Int::ToString(NumNodes, 100, (std::uint64_t)(x / 3), ',');
			printf("\n\tInteger keys, find performance (1 million nodes) - %s nodes/sec", NumNodes);
		}

		{
			// String keys, djb2 hash keys, find performance.
			CubicleSoft::PackedOrderedHash<std::uint32_t> TempHash(3);
			CubicleSoft::PackedOrderedHashNode<std::uint32_t> *Node;
			char Str[30] = {0};
			for (x = 0; x < 1000000; x++)
			{
				((std::uint32_t *)Str)[5] = x;
				TempHash.Set(Str, 30, x);
			}

			x = 0;
			time_t t1 = time(NULL);
			while (time(NULL) == t1)  {}
			t1 = time(NULL) + 3;
			while (t1 > time(NULL))
			{
				((std::uint32_t *)Str)[5] = rand() % 1000000;
				Node = TempHash.Find(Str, 30);
				if (Node == NULL)  printf("Unable to find node!\n");

				x++;
			}

			CubicleSoft::Convert::Int::ToString(NumNodes, 100, (std::uint64_t)(x / 3), ',');
			printf("\n\tString keys, find performance (1 million nodes) - %s nodes/sec", NumNodes);
		}

		printf("\n\n");
	}
	else if (!strcmp("server", argv[1]))
	{
/*
		// Initialize networking.
		CubicleSoft::Network::Init TempInit;

		if (!TempInit.Started())
		{
			printf("Networking failed to initialize.\n");

			return 1;
		}

		// Initialize the async helper.
		CubicleSoft::Network::AsyncHelper TempHelper;

		if (!TempHelper.Initialized())
		{
			printf("Asynchronous helper failed to initialize.\n");

			return 1;
		}

		// Start a TCP/IP server.
		CubicleSoft::Network::Server TempServer;

		if (!TempServer.SetAsyncHelperInfo(TempHelper, 1, 2))
		{
			// This should never happen.
			printf("Failed to set the asynchronous helper.\n");

			return 1;
		}

		if (!TempServer.Start(CubicleSoft::Network::Server::ModeTCP, NULL, 7890))
		{
			printf("Failed to start TCP server on port 7890.\n");

			return 1;
		}

		// Begin accepting connections.
		if (!TempServer.BeginAccept())
		{
			printf("Failed to start accepting connections on port 7890.\n");

			return 1;
		}

		// Enter the main processing loop.
		printf("Echo server started on port 7890.\n");
		CubicleSoft::Network::AsyncHelperNode *HelperNode;
		CubicleSoft::QueueNoCopy<std::int64_t> ClientNums;
		CubicleSoft::QueueNode<std::int64_t> *ClientNum;
		do
		{
			// Wait until something happens.
			if (TempServer.ProcessWait(TempHelper.Wait(TempServer.GetMaxWait())))
			{
				// Queue up remaining waiting packets until either an error occurs or no packets are left.
				while ((HelperNode = TempHelper.Wait(0)) != NULL)
				{
					TempServer.ProcessWait(HelperNode);
				}

				// Handle new connections.
				if (TempServer.GetConnected(ClientNums))
				{
					ClientNum = ClientNums.First();
					while (ClientNum != NULL)
					{
						printf("Client %" PRId64 " connected.\n", ClientNum->Value);

						ClientNum = ClientNum->Next();
					}

					TempServer.Release(ClientNums);
				}

				// Handle disconnects.
				if (TempServer.GetDisconnected(ClientNums))
				{
					ClientNum = ClientNums.First();
					while (ClientNum != NULL)
					{
						TempServer.Disconnect(ClientNum->Value);

						printf("Client %" PRId64 " disconnected.\n", ClientNum->Value);

						ClientNum = ClientNum->Next();
					}

					TempServer.Release(ClientNums);
				}

				// Disallow clients to remain inactive (30 second timeout).
				if (TempServer.GetInactive(ClientNums, 30))
				{
					ClientNum = ClientNums.First();
					while (ClientNum != NULL)
					{
						TempServer.Disconnect(ClientNum->Value);

						printf("Client %" PRId64 " timed out.\n", ClientNum->Value);

						ClientNum = ClientNum->Next();
					}

					TempServer.Release(ClientNums);
				}

				// Read data and echo it back to the client.
				if (TempServer.GetReadReady(ClientNums))
				{
					ClientNum = ClientNums.First();
					while (ClientNum != NULL)
					{
						char TempBuffer[1024];
						size_t BufferSize;

						printf("Client %" PRId64 ":  %d bytes ready\n", ClientNum->Value, TempServer.GetReadBytesReady(ClientNum->Value));

						if (TempServer.Read(ClientNum->Value, (std::uint8_t *)TempBuffer, sizeof(TempBuffer) - 1, &BufferSize))
						{
							TempBuffer[BufferSize] = '\0';
							printf("Received:  %s\n", TempBuffer);

							TempServer.Write(ClientNum->Value, (std::uint8_t *)TempBuffer, BufferSize);
						}

						ClientNum = ClientNum->Next();
					}

					TempServer.Release(ClientNums);
				}

				// Ignore write ready notifications.
				if (TempServer.GetWriteReady(ClientNums))  TempServer.Release(ClientNums);
			}

			// Allow more connections and perform periodic garbage collection.
			TempServer.BeginAccept();
		} while (1);
*/
	}
	else
	{
		printf("Unknown benchmark/option specified.\n");
	}

	return 0;
}

#pragma optimize("", on)
#pragma GCC pop_options
