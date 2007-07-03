/*
 * shared_ptr - simple reference counted pointer.
 *
 * The is a non-intrusive implementation that allocates an additional
 * int and pointer for every counted object.
 */

#ifndef	_ELSHAREDPTR_HPP_
#define _ELSHAREDPTR_HPP_

template <class X> class shared_ptr
{
	public:
		typedef X element_type;

		 // allocate a new counter
		explicit shared_ptr(X* p = 0) : itsCounter(0)
		{
			if (p)
			{
				itsCounter = new counter(p);
			}
		}

		~shared_ptr()
		{
			release();
		}

		shared_ptr(const shared_ptr& r) throw()
		{
			acquire(r.itsCounter);
		}

		shared_ptr& operator=(const shared_ptr& r)
		{
			if (this != &r)
			{
				release();
				acquire(r.itsCounter);
			}

			return *this;
		}

		X& operator*() const throw()
		{
			return *itsCounter->ptr;
		}

		X* operator->() const throw()
		{
			return itsCounter->ptr;
		}

		X* get() const throw()
		{
			return itsCounter ? itsCounter->ptr : 0;
		}

		bool unique() const throw()
		{
			return (itsCounter ? itsCounter->count == 1 : true);
		}

	private:

		struct counter
		{
			X* ptr;
			unsigned count;
			counter(X* p = 0, unsigned c = 1) : ptr(p), count(c) {}
		}* itsCounter;

		void acquire(counter* c) throw()
		{ // increment the count
			itsCounter = c;
			if (c)
			{
				++c->count;
			}
		}

		void release()
		{ // decrement the count, delete if it is 0
			if (itsCounter)
			{
				if (--itsCounter->count == 0)
				{
					delete itsCounter->ptr;
					delete itsCounter;
				}
				itsCounter = 0;
			}
		}
};

#endif // _ELSHAREDPTR_H_

