#include <cstdint>
#include <vector>
#include <stdlib.h>
#include <stdio.h>

// Bug: boolean values seem to be mis-handled with -O2 optimization.
//
// The following attempts to compare vectors of floats.
// The compare is done by casting the floats to int64_t
// Yes, there is reason to "hate this", but its not unusual.
// At any rate, that's not what this bug is about.
// The int64_t (and uint64_t)'s behave as expected.
// A bool is computed. The printed value of the bool is correct.
// Despite the bool evaluating to "true", the if-statement fails !!!!
// The "if" test fails, repeatedly. !!!
//
// g++ -O2 insane-O2.cc -o insane-O2-bad
// g++ -O1 insane-O2.cc -o insane-O2-good
//
// Offending compiler, linker, runtime loader:
// gcc (Debian 12.2.0-14+deb12u1) 12.2.0
// GNU ld (GNU Binutils for Debian) 2.40
// ldd (Debian GLIBC 2.36-9+deb12u10) 2.36


bool equal_p(const std::vector<double>& self, const std::vector<double>& other) 
{
	printf("duuude enter FloatValue::operator==\n");

	if (self.size() != other.size()) return false;

	size_t len = self.size();
	for (size_t i=0; i<len; i++)
	{
		// Sort-of-OK-ish equality compare. Not very good. The ULPS
		// compare below is much better.
		// if (1.0e-15 < fabs(1.0 - other[i]/self[i])) return false;

		// Compare floats with ULPS, because they are lexicographically
		// ordered. For technical explanation, see
		// http://www.cygnus-software.com/papers/comparingfloats/Comparing%20floating%20point%20numbers.htm

		int64_t lili =
			llabs(*(int64_t*) &(self[i]) - *(int64_t*)&(other[i]));
		uint64_t lulu = (uint64_t) lili;

#define MAX_ULPS 24LL
		bool toobig = (MAX_ULPS < lulu);

		// There's a bit of insanity with llabs.
		printf("duuude %zu lili=%lx and ulps = %d\n", i, lili, (MAX_ULPS < lili));
		printf("duuude %zu lulu=%lx and ulps = %d and %d\n", i, lulu, (MAX_ULPS < lulu), toobig);
		printf("duuude %zu lili is neg %d\n", i, (lili < 0LL));
		printf("duuude %zu lulu is neg %d\n", i, (lulu < 0LL));

		if (toobig) return false;

		printf("duuude %zu keep going because toobig = %d and %d\n", i, (MAX_ULPS < lulu), toobig);
		if (toobig) printf("wtf its too big\n");
		else printf("keep going\n");
	}
	return true;
}

int main()
{
	std::vector<double> self = {0, 11, 22, 33};
	std::vector<double> other = {0, 11, -22, -33};

	bool eq = equal_p(self, other);
	printf("The end! Are they equaL? %d\n", eq);
	return 0;
}
