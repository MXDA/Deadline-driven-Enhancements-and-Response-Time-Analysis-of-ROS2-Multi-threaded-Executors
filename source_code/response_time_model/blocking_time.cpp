#include <iostream>
#include <algorithm>
#include <unistd.h>
#include <vector>

int main() {
    std::vector<uint64_t> chain_lengths = {2, 4};
	std::vector<std::vector<uint64_t>> chain_member_ids = {{1, 2}, {3, 4, 5, 6}};

	std::vector<uint64_t> node_runtimes = {50, 60, 15, 25, 35, 45};
	std::vector<uint64_t> chain_periods = {100, 110};
	std::vector<uint64_t> chain_deadlines = {250, 300};
    
    return 0;
}