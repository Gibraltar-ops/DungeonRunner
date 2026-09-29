#pragma once // Hindrar att denna header inkluderas flera gånger i samma kompileringsenhet.

#define ALLOC(arena, type)(type*)Memory::Allocate((arena), sizeof(type)) // En genväg för att boka plats för en sak i minnet. 
#define ALLOC_ARRAY(arena, type, count)(type*)Memory::Allocate((arena), sizeof(type) * count) // En genväg för att boka plats för en hel lista med saker i minnet.

namespace Memory
{
	// En "Arena" fungerar som en stor låda med minne. Istället för att be datorn om minne hela tiden, 
	// så tar vi en stor bit direkt och delar ut små bitar ur den själva. Det är mycket snabbare.
	struct Arena
	{
		unsigned char* base; // Starten på vår minneslåda.
		size_t size;          // Hur stor lådan är totalt (i bytes).
		size_t used;          // Hur mycket av lådan vi har använt hittills.
	};

// Gör i ordning en minneslåda och bestämmer hur stor den ska vara.
void Initialize(Arena* arena, void* memory, size_t size);
// Tar en bit minne ur lådan för att använda till något i spelet.
void* Allocate(Arena* arena, size_t size);
// Tömmer minneslådan så att vi kan börja använda den från början igen.
void Reset(Arena* arena);


// Skapar en mindre minneslåda inuti en större låda.
Arena* CreateSubArena(Arena* parent_arena, size_t size);

}
