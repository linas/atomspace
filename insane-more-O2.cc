#include <cstdint>
#include <vector>
#include <stdlib.h>
#include <stdio.h>

// Bug: boolean values seem to be mis-handled with -O2 optimization.
// Caution: yes, there are floats in this bug report. No, that's not
// "really" the problem, although they are entangled into it.
//
// The following attempts to compare vectors of floats.
// The compare is done by casting the floats to int64_t.
// Yes, there is reason to "hate this", but its not unusual.
// At any rate, that's not what this bug is about.
// The int64_t (and uint64_t)'s behave as expected.
// A bool is computed. The printed value of the bool is correct.
// Despite the bool being "true", the if-statement treats it as false.
// And not just once, but consistently.
//
// g++ -O2 insane-more-O2.cc -o insane-more-O2-bad -save-temps
// g++ -O1 insane-more-O2.cc -o insane-more-O2-good
//
// Offending compiler, linker, runtime loader:
// gcc (Ubuntu 13.3.0-6ubuntu2~24.04) 13.3.0
// GNU ld (GNU Binutils for Ubuntu) 2.42
// ldd (Ubuntu GLIBC 2.39-0ubuntu8.5) 2.39
//
// Also affected:
// gcc (Debian 12.2.0-14+deb12u1) 12.2.0
// GNU ld (GNU Binutils for Debian) 2.40
// ldd (Debian GLIBC 2.36-9+deb12u10) 2.36


bool is_it_big(int64_t lili)
{
	uint64_t lulu = (uint64_t) lili;

#define MAX_ULPS 24LL
	bool yes_big = (MAX_ULPS < lulu);
printf("duude lili=0x%lx lulu=0x%lx is it big? %d\n", lili, lulu, yes_big);
	return yes_big;
}

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

		bool toobig = is_it_big(lili);
		printf("duuude %zu lili=0x%lx and toobig = %d\n", i, lili, toobig);

		if (toobig) return false;

		printf("duuude %zu keep going because toobig = %d\n", i, toobig);
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
	printf("The end! Are they equal? %d\n", eq);
	return 0;
}
