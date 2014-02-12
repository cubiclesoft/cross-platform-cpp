// Cross-platform CSPRNG.
// (C) 2013 CubicleSoft.  All Rights Reserved.

#include "security_csprng.h"

namespace CubicleSoft
{
	namespace Security
	{
#if defined(_WIN32) || defined(WIN32) || defined(_WIN64) || defined(WIN64)
		// Windows.
		CSPRNG::CSPRNG(bool)
		{
			HMODULE TempModule = ::LoadLibraryA("ADVAPI32.DLL");
			if (TempModule == NULL)  MxRtlGenRandom = NULL;
			else  MxRtlGenRandom = (RtlGenRandomFunc)::GetProcAddress(TempModule, "SystemFunction036");
		}

		CSPRNG::~CSPRNG()
		{
		}

		bool CSPRNG::GetBytes(std::uint8_t *Result, size_t Size)
		{
			if (MxRtlGenRandom == NULL)  return false;

			return ((*MxRtlGenRandom)(Result, (ULONG)Size) != FALSE);
		}
#else
		// Linux.
		CSPRNG::CSPRNG(bool CryptoSafe)
		{
			if (CryptoSafe)  MxURandom = open("/dev/random", O_RDONLY);
			else
			{
				MxURandom = open("/dev/arandom", O_RDONLY);
				if (MxURandom < 0)  MxURandom = open("/dev/urandom", O_RDONLY);
			}
		}

		CSPRNG::~CSPRNG()
		{
			close(MxURandom);
		}

		bool CSPRNG::GetBytes(std::uint8_t *Result, size_t Size)
		{
			if (MxURandom < 0)  return false;

			ssize_t BytesRead;

			do
			{
				do
				{
					BytesRead = read(MxURandom, Result, Size);
				} while (BytesRead < 0 && errno == EINTR);
			} while (BytesRead != (ssize_t)Size);

			if (BytesRead < 0)  return false;

			return true;
		}
#endif

		// All platforms.
		bool CSPRNG::GetInteger(std::uint64_t &Result, std::uint64_t Min, std::uint64_t Max)
		{
			std::uint64_t Max2, Mask;

			if (Max < Min)
			{
				Max2 = Min;
				Min = Max;
				Max = Max2;
			}

			Max2 = Max - Min;
			Mask = 0;
			while (Max2 > Mask)  Mask = (Mask << 1) | 0x01;

			// The correct way to get a random integer in a range along bit boundaries
			// is to throw away values outside the allowed range.
			do
			{
				if (!GetBytes((std::uint8_t *)&Result, sizeof(std::uint64_t)))  return false;
				Result &= Mask;

			} while (Result > Max2);

			Result += Min;

			return true;
		}
	}
}
