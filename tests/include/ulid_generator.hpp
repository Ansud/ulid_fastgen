#ifndef UUIDLIKE_FASTGEN_ULID_GENERATOR_HPP
#define UUIDLIKE_FASTGEN_ULID_GENERATOR_HPP

// This is C++ wrapper of the generator, i used it for tests only NOW.
#ifdef __cplusplus
#include <stdint.h>
#include <sstream>
#include <iomanip>
#include <string>
#include <stdexcept>
#include <vector>
#include <cstring>

#include <ulid_generator.h>

class Ulid
{
  public:
	explicit Ulid()
	{
		_ulid = {0};
	}

	explicit Ulid(uint16_t node = 0)
	{
		if (ulid_generate_one(&_ulid, node) != 1) {
			throw std::runtime_error("ulid_generate_one unexpectedly failed");
		}
	}

	explicit Ulid(ulid_t &existing)
	{
		_ulid = existing;
	}

	Ulid(const Ulid &other)			   = default;
	Ulid &operator=(const Ulid &other) = default;
	Ulid(Ulid &&other)				   = default;
	Ulid &operator=(Ulid &&other)	   = default;

	~Ulid()							   = default;

	std::string to_string() const
	{
		std::stringstream stream;

		uint8_t *raw = (uint8_t *)&_ulid;

		for (int i = 0; i < 16; i++) {
			stream << std::setfill('0') << std::setw(2) << std::right << std::hex << static_cast<int>(raw[i]);
		}

		return stream.str();
	}

	bool operator==(const Ulid &other) const
	{
		return std::memcmp(&_ulid, &other._ulid, sizeof(_ulid)) == 0;
	}

	bool operator!=(const Ulid &other) const
	{
		return std::memcmp(&_ulid, &other._ulid, sizeof(_ulid)) != 0;
	}

	bool operator<(const Ulid &other) const
	{
		return std::memcmp(&_ulid, &other._ulid, sizeof(_ulid)) < 0;
	}

	bool operator>(const Ulid &other) const
	{
		return std::memcmp(&_ulid, &other._ulid, sizeof(_ulid)) > 0;
	}

	bool operator<=(const Ulid &other) const
	{
		return std::memcmp(&_ulid, &other._ulid, sizeof(_ulid)) <= 0;
	}

	bool operator>=(const Ulid &other) const
	{
		return std::memcmp(&_ulid, &other._ulid, sizeof(_ulid)) >= 0;
	}

	uint16_t get_node()
	{
		// The raw values is swapped in case of endianess
		uint16_t node = _ulid.b >> 48;
		return ((node & 0xFF) << 8) | (node >> 8);
	}

	static std::vector<Ulid> generate_batch(uint16_t count, uint16_t node = 0)
	{
		// Not a very clever way, we have batch generator. But it produces raw ulids, not objects.
		// And i'm too lazy to copy from one buffer to other
		std::vector<Ulid> batch;

		for (int i = 0; i < count; i++) {
			try {
				batch.emplace_back(Ulid(node));
			} catch (...) {
				return std::vector<Ulid>();
			}
		}

		return batch;
	}

  private:
	ulid_t _ulid;
};

#endif // __cplusplus
#endif // UUIDLIKE_FASTGEN_ULID_GENERATOR_HPP