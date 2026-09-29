#pragma once

#include "FMOD/fmod_common.h"

namespace Memory
{
	struct Arena;
}

enum class SFX_ID
{
	FALLBACK,
	JUMP,

	COUNT
	
};

enum class SONG_ID
{
	NONE,
	THEME
};

struct SoundDataEntry
{
	SFX_ID id;
	const char* path;
};

struct AudioSystem
{
	bool initialized;
	void* fmod_memory; // Pointer till FMODs memory
	FMOD_SYSTEM* sound_system; // Struct för att initializea, håller all information FMOD behöver 
	static const int CHANNEL_COUNT = 32; // Hur många audio threads vi ger FMOD - vi kan inte ha mer än 32 saker som spelar ljud samtidigt
	FMOD_CHANNEL* channels[CHANNEL_COUNT]; 
	FMOD_SOUND* soundEffects[(int)SFX_ID::COUNT]; // Pointer för faktiskt ljudfilen i vårt minne

	SONG_ID song_id;
	FMOD_SOUND* song;
	FMOD_CHANNEL* song_channel;
};

extern AudioSystem* g_audioSystem; // Global promise 

void PlaySFX(SFX_ID id, float volume = 1);
void InitializeAudioSystem(AudioSystem* audio, Memory::Arena* arena_main);
void UpdateAudio(AudioSystem* audio);

void InitializeAudioSystem(AudioSystem* audio, Memory::Arena *arena_main);

void PlaySong(SONG_ID id);


namespace AssetManagement
{
	void LoadAllSFX(AudioSystem* audioSystem);
}

