/* memory.h - v0.1 - Sven A. Schreiber
 *
 * memory.h is a single header file library for allocation and
 * management of memory. It is part of and depends on my C base-layer.
 * 
 * To use this file simply define MEMORY_IMPL once at the start of
 * your project before including it. After that you can include it 
 * without defining MEMORY_IMPL as per usual.
 * 
 * Example:
 * ...
 * #define MEMORY_IMPL
 * #include "memory.h"
 * ...
 */

#ifndef MEMORY_H
#define MEMORY_H

// +============+
// | DEFINTIONS |
// +============+

#define MEM_ARENA_MAX GB(2)
#define MEM_ARENA_COMMIT_SIZE KB(4)
#define MEM_ARENA_ALIGN_DEFAULT 8

typedef struct Mem_Arena Mem_Arena;
struct Mem_Arena {
    u64 max;
    u64 alloc_pos;
    u64 commit_pos;
    void *data;
    u64 align;
};


// +===========+
// | INTERFACE |
// +===========+

Mem_Arena mem_arena_init_with_align(u64 align, u64 size);
Mem_Arena mem_arena_init(u64 size);
void *mem_arena_push(Mem_Arena *arena, u64 size);
void *mem_arena_push_zero(Mem_Arena *arena, u64 size);
void mem_arena_pop(Mem_Arena *arena, u64 size);
void mem_arena_release(Mem_Arena *arena);
void mem_arena_clear(Mem_Arena *arena);


// +===============+
// | HELPER MACROS |
// +===============+

#define PushData(arena,T,c) ( (T*)(mem_arena_push((arena),sizeof(T)*(c))) )
#define PushDataZero(arena,T,c) ( (T*)(mem_arena_push_zero((arena),sizeof(T)*(c))) )
#define PushStruct(arena,T) PushData(arena,T,1);
#define PushStructZero(arena, T) PushDataZero(arena, T, 1);


// +================+
// | IMPLEMENTATION |
// +================+

#ifdef MEMORY_IMPL

Mem_Arena mem_arena_init_with_align(u64 align, u64 size) {
    Mem_Arena arena = {0};
    arena.max        = MEM_ARENA_MAX;
    arena.data       = platform_reserve_memory(arena.max);
    arena.alloc_pos  = 0;
    arena.commit_pos = 0;
    arena.align      = align;
    return arena;
}

Mem_Arena mem_arena_init(u64 size) {
    return mem_arena_init_with_align(MEM_ARENA_ALIGN_DEFAULT, size);
}

void *mem_arena_push(Mem_Arena *arena, u64 size) {
    void *mem = 0;
    if (arena->alloc_pos + size > arena->commit_pos) {
        u64 commit_size = size;
        commit_size += MEM_ARENA_COMMIT_SIZE - 1;
        commit_size -= commit_size % MEM_ARENA_COMMIT_SIZE;
        platform_commit_memory((u8 *)arena->data + arena->commit_pos, commit_size);
        arena->commit_pos += commit_size;
    }
    mem = (u8 *)arena->data + arena->alloc_pos;
    u64 pos = arena->alloc_pos + size;
    arena->alloc_pos = (pos + arena->align - 1) & (~(arena->align - 1));
    return mem;
}

void mem_arena_pop(Mem_Arena *arena, u64 size) {
    if (size > arena->alloc_pos) {
        size = arena->alloc_pos;
    }
    arena->alloc_pos -= size;
}

void mem_arena_release(Mem_Arena *arena) {
    platform_release_memory(arena->data);
}

void mem_arena_clear(Mem_Arena *arena) {
    arena->alloc_pos = 0;
}

void *mem_arena_push_zero(Mem_Arena *arena, u64 size) {
    void *mem = mem_arena_push(arena, size);
    memset(mem, 0, size);
    return mem;
}

#endif

#endif

