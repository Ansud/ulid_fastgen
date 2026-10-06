#include <iostream>
#include <sstream>
#include <iomanip>
#include <stdexcept>
#include <ulid_generator.h>
#include <ulid_generator.hpp>

#include "../../include/ulid_generator.h"

#define GEN_COUNT	64
#define BATCH_COUNT 256

void test_monotonic_increase_batch()
{
	std::cout << "Test monotonic increase batch." << std::endl;
	std::vector<Ulid> result;
	ulid_t *ulids = (ulid_t *)malloc(sizeof(ulid_t) * GEN_COUNT);

	if (!ulids) {
		throw std::runtime_error("memory allocation failed :0");
	}

	for (int i = 0; i < BATCH_COUNT; i++) {
		if (ulid_generate(ulids, 0, GEN_COUNT) != GEN_COUNT) {
			throw std::runtime_error("ulid batch generation failed");
		}
		for (int j = 0; j < GEN_COUNT; j++) {
			result.emplace_back(Ulid(ulids[j]));
		}
	}

	Ulid previous = *result.begin();

	for (auto current = result.begin() + 1; current != result.end(); current++) {
		if (previous >= *current) {
			throw std::runtime_error("previous value is greater than value");
		}
	}
}

void test_monotonic_increase()
{
	std::cout << "Test monotonic increase." << std::endl;
	std::vector<Ulid> result;

	for (int i = 0; i < BATCH_COUNT; i++) {
		auto new_vector = Ulid::generate_batch(GEN_COUNT);
		result.insert(result.end(), new_vector.begin(), new_vector.end());
	}

	Ulid previous = *result.begin();

	for (auto current = result.begin() + 1; current != result.end(); current++) {
		if (previous >= *current) {
			throw std::runtime_error("previous value is greater than value");
		}
	}
}

void test_node_passed()
{
	std::cout << "Test node passing to generator." << std::endl;

	for (int i = 0; i <= 0xFFFF; i++) {
		uint16_t node = (uint16_t)i;
		auto current  = Ulid(node);

		if (current.get_node() != node) {
			throw std::runtime_error("node passing failed");
		}
	}
}

int main(int argc, char *argv[])
{
	test_monotonic_increase();
	test_monotonic_increase_batch();
	test_node_passed();
	std::cout << "Tests completed successfully." << std::endl;
}