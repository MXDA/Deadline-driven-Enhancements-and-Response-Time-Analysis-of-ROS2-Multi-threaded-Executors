#include "priority_executor/test_nodes.hpp"
#include "priority_executor/primes_workload.hpp"
#include "simple_timer/rt-sched.hpp"
#include <string>
#include <vector>
#include <thread>
#include <sstream>
using namespace std::chrono_literals;
using std::placeholders::_1;

PublisherNode::PublisherNode(std::string publish_topic, int chain, int period, double runtime)
    : Node("PublisherNode_" + publish_topic), count_(0)
{
  logger_ = create_logger();
  publisher_ = this->create_publisher<std_msgs::msg::String>(publish_topic, 1);
  this->chain = chain;
  this->runtime = runtime;
  this->period = period;


  std::cout << "creating timer "
            << publish_topic << std::endl;
  auto timer_callback =
      [this]() -> void
  {
    
    /*if (this->count_ > this->count_max)
    {
      rclcpp::shutdown();
      return;
    }
    */
    timespec current_time;
    clock_gettime(CLOCK_MONOTONIC_RAW, &current_time);
    uint64_t millis = (current_time.tv_sec * (uint64_t)1000) + (current_time.tv_nsec / 1000000);
    uint64_t time_until_trigger = timer_->time_until_trigger().count() / 1000000;
    this->logger_.recorded_times->push_back(std::make_pair(std::to_string(this->chain) + " release_time: " + std::to_string(millis + time_until_trigger), get_time_us()));
    //std::cout << "chain_id: " << this->chain << " time_until_trigger: " << time_until_trigger << " release_time: " << millis + time_until_trigger << std::endl;
    //std::cout << "chain_id: " << this->chain << " current_time: " << millis << std::endl;
    //auto thread_id = std::this_thread::get_id();
    //std::stringstream ss;
    //ss << thread_id;
    //std::string thread_id_str = ss.str();
    
    //this->logger_.recorded_times->push_back(std::make_pair(std::string(this->get_name()) + "_publish_" + std::to_string(this->count_) + "_thread_id: " + thread_id_str, get_time_us()));
    //this->logger_.recorded_times->push_back(std::make_pair("chain_" + std::to_string(this->chain) + "_worker_0_recv_MESSAGE" + std::to_string(this->count_) + "_thread_id: " + thread_id_str, get_time_us()));
    clock_gettime(CLOCK_MONOTONIC_RAW, &current_time);
    millis = (current_time.tv_sec * (uint64_t)1000) + (current_time.tv_nsec / 1000000);
    if (millis > end_time) {
      rclcpp::shutdown();
      return;
    }
    double result = nth_prime_silly(100000, this->runtime);
    
    clock_gettime(CLOCK_MONOTONIC_RAW, &current_time);
    millis = (current_time.tv_sec * (uint64_t)1000) + (current_time.tv_nsec / 1000000);
    if (millis > end_time) {
      rclcpp::shutdown();
      return;
    }
    //this->logger_.recorded_times->push_back(std::make_pair("chain_" + std::to_string(this->chain) + "_worker_0_processed_MESSAGE" + std::to_string(this->count_) + "_thread_id: " + thread_id_str, get_time_us()));
    auto message = std_msgs::msg::String();
    message.data = "MESSAGE" + std::to_string(this->count_++);
    this->publisher_->publish(message);

    // RCLCPP_INFO(this->get_logger(), "I did work on: '%s', taking %lf ms", message.data.c_str(), result);
    // usleep(600 * 1000);
  };
  //timespec current_time;
  //clock_gettime(CLOCK_MONOTONIC_RAW, &current_time);
  //uint64_t millis = (current_time.tv_sec * (uint64_t)1000) + (current_time.tv_nsec / 1000000);
  //std::cout << "timer create before: " << millis << std::endl; 
  timer_ = this->create_wall_timer(std::chrono::milliseconds(period), timer_callback);
  //timer_ = this->create_timer(std::chrono::milliseconds(period), timer_callback);
  //clock_gettime(CLOCK_MONOTONIC_RAW, &current_time);
  //millis = (current_time.tv_sec * (uint64_t)1000) + (current_time.tv_nsec / 1000000);
  //std::cout << "timer create after: " << millis << std::endl;
}

DummyWorker::DummyWorker(const std::string &name, double runtime, int chain, int number, bool is_multichain, bool is_last)
    : Node(name)
{
  this->runtime = runtime;
  this->number = number;
  this->chain = chain;
  this->is_last_in_chain = is_last;
  this->logger_ = create_logger();
  std::cout << "creating dummy worker "
            << name << std::endl;
  if (is_multichain)
  {
    subscription_ = this->create_subscription<std_msgs::msg::String>(
        "topic_" + std::to_string(chain - 1), 1, std::bind(&DummyWorker::topic_callback, this, _1));
  }
  else if (number == 1)
  {
    subscription_ = this->create_subscription<std_msgs::msg::String>(
        "topic_" + std::to_string(chain), 1, std::bind(&DummyWorker::topic_callback, this, _1));
  }
  else
  {
    subscription_ = this->create_subscription<std_msgs::msg::String>(
        "chain_" + std::to_string(chain) + "_topic_" + std::to_string(number), 1, std::bind(&DummyWorker::topic_callback, this, _1));
  }
  this->publisher_ = this->create_publisher<std_msgs::msg::String>("chain_" + std::to_string(chain) + "_topic_" + std::to_string(number + 1), 1);
}
void DummyWorker::topic_callback(const std_msgs::msg::String::SharedPtr msg) const
{

  //auto thread_id = std::this_thread::get_id();
  //std::stringstream ss;
  //ss << thread_id;
  //std::string thread_id_str = ss.str();
  //this->logger_.recorded_times->push_back(std::make_pair(std::string(this->get_name()) + "_recv_" + msg->data + "_thread_id: " + thread_id_str, get_time_us()));
  //std::cout << this->chain << " working" <<std::endl;
  timespec current_time;
  clock_gettime(CLOCK_MONOTONIC_RAW, &current_time);
  uint64_t millis = (current_time.tv_sec * (uint64_t)1000) + (current_time.tv_nsec / 1000000);
  if (millis > end_time) {
      rclcpp::shutdown();
      return;
  }
  double result = nth_prime_silly(100000, runtime);

  clock_gettime(CLOCK_MONOTONIC_RAW, &current_time);
  millis = (current_time.tv_sec * (uint64_t)1000) + (current_time.tv_nsec / 1000000);
  if (millis > end_time) {
      rclcpp::shutdown();
      return;
  }
  //std::cout << "is_last_in_chain: " << is_last_in_chain << std::endl;
  if (is_last_in_chain) {
    //std::cout << this->chain << " is_last_in_chain" << std::endl;
    clock_gettime(CLOCK_MONOTONIC_RAW, &current_time);
    uint64_t millis = (current_time.tv_sec * (uint64_t)1000) + (current_time.tv_nsec / 1000000);
    this->logger_.recorded_times->push_back(std::make_pair(std::to_string(this->chain) + " completed_time: " + std::to_string(millis), get_time_us()));
  }
  
  //this->logger_.recorded_times->push_back(std::make_pair(std::string(this->get_name()) + "_processed_" + msg->data + "_thread_id: " + thread_id_str, get_time_us()));
  // RCLCPP_INFO(this->get_logger(), "I did work on: '%s', taking %lf ms", msg->data.c_str(), result);
  auto message = std_msgs::msg::String();
  message.data = msg->data;
  this->publisher_->publish(message);
  // usleep(600 * 1000);
}


MuExWorker::MuExWorker(const std::string &name) : Node(name) {
  this->logger_ = create_logger();
  //this->chain3_runtime = chain3_runtime;
  //this->chain4_runtime = chain4_runtime;
  for(uint i = 0; i < 2; ++i) {
    rclcpp::Subscription<std_msgs::msg::String>::SharedPtr subscription_;
    subscription_chain3.push_back(subscription_);
  }
  subscription_chain3[0] = this->create_subscription<std_msgs::msg::String>(
        "topic_" + std::to_string(3), 1, std::bind(&MuExWorker::topic_callback31, this, _1));
  subscription_chain3[1] = this->create_subscription<std_msgs::msg::String>(
        "chain_" + std::to_string(3) + "_topic_" + std::to_string(2), 1, std::bind(&MuExWorker::topic_callback32, this, _1));
  for(uint i = 0; i < 3; ++i) {
    rclcpp::Subscription<std_msgs::msg::String>::SharedPtr subscription_;
    subscription_chain4.push_back(subscription_);
  }
  subscription_chain4[0] = this->create_subscription<std_msgs::msg::String>(
        "topic_" + std::to_string(4), 1, std::bind(&MuExWorker::topic_callback41, this, _1));
  subscription_chain4[1] = this->create_subscription<std_msgs::msg::String>(
        "chain_" + std::to_string(4) + "_topic_" + std::to_string(2), 1, std::bind(&MuExWorker::topic_callback42, this, _1));
  subscription_chain4[2] = this->create_subscription<std_msgs::msg::String>(
        "chain_" + std::to_string(4) + "_topic_" + std::to_string(3), 1, std::bind(&MuExWorker::topic_callback43, this, _1));


  for(uint i = 0; i < 2; ++i) {
    rclcpp::Publisher<std_msgs::msg::String>::SharedPtr publisher;
    publisher_chain3.push_back(publisher);
  }
  this->publisher_chain3[0] = this->create_publisher<std_msgs::msg::String>("chain_" + std::to_string(3) + "_topic_" + std::to_string(2), 1);
  this->publisher_chain3[1] = this->create_publisher<std_msgs::msg::String>("chain_" + std::to_string(3) + "_topic_" + std::to_string(3), 1);
  for(uint i = 0; i < 3; ++i) {
    rclcpp::Publisher<std_msgs::msg::String>::SharedPtr publisher;
    publisher_chain4.push_back(publisher);
  }
  this->publisher_chain4[0] = this->create_publisher<std_msgs::msg::String>("chain_" + std::to_string(4) + "_topic_" + std::to_string(2), 1);
  this->publisher_chain4[1] = this->create_publisher<std_msgs::msg::String>("chain_" + std::to_string(4) + "_topic_" + std::to_string(3), 1);
  this->publisher_chain4[2] = this->create_publisher<std_msgs::msg::String>("chain_" + std::to_string(4) + "_topic_" + std::to_string(4), 1);
}


void MuExWorker::topic_callback31(const std_msgs::msg::String::SharedPtr msg) const {

  timespec current_time;
  clock_gettime(CLOCK_MONOTONIC_RAW, &current_time);
  uint64_t millis = (current_time.tv_sec * (uint64_t)1000) + (current_time.tv_nsec / 1000000);
  if (millis > ed_time) {
      rclcpp::shutdown();
      return;
  }
  double result = nth_prime_silly(100000, 8.0);

  clock_gettime(CLOCK_MONOTONIC_RAW, &current_time);
  millis = (current_time.tv_sec * (uint64_t)1000) + (current_time.tv_nsec / 1000000);
  if (millis > ed_time) {
      rclcpp::shutdown();
      return;
  }
  //std::cout << "is_last_in_chain: " << is_last_in_chain << std::endl;
  //if (is_last_in_chain) {
  //  //std::cout << this->chain << " is_last_in_chain" << std::endl;
  //  clock_gettime(CLOCK_MONOTONIC_RAW, &current_time);
  //  uint64_t millis = (current_time.tv_sec * (uint64_t)1000) + (current_time.tv_nsec / 1000000);
  //  this->logger_.recorded_times->push_back(std::make_pair(std::to_string(this->chain) + " completed_time: " + std::to_string(millis), get_time_us()));
  //}
  
  //this->logger_.recorded_times->push_back(std::make_pair(std::string(this->get_name()) + "_processed_" + msg->data + "_thread_id: " + thread_id_str, get_time_us()));
  // RCLCPP_INFO(this->get_logger(), "I did work on: '%s', taking %lf ms", msg->data.c_str(), result);
  auto message = std_msgs::msg::String();
  message.data = msg->data;
  //this->publisher_->publish(message);
  this->publisher_chain3[0]->publish(message);
}

void MuExWorker::topic_callback32(const std_msgs::msg::String::SharedPtr msg) const {

  timespec current_time;
  clock_gettime(CLOCK_MONOTONIC_RAW, &current_time);
  uint64_t millis = (current_time.tv_sec * (uint64_t)1000) + (current_time.tv_nsec / 1000000);
  if (millis > ed_time) {
      rclcpp::shutdown();
      return;
  }
  double result = nth_prime_silly(100000, 14.0);

  clock_gettime(CLOCK_MONOTONIC_RAW, &current_time);
  millis = (current_time.tv_sec * (uint64_t)1000) + (current_time.tv_nsec / 1000000);
  if (millis > ed_time) {
      rclcpp::shutdown();
      return;
  }
  //std::cout << "is_last_in_chain: " << is_last_in_chain << std::endl;
  //if (is_last_in_chain) {
  //  //std::cout << this->chain << " is_last_in_chain" << std::endl;
  clock_gettime(CLOCK_MONOTONIC_RAW, &current_time);
  millis = (current_time.tv_sec * (uint64_t)1000) + (current_time.tv_nsec / 1000000);
  this->logger_.recorded_times->push_back(std::make_pair(std::to_string(3) + " completed_time: " + std::to_string(millis), get_time_us()));
  //}
  
  //this->logger_.recorded_times->push_back(std::make_pair(std::string(this->get_name()) + "_processed_" + msg->data + "_thread_id: " + thread_id_str, get_time_us()));
  // RCLCPP_INFO(this->get_logger(), "I did work on: '%s', taking %lf ms", msg->data.c_str(), result);
  auto message = std_msgs::msg::String();
  message.data = msg->data;
  //this->publisher_->publish(message);
  this->publisher_chain3[1]->publish(message);
}


void MuExWorker::topic_callback41(const std_msgs::msg::String::SharedPtr msg) const {

  timespec current_time;
  clock_gettime(CLOCK_MONOTONIC_RAW, &current_time);
  uint64_t millis = (current_time.tv_sec * (uint64_t)1000) + (current_time.tv_nsec / 1000000);
  if (millis > ed_time) {
      rclcpp::shutdown();
      return;
  }
  double result = nth_prime_silly(100000, 11.0);

  clock_gettime(CLOCK_MONOTONIC_RAW, &current_time);
  millis = (current_time.tv_sec * (uint64_t)1000) + (current_time.tv_nsec / 1000000);
  if (millis > ed_time) {
      rclcpp::shutdown();
      return;
  }
  //std::cout << "is_last_in_chain: " << is_last_in_chain << std::endl;
  //if (is_last_in_chain) {
  //  //std::cout << this->chain << " is_last_in_chain" << std::endl;
  //  clock_gettime(CLOCK_MONOTONIC_RAW, &current_time);
  //  uint64_t millis = (current_time.tv_sec * (uint64_t)1000) + (current_time.tv_nsec / 1000000);
  //  this->logger_.recorded_times->push_back(std::make_pair(std::to_string(this->chain) + " completed_time: " + std::to_string(millis), get_time_us()));
  //}
  
  //this->logger_.recorded_times->push_back(std::make_pair(std::string(this->get_name()) + "_processed_" + msg->data + "_thread_id: " + thread_id_str, get_time_us()));
  // RCLCPP_INFO(this->get_logger(), "I did work on: '%s', taking %lf ms", msg->data.c_str(), result);
  auto message = std_msgs::msg::String();
  message.data = msg->data;
  //this->publisher_->publish(message);
  this->publisher_chain4[0]->publish(message);
}

void MuExWorker::topic_callback42(const std_msgs::msg::String::SharedPtr msg) const {

  timespec current_time;
  clock_gettime(CLOCK_MONOTONIC_RAW, &current_time);
  uint64_t millis = (current_time.tv_sec * (uint64_t)1000) + (current_time.tv_nsec / 1000000);
  if (millis > ed_time) {
      rclcpp::shutdown();
      return;
  }
  double result = nth_prime_silly(100000, 8.0);

  clock_gettime(CLOCK_MONOTONIC_RAW, &current_time);
  millis = (current_time.tv_sec * (uint64_t)1000) + (current_time.tv_nsec / 1000000);
  if (millis > ed_time) {
      rclcpp::shutdown();
      return;
  }
  //std::cout << "is_last_in_chain: " << is_last_in_chain << std::endl;
  //if (is_last_in_chain) {
  //  //std::cout << this->chain << " is_last_in_chain" << std::endl;
  //  clock_gettime(CLOCK_MONOTONIC_RAW, &current_time);
  //  uint64_t millis = (current_time.tv_sec * (uint64_t)1000) + (current_time.tv_nsec / 1000000);
  //  this->logger_.recorded_times->push_back(std::make_pair(std::to_string(this->chain) + " completed_time: " + std::to_string(millis), get_time_us()));
  //}
  
  //this->logger_.recorded_times->push_back(std::make_pair(std::string(this->get_name()) + "_processed_" + msg->data + "_thread_id: " + thread_id_str, get_time_us()));
  // RCLCPP_INFO(this->get_logger(), "I did work on: '%s', taking %lf ms", msg->data.c_str(), result);
  auto message = std_msgs::msg::String();
  message.data = msg->data;
  //this->publisher_->publish(message);
  this->publisher_chain4[1]->publish(message);
}

void MuExWorker::topic_callback43(const std_msgs::msg::String::SharedPtr msg) const {

  timespec current_time;
  clock_gettime(CLOCK_MONOTONIC_RAW, &current_time);
  uint64_t millis = (current_time.tv_sec * (uint64_t)1000) + (current_time.tv_nsec / 1000000);
  if (millis > ed_time) {
      rclcpp::shutdown();
      return;
  }
  double result = nth_prime_silly(100000, 8.0);

  clock_gettime(CLOCK_MONOTONIC_RAW, &current_time);
  millis = (current_time.tv_sec * (uint64_t)1000) + (current_time.tv_nsec / 1000000);
  if (millis > ed_time) {
      rclcpp::shutdown();
      return;
  }
  //std::cout << "is_last_in_chain: " << is_last_in_chain << std::endl;
  //if (is_last_in_chain) {
  //  //std::cout << this->chain << " is_last_in_chain" << std::endl;
  clock_gettime(CLOCK_MONOTONIC_RAW, &current_time);
  millis = (current_time.tv_sec * (uint64_t)1000) + (current_time.tv_nsec / 1000000);
  this->logger_.recorded_times->push_back(std::make_pair(std::to_string(4) + " completed_time: " + std::to_string(millis), get_time_us()));
  //}
  
  //this->logger_.recorded_times->push_back(std::make_pair(std::string(this->get_name()) + "_processed_" + msg->data + "_thread_id: " + thread_id_str, get_time_us()));
  // RCLCPP_INFO(this->get_logger(), "I did work on: '%s', taking %lf ms", msg->data.c_str(), result);
  auto message = std_msgs::msg::String();
  message.data = msg->data;
  //this->publisher_->publish(message);
  this->publisher_chain4[2]->publish(message);
}
