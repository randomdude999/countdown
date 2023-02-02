#include <array>
#include <bitset>
#include <iostream>

using VAL = unsigned int;
using SET = std::array<VAL, 6>;
using RES = std::bitset<900>;

// this is basically just FNV except operating on integers instead of bytes.
std::size_t myhash(SET const& s) {
	std::size_t res = 0xcbf29ce484222325;
	for(int i=0;i<6;i++) {
		res ^= s[i];
		res *= 0x00000100000001B3;
	}
	return res;
}

std::array<std::pair<SET, RES>, 8'000'000> newcache;

// input format: array of 6 numbers, in nonincreasing order. padded with zeros
// at the end if some numbers are used already. output: a bitset where the
// lowest bit means "100 is reachable", and so on up to 999 (900 bits total)
RES reach_set(SET inp) {
	RES res{};

	// if we only have 2 numbers left, find all possible numbers directly
	if(inp[2] == 0) {
		VAL x = inp[0], y = inp[1];
		for(VAL r : std::array<VAL,4> {x+y,x-y,x*y, (y && (x%y==0)) ? x/y : 0})
			if(r > 99 && r < 1000) res.set(r-100);
		return res;
	}

	uint64_t inp_h = myhash(inp) % newcache.size();

	if(newcache[inp_h].first == inp) return newcache[inp_h].second;
	
	// if we have seen this exact set of numbers already, return the same answer
	//if(localcache.count(inp)) return localcache[inp];
	// if it was seen in the previous iteration, return it and save it to this
	// iteration's cache
	//if(oldcache.count(inp)) { return localcache[inp] = oldcache[inp]; }

	// otherwise, find new possible sets of numbers and call reach_set recursively
	for(int i=0;i<6;i++) {
		if(inp[i] == 0) break;
		VAL x = inp[i];
		for(int j=i+1;j<6;j++) {
			if(inp[j] == 0) break;
			VAL y = inp[j];
			for(VAL r : std::array<VAL,4> {x+y,x-y,x*y, (y && (x%y==0)) ? x/y : 0}) {
				// negative numbers aren't possible because x>=y. we can still
				// get 0 from subtraction though
				if(r == 0) continue;
				if(r == x || r == y) continue; // mul or div by 1
				if(r > 99 && r < 1000) res.set(r-100);
				// this is basically a messy way to do the equivalent of
				// remaining = sorted(inp - {x} - {y} + {r})
				// but hopefully it's faster
				SET remaining{};
				int k=0;
				bool r_added = false;
				for(int l=0;l<6;l++) {
					if(inp[l] < r && !r_added) remaining[k++] = r, r_added = true;
					if(l!=i && l!=j) remaining[k++] = inp[l];
				}
				if(!r_added) remaining[k++] = r;
				res |= reach_set(remaining);
			}
			
		}
	}
	//return localcache[inp] = res;
	newcache[inp_h].first = inp;
	newcache[inp_h].second = res;
	return res;
};

size_t reach_set_wrap(VAL a, VAL b, VAL c, VAL d, VAL e, VAL f) {
	RES r = reach_set(SET{a,b,c,d,e,f});
	// the only number that needs to be manually checked like this: all other
	// inputs are less than 100 and thus every other target must be obtained by
	// an operation, so it'll be handled in reach_set's main recursion
	if(a == 100) r.set(0);
	return r.count();
};

void doset(std::array<VAL, 4> bigs) {
	int tot = 0;
	for(int a=1;a<11;a++) {
		for(int b=a;b<11;b++) {
			size_t res = reach_set_wrap(bigs[0],bigs[1],bigs[2],bigs[3],b,a);
			tot += res;
			//printf("%d,%d,%d,%d,%d,%d = %zu\n", bigs[0],bigs[1],bigs[2],bigs[3],b,a, res);
		}
	}
	std::cout << "4L:" << bigs[0] << ',' << bigs[1] << ',' << bigs[2] << ',' << bigs[3] << ": " << tot << '\n';

	tot = 0;
	for(int exc = 0; exc < 4; exc++) {
		std::array<VAL, 3> thisbigs;
		for(int i=0,j=0;i<4;i++) if(i != exc) thisbigs[j++] = bigs[i];
		for(int a=1;a<11;a++) {
			for(int b=a;b<11;b++) {
				for(int c=b;c<11;c++) {
					if(a==b && a==c) continue;
					size_t res = reach_set_wrap(thisbigs[0], thisbigs[1], thisbigs[2], c,b,a);
					//printf("%d,%d,%d,%d,%d,%d = %zu\n", thisbigs[0], thisbigs[1], thisbigs[2], c,b,a, res);
					tot += res;
				}
			}
		}
	}
	std::cout << "3L:" << bigs[0] << ',' << bigs[1] << ',' << bigs[2] << ',' << bigs[3] << ": " << tot << '\n';

	tot = 0;
	for(int big1 = 0; big1 < 4; big1++) {
		for(int big2 = big1+1; big2 < 4; big2++) {
			for(int a=1;a<11;a++) {
				for(int b=a;b<11;b++) {
					for(int c=b;c<11;c++) {
						for(int d=c;d<11;d++) {
							// increasing or equal order right now
							// make sure there isn't 3 of any?
							if(a == b && a == c) continue;
							if(b == c && b == d) continue;
							size_t res = reach_set_wrap(bigs[big1], bigs[big2],d,c,b,a);
							tot += res;
							//printf("%d,%d,%d,%d,%d,%d = %zu\n", bigs[big1],bigs[big2],d,c,b,a, res);
						}
					}
				}
			}
		}
	}
	std::cout << "2L:" << bigs[0] << ',' << bigs[1] << ',' << bigs[2] << ',' << bigs[3] << ": " << tot << '\n';

	tot = 0;
	for(int big1 = 0; big1 < 4; big1++) {
		for(int a=1;a<11;a++) {
			for(int b=a;b<11;b++) {
				for(int c=b;c<11;c++) {
					for(int d=c;d<11;d++) {
						for(int e=d;e<11;e++) {
							// increasing or equal order right now
							// make sure there isn't 3 of any?
							if(a == b && a == c) continue;
							if(b == c && b == d) continue;
							if(c == d && d == e) continue;
							size_t res = reach_set_wrap(bigs[big1],e,d,c,b,a);
							tot += res;
							//printf("%d,%d,%d,%d,%d,%d = %zu\n", bigs[big1],e,d,c,b,a, res);
						}
					}
				}
			}
		}
	}
	std::cout << "1L:" << bigs[0] << ',' << bigs[1] << ',' << bigs[2] << ',' << bigs[3] << ": " << tot << '\n';
}

void lut_ones() {
	for(int big1 = 11; big1 < 101; big1++) {
		int tot = 0;
		size_t alttot = 0;
		for(int a=1;a<11;a++) {
			for(int b=a;b<11;b++) {
				for(int c=b;c<11;c++) {
					for(int d=c;d<11;d++) {
						for(int e=d;e<11;e++) {
							// increasing or equal order right now
							// make sure there isn't 3 of any?
							if(a == b && a == c) continue;
							if(b == c && b == d) continue;
							if(c == d && d == e) continue;
							size_t res = reach_set_wrap(big1,e,d,c,b,a);
							tot += res;
							// 2 pairs
							if((a == b && c == d) || (a == b && d == e) || (b == c && d == e)) alttot += res;
							// 1 pair
							else if(a==b || b==c || c==d || d==e) alttot += 4*res;
							else alttot += 16*res;
						}
					}
				}
			}
		}
		std::cout << "1LUT:" << big1 << ": " << tot << "/" << alttot << std::endl;
	}
}

void lut_twos(int big1) {
	for(int big2 = 11; big2 < big1; big2++) {
		size_t tot = 0;
		size_t alttot = 0;
		for(int a=1;a<11;a++) {
			for(int b=a;b<11;b++) {
				for(int c=b;c<11;c++) {
					for(int d=c;d<11;d++) {
						if(a == b && a == c) continue;
						if(b == c && b == d) continue;
						size_t res = reach_set_wrap(big1,big2,d,c,b,a);
						tot += res;
						if(a == b && c == d) alttot += res;
						else if(a==b || b==c || c==d) alttot += 4*res;
						else alttot += 16*res;
					}
				}
			}
		}
		std::cout << "2LUT:" << big1 << "," << big2 << ": " << tot << "/" << alttot << std::endl;
	}
}

void lut_3(int big1, int big2) {
	for(int big3 = 11; big3 < big2; big3++) {
		int tot = 0;
		int tot_alt = 0;
		for(int a=1;a<11;a++) {
			for(int b=a;b<11;b++) {
				for(int c=b;c<11;c++) {
						if(a == b && a == c) continue;
						size_t res = reach_set_wrap(big1,big2,big3,c,b,a);
						tot += res;
						if(a == b || b == c) tot_alt += res;
						else tot_alt += 4*res;
				}
			}
		}
		std::cout << "3LUT:" << big1 << "," << big2 << "," << big3 << ": " << tot << "/" << tot_alt << std::endl;
	}
}

void only4(int big1, int big2) {
	for(int big3 = 11; big3 < big2; big3++) {
		for(int big4 = 11; big4 < big3; big4++) {
			size_t tot = 0, tot_alt = 0;
			for(int a=1;a<11;a++) {
				for(int b=a;b<11;b++) {
					size_t res = reach_set_wrap(big1,big2,big3,big4,b,a);
					tot += res;
					tot_alt += res * (a == b ? 1 : 4);
				}
			}
			std::cout << "4L:" << big1 << "," << big2 << "," << big3 << "," << big4 << ": " << tot << "/" << tot_alt << "\n";
		}
	}
}

int main(int argc, char** argv) {
	if(argc == 2 && std::string(argv[1]) == "lut1") {
		lut_ones();
		return 0;
	}
	if(argc == 3 && std::string(argv[1]) == "lut2") {
		int arg = std::stoi(argv[2]);
		if(arg > 100 || arg < 12) {
			std::cerr << "invalid argument\n";
			return 1;
		}
		lut_twos(arg);
		return 0;
	}
	if(argc == 4 && std::string(argv[1]) == "lut3") {
		int arg1 = std::stoi(argv[2]);
		int arg2 = std::stoi(argv[3]);
		if(arg1 > 100 || arg2 < 12 || arg2 >= arg1) {
			std::cerr << "invalid arguments\n";
			return 1;
		}
		lut_3(arg1, arg2);
		return 0;
	}
	if(argc == 4 && std::string(argv[1]) == "4only") {
		int arg1 = std::stoi(argv[2]);
		int arg2 = std::stoi(argv[3]);
		if(arg1 > 100 || arg2 < 13 || arg2 >= arg1) {
			std::cerr << "invalid arguments\n";
			return 1;
		}
		only4(arg1, arg2);
		return 0;
	}
	if(argc != 4) {
		std::cerr << "usage: ./a.out [large1] [large2] [large3]\n";
		std::cerr << "or:    ./a.out lut1\n";
		std::cerr << "or:    ./a.out lut2 [large1]\n";
		std::cerr << "or:    ./a.out lut3 [large1] [large2]\n";
		std::cerr << "or:    ./a.out 4only [large1] [large2]\n";
		std::cerr << "large1,2,3 must be in descending order\n";
		return 1;
	}
	VAL l1 = std::stoi(argv[1]);
	VAL l2 = std::stoi(argv[2]);
	VAL l3 = std::stoi(argv[3]);
	if(l2 > l1 || l3 > l2 || l1 > 100 || l3 < 11) {
		std::cerr << "invalid numbers\n";
		return 1;
	}
	for(int i=l3-1; i>10; i--) {
		std::array<VAL, 4> bigs{l1,l2,l3,(VAL)i};
		doset(bigs);
	}
}
