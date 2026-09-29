#include "arena.h" // Deklarationerna för Memory::Arena och dess funktioner.
#include <cstring>  // memset används för att nollställa allokerat minne.

void Memory::Initialize(Arena* arena, void* mem_start, size_t size)
{
	arena->base = (unsigned char*)mem_start; // Sparar blockets startadress i byte-format.
	arena->size = size;                      // Sparar den tillgängliga kapaciteten.
	arena->used = 0;                         // Inget minne är ännu utdelat.
}

void* Memory::Allocate(Arena *arena, size_t size)
{
	void* front = arena->base + arena->used; // Adressen där nästa allokering börjar.
	arena->used += size;                      // Flyttar fram arenans lediga position.
	memset(front, 0, size);                   // Ger anroparen nollinitierat minne.
	return front;                             // Returnerar början av det allokerade blocket.
}

void Memory::Reset(Arena *arena)
{
	arena->used = 0; // Gör hela blocket tillgängligt igen utan att frigöra det.
}

Memory::Arena* Memory::CreateSubArena(Arena* parent_arena, size_t size)
{
	Memory::Arena* sub_arena = (Memory::Arena*)Allocate(parent_arena, sizeof (Memory::Arena)); // Struktur som beskriver den nya delarenan.

	void* memory_start = Allocate(parent_arena, size); // Minnesblocket som underarenan får använda.
	Memory::Initialize(sub_arena, memory_start, size); // Initierar underarenans metadata.
	return sub_arena;                                  // Returnerar den färdiga underarenan.
}
