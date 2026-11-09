#define HZ_USE_SSE2

#include "hz/rb_tree.hpp"
#include "hz/math.hpp"
#include "hz/bit.hpp"
#include "hz/optional.hpp"
#include "hz/variant.hpp"
#include "hz/result.hpp"
#include "hz/atomic.hpp"

#if HZ_FREESTANDING
#include "hz/limits.hpp"
#endif

#include "hz/string_view.hpp"
#include "hz/fixed_vector.hpp"
#include "hz/ring_buffer.hpp"
#include "hz/span.hpp"
#include "hz/clist.hpp"
#include "hz/hash_impl.hpp"
#include "hz/fnv_hash.hpp"
#include "hz/vector.hpp"

struct Node {
	int key {};
	hz::rb_hook<Node> hook;

	std::strong_ordering operator<=>(const Node& other) const {
		return key <=> other.key;
	}

	bool operator==(const Node& other) const {
		return key == other.key;
	}

	std::strong_ordering operator<=>(int other_key) const {
		return key <=> other_key;
	}
};

#include "hz/flags_enum.hpp"
#include "hz/functional.hpp"
#include "hz/spinlock.hpp"

enum class E {

};

HZ_FLAGS_ENUM(E);

#include "hz/numeric.hpp"
#include "hz/formatting.hpp"
#include "hz/lock_guard.hpp"
#include "hz/fixed_string.hpp"
#include "hz/algorithm.hpp"
#include "hz/time.hpp"
#include "hz/unique_ptr.hpp"
#include "hz/string_view_utf.hpp"
#include "hz/refcount.hpp"
#include "hz/list.hpp"
#include "hz/unordered_map.hpp"
#include "hz/string.hpp"
#include "hz/printf.hpp"
#include "hz/log.hpp"

using namespace hz::literals;

template<hz::formatting::format_sink<char> Sink>
struct printf_impl {
	int handle_format(Sink& sink, hz::printf_info<char>& info, va_list& ap) {
		switch (*info.fmt_ptr) {
			case 'd':
			{
				++info.fmt_ptr;
				int value = va_arg(ap, int);
				return hz::formatting::format_int<char, int, Sink>(sink, value, info.opts);
			}
			default:
				return -1;
		}
	}
};

void test_printf(const char* fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	hz::fixed_string<256> test_sink;
	printf_impl<hz::fixed_string<256>> printf_handler;
	hz::printf_impl(test_sink, printf_handler, fmt, ap);
	va_end(ap);
}

extern "C" int main() {
	test_printf("hello %d", 10);

	hz::fnv_hasher64 hasher;
	hasher.hash(1);
	hasher.hash("abc"_sv);

	hz::unique_ptr<int> ptr_;
	ptr_ = std::move(ptr_);
	*ptr_ = 20;
	ptr_ = nullptr;
	ptr_.reset();

	int testing[4] {1, 2, 3, 4};
	auto val = hz::upper_bound(testing, testing + 4, 2);
	auto val2 = hz::lower_bound(testing, testing + 4, 2);

	for (auto s : hz::string_view {"a,b"}.split(",")) {
		int duh = 20;
	}

	Node a {1};
	Node b {10};
	Node c {5};
	Node d {20};
	hz::rb_tree<Node, &Node::hook> tree;
	tree.insert(&a);
	tree.insert(&b);
	tree.insert(&c);
	tree.insert(&d);

	hz::optional<int> abc {20};
	hz::fixed_vector<int, 20> vec;
	vec.push(20);
	vec.pop();
	vec.resize(2);
	vec.begin();
	vec.end();

	hz::fixed_vector<char, 256> ring_data;
	hz::ring_buffer<char> ring {ring_data.data(), ring_data.capacity()};
	ring.write("hello world!");
	size_t s = ring.size();
	size_t avail = ring.space_available();
	char test[200];
	ring.read(test);

	hz::span<int> g;
	hz::span<const int> h {g};

	void (*ptr)(int a) = [](int a) {};

	hz::function_ref<void()> p;

	hz::small_function<void(int a)> fn;
	fn = ptr;
	fn = [&](int a) {
		if (a == g.size()) {
			return;
		}
	};
	fn(1);

	hz::spinlock<int> locked;
	locked.unlock();
	locked.lock();
	auto ga = hz::guard(locked);
	auto bru = locked.guarded_lock();

	return 0;
}
