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
	ulid_t *ulids = (ulid_t *)malloc(sizeof(ulid_t) * GEN_COUNT);

	EXPECT_TRUE(ulids != nullptr);

	for (int i = 0; i < BATCH_COUNT; i++) {
		EXPECT_TRUE(ulid_generate(ulids, 0, GEN_COUNT) == GEN_COUNT);

		for (int j = 0; j < GEN_COUNT; j++) {
			result.emplace_back(Ulid(ulids[j]));
		}
	}

	Ulid previous = *result.begin();

	for (auto current = result.begin() + 1; current != result.end(); current++) {
		EXPECT_TRUE(previous < *current);
	}
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
		auto current  = Ulid(node);

		EXPECT_TRUE(current.get_node() == node);
	}
}
