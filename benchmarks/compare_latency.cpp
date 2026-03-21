#include <algorithm>
#include <atomic>
#include <format>
#include <fstream>
#include <future>
#include <print>
#include <set>
#include <string>
#include <thread>
#include <vector>

#include <boost/lockfree/queue.hpp>
#include <boost/lockfree/spsc_queue.hpp>

#include <atomic_queue/atomic_queue.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#include <xenium/michael_scott_queue.hpp>
#include <xenium/nikolaev_bounded_queue.hpp>
#include <xenium/ramalhete_queue.hpp>
#include <xenium/reclamation/generic_epoch_based.hpp>
#include <xenium/vyukov_bounded_queue.hpp>
#pragma GCC diagnostic pop

#include <concurrentqueue.h>
#include <readerwritercircularbuffer.h>
#include <readerwriterqueue.h>

#include "orbit/mpmc_queue.h"
#include "timer.h"

template <size_t QUEUE_SIZE = 128, size_t PS = 3, size_t PL = 40>
class latency_test
{
public:
  latency_test() : _bdd_spsc_moodycamel_queue1(QUEUE_SIZE), _bdd_spsc_moodycamel_queue2(QUEUE_SIZE)
  {
  }

  void run_benchmark(int32_t num_values)
  {
    do_run_benchmark("lat_orbit_mpmc_queue", _lat_queue1, _lat_queue2, &latency_test::mpmc_queue_bounce_thread, num_values);
    do_run_benchmark("tp_orbit_mpmc_queue", _tp_queue1, _tp_queue2, &latency_test::mpmc_queue_bounce_thread, num_values);
    do_run_benchmark("ultra_tp_orbit_mpmc_queue", _ultra_tp_queue1, _ultra_tp_queue2, &latency_test::mpmc_queue_bounce_thread, num_values);

    do_run_benchmark("lat_blocking_orbit_mpmc_queue", _lat_blocking_queue1, _lat_blocking_queue2, &latency_test::mpmc_queue_bounce_thread, num_values);
    do_run_benchmark("tp_blocking_orbit_mpmc_queue", _tp_blocking_queue1, _tp_blocking_queue2, &latency_test::mpmc_queue_bounce_thread, num_values);
    do_run_benchmark("ultra_tp_blocking_orbit_mpmc_queue", _ultra_tp_blocking_queue1, _ultra_tp_blocking_queue2, &latency_test::mpmc_queue_bounce_thread, num_values);

    do_run_benchmark("atomic_queue", _atomic_queue1, _atomic_queue2, &latency_test::atomic_queue_bounce_thread, num_values);
    do_run_benchmark("atomic_queue2", _atomic_queue21, _atomic_queue22, &latency_test::atomic_queue_bounce_thread, num_values);
    do_run_benchmark("try_atomic_queue2", _atomic_queue21, _atomic_queue22, &latency_test::try_atomic_queue_bounce_thread, num_values);
    do_run_benchmark("spsc_atomic_queue", _spsc_atomic_queue1, _spsc_atomic_queue2, &latency_test::atomic_queue_bounce_thread, num_values);
    do_run_benchmark("spsc_atomic_queue2", _spsc_atomic_queue1, _spsc_atomic_queue2, &latency_test::atomic_queue_bounce_thread, num_values);

    do_run_benchmark("boost_queue", _boost_queue1, _boost_queue2, &latency_test::boost_queue_bounce_thread, num_values);
    do_run_benchmark("spsc_boost_queue", _spsc_boost_queue1, _spsc_boost_queue2, &latency_test::boost_queue_bounce_thread, num_values);

    do_run_benchmark("xenium_ramalhete_queue", _ramalhete_queue1, _ramalhete_queue2, &latency_test::ramalhete_queue_bounce_thread, num_values);

    do_run_benchmark("moodycamel_concurrentqueue", _moodycamel_queue1, _moodycamel_queue2, &latency_test::moodycamel_queue_bounce_thread, num_values);
    do_run_benchmark("spsc_moodycamel_readerwriterqueue", _spsc_moodycamel_queue1, _spsc_moodycamel_queue2, &latency_test::moodycamel_queue_bounce_thread, num_values);
    do_run_benchmark("spsc_moodycamel_readerwritercircularbuffer", _bdd_spsc_moodycamel_queue1, _bdd_spsc_moodycamel_queue2, &latency_test::moodycamel_bdd_queue_bounce_thread, num_values);
  };

private:
  template <typename T>
  void do_run_benchmark(const std::string& queue_name, T& q1, T& q2, void (latency_test::*bounce_func)(T&, T&, int32_t), int32_t num_values)
  {
    timer tm;
    tm.start();

    // Start threads
    std::thread t1([this, bounce_func, &q1, &q2, num_values]() { return (this->*bounce_func)(q1, q2, num_values); });
    std::thread t2([this, bounce_func, &q1, &q2, num_values]() { return (this->*bounce_func)(q2, q1, num_values); });

    // std::thread t3([this, bounce_func, &q1, &q2, num_values]() { return (this->*bounce_func)(q1, q2, num_values); });
    // std::thread t4([this, bounce_func, &q1, &q2, num_values]() { return (this->*bounce_func)(q2, q1, num_values); });

    // Wait for threads to finish
    t1.join();
    t2.join();

    // t3.join();
    // t4.join();

    // Get and store all results
    tm.stop();
    std::ofstream file("benchmarks/data/comparison/latency_data.csv", std::ios::app);
    std::println(file, "{}, {}, {}, {}, {}", queue_name, QUEUE_SIZE, "int32_t", num_values, tm.get_us());
    file.close();
  }

  /*
  Each loop in these threads, the element has to be both pushed and popped from both queues.

  There is duplication here, but this is to keep it simple for ease of changing when trying out different things during benchmarks.
  */
  template <typename T>
  void mpmc_queue_bounce_thread(T& q1, T& q2, int32_t num_bounces)
  {
    for (int32_t value = 1; value < num_bounces + 1; ++value)
    {
      q1.push(value);
      q2.pop();
    }
  }

  template <typename T>
  void atomic_queue_bounce_thread(T& q1, T& q2, int32_t num_bounces)
  {
    for (int32_t value = 1; value < num_bounces + 1; ++value)
    {
      q1.push(value);
      q2.pop();
    }
  }

  template <typename T>
  void try_atomic_queue_bounce_thread(T& q1, T& q2, int32_t num_bounces)
  {
    int32_t val;
    for (int32_t value = 1; value < num_bounces + 1; ++value)
    {
      while (!q1.try_push(value))
      {
        orbit::spin_pause<1>();
      }
      while (!q2.try_pop(val))
      {
        orbit::spin_pause<1>();
      }
    }
  }

  template <typename T>
  void boost_queue_bounce_thread(T& q1, T& q2, int32_t num_bounces)
  {
    int32_t val;
    for (int32_t value = 1; value < num_bounces + 1; ++value)
    {
      while (!q1.push(value))
      {
        orbit::spin_pause<1>();
      }
      while (!q2.pop(val))
      {
        orbit::spin_pause<1>();
      }
    }
  }

  template <typename T>
  void ramalhete_queue_bounce_thread(T& q1, T& q2, int32_t num_bounces)
  {
    for (int32_t value = 1; value < num_bounces + 1; ++value)
    {
      q1.push(value);
      while (!q2.pop())
      {
        orbit::spin_pause<1>();
      }
    }
  }

  template <typename T>
  void moodycamel_queue_bounce_thread(T& q1, T& q2, int32_t num_bounces)
  {
    int32_t val;
    for (int32_t value = 1; value < num_bounces + 1; ++value)
    {
      q1.enqueue(value);
      while (!q2.try_dequeue(val))
      {
        orbit::spin_pause<1>();
      }
    }
  }

  template <typename T>
  void moodycamel_bdd_queue_bounce_thread(T& q1, T& q2, int32_t num_bounces)
  {
    int32_t val;
    for (int32_t value = 1; value < num_bounces + 1; ++value)
    {
      while (!q1.try_enqueue(value))
      {
        orbit::spin_pause<1>();
      }
      while (!q2.try_dequeue(val))
      {
        orbit::spin_pause<1>();
      }
    }
  }

private:
  orbit::mpmc_queue<int32_t, QUEUE_SIZE, true, true, PS, PL> _lat_queue1;
  orbit::mpmc_queue<int32_t, QUEUE_SIZE, true, true, PS, PL> _lat_queue2;

  orbit::mpmc_queue<int32_t, QUEUE_SIZE, false, true, PS, PL> _tp_queue1;
  orbit::mpmc_queue<int32_t, QUEUE_SIZE, false, true, PS, PL> _tp_queue2;

  orbit::mpmc_queue<int32_t, QUEUE_SIZE, false, true, PS, 100> _ultra_tp_queue1;
  orbit::mpmc_queue<int32_t, QUEUE_SIZE, false, true, PS, 100> _ultra_tp_queue2;

  orbit::mpmc_queue<int32_t, QUEUE_SIZE, true, false, 2, PL> _lat_blocking_queue1;
  orbit::mpmc_queue<int32_t, QUEUE_SIZE, true, false, 2, PL> _lat_blocking_queue2;

  orbit::mpmc_queue<int32_t, QUEUE_SIZE, false, false, 2, PL> _tp_blocking_queue1;
  orbit::mpmc_queue<int32_t, QUEUE_SIZE, false, false, 2, PL> _tp_blocking_queue2;

  orbit::mpmc_queue<int32_t, QUEUE_SIZE, false, false, 2, 100> _ultra_tp_blocking_queue1;
  orbit::mpmc_queue<int32_t, QUEUE_SIZE, false, false, 2, 100> _ultra_tp_blocking_queue2;

  atomic_queue::AtomicQueue<int32_t, QUEUE_SIZE, 0, true, true, false, false> _atomic_queue1;
  atomic_queue::AtomicQueue<int32_t, QUEUE_SIZE, 0, true, true, false, false> _atomic_queue2;

  atomic_queue::AtomicQueue2<int32_t, QUEUE_SIZE, true, true, false, false> _atomic_queue21;
  atomic_queue::AtomicQueue2<int32_t, QUEUE_SIZE, true, true, false, false> _atomic_queue22;

  atomic_queue::AtomicQueue<int32_t, QUEUE_SIZE, 0, true, true, false, true> _spsc_atomic_queue1;
  atomic_queue::AtomicQueue<int32_t, QUEUE_SIZE, 0, true, true, false, true> _spsc_atomic_queue2;

  atomic_queue::AtomicQueue2<int32_t, QUEUE_SIZE, true, true, false, true> _spsc_atomic_queue21;
  atomic_queue::AtomicQueue2<int32_t, QUEUE_SIZE, true, true, false, true> _spsc_atomic_queue22;

  xenium::ramalhete_queue<int32_t, xenium::policy::reclaimer<xenium::reclamation::generic_epoch_based<>>, xenium::policy::entries_per_node<QUEUE_SIZE>> _ramalhete_queue1;
  xenium::ramalhete_queue<int32_t, xenium::policy::reclaimer<xenium::reclamation::generic_epoch_based<>>, xenium::policy::entries_per_node<QUEUE_SIZE>> _ramalhete_queue2;

  moodycamel::ConcurrentQueue<int32_t> _moodycamel_queue1;
  moodycamel::ConcurrentQueue<int32_t> _moodycamel_queue2;

  moodycamel::ReaderWriterQueue<int32_t> _spsc_moodycamel_queue1;
  moodycamel::ReaderWriterQueue<int32_t> _spsc_moodycamel_queue2;

  moodycamel::BlockingReaderWriterCircularBuffer<int32_t> _bdd_spsc_moodycamel_queue1;
  moodycamel::BlockingReaderWriterCircularBuffer<int32_t> _bdd_spsc_moodycamel_queue2;

  boost::lockfree::queue<int32_t, boost::lockfree::capacity<QUEUE_SIZE>> _boost_queue1;
  boost::lockfree::queue<int32_t, boost::lockfree::capacity<QUEUE_SIZE>> _boost_queue2;

  boost::lockfree::spsc_queue<int32_t, boost::lockfree::capacity<QUEUE_SIZE>> _spsc_boost_queue1;
  boost::lockfree::spsc_queue<int32_t, boost::lockfree::capacity<QUEUE_SIZE>> _spsc_boost_queue2;
};

int main()
{
  std::ofstream file("benchmarks/data/comparison/latency_data.csv", std::ios::trunc);
  std::println(file, "Name, Queue Size, Data Type, Num Bounces, Time (us)");
  file.close();

  const int32_t num_values = 100000;
  const size_t num_test_runs = 50;

  for (size_t test_count = 0; test_count < num_test_runs; ++test_count)
  {
    latency_test t;
    t.run_benchmark(num_values);
    std::println("Finished test run {}/{} with {} values.", test_count + 1, num_test_runs, num_values);
  }

  return 0;
}
