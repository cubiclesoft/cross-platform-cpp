// Cross-platform CSPRNG.
// (C) 2013 CubicleSoft.  All Rights Reserved.

#ifndef CUBICLESOFT_SECURITY_CSPRNG
#define CUBICLESOFT_SECURITY_CSPRNG

#include <cstdint>
#include <cstring>

#if defined(_WIN32) || defined(WIN32) || defined(_WIN64) || defined(WIN64)
	#include <windows.h>
#else
	#include <fcntl.h>
	#include <unistd.h>
	#include <errno.h>
#endif

namespace CubicleSoft
{
	namespace Security
	{
		class CSPRNG
		{
		public:
			CSPRNG(bool CryptoSafe);
			~CSPRNG();

			bool GetBytes(std::uint8_t *Result, size_t Size);
			bool GetInteger(std::uint64_t &Result, std::uint64_t Min, std::uint64_t Max);

		private:
			// Deny copy constructor and assignment operator.  Use a (smart) pointer instead.
			CSPRNG(const CSPRNG &);
			CSPRNG &operator=(const CSPRNG &);

#if defined(_WIN32) || defined(WIN32) || defined(_WIN64) || defined(WIN64)
			typedef BOOL (APIENTRY *RtlGenRandomFunc)(PVOID, ULONG);

			RtlGenRandomFunc MxRtlGenRandom;
#else
			int MxURandom;
#endif
		};
	}
}

#endif
