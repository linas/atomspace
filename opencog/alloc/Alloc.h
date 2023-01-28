
#ifndef __ATOMSPACE_ALLOCATOR_H__
#define __ATOMSPACE_ALLOCATOR_H__

#include <atomic>
#include <opencog/util/Logger.h>

template <class T>
class asallo
{
private:
	std::atomic_size_t nallocs(0);
	std::atomic_size_t nfrees(0);
	std::atomic_size_t nbytes_req(0);
	std::atomic_size_t nbytes_use(0);

public:
	using value_type	= T;

	asallo() noexcept {}
	template <class U> asallo(asallo<U> const&) noexcept {}

	value_type*
	allocate(std::size_t n)
	{
		nallocs++;
		void * mem = malloc(n*sizeof(value_type));
		nbytes_req += n*sizeof(value_type);
		nbytes_use += malloc_usable_size(mem);

		size_t na = nallocs.load();
		size_t req = nbytes_req.load();
		size_t use = nbytes_use.load();
		if (0 == na % (1024*1024))
		{
			size_t nf = nfrees.load();
			logger().info("Allo: nallo=%lu nfree=%lu diff=%lu req=%lu MiB use=%lu MiB avg = %lu",
				na, nf, na-nf, req/(1024*1024), use/(1024*1024), use/(na-nf));
		}

		return static_cast<value_type*>(mem);
	}

	void
	deallocate(value_type* p, std::size_t n) noexcept
	{
		nfrees++;
		nbytes_req -= n*sizeof(value_type);
		nbytes_use -= malloc_usable_size(p);
		free(p);
	}
};
template <class T, class U>
bool
operator==(asallo<T> const&, asallo<U> const&) noexcept
{
	return true;
}
template <class T, class U>
bool
operator!=(asallo<T> const& x, asallo<U> const& y) noexcept
{
	return !(x == y);
}

#endif // __ATOMSPACE_ALLOCATOR_H__
