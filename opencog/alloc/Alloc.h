
#ifndef __ATOMSPACE_ALLOCATOR_H__
#define __ATOMSPACE_ALLOCATOR_H__

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
		void * mem = malloc(n*sizeof(value_type));
		nbytes_req += n*sizeof(value_type);
		return static_cast<value_type*>(mem);
	}

	void
	deallocate(value_type* p, std::size_t) noexcept
	{
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
