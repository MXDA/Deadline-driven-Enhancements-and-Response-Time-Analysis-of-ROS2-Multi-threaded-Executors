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
  	int COUNT_MAX = parameters_client->get_parameter("count_max", 3);
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
	bool YieldBeforeExecute = true;
	std::cout << "creating MultiThreadExecutors" << std::endl;
	executors.strat = std::make_shared<PriorityMemoryStrategy<>>();
	rclcpp::ExecutorOptions options;
	options.memory_strategy = executors.strat;
	executors.strat->logger = create_logger();
	executors.strat->is_f1tenth = false;
	executors.executor = std::make_shared<timed_executor::MultiThreadTimedExecutor>(options, NumThreads, YieldBeforeExecute, std::chrono::nanoseconds(-1), "test1");
	//executors.executor->set_use_priorities(true);

	std::cout << "MultiThreadExecutors created" << std::endl;
	/*
	std::vector<uint64_t> chain_lengths = {2, 4, 4, 3, 4, 2, 2, 2, 2, 2, 2, 2};
	std::vector<std::vector<uint64_t>> chain_member_ids = {{1, 2}, {1, 3, 4, 5}, {6, 7, 8, 9}, {10, 11, 12}, {13, 14, 15, 16}, {17, 18}, {19, 20}, {21, 22}, {23, 24}, {25, 26}, {27, 28}, {29, 30}};
	std::vector<std::vector<uint64_t>> chain_priorities = {{1, 0}, {5, 4, 3, 2, 1}, {9, 8, 7, 6}, {12, 11, 10}, {16, 15, 14, 13}, {18, 17}, {20, 19}, {22, 21}, {24, 23}, {26, 25}, {28, 27}, {30, 29}};
	//多线程执行器线程对cpu的分配怎么解决
	std::vector<double_t> node_runtimes = {2.3, 16.1, 2.3, 2.2, 18.4, 9.1, 23.1, 7.9, 14.2, 17.9, 20.6, 17.9, 6.6, 1.7, 11.0, 6.6, 7.9, 1.7, 195.9, 33.2, 2.2, 33.2, 2.2, 33.2, 2.2, 33.2, 2.2, 33.2, 2.2, 33.2, 2.2};
	std::vector<uint64_t> chain_periods = {80, 80, 100, 100, 160, 1000, 120, 120, 120, 120, 120, 120};
	std::vector<uint64_t> chain_deadlines = {80, 80, 100, 100, 160, 1000, 120, 120, 120, 120, 120, 120};
	*/
	std::vector<uint64_t> chain_lengths = {2, 4, 4};
	std::vector<std::vector<uint64_t>> chain_member_ids = {{1, 2}, {3, 4, 5, 6}, {7, 8, 9, 10}};
	std::vector<double_t> node_runtimes = {4, 16, 2, 2, 18, 8, 23, 7, 13, 17}; 
	std::vector<uint64_t> chain_periods = {80, 100, 150};
	std::vector<uint64_t> chain_deadlines = {80, 100, 150};

	std::vector<std::vector<std::shared_ptr<rclcpp::Node>>> nodes;
	std::vector<std::shared_ptr<PublisherNode>> publishers;
	std::vector<std::shared_ptr<DummyWorker>> workers;
	std::vector<std::deque<uint> *> chain_deadlines_deque;

	uint64_t current_node_id = 0;
	for (uint chain_index = 0; chain_index < chain_lengths.size(); ++chain_index) {
		std::cout << "making chain" << std::to_string(chain_index) << std::endl;
		std::shared_ptr<rclcpp::TimerBase> this_chain_timer_handle;
		std::deque<uint> *this_chain_deadlines_deque = new std::deque<uint>();
		nodes.push_back(std::vector<std::shared_ptr<rclcpp::Node>>());
		
		for (uint cb_index = 0; cb_index < chain_lengths[chain_index]; cb_index++) {
			std::cout << "making node " << std::to_string(current_node_id) << " with runtime " << node_runtimes[current_node_id] << std::endl;
			if (cb_index == 0) {
				std::shared_ptr<PublisherNode> publisher_node;
			
				publisher_node = std::make_shared<PublisherNode>("topic_" + std::to_string(chain_index), chain_index, chain_periods[chain_index], node_runtimes[current_node_id]);
          			publishers.push_back(publisher_node);
				publisher_node->count_max = COUNT_MAX;
					
				assert(executors.strat != nullptr);
				auto timer_handle = publisher_node->timer_->get_timer_handle();
				assert(timer_handle != nullptr);
				executors.strat->set_executable_deadline(publisher_node->timer_->get_timer_handle(), chain_deadlines[chain_index], TIMER, chain_index);

				executors.strat->set_first_in_chain(publisher_node->timer_->get_timer_handle());
				executors.strat->assign_deadlines_queue(publisher_node->timer_->get_timer_handle(), this_chain_deadlines_deque);
				this_chain_timer_handle = publisher_node->timer_;
				executors.strat->get_priority_settings(publisher_node->timer_->get_timer_handle())->timer_handle = this_chain_timer_handle;
				executors.executor->add_node(publisher_node);
				nodes[chain_index].push_back(std::static_pointer_cast<rclcpp::Node>(publisher_node));
			}
			else {
				std::shared_ptr<DummyWorker> sub_node;
				
				
				sub_node = std::make_shared<DummyWorker>("chain_" + std::to_string(chain_index) + "_worker_" + std::to_string(cb_index), node_runtimes[current_node_id], chain_index, cb_index);
				
				workers.push_back(sub_node);
				executors.strat->set_executable_deadline(sub_node->subscription_->get_subscription_handle(), chain_deadlines[chain_index], SUBSCRIPTION, chain_index);
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
	}
	std::cout << "initialized nodes" << std::endl;
	node_time_logger logger = create_logger();
	timespec current_time;
	clock_gettime(CLOCK_MONOTONIC_RAW, &current_time);
	uint64_t millis = (current_time.tv_sec * (uint64_t)1000) + (current_time.tv_nsec / 1000000);
	for (uint chain_index = 0; chain_index < chain_lengths.size(); ++chain_index) {
		log_entry(logger, "deadline_" + std::to_string(chain_index) + "_" + std::to_string(millis + chain_deadlines[chain_index]));
		//log_entry(logger, "timer_" + std::to_string(chain_index) + "_release_" + std::to_string(millis));
		std::cout << "timer_" + std::to_string(chain_index) + "_release_" + std::to_string(millis) << std::endl;
		std::cout << "deadline_" + std::to_string(chain_index) + "_" + std::to_string(millis + chain_deadlines[chain_index]) << std::endl;
		chain_deadlines_deque[chain_index]->push_back(millis + chain_deadlines[chain_index]);
	}
	//executors.strat->print_all_handle_schedule_type();
	//std::cout << "------------------------" << std::endl;	
	//clock_gettime(CLOCK_MONOTONIC_RAW, &current_time);
	//uint64_t millis1 = (current_time.tv_sec * (uint64_t)1000) + (current_time.tv_nsec / 1000000);
	//std::cout << "time_gap1:" << millis1 - millis << std::endl;
	//executors.executor.cpus = {};
		
	executors.executor->spin();
	//executors.strat->print_all_handle_schedule_type();
	rclcpp::shutdown();
	std::cout << "rclcpp::shutdown()" << std::endl;
	
	std::ofstream output_file;
	std::string suffix = "";
	output_file.open("experiments/test1" + std::to_string(NumThreads) + "c" + suffix + ".txt");
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
	std::sort(combined_logs.begin(), combined_logs.end(), [](const std::pair<std::string, u64> &a, const std::pair<std::string, u64> &b)
        { return a.second < b.second; });
	for (auto p : combined_logs)
  	{
    	output_file << p.second << " " << p.first << std::endl;
  	}
  	output_file.close();
  	std::cout<<"data written"<<std::endl;
	
}
