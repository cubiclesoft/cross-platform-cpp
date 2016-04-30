// Statically allocated 2D array.  No resize.
// (C) 2015 CubicleSoft.  All Rights Reserved.

#ifndef CUBICLESOFT_STATIC_2D_ARRAY
#define CUBICLESOFT_STATIC_2D_ARRAY

namespace CubicleSoft
{
	template <class T>
	class Static2DArray
	{
	public:
		Static2DArray(size_t FixedWidth, size_t FixedHeight)
		{
			baseptr = new T[FixedWidth * FixedHeight];
			ptr = new T *[FixedHeight];
			for (size_t x = 0; x < FixedHeight; x++)  ptr[x] = baseptr + (FixedWidth * x);
			Width = FixedWidth;
			Height = FixedHeight;
		}

		~Static2DArray()
		{
			if (ptr != NULL)  delete[] ptr;
			if (baseptr != NULL)  delete[] baseptr;
		}

		// Copy constructor.
		Static2DArray(const Static2DArray<T> &TempArray)
		{
			Width = TempArray.Width;
			Height = TempArray.Height;

			size_t y = Width * Height;
			baseptr = new T[y];
			ptr = new T *[Height];
			for (size_t x = 0; x < Height; x++)  ptr[x] = baseptr + (Width * x);

			for (size_t x = 0; x < y; x++)  baseptr[x] = TempArray.baseptr[x];
		}

		// Assignment operator.
		Static2DArray<T> &operator=(const Static2DArray<T> &TempArray)
		{
			if (this != &TempArray)
			{
				if (Width != TempArray.Width || Height != TempArray.Height)
				{
					if (ptr != NULL)  delete[] ptr;
					if (baseptr != NULL)  delete[] baseptr;

					Width = TempArray.Width;
					Height = TempArray.Height;

					size_t y = Width * Height;
					if (y)
					{
						baseptr = NULL;
						ptr = NULL;
					}
					else
					{
						baseptr = new T[y];
						ptr = new T *[Height];
						for (size_t x = 0; x < Height; x++)  ptr[x] = baseptr + (Width * x);
					}
				}

				size_t y = Width * Height;
				for (size_t x = 0; x < y; x++)  baseptr[x] = TempArray.baseptr[x];
			}

			return *this;
		}

		inline size_t GetWidth() const { return Width; }
		inline size_t GetHeight() const { return Height; }

		// Lets the caller access a specific data element.
		// If the position is out of bounds, then epic fail.
		inline T *operator[](size_t Pos) { return ptr[Pos]; }

		// Grants the caller access to the underlying raw data.
		inline T **RawData() const { return ptr; }

		// Raw comparison of another Static2DArray with this Static2DArray.
		bool operator==(const Static2DArray<T> &TempArray) const
		{
			if (Width != TempArray.Width || Height != TempArray.Height)  return false;

			size_t x, y = Width * Height;
			for (x = 0; x < y && baseptr[x] == TempArray.ptr[x]; x++);
			if (x < y)  return false;

			return true;
		}

		// Raw comparison of another Static2DArray with this Static2DArray.
		inline bool operator!=(const Static2DArray<T> &TempArray) const
		{
			return !((*this) == TempArray);
		}

	private:
		T *baseptr;
		T **ptr;
		size_t Width, Height;
	};
}

#endif
