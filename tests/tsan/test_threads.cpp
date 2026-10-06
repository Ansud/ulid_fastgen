#include <iostream>
#include <sstream>
#include <iomanip>
#include <future>
#include <thread>
#include <unordered_set>
#include <stdexcept>

#include <gtest/gtest.h>

#include <ulid_generator.h>
#include <ulid_generator.hpp>

#include "../../include/ulid_generator.h"

#define GEN_COUNT  8
#define ITERATIONS 16384

typedef struct _ulid_test_t : ulid_t {
	bool operator==(const _ulid_test_t &other) const
	{
		return (a == other.a) && (b == other.b);
	}
} ulid_test_t;

template <> struct std::hash<ulid_test_t> {
	const size_t operator()(const ulid_test_t &item) const
	{
		return std::hash<uint64_t>()(item.a) ^ std::hash<uint64_t>()(item.b);
	}
};

void test_task(uint8_t shard, std::promise<std::unordered_set<ulid_test_t>> &&result)
{
	std::unordered_set<ulid_test_t> output_set;
	ulid_test_t output[GEN_COUNT];

	for (int i = 0; i < ITERATIONS; i++) {
		if (ulid_generate_unbounced(output, GEN_COUNT, shard) != GEN_COUNT) {
			// No asserts in thread, report as empty set
			result.set_value(std::unordered_set<ulid_test_t>());
			return;
		}

		for (int j = 0; j < GEN_COUNT; j++) {
			auto insert_result = output_set.insert(output[j]);

			if (!insert_result.second) {
				// No asserts in thread, report as empty set
				result.set_value(std::unordered_set<ulid_test_t>());
				return;
			}
		}
	}

	result.set_value(output_set);
}

static void _execute_treading_test(bool multi)
{
	typedef struct _thread_and_promise_t {
		std::future<std::unordered_set<ulid_test_t>> future;
		std::thread thread;
	} tap_t;

	int processor_count = std::thread::hardware_concurrency();
	std::vector<tap_t> threads;

	for (int i = 0; i < processor_count; i++) {
		std::promise<std::unordered_set<ulid_test_t>> p;

		uint8_t shard;

		if (multi) {
			shard = i;
		} else {
			shard = 0;
		}

		threads.emplace_back(tap_t{p.get_future(), std::thread(test_task, shard, std::move(p))});
	}

	std::unordered_set<ulid_test_t> overall;

	for (auto t = threads.begin(); t != threads.end(); ++t) {
		t->thread.join();
		auto future_result = t->future.get();

		ASSERT_TRUE(future_result.size());

		for (auto value : future_result) {
			ASSERT_TRUE(overall.insert(value).second);
		}
	}
}

TEST(ThreadTests, MultipleGenerationsBounced)
{
	std::cout << "Single shard threading." << std::endl;
	_execute_treading_test(false);
}

TEST(ThreadTests, MultipleGenerationsUnbounced)
{
	std::cout << "Multi shard threading." << std::endl;
	_execute_treading_test(true);
}
