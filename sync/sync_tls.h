// Cross-platform thread local storage memory allocation class.  Built for short-lived, small, in-thread memory allocations.
// (C) 2016 CubicleSoft.  All Rights Reserved.

#ifndef CUBICLESOFT_SYNC_TLS
#define CUBICLESOFT_SYNC_TLS

#include "sync_util.h"
#include "../templates/detachable_queue.h"
#include "../templates/static_vector.h"

#if defined(_WIN32) || defined(WIN32) || defined(_WIN64) || defined(WIN64)
	#include <windows.h>
#else
	#include <pthread.h>
#endif

namespace CubicleSoft
{
	namespace Sync
	{
		// Only the main thread in the main executable should instantiate this object.
		// Pass to other threads and shared objects/DLLs or use globally via shared memory.
		class TLS
		{
		public:
			TLS();
			~TLS();

			// Intended for use as a large, fixed-sized stack.
			class AutoFree
			{
			public:
				AutoFree(TLS *TLSPtr, void *Data);
				~AutoFree();

			private:
				// Deny copy constructor and assignment operator.  Use a (smart) pointer instead.
				AutoFree(const AutoFree &);
				AutoFree &operator=(const AutoFree &);

				TLS *MxTLS;
				void *MxData;
			};

			// Initializes the local thread cache to cache allocations (Default is 15, 2 ^ 15 = up to 32K allocations).
			// It is highly recommended to surround all code between ThreadInit() and ThreadEnd() with braces, especially when using AutoFree.
			// Only call this function once per thread per TLS instance.
			bool ThreadInit(size_t MaxCacheBits = 15);

			// Standard malloc()-like call.  Do not mix with real malloc/realloc/free!  Do not send to other threads.
			void *malloc(size_t Size);

			// Standard realloc()-like call.
			void *realloc(void *Data, size_t NewSize, bool Cache = true);

			// Duplicates memory using the real malloc() call, shallow copies Data, and free()'s Data for reuse.
			// For real new, just use a copy constructor and then free the memory.  That way a deep copy can more naturally happen.
			void *dup_malloc(void *Data, bool Cache = true);

			// Standard free()-like call.
			void free(void *Data, bool Cache = true);

			// Some static versions of the above to be able to pass the class around to other snippet library functions.
			inline static void *malloc(void *TLSPtr, size_t Size)
			{
				return ((TLS *)TLSPtr)->malloc(Size);
			}

			inline static void *realloc(void *TLSPtr, void *Data, size_t NewSize)
			{
				return ((TLS *)TLSPtr)->realloc(Data, NewSize);
			}

			inline static void free(void *TLSPtr, void *Data)
			{
				((TLS *)TLSPtr)->free(Data);
			}

			// Extract stats.
			bool GetBucketInfo(size_t Num, size_t &Nodes, size_t &Size);

			// Frees up all resources associated with the local thread cache.
			bool ThreadEnd();


			// A MixedVar-style class with raw Sync::TLS support.
			// Little to no error checking for that awesome raw performance feeling.
			enum TLSMixedVarModes
			{
				TMV_None,
				TMV_Bool,
				TMV_Int,
				TMV_UInt,
				TMV_Double,
				TMV_Str
			};

			// Designed to be extended but not overridden.
			class MixedVar
			{
			public:
				TLSMixedVarModes MxMode;
				std::int64_t MxInt;
				double MxDouble;
				char *MxStr;
				size_t MxStrPos;

			protected:
				TLS *MxTLS;

			public:
				MixedVar(TLS *TLSPtr = NULL);
				~MixedVar();

				MixedVar(const MixedVar &TempVar);
				MixedVar &operator=(const MixedVar &TempVar);

				inline void SetTLS(TLS *TLSPtr)
				{
					MxTLS = TLSPtr;
				}

				inline TLS *GetTLS()
				{
					return MxTLS;
				}

				// Some functions for those who prefer member functions over directly accessing raw class data.
				inline bool IsNone()
				{
					return (MxMode == TMV_None);
				}

				inline bool IsBool()
				{
				return (MxMode == TMV_Bool);
				}

				inline bool IsInt()
				{
					return (MxMode == TMV_Int);
				}

				inline bool IsUInt()
				{
					return (MxMode == TMV_UInt);
				}

				inline bool IsDouble()
				{
					return (MxMode == TMV_Double);
				}

				inline bool IsStr()
				{
					return (MxMode == TMV_Str);
				}

				inline bool GetBool()
				{
					return (MxInt != 0);
				}

				inline std::int64_t GetInt()
				{
					return MxInt;
				}

				inline std::uint64_t GetUInt()
				{
					return (std::uint64_t)MxInt;
				}

				inline double GetDouble()
				{
					return MxDouble;
				}

				inline char *GetStr()
				{
					return MxStr;
				}

				inline size_t GetSize()
				{
					return MxStrPos;
				}

				inline void SetBool(bool newbool)
				{
					MxMode = TMV_Bool;
					MxInt = (int)newbool;
				}

				inline void SetInt(std::int64_t newint)
				{
					MxMode = TMV_Int;
					MxInt = newint;
				}

				inline void SetUInt(std::uint64_t newint)
				{
					MxMode = TMV_UInt;
					MxInt = (std::int64_t)newint;
				}

				inline void SetDouble(double newdouble)
				{
					MxMode = TMV_Double;
					MxDouble = newdouble;
				}

				void SetData(const char *str, size_t size);
				void SetStr(const char *str);
				void PrependData(const char *str, size_t size);
				void PrependStr(const char *str);
				void AppendData(const char *str, size_t size);
				void AppendStr(const char *str);
				void AppendChar(char chr);
				void AppendMissingChar(char chr);
				void SetSize(size_t pos);
				size_t ReplaceData(const char *src, size_t srcsize, const char *dest, size_t destsize);
				size_t ReplaceStr(const char *src, const char *dest);

				inline bool RemoveTrailingChar(char chr)
				{
					if (!MxStrPos || MxStr[MxStrPos - 1] != chr)  return false;

					MxStr[--MxStrPos] = '\0';

					return true;
				}
			};

		private:
			// Deny copy constructor and assignment operator.  Use a (smart) pointer instead.
			TLS(const TLS &);
			TLS &operator=(const TLS &);

#if defined(_WIN32) || defined(WIN32) || defined(_WIN64) || defined(WIN64)
			DWORD MxTlsIndex;
#else
			pthread_key_t MxKey;
#endif

			bool SetMainPtr(StaticVector<Queue<char>> *MainPtr);
			StaticVector<Queue<char>> *GetMainPtr();

			static size_t NormalizeBitPosition(size_t &Size);
		};
	}
}

#endif
