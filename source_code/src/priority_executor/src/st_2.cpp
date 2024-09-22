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
uint64_t MuExWorker::ed_time;
int main(int argc, char **argv) {
    rclcpp::init(argc, argv);
    ExecutableScheduleType schedule_type = CHAIN_AWARE_PRIORITY;
    
    auto node = rclcpp::Node::make_shared("experiment_parameters");
	node->declare_parameter("experiment_name");
 	node->declare_parameter("count_max");
 	node->declare_parameter("schedule_type");
    auto parameters_client = std::make_shared<rclcpp::SyncParametersClient>(node);
  	// parameters_client->wait_for_service();
  	const std::string schedule_type_str = parameters_client->get_parameter("schedule_type", std::string("deadline"));
  	std::cout << schedule_type_str << std::endl;
  	int COUNT_MAX = parameters_client->get_parameter("count_max", 500);
    executor_strat executors;
    size_t NumThreads = 1;
	bool YieldBeforeExecute = false;
	std::cout << "creating MultiThreadExecutors" << std::endl;
	executors.strat = std::make_shared<PriorityMemoryStrategy<>>();
	rclcpp::ExecutorOptions options;
	options.memory_strategy = executors.strat;
	executors.strat->logger = create_logger();
	executors.strat->is_f1tenth = false;
	executors.executor = std::make_shared<timed_executor::MultiThreadTimedExecutor>(options, NumThreads, YieldBeforeExecute, std::chrono::nanoseconds(-1), "multi_test");
    /*
    std::vector<uint64_t> chain_lengths = {2, 4};
	std::vector<std::vector<uint64_t>> chain_member_ids = {{1, 2}, {3, 4, 5, 6}};
    std::vector<std::vector<uint64_t>> chain_priorities = {{0, 1}, {2, 3, 4, 5}};
	std::vector<double_t> node_runtimes = {2.3, 16.1, 2.3, 2.2, 18.4, 9.1};
	std::vector<uint64_t> chain_periods = {80, 100};
	std::vector<uint64_t> chain_deadlines = {80, 100};
    */
    std::vector<uint64_t> chain_lengths = {4, 4};
	std::vector<std::vector<uint64_t>> chain_member_ids = {{1, 2, 3, 4}, {5, 6, 7, 8}};
    std::vector<std::vector<uint64_t>> chain_priorities = {{0, 1, 2, 3}, {4, 5, 6, 7}};
	//std::vector<double_t> node_runtimes = {2, 16, 2, 2, 18, 9, 23, 8, 14, 18, 21, 18, 7, 2, 11, 8, 8, 2, 196};
	//std::vector<uint64_t> chain_periods = {80, 80, 100, 100, 160, 1000};
	//std::vector<uint64_t> chain_deadlines = {80, 80, 100, 100, 160, 1000};
	std::vector<double_t> node_runtimes = {2, 16, 3, 8, 2, 2, 9, 9};
	std::vector<uint64_t> chain_periods = {100, 100};
	std::vector<uint64_t> chain_deadlines = {100, 100};

	std::vector<std::vector<std::shared_ptr<rclcpp::Node>>> nodes;
	std::vector<std::shared_ptr<PublisherNode>> publishers;
	std::vector<std::shared_ptr<DummyWorker>> workers;
    node_time_logger logger = create_logger();
	timespec current_time;
	clock_gettime(CLOCK_MONOTONIC_RAW, &current_time);
	uint64_t millis = (current_time.tv_sec * (uint64_t)1000) + (current_time.tv_nsec / 1000000);
	PublisherNode::set_end_time(millis + 50000);
	DummyWorker::set_end_time(millis + 50000);
    uint64_t current_node_id = 0;
    for (uint chain_index = 0; chain_index < chain_lengths.size(); ++chain_index) {
        std::shared_ptr<rclcpp::TimerBase> this_chain_timer_handle;
        nodes.push_back(std::vector<std::shared_ptr<rclcpp::Node>>());
        for (uint cb_index = 0; cb_index < chain_lengths[chain_index]; cb_index++) {
            if (cb_index == 0) {
                std::shared_ptr<PublisherNode> publisher_node;
                //if (chain_index == 1) {
                //    publisher_node = std::static_pointer_cast<PublisherNode>(nodes[0][0]);
                //    this_chain_timer_handle = publisher_node->timer_;
                //}
                //else {
                    publisher_node = std::make_shared<PublisherNode>("topic_" + std::to_string(chain_index), chain_index, chain_periods[chain_index], node_runtimes[current_node_id]);
                    publishers.push_back(publisher_node);
                    publisher_node->count_max = COUNT_MAX;
                    auto timer_handle = publisher_node->timer_->get_timer_handle();
                    executors.strat->set_executable_priority(publisher_node->timer_->get_timer_handle(), chain_priorities[chain_index][cb_index], TIMER, CHAIN_AWARE_PRIORITY, chain_index);
                    executors.strat->set_first_in_chain(publisher_node->timer_->get_timer_handle());
                    this_chain_timer_handle = publisher_node->timer_;
					executors.strat->get_priority_settings(publisher_node->timer_->get_timer_handle())->timer_handle = this_chain_timer_handle;
                    executors.executor->add_node(publisher_node);

					clock_gettime(CLOCK_MONOTONIC_RAW, &current_time);
    				uint64_t millis = (current_time.tv_sec * (uint64_t)1000) + (current_time.tv_nsec / 1000000);
    				uint64_t time_until_trigger = this_chain_timer_handle->time_until_trigger().count() / 1000000;
					//if (chain_index != 1)
						log_entry(logger, std::to_string(chain_index) + " release_time: " + std::to_string(millis + time_until_trigger)); 
                nodes[chain_index].push_back(std::static_pointer_cast<rclcpp::Node>(publisher_node));
            }
            else {
                std::shared_ptr<DummyWorker> sub_node;

				//if (chain_index == 1 && cb_index == 1) {
				//	sub_node = std::make_shared<DummyWorker>("chain_" + std::to_string(chain_index) + "_worker_" + std::to_string(cb_index), node_runtimes[current_node_id], chain_index, cb_index, true);
				//}
				//else {
					
					if (cb_index == chain_lengths[chain_index] - 1)
						sub_node = std::make_shared<DummyWorker>("chain_" + std::to_string(chain_index) + "_worker_" + std::to_string(cb_index), node_runtimes[current_node_id], chain_index, cb_index, false, true);
					else
						sub_node = std::make_shared<DummyWorker>("chain_" + std::to_string(chain_index) + "_worker_" + std::to_string(cb_index), node_runtimes[current_node_id], chain_index, cb_index, false, false);
					
					//sub_node = std::make_shared<DummyWorker>("chain_" + std::to_string(chain_index) + "_worker_" + std::to_string(cb_index), node_runtimes[current_node_id], chain_index, cb_index);
                //sub_node = std::make_shared<DummyWorker>("chain_" + std::to_string(chain_index) + "_worker_" + std::to_string(cb_index), node_runtimes[current_node_id], chain_index, cb_index);
                workers.push_back(sub_node);
                executors.strat->set_executable_priority(sub_node->subscription_->get_subscription_handle(), chain_priorities[chain_index][cb_index], SUBSCRIPTION, CHAIN_AWARE_PRIORITY, chain_index);
                executors.executor->add_node(sub_node);
                nodes[chain_index].push_back(std::static_pointer_cast<rclcpp::Node>(sub_node));	
            }
            current_node_id++;	
        }
    }
    executors.executor->spin();
    rclcpp::shutdown();

	std::ofstream output_file;
	std::string suffix = "";
	output_file.open("experiments/st_2" + std::to_string(NumThreads) + "c" + suffix + ".txt");
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
    return 0;
}