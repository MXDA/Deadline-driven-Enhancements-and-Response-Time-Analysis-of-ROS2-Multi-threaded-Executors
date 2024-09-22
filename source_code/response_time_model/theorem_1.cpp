#include <iostream>
#include <algorithm>
#include <unistd.h>
#include <vector>


int main() {
    
	std::vector<uint64_t> chain_lengths = {2, 4, 4, 3, 4, 4};
	std::vector<std::vector<uint64_t>> chain_member_ids = {{1, 2}, {1, 3, 4, 5}, {6, 7, 8, 9}, {10, 11, 12}, {13, 14, 15, 16}, {17, 18, 19, 20}};
	//std::vector<std::vector<uint64_t>> chain_member_ids = {{1, 2}, {3, 4, 5, 6}, {7, 8, 9, 10}, {11, 12, 13}, {14, 15, 16, 17}, {18, 19, 20, 21}};
	//std::vector<double_t> node_runtimes = {2, 16, 2, 2, 18, 9, 23, 8, 14, 18, 21, 18, 7, 2, 11, 8, 8, 2, 196};
	std::vector<uint64_t> node_runtimes = {2, 16, 2, 2, 9, 9, 21, 8, 7, 2, 23, 8, 14, 18, 11, 8, 8, 11, 22, 5, 18};
	std::vector<uint64_t> chain_periods = {80, 80, 120, 140, 160, 180};
	//std::vector<uint64_t> chain_periods = {80, 100, 120, 140, 160, 180};
	std::vector<uint64_t> chain_deadlines = {80, 80, 120, 140, 160, 180};
    
    /*
    	std::vector<uint64_t> chain_lengths = {2, 4};
	std::vector<std::vector<uint64_t>> chain_member_ids = {{1, 2}, {3, 4, 5, 6}};

	std::vector<uint64_t> node_runtimes = {50, 30, 15, 15, 35, 35};
	std::vector<uint64_t> chain_periods = {100, 110};
	std::vector<uint64_t> chain_deadlines = {100, 110};
    */
    int m = 3;
    int n = chain_lengths.size();
    std::vector<uint64_t> chain_runtimes;
    std::vector<uint64_t> last_chain_callbacks;
	uint index = 0;
	for(uint i = 0; i < chain_lengths.size(); ++i) {
		uint64_t sum = 0;
		std::vector<uint64_t> chain_member = chain_member_ids[i];
		uint len = chain_member.size();
		while(len--) {
			sum += node_runtimes[index++];
		}
		chain_runtimes.push_back(sum);
        last_chain_callbacks.push_back(node_runtimes[index - 1]);
    }
    uint64_t max_chains_time = chain_runtimes[0];
    for(uint i = 1; i < chain_runtimes.size(); ++i) {
        max_chains_time = std::max(max_chains_time, chain_runtimes[i]);
    }
    
    std::vector<uint64_t> WCRT;
    for(uint i = 0; i < chain_lengths.size(); ++i) {
        uint64_t time_gap = chain_runtimes[i] - last_chain_callbacks[i];
        //uint64_t time_gap = 1;
        while(true) {
            uint64_t dbf = m * (chain_runtimes[i] - last_chain_callbacks[i]);
            for(uint j = 0; j < chain_lengths.size(); ++j) {
                if(chain_deadlines[j] < chain_deadlines[i]) {
                     //dbf += (time_gap / chain_periods[j]) * chain_runtimes[j] + std::min(chain_runtimes[j], time_gap - (time_gap / chain_periods[j]) * chain_periods[j]);
		     dbf += ((std::min(time_gap, chain_deadlines[i] - chain_deadlines[j]) / chain_periods[j]) * chain_runtimes[j] + std::min(chain_runtimes[j], time_gap - (std::min(time_gap, chain_deadlines[i] - chain_deadlines[j])) * chain_periods[j]));
             	    //dbf += dbf1;
		    //std::cout << dbf1 << " " << dbf2 << std::endl;
		     //std::cout << time_gap / chain_periods[j]) * chain_runtimes[j]
		   // std::cout << std::min(time_gap, chain_deadlines[i] - chain_deadlines[j]) /  * chain_runtimes[j] << " " << std::min(chain_runtimes[j], time_gap - (std::min(time_gap, chain_deadlines[i] - chain_deadlines[j])) * chain_periods[j]) << std::endl;
		}
            }
            for(uint k = 0; k < chain_lengths.size(); ++k) {
                    if (i != k)
                        dbf += std::min(chain_runtimes[k], time_gap); 
            }
            //dbf += std::max(0,  n - m) * std::min(time_gap, max_chains_time);
            if(dbf < m * time_gap) {
                //std::cout << "true" << std::endl;
                //std::cout << time_gap << ' ' << dbf << std::endl;
                uint wcrt = time_gap + last_chain_callbacks[i] - 1;
                WCRT.push_back(wcrt);
                break;
            }
            time_gap++;
            //std::cout << "chain_id: " << i << " time_gap: " << time_gap << " dbf: " << dbf << std::endl;
        }
    }
    for(int i = 0; i < WCRT.size(); ++i) 
        std::cout << WCRT[i] << ' ';
    return 0;
}
