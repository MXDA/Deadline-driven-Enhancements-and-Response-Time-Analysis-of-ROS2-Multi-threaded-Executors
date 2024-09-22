#include "rclcpp/rclcpp.hpp"
#include "simple_timer/rt-sched.hpp"
#include "std_msgs/msg/string.hpp"
#include <vector>
using namespace std::chrono_literals;
using std::placeholders::_1;

class PublisherNode : public rclcpp::Node
{
public:
  PublisherNode(std::string publish_topic, int chain, int period, double runtime);

  rclcpp::TimerBase::SharedPtr timer_;
  uint count_max = 20;
  node_time_logger logger_;
  static uint64_t end_time;
  static void set_end_time(uint64_t time) {
    end_time = time;
  }
private:
  rclcpp::Publisher<std_msgs::msg::String>::SharedPtr publisher_;
  uint count_;
  int chain;
  double runtime;
  int period;
};
class DummyWorker : public rclcpp::Node
{
public:
  DummyWorker(const std::string &name, double runtime, int chain, int number, bool is_multichain = false, bool is_last = false);
  node_time_logger logger_;
  static uint64_t end_time;
  static void set_end_time(uint64_t time) {
    end_time = time;
  }
private:
  double runtime;
  int number;
  int chain;
  bool is_last_in_chain;
  rclcpp::Publisher<std_msgs::msg::String>::SharedPtr publisher_;
  void topic_callback(const std_msgs::msg::String::SharedPtr msg) const;

public:
  rclcpp::Subscription<std_msgs::msg::String>::SharedPtr subscription_;
};

class MuExWorker : public rclcpp::Node 
{
public:
  MuExWorker(const std::string &name);
  node_time_logger logger_;
  static uint64_t ed_time;
  static void set_end_time(uint64_t time) {
    ed_time = time;
  }

private:
  std::vector<rclcpp::Publisher<std_msgs::msg::String>::SharedPtr> publisher_chain3;
  std::vector<rclcpp::Publisher<std_msgs::msg::String>::SharedPtr> publisher_chain4;
  void topic_callback31(const std_msgs::msg::String::SharedPtr msg) const;
  void topic_callback32(const std_msgs::msg::String::SharedPtr msg) const;
  void topic_callback41(const std_msgs::msg::String::SharedPtr msg) const;
  void topic_callback42(const std_msgs::msg::String::SharedPtr msg) const;
  void topic_callback43(const std_msgs::msg::String::SharedPtr msg) const;
public:
  std::vector<rclcpp::Subscription<std_msgs::msg::String>::SharedPtr> subscription_chain3;
  std::vector<rclcpp::Subscription<std_msgs::msg::String>::SharedPtr> subscription_chain4;
};