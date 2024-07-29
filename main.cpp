#include <gtest/gtest.h>
#include <hz/string_view.hpp>
#include <hz/manually_destroy.hpp>
#include <hz/manually_init.hpp>
#include <hz/double_list.hpp>
#include <hz/spinlock.hpp>
#include <hz/atomic.hpp>
#include <hz/bit.hpp>
#include <hz/vector.hpp>
#include <hz/optional.hpp>
#include <hz/string.hpp>
#include <hz/algorithm.hpp>
#include <hz/result.hpp>
#include <hz/unordered_map.hpp>
#include <hz/string_utils.hpp>
#include <hz/rb_tree.hpp>
#include <hz/slab.hpp>
#include <compare>

TEST(Basic, StringView) {
	hz::string_view hello {"hello"};
	hz::string_view foo {"foo"};
	EXPECT_EQ(hello.size(), 5);
	EXPECT_EQ(hello, hello);
	EXPECT_NE(hello, foo);
	EXPECT_EQ(hello.find('o'), 4);
	EXPECT_EQ(hello.find(' '), hz::string_view::npos);
	EXPECT_EQ(hello.find('o', 5), hz::string_view::npos);
	EXPECT_EQ(hello.find("helloworld"), hz::string_view::npos);
	EXPECT_EQ(hello.find("hello"), 0);
	EXPECT_EQ(hello.find("hello", 1), hz::string_view::npos);
	EXPECT_EQ(hello.find("o"), 4);
	EXPECT_EQ(hello.find("lo", 3), 3);
	EXPECT_EQ(hello.find("a", 4), hz::string_view::npos);
	EXPECT_EQ(hello.find_first_of("el"), 1);
	EXPECT_EQ(hello.find_last_of("o"), 4);
	EXPECT_EQ(hello.find_last_of("l"), 3);
	EXPECT_EQ(hello.substr(0, 2), "he");
	EXPECT_EQ(hello.substr(2), "llo");
	EXPECT_EQ(hello.substr(5).size(), 0);
	EXPECT_EQ(hello.substr(5, 1).size(), 0);
	EXPECT_EQ(hello.starts_with("hel"), true);
	EXPECT_EQ(hello.starts_with("hello"), true);
	EXPECT_EQ(hello.starts_with("foo"), false);
	EXPECT_EQ(hello.ends_with("llo"), true);
	EXPECT_EQ(hello.ends_with("hello"), true);
	EXPECT_EQ(hello.ends_with("foo"), false);
}

static bool TMP = false;

TEST(Basic, ManuallyDestroy) {
	struct A {
		A() {
			TMP = true;
		}
		~A() {
			TMP = true;
		}
	};

	{
		TMP = false;
		hz::manually_destroy<A> a {};
		EXPECT_EQ(TMP, true);
		TMP = false;
		a.destroy();
		EXPECT_EQ(TMP, true);
		TMP = false;
	}
	EXPECT_EQ(TMP, false);

	{
		TMP = false;
		hz::manually_init<A> a {};
		EXPECT_EQ(TMP, false);
		a.initialize();
		EXPECT_EQ(TMP, true);
		TMP = false;
		a.destroy();
		EXPECT_EQ(TMP, true);
		TMP = false;
	}
	EXPECT_EQ(TMP, false);
}

TEST(Basic, DoubleList) {
	struct A {
		hz::list_hook hook {};
	};

	hz::list<A, &A::hook> list {};

	A a {};
	A b {};
	A c {};

	list.push(&a);
	EXPECT_EQ(list.front(), &a);
	EXPECT_EQ(list.back(), &a);
	list.push_front(&b);
	EXPECT_EQ(list.front(), &b);
	EXPECT_EQ(list.back(), &a);
	list.push(&c);
	EXPECT_EQ(list.back(), &c);

	auto iter = list.begin();
	EXPECT_EQ(&*iter, &b);
	EXPECT_EQ(list.pop_front(), &b);
	++iter;
	EXPECT_EQ(&*iter, &a);
	list.remove(&*iter);
	++iter;
	EXPECT_EQ(&*iter, &c);
	EXPECT_EQ(list.pop(), &c);
	++iter;
	EXPECT_EQ(iter != list.end(), false);

	EXPECT_EQ(list.is_empty(), true);
	EXPECT_EQ(list.front(), nullptr);
	EXPECT_EQ(list.back(), nullptr);
}

TEST(Basic, Spinlock) {
	hz::spinlock<int> lock {1};
	{
		auto guard = lock.lock();
		EXPECT_EQ(*guard, 1);
		++*guard;
		EXPECT_EQ(*guard, 2);
	}
	{
		auto guard = lock.lock();
		EXPECT_EQ(*guard, 2);
		EXPECT_EQ(lock.get_unsafe(), 2);
	}
}

TEST(Basic, Bit) {
	EXPECT_EQ(hz::byteswap(uint16_t {0xCAFE}), 0xFECA);
	EXPECT_EQ(hz::byteswap(uint32_t {0xCAFEBABE}), 0xBEBAFECA);
	EXPECT_EQ(hz::byteswap(uint64_t {0xCAFEBABEDEADBEEF}), 0xEFBEADDEBEBAFECA);

	EXPECT_EQ(hz::has_single_bit(0), false);
	EXPECT_EQ(hz::has_single_bit(1), true);
	EXPECT_EQ(hz::has_single_bit(2), true);
	EXPECT_EQ(hz::has_single_bit(3), false);
	EXPECT_EQ(hz::has_single_bit(4), true);

	EXPECT_EQ(hz::countl_zero(uint8_t {0}), 8);
	EXPECT_EQ(hz::countl_zero(uint16_t {0}), 16);
	EXPECT_EQ(hz::countl_zero(uint32_t {0}), 32);
	EXPECT_EQ(hz::countl_zero(uint64_t {0}), 64);
	EXPECT_EQ(hz::countl_zero(uint8_t {0b1}), 7);
	EXPECT_EQ(hz::countl_zero(uint8_t {0b10}), 6);

	EXPECT_EQ(hz::countl_one(uint8_t {0}), 0);
	EXPECT_EQ(hz::countl_one(uint16_t {0}), 0);
	EXPECT_EQ(hz::countl_one(uint32_t {0}), 0);
	EXPECT_EQ(hz::countl_one(uint64_t {0}), 0);
	EXPECT_EQ(hz::countl_one(uint8_t {1 << 7}), 1);
	EXPECT_EQ(hz::countl_one(uint8_t {1 << 7 | 1 << 6}), 2);

	EXPECT_EQ(hz::countr_zero(uint8_t {0}), 8);
	EXPECT_EQ(hz::countr_zero(uint16_t {0}), 16);
	EXPECT_EQ(hz::countr_zero(uint32_t {0}), 32);
	EXPECT_EQ(hz::countr_zero(uint64_t {0}), 64);
	EXPECT_EQ(hz::countr_zero(uint8_t {0b1}), 0);
	EXPECT_EQ(hz::countr_zero(uint8_t {0b10}), 1);

	EXPECT_EQ(hz::countr_one(uint8_t {0}), 0);
	EXPECT_EQ(hz::countr_one(uint16_t {0}), 0);
	EXPECT_EQ(hz::countr_one(uint32_t {0}), 0);
	EXPECT_EQ(hz::countr_one(uint64_t {0}), 0);
	EXPECT_EQ(hz::countr_one(uint8_t {1}), 1);
	EXPECT_EQ(hz::countr_one(uint8_t {1 << 1 | 1 << 0}), 2);

	EXPECT_EQ(hz::bit_width(uint8_t {0}), 0);
	EXPECT_EQ(hz::bit_width(uint16_t {0}), 0);
	EXPECT_EQ(hz::bit_width(uint32_t {0}), 0);
	EXPECT_EQ(hz::bit_width(uint64_t {0}), 0);
	EXPECT_EQ(hz::bit_width(uint8_t {0b1}), 1);
	EXPECT_EQ(hz::bit_width(uint8_t {0b11}), 2);
	EXPECT_EQ(hz::bit_width(uint8_t {0b111}), 3);
	EXPECT_EQ(hz::bit_width(uint8_t {0b1000}), 4);

	EXPECT_EQ(hz::bit_ceil(1), 1);
	EXPECT_EQ(hz::bit_ceil(2), 2);
	EXPECT_EQ(hz::bit_ceil(3), 4);
	EXPECT_EQ(hz::bit_ceil(4), 4);
	EXPECT_EQ(hz::bit_ceil(5), 8);

	EXPECT_EQ(hz::bit_floor(1), 1);
	EXPECT_EQ(hz::bit_floor(2), 2);
	EXPECT_EQ(hz::bit_floor(3), 2);
	EXPECT_EQ(hz::bit_floor(4), 4);
	EXPECT_EQ(hz::bit_floor(5), 4);

	EXPECT_EQ(hz::popcount(0), 0);
	EXPECT_EQ(hz::popcount(0b1), 1);
	EXPECT_EQ(hz::popcount(0b101), 2);
	EXPECT_EQ(hz::popcount(0b1011), 3);

	float value = 1.0f;
	EXPECT_EQ(hz::bit_cast<uint32_t>(value), 0x3F800000);

	EXPECT_EQ(hz::rotr(uint8_t {0b11101}, 0), 0b11101);
	EXPECT_EQ(hz::rotr(uint8_t {0b11101}, 1), 0b10001110);
	EXPECT_EQ(hz::rotr(uint8_t {0b11101}, 9), 0b10001110);
	EXPECT_EQ(hz::rotr(uint8_t {0b11101}, -1), 0b111010);

	EXPECT_EQ(hz::rotl(uint8_t {0b11101}, 0), 0b11101);
	EXPECT_EQ(hz::rotl(uint8_t {0b11101}, 1), 0b111010);
	EXPECT_EQ(hz::rotl(uint8_t {0b11101}, 9), 0b111010);
	EXPECT_EQ(hz::rotl(uint8_t {0b11101}, -1), 0b10001110);
}

struct Allocator {
	static void* allocate(size_t size) {
		return malloc(size);
	}

	static void deallocate(void* ptr) {
		return free(ptr);
	}
};

TEST(Basic, Vector) {
	struct InstanceAllocator {
		void* allocate(size_t size) {
			return malloc(size);
		}

		void deallocate(void* ptr) {
			return free(ptr);
		}
	};

	hz::vector<int, Allocator> vec {Allocator {}};
	hz::vector<int, InstanceAllocator> vec2 {InstanceAllocator {}};

	EXPECT_EQ(vec.size(), 0);
	EXPECT_EQ(vec.capacity(), 0);
	vec.push_back(1);
	EXPECT_EQ(vec.size(), 1);
	EXPECT_GE(vec.capacity(), 1);
	EXPECT_EQ(vec[0], 1);
	vec.emplace_back(10);
	EXPECT_EQ(vec.size(), 2);
	EXPECT_EQ(vec[1], 10);
	vec.clear();
	EXPECT_EQ(vec.size(), 0);
	EXPECT_GE(vec.capacity(), 2);
	vec.shrink_to_fit();
	EXPECT_EQ(vec.capacity(), 0);

	vec.push_back(0);
	vec.push_back(20);
	EXPECT_EQ(vec.front(), 0);
	EXPECT_EQ(vec.back(), 20);
	vec.push_back(30);
	vec.pop_back();
	EXPECT_EQ(vec.back(), 20);
	vec[1] = 0;

	for (auto& item : vec) {
		EXPECT_EQ(item, 0);
	}
}

TEST(Basic, Optional) {
	hz::optional<int> a;
	EXPECT_EQ(a.has_value(), false);
	a = 10;
	EXPECT_EQ(a.has_value(), true);
	EXPECT_EQ(a.value(), 10);
	a.reset();
	EXPECT_EQ(a.has_value(), false);
}

TEST(Basic, String) {
	hz::string<Allocator> str {Allocator {}};
	EXPECT_EQ(str.size(), 0);
	str += "hello";
	EXPECT_EQ(str.size(), 5);
	EXPECT_EQ(strcmp(str.data(), "hello"), 0);
	str.resize(32, ' ');
	EXPECT_EQ(str.size(), 32);
	EXPECT_EQ(str[5], ' ');
	EXPECT_EQ(str[31], ' ');
	EXPECT_EQ(str[32], 0);
	str.clear();
	EXPECT_EQ(str.size(), 0);
	EXPECT_EQ(str[0], 0);
}

TEST(Basic, Algorithm) {
	EXPECT_EQ(hz::min(1, 2), 1);
	EXPECT_EQ(hz::min(2, 1, [](int a, int b) {
		return a < b;
	}), 1);

	EXPECT_EQ(hz::min({5, 8, 1, 4}), 1);
	EXPECT_EQ(hz::min({5, 8, 1, 4}, [](int a, int b) {
		return a < b;
	}), 1);

	EXPECT_EQ(hz::max(1, 2), 2);
	EXPECT_EQ(hz::max(2, 1, [](int a, int b) {
		return a < b;
	}), 2);

	EXPECT_EQ(hz::max({5, 8, 1, 4}), 8);
	EXPECT_EQ(hz::max({5, 8, 1, 4}, [](int a, int b) {
		return a < b;
	}), 8);
}

TEST(Basic, Result) {
	hz::result<int, int> res;
	EXPECT_EQ(res.has_error(), true);
	EXPECT_EQ(res.error(), 0);
	res = hz::error(20);
	EXPECT_EQ(res.has_error(), true);
	EXPECT_EQ(res.error(), 20);
	res = hz::success(30);
	EXPECT_EQ(res.has_value(), true);
	EXPECT_EQ(res.value(), 30);

	hz::result<int, int> res2 = res;
	EXPECT_EQ(res2.has_value(), true);
	EXPECT_EQ(res2.value(), 30);
}

using namespace hz::literals;

TEST(Basic, Map) {
	hz::unordered_map<int, int, Allocator> table {Allocator {}};
	EXPECT_EQ(table.get(0), nullptr);
	table.insert(1, 1);
	EXPECT_NE(table.get(1), nullptr);
	EXPECT_EQ(*table.get(1), 1);
	table.insert(2, 2);
	table.insert(3, 3);
	table.remove(1);
	EXPECT_EQ(table.get(1), nullptr);
	EXPECT_NE(table.get(2), nullptr);
	EXPECT_NE(table.get(3), nullptr);

	hz::unordered_map<hz::string<Allocator>, int, Allocator> table2 {Allocator {}};
	hz::string<Allocator> key {Allocator {}};
	const auto& b = key;
	b.begin();
	key = "hello";
	table2.insert(key, 1);
	EXPECT_NE(table2.get("hello"_view), nullptr);
	EXPECT_EQ(table2.get("helloworld"_view), nullptr);
	EXPECT_EQ(*table2.get("hello"_view), 1);
}

TEST(Basic, StringUtils) {
	size_t count;
	EXPECT_EQ(hz::to_integer<int>("1234"_view, 10, &count), 1234);
	EXPECT_EQ(count, 4);
	EXPECT_EQ(hz::to_integer<int>("123a"_view, 10, &count), 123);
	EXPECT_EQ(count, 3);
}

TEST(Basic, RbTree) {
	struct Node {
		hz::rb_tree_hook hook;
		int key;

		constexpr std::strong_ordering operator<=>(const Node& other) const {
			return key <=> other.key;
		}

		constexpr bool operator==(const Node& other) const {
			return key == other.key;
		}
	};

	hz::rb_tree<Node, &Node::hook> tree;
	EXPECT_EQ((tree.find<int, &Node::key>(1)), nullptr);
	Node a {
		.hook {},
		.key = 1
	};
	Node b {
		.hook {},
		.key = 2
	};
	Node c {
		.hook {},
		.key = 100
	};
	Node d {
		.hook {},
		.key = 4
	};
	EXPECT_EQ(tree.insert(&a), true);
	EXPECT_EQ(tree.insert(&a), false);
	EXPECT_EQ(tree.insert(&b), true);
	EXPECT_EQ(tree.insert(&c), true);
	EXPECT_EQ(tree.insert(&d), true);
	EXPECT_EQ((tree.find<int, &Node::key>(1)), &a);
	tree.remove(&a);
	EXPECT_EQ((tree.find<int, &Node::key>(1)), nullptr);
	EXPECT_EQ((tree.find<int, &Node::key>(2)), &b);
	EXPECT_EQ((tree.find<int, &Node::key>(4)), &d);
	EXPECT_EQ((tree.find<int, &Node::key>(100)), &c);
	tree.remove(&d);
	EXPECT_EQ((tree.find<int, &Node::key>(4)), nullptr);
	EXPECT_EQ((tree.find<int, &Node::key>(2)), &b);
	EXPECT_EQ((tree.find<int, &Node::key>(100)), &c);
}

TEST(Basic, Slab) {
	struct ArenaAllocator {
		static void* allocate(size_t size) {
			return malloc(size);
		}

		static void deallocate(void* ptr, size_t) {
			return free(ptr);
		}
	};

	hz::slab_allocator<ArenaAllocator> alloc {ArenaAllocator {}};

	auto ptr = alloc.alloc(1);
	EXPECT_NE(ptr, nullptr);
	memset(ptr, 0xAB, 1);
	EXPECT_EQ(alloc.get_size_for_allocation(ptr), 1);
	auto ptr2 = alloc.alloc(8);
	EXPECT_NE(ptr2, nullptr);
	memset(ptr2, 0xCD, 8);
	EXPECT_EQ(alloc.get_size_for_allocation(ptr2), 8);
	auto ptr3 = alloc.alloc(31);
	EXPECT_NE(ptr3, nullptr);
	memset(ptr3, 0x78, 31);
	EXPECT_EQ(alloc.get_size_for_allocation(ptr3), 31);
	auto ptr4 = alloc.alloc(2048);
	EXPECT_NE(ptr4, nullptr);
	memset(ptr4, 0x56, 2048);
	EXPECT_EQ(alloc.get_size_for_allocation(ptr4), 2048);
	auto ptr5 = alloc.alloc(4097);
	EXPECT_NE(ptr5, nullptr);
	memset(ptr5, 0x32, 4097);
	EXPECT_EQ(alloc.get_size_for_allocation(ptr5), 4097);
	auto ptr6 = alloc.alloc(1024 * 256);
	EXPECT_NE(ptr6, nullptr);
	memset(ptr6, 0x54, 1024 * 256);
	EXPECT_EQ(alloc.get_size_for_allocation(ptr6), 1024 * 256);
	alloc.free(ptr);
	alloc.free(ptr5);
	alloc.free(ptr3);
	alloc.free(ptr2);
	alloc.free(ptr6);
	alloc.free(ptr4);
}


