#include "rclcpp/rclcpp.hpp"
#include "simple_timer/rt-sched.hpp"
#include "priority_executor/priority_memory_strategy.hpp"
#include "priority_executor/priority_executor.hpp"
#include "priority_executor/test_nodes.hpp"
#include "priority_executor/default_executor.hpp"
#include <iostream>
#include <vector>
#include <fstream>
#include <unistd.h>


typedef struct {
	std::shared_ptr<timed_executor::MultiThreadTimedExecutor> executor;
 	std::shared_ptr<PriorityMemoryStrategy<>> strat;
} executor_strat;

uint64_t PublisherNode::end_time;
uint64_t DummyWorker::end_time;

int main(int argc, char **argv) {
	rclcpp::init(argc, argv);
	std::cout << "starting.." << std::endl;
	auto node = rclcpp::Node::make_shared("experiment_parameters");
	node->declare_parameter("experiment_name");
 	node->declare_parameter("count_max");
 	node->declare_parameter("schedule_type");

	auto parameters_client = std::make_shared<rclcpp::SyncParametersClient>(node);
  	// parameters_client->wait_for_service();
  	const std::string schedule_type_str = parameters_client->get_parameter("schedule_type", std::string("deadline"));
  	std::cout << schedule_type_str << std::endl;
  	int COUNT_MAX = parameters_client->get_parameter("count_max", 500);
  	ExecutableScheduleType schedule_type = DEFAULT;
	
	if (schedule_type_str == "deadline") {
    	schedule_type = DEADLINE;
  	}
  	else if (schedule_type_str == "chain_priority") {
    	schedule_type = CHAIN_AWARE_PRIORITY;
  	}
  	else {
    	schedule_type = DEFAULT;
  	}

	executor_strat executors;
	size_t NumThreads = 2;
	bool YieldBeforeExecute = false;
	std::cout << "creating MultiThreadExecutors" << std::endl;
	executors.strat = std::make_shared<PriorityMemoryStrategy<>>();
	rclcpp::ExecutorOptions options;
	options.memory_strategy = executors.strat;
	executors.strat->logger = create_logger();
	executors.strat->is_f1tenth = true;
	executors.executor = std::make_shared<timed_executor::MultiThreadTimedExecutor>(options, NumThreads, YieldBeforeExecute, std::chrono::nanoseconds(-1), "multi_test");
	//executors.executor->set_use_priorities(true);

	std::cout << "MultiThreadExecutors created" << std::endl;
	/*
	std::vector<uint64_t> chain_lengths = {2, 4, 4, 3, 4, 2, 2, 2, 2, 2, 2, 2};
	std::vector<std::vector<uint64_t>> chain_member_ids = {{1, 2}, {1, 3, 4, 5}, {6, 7, 8, 9}, {10, 11, 12}, {13, 14, 15, 16}, {17, 18}, {19, 20}, {21, 22}, {23, 24}, {25, 26}, {27, 28}, {29, 30}};
	std::vector<std::vector<uint64_t>> chain_priorities = {{1, 0}, {5, 4, 3, 2, 1}, {9, 8, 7, 6}, {12, 11, 10}, {16, 15, 14, 13}, {18, 17}, {20, 19}, {22, 21}, {24, 23}, {26, 25}, {28, 27}, {30, 29}};
	
	std::vector<double_t> node_runtimes = {2.3, 16.1, 2.3, 2.2, 18.4, 9.1, 23.1, 7.9, 14.2, 17.9, 20.6, 17.9, 6.6, 1.7, 11.0, 6.6, 7.9, 1.7, 195.9, 33.2, 2.2, 33.2, 2.2, 33.2, 2.2, 33.2, 2.2, 33.2, 2.2, 33.2, 2.2};
	std::vector<uint64_t> chain_periods = {80, 80, 100, 100, 160, 1000, 120, 120, 120, 120, 120, 120};
	std::vector<uint64_t> chain_deadlines = {80, 80, 100, 100, 160, 1000, 120, 120, 120, 120, 120, 120};
	*/
	
	std::vector<uint64_t> chain_lengths = {2, 4, 4, 3, 4, 4};
	std::vector<std::vector<uint64_t>> chain_member_ids = {{1, 2}, {1, 3, 4, 5}, {6, 7, 8, 9}, {10, 11, 12}, {13, 14, 15, 16}, {17, 18, 19, 20}};
	//std::vector<std::vector<uint64_t>> chain_member_ids = {{1, 2}, {3, 4, 5, 6}, {7, 8, 9, 10}, {11, 12, 13}, {14, 15, 16, 17}, {18, 19, 20, 21}};
	//std::vector<double_t> node_runtimes = {2, 16, 2, 2, 18, 9, 23, 8, 14, 18, 21, 18, 7, 2, 11, 8, 8, 2, 196};
	std::vector<double_t> node_runtimes = {2, 16, 2, 2, 9, 9, 21, 8, 7, 2, 23, 8, 14, 18, 11, 8, 8, 11, 22, 5, 18};
	std::vector<uint64_t> chain_periods = {80, 80, 120, 140, 160, 180};
	//std::vector<uint64_t> chain_periods = {80, 100, 120, 140, 160, 180};
	std::vector<uint64_t> chain_deadlines = {80, 80, 120, 140, 160, 180};
	
	std::vector<double_t> chain_runtimes;
	uint index = 0;
	for(uint i = 0; i < chain_lengths.size(); ++i) {
		double_t sum = 0;
		std::vector<uint64_t> chain_member = chain_member_ids[i];
		uint len = chain_member.size();
		while(len--) {
			sum += node_runtimes[index++];
		}
		chain_runtimes.push_back(sum);
	}

	for(uint i = 0; i < chain_runtimes.size(); ++i) {
		std::cout << "chain " << i << " runtimes: " << chain_runtimes[i] << std::endl;
	}



	std::vector<std::vector<std::shared_ptr<rclcpp::Node>>> nodes;
	std::vector<std::shared_ptr<PublisherNode>> publishers;
	std::vector<std::shared_ptr<DummyWorker>> workers;
	std::vector<std::vector<std::deque<uint> *> *> chain_deadlines_deque;

	//std::deque<uint> *shared_chain_deadlines_deque = new std::deque<uint>();
	//node_time_logger logger = create_logger();
	//timespec current_time;
	timespec current_time;
	clock_gettime(CLOCK_MONOTONIC_RAW, &current_time);
	uint64_t millis = (current_time.tv_sec * (uint64_t)1000) + (current_time.tv_nsec / 1000000);
	PublisherNode::set_end_time(millis + 50000);
	DummyWorker::set_end_time(millis + 50000);
	uint64_t current_node_id = 0;
	for (uint chain_index = 0; chain_index < chain_lengths.size(); ++chain_index) {
		std::cout << "making chain" << std::to_string(chain_index) << std::endl;
		std::shared_ptr<rclcpp::TimerBase> this_chain_timer_handle;

		int max_chain_num = std::ceil(chain_deadlines[chain_index] / (double)chain_periods[chain_index]);
		std::vector<std::deque<uint> *>* this_chain_deadlines_deque = new std::vector<std::deque<uint> *>(max_chain_num, nullptr);
		for(uint index = 0; index < max_chain_num; ++index) (*this_chain_deadlines_deque)[index] = new std::deque<uint>();
		//std::deque<uint> *this_chain_deadlines_deque = new std::deque<uint>();
		nodes.push_back(std::vector<std::shared_ptr<rclcpp::Node>>());
		
		for (uint cb_index = 0; cb_index < chain_lengths[chain_index]; cb_index++) {
			std::cout << "making node " << std::to_string(current_node_id) << " with runtime " << node_runtimes[current_node_id] << std::endl;
			if (cb_index == 0) {
				std::shared_ptr<PublisherNode> publisher_node;
				/*if (chain_index == 0) {
					chain_deadlines_deque.push_back(shared_chain_deadlines_deque);
				}*/
				if (chain_index == 1 && executors.strat->is_f1tenth) {
					publisher_node = std::static_pointer_cast<PublisherNode>(nodes[0][0]);
					this_chain_timer_handle = publisher_node->timer_;
				}
				else {
					publisher_node = std::make_shared<PublisherNode>("topic_" + std::to_string(chain_index), chain_index, chain_periods[chain_index], node_runtimes[current_node_id]);
          			publishers.push_back(publisher_node);
					publisher_node->count_max = COUNT_MAX;
					
					assert(executors.strat != nullptr);
					auto timer_handle = publisher_node->timer_->get_timer_handle();
					assert(timer_handle != nullptr);
					executors.strat->set_executable_deadline(publisher_node->timer_->get_timer_handle(), chain_periods[chain_index], chain_deadlines[chain_index], TIMER, chain_index);

					executors.strat->set_first_in_chain(publisher_node->timer_->get_timer_handle());

					//if (executors.strat->is_f1tenth && chain_index == 0)
					//	executors.strat->assign_deadlines_queue(publisher_node->timer_->get_timer_handle(), shared_chain_deadlines_deque);
					//else 
						executors.strat->assign_deadlines_queue(publisher_node->timer_->get_timer_handle(), this_chain_deadlines_deque);

					this_chain_timer_handle = publisher_node->timer_;
					executors.strat->get_priority_settings(publisher_node->timer_->get_timer_handle())->timer_handle = this_chain_timer_handle;
					executors.executor->add_node(publisher_node);

					//timespec current_time;
					//clock_gettime(CLOCK_MONOTONIC_RAW, &current_time);
					//uint64_t millis = (current_time.tv_sec * (uint64_t)1000) + (current_time.tv_nsec / 1000000);
					//std::cout << "chain_index: " << chain_index << " cb_index: " << cb_index << " current_time: " << millis << std::endl;
					//std::cout << " time_until_trigger: " << this_chain_timer_handle->time_until_trigger().count() / 1000000 <<std::endl;
				}
				nodes[chain_index].push_back(std::static_pointer_cast<rclcpp::Node>(publisher_node));
			}
			else {
				std::shared_ptr<DummyWorker> sub_node;
				if (chain_index == 1 && cb_index == 1 && executors.strat->is_f1tenth) {
					sub_node = std::make_shared<DummyWorker>("chain_" + std::to_string(chain_index) + "_worker_" + std::to_string(cb_index), node_runtimes[current_node_id], chain_index, cb_index, true);
				}
				else {
					
					if (cb_index == chain_lengths[chain_index] - 1)
						sub_node = std::make_shared<DummyWorker>("chain_" + std::to_string(chain_index) + "_worker_" + std::to_string(cb_index), node_runtimes[current_node_id], chain_index, cb_index, false, true);
					else
						sub_node = std::make_shared<DummyWorker>("chain_" + std::to_string(chain_index) + "_worker_" + std::to_string(cb_index), node_runtimes[current_node_id], chain_index, cb_index, false, false);
					
					//sub_node = std::make_shared<DummyWorker>("chain_" + std::to_string(chain_index) + "_worker_" + std::to_string(cb_index), node_runtimes[current_node_id], chain_index, cb_index);
				}
				workers.push_back(sub_node);
				executors.strat->set_executable_deadline(sub_node->subscription_->get_subscription_handle(), chain_periods[chain_index], chain_deadlines[chain_index], SUBSCRIPTION, chain_index);
				executors.executor->add_node(sub_node);
				executors.strat->assign_deadlines_queue(sub_node->subscription_->get_subscription_handle(), this_chain_deadlines_deque);
				if (cb_index == chain_lengths[chain_index] - 1) {
					executors.strat->set_last_in_chain(sub_node->subscription_->get_subscription_handle());
					executors.strat->get_priority_settings(sub_node->subscription_->get_subscription_handle())->timer_handle = this_chain_timer_handle;
				}
				nodes[chain_index].push_back(std::static_pointer_cast<rclcpp::Node>(sub_node));	
			}
			current_node_id++;			
		}
		chain_deadlines_deque.push_back(this_chain_deadlines_deque);
		/*
		clock_gettime(CLOCK_MONOTONIC_RAW, &current_time);
		uint64_t millis = (current_time.tv_sec * (uint64_t)1000) + (current_time.tv_nsec / 1000000);
		
		log_entry(logger, "deadline_" + std::to_string(chain_index) + "_" + std::to_string(millis + chain_deadlines[chain_index]));
		log_entry(logger, "timer_" + std::to_string(chain_index) + "_release_" + std::to_string(millis));
		std::cout << "chain_index: " << chain_index << " " << "deadlines: " << millis + chain_deadlines[chain_index] << std::endl;
		std::cout << " release: " << millis << std::endl;
		chain_deadlines_deque[chain_index]->push_back(millis + chain_deadlines[chain_index]); 
		*/
	}
	std::cout << "initialized nodes" << std::endl;
	
	node_time_logger logger = create_logger();
	

	
	//chain_deadlines_deque[0]->push_back(millis + chain_deadlines[0]);
	for (uint chain_index = 0; chain_index < chain_lengths.size(); ++chain_index) {
		
		std::shared_ptr<rclcpp::TimerBase> this_chain_timer_handle;
		if(chain_index == 1) 
			this_chain_timer_handle = std::static_pointer_cast<PublisherNode>(nodes[0][0])->timer_;
		else
			this_chain_timer_handle = std::static_pointer_cast<PublisherNode>(nodes[chain_index][0])->timer_;
		
		clock_gettime(CLOCK_MONOTONIC_RAW, &current_time);
		uint64_t millis = (current_time.tv_sec * (uint64_t)1000) + (current_time.tv_nsec / 1000000);
		uint64_t time_until_trigger = this_chain_timer_handle->time_until_trigger().count() / 1000000;
		//log_entry(logger, "deadline_" + std::to_string(chain_index) + "_" + std::to_string(millis + time_until_trigger + chain_deadlines[chain_index]));
		//log_entry(logger, "timer_" + std::to_string(chain_index) + "_release_" + std::to_string(millis + time_until_trigger));
		//std::cout << "chain_index: " << chain_index << " " << "deadlines: " << millis + time_until_trigger + chain_deadlines[chain_index] << std::endl;
		//std::cout << " release: " << millis + time_until_trigger << std::endl;
		//std::cout << " time_until_trigger: " << (this_chain_timer_handle->time_until_trigger().count() / 1000000) << std::endl;
		//chain_deadlines_deque[chain_index + 1]->push_back(millis + chain_deadlines[chain_index]);
		
		executors.strat->assign_release_time(this_chain_timer_handle->get_timer_handle(), millis + time_until_trigger);
		(*chain_deadlines_deque[chain_index])[0]->push_back(millis + time_until_trigger + chain_deadlines[chain_index]);
		//chain_deadlines_deque[chain_index]->push_back(millis + time_until_trigger + chain_deadlines[chain_index]);
		if (chain_index != 1)
			log_entry(logger, std::to_string(chain_index) + " release_time: " + std::to_string(millis + time_until_trigger)); 
	}
	

	/*std::cout << "************************" << std::endl;
	std::cout << "size: " << chain_deadlines_deque.size() << std::endl;
	for (uint chain_index = 0; chain_index < chain_lengths.size() + 1; ++chain_index) {
		std::cout << "chain_index: " << chain_index << " " << chain_deadlines_deque[chain_index]->front();
		if(chain_deadlines_deque[chain_index]->empty()) std::cout << "nullptr";
		std::cout << " deadlines size: " << chain_deadlines_deque[chain_index]->size() << std::endl;
	}
	std::cout << "************************" << std::endl;*/
	
	//executors.executor.cpus = {};
	//executors.strat->print_all_handle_schedule_type();
	
	//clock_gettime(CLOCK_MONOTONIC_RAW, &current_time);
	//int64_t millis = (current_time.tv_sec * (uint64_t)1000) + (current_time.tv_nsec / 1000000);
	//std::cout << "spin time: " << millis << std::endl;
	//std::cout << "---------------" << std::endl;
	executors.executor->spin();
	rclcpp::shutdown();
	//executors.strat->print_all_handle_schedule_type();
	std::cout << "rclcpp::shutdown()" << std::endl;
	
	std::ofstream output_file;
	std::string suffix = "";
	output_file.open("experiments/multi_test" + std::to_string(NumThreads) + "c" + suffix + ".txt");
	std::vector<std::pair<std::string, u64>> combined_logs;

	for (auto &publisher : publishers)
  	{
    	for (auto &log : *(publisher->logger_.recorded_times))
    	{
     		combined_logs.push_back(log);
    	}
  	}

	for (auto &worker : workers)
  	{
    	for (auto &log : *(worker->logger_.recorded_times))
    	{
      		combined_logs.push_back(log);
    	}
  	}

	//combined_logs.push_back(*(executors.strat->logger.recorded_times));
	for (auto &log : *(executors.strat->logger.recorded_times)) 
	{
		combined_logs.push_back(log);
	}

	for (auto &log : *(logger.recorded_times)) 
	{
		combined_logs.push_back(log);
	}
	std::sort(combined_logs.begin(), combined_logs.end(), [](const std::pair<std::string, u64> &a, const std::pair<std::string, u64> &b)
        { return a.second < b.second; });
	for (auto p : combined_logs)
  	{
    	output_file << p.second << " " << p.first << std::endl;
  	}
  	output_file.close();
  	std::cout<<"data written"<<std::endl;
	
}
