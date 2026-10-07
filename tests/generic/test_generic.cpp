#include <iostream>
#include <sstream>
#include <iomanip>
#include <stdexcept>

#include <gtest/gtest.h>

#include <ulid_generator.h>
#include <ulid_generator.hpp>

#define GEN_COUNT	64
#define BATCH_COUNT 256

TEST(GenericTests, MonotonicIncreaseBatch)
{
	std::vector<Ulid> result;
	ulid_t ulids[GEN_COUNT];

	for (int i = 0; i < BATCH_COUNT; i++) {
		EXPECT_TRUE(ulid_generate_simple(ulids, GEN_COUNT) == GEN_COUNT);

		for (int j = 0; j < GEN_COUNT; j++) {
			result.emplace_back(Ulid(ulids[j]));
		}
	}

	Ulid previous	 = *result.begin();
	int found_zeroes = 0;

	for (auto current = result.begin() + 1; current != result.end(); current++) {
		EXPECT_TRUE(previous < *current);

		if (current->get_node() == 0 && current->get_shard() == 0) {
			found_zeroes++;
		}
	}

	// I think this variable is enough to prevent test be flaky (in reality no one or one may be zero)
	EXPECT_TRUE(found_zeroes < 3);
}

TEST(GenericTests, MonotonicIncrease)
{
	std::vector<Ulid> result;

	for (int i = 0; i < BATCH_COUNT; i++) {
		auto new_vector = Ulid::generate_batch(GEN_COUNT);
		EXPECT_TRUE(new_vector.size() == GEN_COUNT);
		result.insert(result.end(), new_vector.begin(), new_vector.end());
	}

	Ulid previous = *result.begin();

	for (auto current = result.begin() + 1; current != result.end(); current++) {
		EXPECT_TRUE(previous < *current);
	}
}

TEST(GenericTests, NodePassed)
{
	std::cout << "Test node passing to generator." << std::endl;

	for (int i = 0; i <= 0xFFFF; i++) {
		uint16_t node = (uint16_t)i;
		auto current  = Ulid(node, 0);

		EXPECT_TRUE(current.get_node() == node);
	}
}

TEST(GenericTests, ShardPassed)
{
	std::cout << "Test shard passing to generator." << std::endl;

	for (int i = 0; i <= 0xFF; i++) {
		uint8_t shard = (uint8_t)i;
		auto current  = Ulid(0, shard);

		EXPECT_TRUE(current.get_shard() == shard);
	}
}
