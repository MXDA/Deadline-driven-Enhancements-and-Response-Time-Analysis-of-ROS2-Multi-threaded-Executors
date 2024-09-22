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
	std::vector<uint64_t> chain_periods = {40, 40, 60, 70, 80, 90};
	//std::vector<uint64_t> chain_periods = {80, 100, 120, 140, 160, 180};
	std::vector<uint64_t> chain_deadlines = {80, 80, 120, 140, 160, 180};


    uint m = 2;
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
    /*
    for(uint i = 0; i < chain_lengths.size(); ++i) {
        std::cout << chain_runtimes[i] << " " << last_chain_callbacks[i] << std::endl;
    }
    */
    std::vector<uint64_t> WCRT;
    for(uint i = 0; i < chain_lengths.size(); ++i) {
        uint64_t time_gap = chain_runtimes[i] - last_chain_callbacks[i];
        while(true) {
            uint64_t dbf = m * (chain_runtimes[i] - last_chain_callbacks[i]);
            //std::cout << "dbf: " << dbf << std::endl;
            for (uint j = 0; j < chain_lengths.size(); ++j) {
                if (chain_deadlines[j] < chain_deadlines[i]) {
                   //dbf += (((time_gap - 1) / chain_periods[j]) + 1) * chain_runtimes[j];
                    dbf += (((std::min(time_gap, chain_deadlines[i] - chain_deadlines[j])- 1) / chain_periods[j]) + 1) * chain_runtimes[j];
		}
            }
            for(uint k = 0; k < chain_lengths.size(); ++k) {
                dbf += (((chain_deadlines[k] - 1) / chain_periods[k]) + 1) * std::min(chain_runtimes[k], time_gap);
            }
            //std::cout << "49: " << dbf << std::endl;
            dbf -= std::min(chain_runtimes[i], time_gap);
            //dbf -= time_gap;
            //std::cout << "51: " << dbf << std::endl;
            if(dbf < m * time_gap) {
                //std::cout << dbf << ' ' << time_gap << std::endl;
                //std::cout << (((chain_deadlines[i] - 1) / chain_periods[i]) + 1) << std::endl;
                uint64_t wcrt = time_gap + last_chain_callbacks[i] - 1;
                WCRT.push_back(wcrt);
                break;
            }
            time_gap++;
        }
    }
    for(uint i = 0; i < WCRT.size(); ++i) 
        std::cout << WCRT[i] << ' ';
    return 0;
}
