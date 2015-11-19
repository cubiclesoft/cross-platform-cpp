// Statically allocated vector.  No resize.
// (C) 2013 CubicleSoft.  All Rights Reserved.

#ifndef CUBICLESOFT_STATIC_VECTOR
#define CUBICLESOFT_STATIC_VECTOR

namespace CubicleSoft
{
	template <class T>
	class StaticVector
	{
	public:
		StaticVector(size_t Num)
		{
			ptr = new T[Num];
			Size = Num;
		}

		~StaticVector()
		{
			if (ptr != NULL)  delete[] ptr;
		}

		// Copy constructor.
		StaticVector(const StaticVector<T> &TempVector)
		{
			Size = TempVector.Size;
			ptr = new T[Size];
			for (size_t x = 0; x < Size; x++)  ptr[x] = TempVector.ptr[x];
		}

		// Assignment operator.
		StaticVector<T> &operator=(const StaticVector<T> &TempVector)
		{
			if (this != &TempVector)
			{
				if (Size != TempVector.Size)
				{
					if (ptr != NULL)  delete [] ptr;

					Size = TempVector.Size;
					if (!Size)  ptr = NULL;
					else  ptr = new T[Size];
				}

				for (size_t x = 0; x < Size; x++)  ptr[x] = TempVector.ptr[x];
			}

			return *this;
		}

		inline size_t GetSize() const { return Size; }

		// Lets the caller access a specific data element.
		// If the position is out of bounds, then epic fail.
		inline T &operator[](size_t Pos) { return ptr[Pos]; }

		// Grants the caller access to the underlying raw data.
		inline T *RawData() const { return ptr; }

		// Raw comparison of another StaticVector with this StaticVector.
		bool operator==(const StaticVector<T> &TempVector) const
		{
			if (Size != TempVector.Size)  return false;

			size_t x;
			for (x = 0; x < Size && ptr[x] == TempVector.ptr[x]; x++);
			if (x < Size)  return false;

			return true;
		}

		// Raw comparison of another StaticVector with this StaticVector.
		inline bool operator!=(const StaticVector<T> &TempVector) const
		{
			return !((*this) == TempVector);
		}

	private:
		T *ptr;
		size_t Size;
	};
}

#endif
