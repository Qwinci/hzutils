#pragma once
#include <stddef.h>
#include <stdint.h>
#include "array.hpp"
#include "allocator.hpp"
#include "rb_tree.hpp"
#include "double_list.hpp"
#include "spinlock.hpp"
#include "pair.hpp"
#include "bit.hpp"
#if __STDC_HOSTED__ == 1
#include <utility>
#include <new>
#else
#include "utility.hpp"
#include "new.hpp"
#endif

namespace hz {
	struct default_slab_config {
		static constexpr hz::array SMALL_SLABS {
			hz::pair {size_t {16}, size_t {0x1000 / 16}},
			hz::pair {size_t {32}, size_t {0x1000 / 32}},
			hz::pair {size_t {64}, size_t {0x1000 / 32}},
			hz::pair {size_t {128}, size_t {50}},
			hz::pair {size_t {256}, size_t {40}},
			hz::pair {size_t {512}, size_t {30}},
			hz::pair {size_t {1024}, size_t {20}},
			hz::pair {size_t {2048}, size_t {10}},
		};

		static constexpr size_t POW2_SLABS_BEGIN = 2048;
		static constexpr size_t POW2_SLABS_END = 1024 * 128;
		static constexpr size_t POW2_ARENA_SIZE = 1024 * 128;
	};

	template<typename T>
	concept slab_verifier = requires(T) {
		T::double_free_or_corruption();
		T::invalid_config("");
	};

	struct slab_trap_verifier {
		static void double_free_or_corruption() {
			__builtin_trap();
		}

		static void invalid_config(const char*) {
			__builtin_trap();
		}
	};

	template<SizedAllocator ArenaAllocator, typename Config = default_slab_config, slab_verifier Verifier = slab_trap_verifier>
	class slab_allocator {
	public:
		constexpr explicit slab_allocator(ArenaAllocator arena_alloc) : arena_alloc {std::move(arena_alloc)} {}

		void* alloc(size_t size) {
			if (!size) {
				size = 1;
			}

			AllocInfo* info;
			MetadataPage* meta_arena;
			{
				auto meta_guard = free_metadatas.lock();
				if (!meta_guard->is_empty()) {
					meta_arena = meta_guard->front();
					++meta_arena->count;
					info = meta_arena->freelist.pop();
					if (meta_arena->count == meta_arena->max) {
						meta_guard->remove(meta_arena);
					}
				}
				else {
					auto* meta_mem = arena_alloc.allocate(0x1000);
					if (!meta_mem) {
						return nullptr;
					}

					meta_arena = new (meta_mem) MetadataPage {};
					meta_arena->max = (0x1000 - sizeof(MetadataPage)) / sizeof(AllocInfo);

					for (size_t i = 0; i < meta_arena->max; ++i) {
						auto* new_info = new (reinterpret_cast<char*>(&meta_arena[1]) + i * sizeof(AllocInfo))
							AllocInfo {};
						meta_arena->freelist.push(new_info);
					}

					++meta_arena->count;
					info = meta_arena->freelist.pop();
					if (meta_arena->count != meta_arena->max) {
						meta_guard->push(meta_arena);
					}
				}
			}

			info->metadata_arena = meta_arena;

			if (size > Config::POW2_SLABS_END) {
				auto* mem = arena_alloc.allocate(size);
				if (!mem) {
					auto guard = free_metadatas.lock();
					meta_arena->freelist.push(info);
					--meta_arena->count;
					return nullptr;
				}

				info->ptr = mem;
				info->size = size;
				allocations.lock()->insert(info);
				return mem;
			}
			else if (size >= Config::POW2_SLABS_BEGIN) {
				auto index = size_to_pow2_index(size);

				auto guard = free_pow2_arenas[index].lock();
				Arena* arena;
				void* mem;
				if (!guard->is_empty()) {
					arena = guard->front();
					++arena->count;
					mem = arena->freelist.pop();
					if (arena->count == arena->max) {
						guard->remove(arena);
					}
				}
				else {
					auto* arena_mem = arena_alloc.allocate(0x1000 + Config::POW2_ARENA_SIZE);
					if (!arena_mem) {
						auto meta_guard = free_metadatas.lock();
						meta_arena->freelist.push(info);
						--meta_arena->count;
						return nullptr;
					}

					auto block_size = pow2_index_to_size(index);

					arena = new (arena_mem) Arena {};
					arena->max = Config::POW2_ARENA_SIZE / block_size;

					for (size_t i = 0; i < arena->max; ++i) {
						auto* hdr = new (static_cast<char*>(arena_mem) + 0x1000 + i * block_size) Header {};
						arena->freelist.push(hdr);
					}

					++arena->count;
					mem = arena->freelist.pop();
					if (arena->count != arena->max) {
						guard->push(arena);
					}
				}

				info->ptr = mem;
				info->size = size;
				info->arena = arena;
				allocations.lock()->insert(info);
				return mem;
			}
			else {
				size_t index = SIZE_MAX;

				for (size_t i = 0; i < Config::SMALL_SLABS.size(); ++i) {
					const auto& slab_info = Config::SMALL_SLABS[i];
					if (slab_info.first >= size) {
						index = i;
						break;
					}
				}

				if (index == SIZE_MAX) {
					Verifier::invalid_config("no small slabs");
					__builtin_trap();
				}

				auto guard = free_small_arenas[index].lock();
				Arena* arena;
				void* mem;
				if (!guard->is_empty()) {
					arena = guard->front();
					++arena->count;
					mem = arena->freelist.pop();
					if (arena->count == arena->max) {
						guard->remove(arena);
					}
				}
				else {
					auto block_size = Config::SMALL_SLABS[index].first;
					auto block_count = Config::SMALL_SLABS[index].second;
					auto total_size = block_size * block_count;
					auto* arena_mem = arena_alloc.allocate(0x1000 + total_size);
					if (!arena_mem) {
						auto meta_guard = free_metadatas.lock();
						meta_arena->freelist.push(info);
						--meta_arena->count;
						return nullptr;
					}

					arena = new (arena_mem) Arena {};
					arena->max = block_count;

					for (size_t i = 0; i < arena->max; ++i) {
						auto* hdr = new (static_cast<char*>(arena_mem) + 0x1000 + i * block_size) Header {};
						arena->freelist.push(hdr);
					}

					++arena->count;
					mem = arena->freelist.pop();
					if (arena->count != arena->max) {
						guard->push(arena);
					}
				}

				info->ptr = mem;
				info->size = size;
				info->arena = arena;
				allocations.lock()->insert(info);
				return mem;
			}
		}

		size_t get_size_for_allocation(void* ptr) {
			auto guard = allocations.lock();

			auto* info = guard->template find<void*, &AllocInfo::ptr>(ptr);
			if (!info) {
				Verifier::double_free_or_corruption();
			}

			return info->size;
		}

		void free(void* ptr) {
			if (!ptr) {
				return;
			}

			AllocInfo* info;
			{
				auto guard = allocations.lock();

				info = guard->template find<void*, &AllocInfo::ptr>(ptr);
				if (!info) {
					Verifier::double_free_or_corruption();
					return;
				}

				guard->remove(info);
			}

			if (info->size > Config::POW2_SLABS_END) {
				arena_alloc.deallocate(info->ptr, info->size);
			}
			else if (info->size >= Config::POW2_SLABS_BEGIN) {
				auto index = size_to_pow2_index(info->size);
				auto guard = free_pow2_arenas[index].lock();

				auto* arena = info->arena;
				if (arena->count == 1) {
					if (arena->count != arena->max) {
						guard->remove(arena);
					}
					arena_alloc.deallocate(arena, 0x1000 + Config::POW2_ARENA_SIZE);
				}
				else {
					--arena->count;
					if (arena->count == arena->max - 1) {
						guard->push(arena);
					}
					arena->freelist.push(new (ptr) Header {});
				}
			}
			else {
				size_t index = SIZE_MAX;

				for (size_t i = 0; i < Config::SMALL_SLABS.size(); ++i) {
					const auto& slab_info = Config::SMALL_SLABS[i];
					if (slab_info.first >= info->size) {
						index = i;
						break;
					}
				}

				if (index == SIZE_MAX) {
					Verifier::invalid_config("no small slabs");
					__builtin_trap();
				}

				auto guard = free_small_arenas[index].lock();
				auto* arena = info->arena;
				if (arena->count == 1) {
					if (arena->count != arena->max) {
						guard->remove(arena);
					}
					auto block_size = Config::SMALL_SLABS[index].first;
					auto block_count = Config::SMALL_SLABS[index].second;
					auto total_size = block_size * block_count;
					arena_alloc.deallocate(arena, 0x1000 + total_size);
				}
				else {
					--arena->count;
					if (arena->count == arena->max - 1) {
						guard->push(arena);
					}
					arena->freelist.push(new (ptr) Header {});
				}
			}

			auto guard = free_metadatas.lock();
			auto* arena = info->metadata_arena;
			if (arena->count == 1) {
				if (arena->count != arena->max) {
					guard->remove(arena);
				}
				arena_alloc.deallocate(arena, 0x1000);
			}
			else {
				--arena->count;
				if (arena->count == arena->max - 1) {
					guard->push(arena);
				}
				arena->freelist.push(info);
			}
		}

	private:
		struct Header {
			list_hook hook;
		};

		struct Arena {
			list_hook hook;
			list<Header, &Header::hook> freelist;
			size_t max {};
			size_t count {};
		};

		struct MetadataPage;

		struct AllocInfo {
			union {
				rb_tree_hook tree_hook {};
				list_hook freelist_hook;
			};
			void* ptr;
			size_t size;
			Arena* arena;
			MetadataPage* metadata_arena;

			constexpr bool operator==(const AllocInfo& other) const {
				return ptr == other.ptr;
			}

			constexpr bool operator>(const AllocInfo& other) const {
				return ptr > other.ptr;
			}

			constexpr bool operator<(const AllocInfo& other) const {
				return ptr < other.ptr;
			}
		};

		struct MetadataPage {
			list_hook hook;
			list<AllocInfo, &AllocInfo::freelist_hook> freelist;
			size_t max {};
			size_t count {};
		};

		static constexpr size_t size_to_pow2_index(size_t size) {
			if (size <= Config::POW2_SLABS_BEGIN) {
				return 0;
			}
			return bit_width(size - 1) - bit_width(Config::POW2_SLABS_BEGIN - 1);
		}

		static constexpr size_t pow2_index_to_size(size_t index) {
			return Config::POW2_SLABS_BEGIN << index;
		}

		static_assert((Config::SMALL_SLABS.size() && Config::SMALL_SLABS[0].first >= sizeof(Header)) ||
			!Config::SMALL_SLABS.size());

		ArenaAllocator arena_alloc;
		spinlock<rb_tree<AllocInfo, &AllocInfo::tree_hook>> allocations {};
		spinlock<list<Arena, &Arena::hook>> free_small_arenas[Config::SMALL_SLABS.size()] {};
		spinlock<list<Arena, &Arena::hook>> free_pow2_arenas[
			popcount(Config::POW2_SLABS_END - Config::POW2_SLABS_BEGIN) + 1] {};
		spinlock<list<MetadataPage, &MetadataPage::hook>> free_metadatas {};
	};
}
