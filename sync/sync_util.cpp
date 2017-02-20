// Cross-platform, cross-process synchronization utility functions.
// (C) 2016 CubicleSoft.  All Rights Reserved.

#include "sync_util.h"

#if defined(_WIN32) || defined(WIN32) || defined(_WIN64) || defined(WIN64)
#else
	#include <sys/mman.h>
	#include <sys/types.h>
	#include <sys/stat.h>
	#include <sys/time.h>
	#include <fcntl.h>
	#include <unistd.h>
	#include <errno.h>
	#include <limits.h>

	#include <cstring>

	#ifdef __APPLE__
		#include <mach/clock.h>
		#include <mach/mach.h>

		#ifndef SHM_NAME_MAX
			#define SHM_NAME_MAX 31
		#endif
	#else
		#ifndef SHM_NAME_MAX
			#define SHM_NAME_MAX 255
		#endif
	#endif
#endif

#include <cstdio>

namespace CubicleSoft
{
	namespace Sync
	{
#if defined(_WIN32) || defined(WIN32) || defined(_WIN64) || defined(WIN64)
		// Windows.
		ThreadIDType Util::GetCurrentThreadID()
		{
			return ::GetCurrentThreadId();
		}

		std::uint64_t Util::GetUnixMicrosecondTime()
		{
			FILETIME TempTime;
			ULARGE_INTEGER TempTime2;
			std::uint64_t Result;

			::GetSystemTimeAsFileTime(&TempTime);
			TempTime2.HighPart = TempTime.dwHighDateTime;
			TempTime2.LowPart = TempTime.dwLowDateTime;
			Result = TempTime2.QuadPart;

			Result = (Result / 10) - (std::uint64_t)11644473600000000ULL;

			return Result;
		}
#else
		// Dear Apple:  You hire plenty of developers, so please fix your OS.
		int CSGX__ClockGetTimeRealtime(struct timespec *ts)
		{
	#ifdef __APPLE__
			clock_serv_t cclock;
			mach_timespec_t mts;

			if (host_get_clock_service(mach_host_self(), CALENDAR_CLOCK, &cclock) != KERN_SUCCESS)  return -1;
			if (clock_get_time(cclock, &mts) != KERN_SUCCESS)  return -1;
			if (mach_port_deallocate(mach_task_self(), cclock) != KERN_SUCCESS)  return -1;

			ts->tv_sec = mts.tv_sec;
			ts->tv_nsec = mts.tv_nsec;

			return 0;
	#else
			return clock_gettime(CLOCK_REALTIME, ts);
	#endif
		}

		// POSIX pthreads.
		ThreadIDType Util::GetCurrentThreadID()
		{
			return pthread_self();
		}

		std::uint64_t Util::GetUnixMicrosecondTime()
		{
			struct timeval TempTime;

			if (gettimeofday(&TempTime, NULL))  return 0;

			return (std::uint64_t)((std::uint64_t)TempTime.tv_sec * (std::uint64_t)1000000 + (std::uint64_t)TempTime.tv_usec);
		}

		size_t Util::GetUnixSystemAlignmentSize()
		{
			struct {
				int MxInt;
			} x;

			struct {
				int MxInt;
				char MxChar;
			} y;

			return sizeof(y) - sizeof(x);
		}

		size_t Util::AlignUnixSize(size_t Size)
		{
			size_t AlignSize = GetUnixSystemAlignmentSize();

			if (Size % AlignSize)  Size += AlignSize - (Size % AlignSize);

			return Size;
		}

		int Util::InitUnixNamedMem(char *&ResultMem, size_t &StartPos, const char *Prefix, const char *Name, size_t Size)
		{
			int Result = -1;
			ResultMem = NULL;
			StartPos = (Name != NULL ? AlignUnixSize(1) + AlignUnixSize(sizeof(pthread_mutex_t)) + AlignUnixSize(sizeof(std::uint32_t)) : 0);

			// First byte indicates initialization status (0 = completely uninitialized, 1 = first mutex initialized, 2 = ready).
			// Next few bytes are a shared mutex object.
			// Size bytes follow for whatever.
			Size += StartPos;
			Size = AlignUnixSize(Size);

			if (Name == NULL)
			{
				ResultMem = new char[Size];

				Result = 0;
			}
			else
			{
				// Deal with really small name limits with a pseudo-hash.
				char Name2[SHM_NAME_MAX], Nums[50];
				size_t x, x2 = 0, y = strlen(Prefix), z = 0;

				memset(Name2, 0, sizeof(Name2));

				for (x = 0; x < y; x++)
				{
					Name2[x2] = (char)(((unsigned int)(unsigned char)Name2[x2]) * 37 + ((unsigned int)(unsigned char)Prefix[x]));
					x2++;

					if (x2 == sizeof(Name2) - 1)
					{
						x2 = 1;
						z++;
					}
				}

				sprintf(Nums, "-%u-%u-", (unsigned int)GetUnixSystemAlignmentSize(), (unsigned int)Size);

				y = strlen(Nums);
				for (x = 0; x < y; x++)
				{
					Name2[x2] = (char)(((unsigned int)(unsigned char)Name2[x2]) * 37 + ((unsigned int)(unsigned char)Nums[x]));
					x2++;

					if (x2 == sizeof(Name2) - 1)
					{
						x2 = 1;
						z++;
					}
				}

				y = strlen(Name);
				for (x = 0; x < y; x++)
				{
					Name2[x2] = (char)(((unsigned int)(unsigned char)Name2[x2]) * 37 + ((unsigned int)(unsigned char)Name[x]));
					x2++;

					if (x2 == sizeof(Name2) - 1)
					{
						x2 = 1;
						z++;
					}
				}

				// Normalize the alphabet if it looped.
				if (z)
				{
					unsigned char TempChr;
					y = (z > 1 ? sizeof(Name2) - 1 : x2);
					for (x = 1; x < y; x++)
					{
						TempChr = ((unsigned char)Name2[x]) & 0x3F;

						if (TempChr < 10)  TempChr += '0';
						else if (TempChr < 36)  TempChr = TempChr - 10 + 'A';
						else if (TempChr < 62)  TempChr = TempChr - 36 + 'a';
						else if (TempChr == 62)  TempChr = '_';
						else  TempChr = '-';

						Name2[x] = (char)TempChr;
					}
				}

				for (x = 1; x < sizeof(Name2) && Name2[x]; x++)
				{
					if (Name2[x] == '\\' || Name2[x] == '/')  Name2[x] = '_';
				}

				pthread_mutex_t *MutexPtr;
				std::uint32_t *RefCountPtr;

				// Attempt to create the named shared memory object.
				mode_t PrevMask = umask(0);
				int fp = shm_open(Name2, O_RDWR | O_CREAT | O_EXCL, 0666);
				if (fp > -1)
				{
					// Ignore platform errors (for now).
					while (ftruncate(fp, Size) < 0 && errno == EINTR)
					{
					}

					ResultMem = (char *)mmap(NULL, Size, PROT_READ | PROT_WRITE, MAP_SHARED, fp, 0);
					if (ResultMem == MAP_FAILED)  ResultMem = NULL;
					else
					{
						pthread_mutexattr_t MutexAttr;

						pthread_mutexattr_init(&MutexAttr);
						pthread_mutexattr_setpshared(&MutexAttr, PTHREAD_PROCESS_SHARED);

						MutexPtr = reinterpret_cast<pthread_mutex_t *>(ResultMem + AlignUnixSize(1));
						RefCountPtr = reinterpret_cast<std::uint32_t *>(ResultMem + AlignUnixSize(1) + AlignUnixSize(sizeof(pthread_mutex_t)));

						pthread_mutex_init(MutexPtr, &MutexAttr);
						pthread_mutex_lock(MutexPtr);

						ResultMem[0] = '\x01';
						RefCountPtr[0] = 1;

						Result = 0;
					}

					close(fp);
				}
				else
				{
					// Attempt to open the named shared memory object.
					fp = shm_open(Name2, O_RDWR, 0666);
					if (fp > -1)
					{
						// Ignore platform errors (for now).
						while (ftruncate(fp, Size) < 0 && errno == EINTR)
						{
						}

						ResultMem = (char *)mmap(NULL, Size, PROT_READ | PROT_WRITE, MAP_SHARED, fp, 0);
						if (ResultMem == MAP_FAILED)  ResultMem = NULL;
						else
						{
							// Wait until the space is fully initialized.
							if (ResultMem[0] == '\x00')
							{
								while (ResultMem[0] == '\x00')
								{
									usleep(2000);
								}
							}

							char *MemPtr = ResultMem + AlignUnixSize(1);
							MutexPtr = reinterpret_cast<pthread_mutex_t *>(MemPtr);
							MemPtr += AlignUnixSize(sizeof(pthread_mutex_t));

							RefCountPtr = reinterpret_cast<std::uint32_t *>(MemPtr);
							MemPtr += AlignUnixSize(sizeof(std::uint32_t));

							pthread_mutex_lock(MutexPtr);

							if (RefCountPtr[0])  Result = 1;
							else
							{
								// If this is the first reference, reset the RAM to 0's for platform consistency to force a rebuild of the object.
								memset(MemPtr, 0, Size);

								Result = 0;
							}

							RefCountPtr[0]++;

							pthread_mutex_unlock(MutexPtr);
						}

						close(fp);
					}
				}

				umask(PrevMask);
			}

			return Result;
		}

		void Util::UnixNamedMemReady(char *MemPtr)
		{
			pthread_mutex_unlock(reinterpret_cast<pthread_mutex_t *>(MemPtr + AlignUnixSize(1)));
		}

		void Util::UnmapUnixNamedMem(char *MemPtr, size_t Size)
		{
			pthread_mutex_t *MutexPtr;
			std::uint32_t *RefCountPtr;

			char *MemPtr2 = MemPtr + AlignUnixSize(1);
			MutexPtr = reinterpret_cast<pthread_mutex_t *>(MemPtr2);
			MemPtr2 += AlignUnixSize(sizeof(pthread_mutex_t));

			RefCountPtr = reinterpret_cast<std::uint32_t *>(MemPtr2);

			pthread_mutex_lock(MutexPtr);
			if (RefCountPtr[0])  RefCountPtr[0]--;
			pthread_mutex_unlock(MutexPtr);

			munmap(MemPtr, AlignUnixSize(1) + AlignUnixSize(sizeof(pthread_mutex_t)) + AlignUnixSize(sizeof(std::uint32_t)) + Size);
		}


		size_t Util::GetUnixSemaphoreSize()
		{
			return AlignUnixSize(sizeof(pthread_mutex_t)) + AlignUnixSize(sizeof(std::uint32_t)) + AlignUnixSize(sizeof(std::uint32_t)) + AlignUnixSize(sizeof(pthread_cond_t));
		}

		void Util::GetUnixSemaphore(UnixSemaphoreWrapper &Result, char *Mem)
		{
			Result.MxMutex = reinterpret_cast<pthread_mutex_t *>(Mem);
			Mem += AlignUnixSize(sizeof(pthread_mutex_t));

			Result.MxCount = reinterpret_cast<std::uint32_t *>(Mem);
			Mem += AlignUnixSize(sizeof(std::uint32_t));

			Result.MxMax = reinterpret_cast<std::uint32_t *>(Mem);
			Mem += AlignUnixSize(sizeof(std::uint32_t));

			Result.MxCond = reinterpret_cast<pthread_cond_t *>(Mem);
		}

		void Util::InitUnixSemaphore(UnixSemaphoreWrapper &UnixSemaphore, bool Shared, std::uint32_t Start, std::uint32_t Max)
		{
			pthread_mutexattr_t MutexAttr;
			pthread_condattr_t CondAttr;

			pthread_mutexattr_init(&MutexAttr);
			pthread_condattr_init(&CondAttr);

			if (Shared)
			{
				pthread_mutexattr_setpshared(&MutexAttr, PTHREAD_PROCESS_SHARED);
				pthread_condattr_setpshared(&CondAttr, PTHREAD_PROCESS_SHARED);
			}

			pthread_mutex_init(UnixSemaphore.MxMutex, &MutexAttr);
			if (Start > Max)  Start = Max;
			UnixSemaphore.MxCount[0] = Start;
			UnixSemaphore.MxMax[0] = Max;
			pthread_cond_init(UnixSemaphore.MxCond, &CondAttr);

			pthread_condattr_destroy(&CondAttr);
			pthread_mutexattr_destroy(&MutexAttr);
		}

		bool Util::WaitForUnixSemaphore(UnixSemaphoreWrapper &UnixSemaphore, std::uint32_t Wait)
		{
			if (Wait == 0)
			{
				// Avoid the scenario of deadlock on the semaphore itself for 0 wait.
				if (pthread_mutex_trylock(UnixSemaphore.MxMutex) != 0)  return false;
			}
			else
			{
				if (pthread_mutex_lock(UnixSemaphore.MxMutex) != 0)  return false;
			}

			bool Result = false;

			if (UnixSemaphore.MxCount[0])
			{
				UnixSemaphore.MxCount[0]--;

				Result = true;
			}
			else if (Wait == INFINITE)
			{
				int Result2;
				do
				{
					Result2 = pthread_cond_wait(UnixSemaphore.MxCond, UnixSemaphore.MxMutex);
					if (Result2 != 0)  break;
				} while (!UnixSemaphore.MxCount[0]);

				if (Result2 == 0)
				{
					UnixSemaphore.MxCount[0]--;

					Result = true;
				}
			}
			else if (Wait == 0)
			{
				// Failed to obtain lock.  Nothing to do.
			}
			else
			{
				struct timespec TempTime;

				if (CSGX__ClockGetTimeRealtime(&TempTime) == -1)  return false;
				TempTime.tv_sec += Wait / 1000;
				TempTime.tv_nsec += (Wait % 1000) * 1000000;
				TempTime.tv_sec += TempTime.tv_nsec / 1000000000;
				TempTime.tv_nsec = TempTime.tv_nsec % 1000000000;

				int Result2;
				do
				{
					// Some platforms have pthread_cond_timedwait() but not pthread_mutex_timedlock() or sem_timedwait() (e.g. Mac OSX).
					Result2 = pthread_cond_timedwait(UnixSemaphore.MxCond, UnixSemaphore.MxMutex, &TempTime);
					if (Result2 != 0)  break;
				} while (!UnixSemaphore.MxCount[0]);

				if (Result2 == 0)
				{
					UnixSemaphore.MxCount[0]--;

					Result = true;
				}
			}

			pthread_mutex_unlock(UnixSemaphore.MxMutex);

			return Result;
		}

		bool Util::ReleaseUnixSemaphore(UnixSemaphoreWrapper &UnixSemaphore, std::uint32_t *PrevVal)
		{
			if (pthread_mutex_lock(UnixSemaphore.MxMutex) != 0)  return false;

			if (PrevVal != NULL)  *PrevVal = UnixSemaphore.MxCount[0];
			UnixSemaphore.MxCount[0]++;
			if (UnixSemaphore.MxCount[0] > UnixSemaphore.MxMax[0])  UnixSemaphore.MxCount[0] = UnixSemaphore.MxMax[0];

			// Let a waiting thread have at it.
			pthread_cond_signal(UnixSemaphore.MxCond);

			pthread_mutex_unlock(UnixSemaphore.MxMutex);

			return true;
		}

		void Util::FreeUnixSemaphore(UnixSemaphoreWrapper &UnixSemaphore)
		{
			pthread_mutex_destroy(UnixSemaphore.MxMutex);
			pthread_cond_destroy(UnixSemaphore.MxCond);
		}


		size_t Util::GetUnixEventSize()
		{
			return AlignUnixSize(sizeof(pthread_mutex_t)) + AlignUnixSize(2) + AlignUnixSize(sizeof(std::uint32_t)) + AlignUnixSize(sizeof(pthread_cond_t));
		}

		void Util::GetUnixEvent(UnixEventWrapper &Result, char *Mem)
		{
			Result.MxMutex = reinterpret_cast<pthread_mutex_t *>(Mem);
			Mem += AlignUnixSize(sizeof(pthread_mutex_t));

			Result.MxManual = Mem;
			Result.MxSignaled = Mem + 1;
			Mem += AlignUnixSize(2);

			Result.MxWaiting = reinterpret_cast<std::uint32_t *>(Mem);
			Mem += AlignUnixSize(sizeof(std::uint32_t));

			Result.MxCond = reinterpret_cast<pthread_cond_t *>(Mem);
		}

		void Util::InitUnixEvent(UnixEventWrapper &UnixEvent, bool Shared, bool Manual, bool Signaled)
		{
			pthread_mutexattr_t MutexAttr;
			pthread_condattr_t CondAttr;

			pthread_mutexattr_init(&MutexAttr);
			pthread_condattr_init(&CondAttr);

			if (Shared)
			{
				pthread_mutexattr_setpshared(&MutexAttr, PTHREAD_PROCESS_SHARED);
				pthread_condattr_setpshared(&CondAttr, PTHREAD_PROCESS_SHARED);
			}

			pthread_mutex_init(UnixEvent.MxMutex, &MutexAttr);
			UnixEvent.MxManual[0] = (Manual ? '\x01' : '\x00');
			UnixEvent.MxSignaled[0] = (Signaled ? '\x01' : '\x00');
			UnixEvent.MxWaiting[0] = 0;
			pthread_cond_init(UnixEvent.MxCond, &CondAttr);

			pthread_condattr_destroy(&CondAttr);
			pthread_mutexattr_destroy(&MutexAttr);
		}

		bool Util::WaitForUnixEvent(UnixEventWrapper &UnixEvent, std::uint32_t Wait)
		{
			if (Wait == 0)
			{
				// Avoid the scenario of deadlock on the semaphore itself for 0 wait.
				if (pthread_mutex_trylock(UnixEvent.MxMutex) != 0)  return false;
			}
			else
			{
				if (pthread_mutex_lock(UnixEvent.MxMutex) != 0)  return false;
			}

			bool Result = false;

			// Avoid a potential starvation issue by only allowing signaled manual events OR if there are no other waiting threads.
			if (UnixEvent.MxSignaled[0] != '\x00' && (UnixEvent.MxManual[0] != '\x00' || !UnixEvent.MxWaiting[0]))
			{
				// Reset auto events.
				if (UnixEvent.MxManual[0] == '\x00')  UnixEvent.MxSignaled[0] = '\x00';

				Result = true;
			}
			else if (Wait == INFINITE)
			{
				UnixEvent.MxWaiting[0]++;

				int Result2;
				do
				{
					Result2 = pthread_cond_wait(UnixEvent.MxCond, UnixEvent.MxMutex);
					if (Result2 != 0)  break;
				} while (UnixEvent.MxSignaled[0] == '\x00');

				UnixEvent.MxWaiting[0]--;

				if (Result2 == 0)
				{
					// Reset auto events.
					if (UnixEvent.MxManual[0] == '\x00')  UnixEvent.MxSignaled[0] = '\x00';

					Result = true;
				}
			}
			else if (Wait == 0)
			{
				// Failed to obtain lock.  Nothing to do.
			}
			else
			{
				struct timespec TempTime;

				if (CSGX__ClockGetTimeRealtime(&TempTime) == -1)
				{
					pthread_mutex_unlock(UnixEvent.MxMutex);

					return false;
				}

				TempTime.tv_sec += Wait / 1000;
				TempTime.tv_nsec += (Wait % 1000) * 1000000;
				TempTime.tv_sec += TempTime.tv_nsec / 1000000000;
				TempTime.tv_nsec = TempTime.tv_nsec % 1000000000;

				UnixEvent.MxWaiting[0]++;

				int Result2;
				do
				{
					// Some platforms have pthread_cond_timedwait() but not pthread_mutex_timedlock() or sem_timedwait() (e.g. Mac OSX).
					Result2 = pthread_cond_timedwait(UnixEvent.MxCond, UnixEvent.MxMutex, &TempTime);
					if (Result2 != 0)  break;
				} while (UnixEvent.MxSignaled[0] == '\x00');

				UnixEvent.MxWaiting[0]--;

				if (Result2 == 0)
				{
					// Reset auto events.
					if (UnixEvent.MxManual[0] == '\x00')  UnixEvent.MxSignaled[0] = '\x00';

					Result = true;
				}
			}

			pthread_mutex_unlock(UnixEvent.MxMutex);

			return Result;
		}

		bool Util::FireUnixEvent(UnixEventWrapper &UnixEvent)
		{
			if (pthread_mutex_lock(UnixEvent.MxMutex) != 0)  return false;

			UnixEvent.MxSignaled[0] = '\x01';

			// Let all waiting threads through for manual events, otherwise just one waiting thread (if any).
			if (UnixEvent.MxManual[0] != '\x00')  pthread_cond_broadcast(UnixEvent.MxCond);
			else  pthread_cond_signal(UnixEvent.MxCond);

			pthread_mutex_unlock(UnixEvent.MxMutex);

			return true;
		}

		// Only call for manual events.
		bool Util::ResetUnixEvent(UnixEventWrapper &UnixEvent)
		{
			if (UnixEvent.MxManual[0] == '\x00')  return false;
			if (pthread_mutex_lock(UnixEvent.MxMutex) != 0)  return false;

			UnixEvent.MxSignaled[0] = '\x00';

			pthread_mutex_unlock(UnixEvent.MxMutex);

			return true;
		}

		void Util::FreeUnixEvent(UnixEventWrapper &UnixEvent)
		{
			pthread_mutex_destroy(UnixEvent.MxMutex);
			pthread_cond_destroy(UnixEvent.MxCond);
		}
#endif
	}
}
