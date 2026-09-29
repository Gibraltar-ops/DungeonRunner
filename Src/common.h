#pragma once // Hindrar dubbla inkluderingar av gemensamma konstanter och makron.

#define KILOBYTES(n) ((size_t)n * 1024)       // Omvandlar ett antal KiB till byte.
#define MEGABYTES(n) (KILOBYTES(n) * 1024)    // Omvandlar ett antal MiB till byte.
#define GIGABYTES(n) (MEGABYTES(n) * 1024)    // Omvandlar ett antal GiB till byte.

constexpr size_t GAME_MEMORY_ALLOWANCE = MEGABYTES(15); // Här bestämmer vi hur mycket arbetsminne (RAM) spelet får använda totalt.
constexpr size_t AUDIO_MEMORY_ALLOWANCE = MEGABYTES(5); // Här bestämmer vi hur mycket minne som ska reserveras specifikt för ljud.

constexpr int FPS = 240; // Vi siktar på 240 bilder i sekunden för ett riktigt mjukt flyt.


// 60 FPS = ca 16.66 ms per bildruta
constexpr double FRAME_TIME_MS = 1000.0 / FPS; // Räknar ut exakt hur många millisekunder varje bildruta får ta.

const float UNDO_REPEAT_TIME = 0.15; // Bestämmer hur snabbt man kan "ångra" sina drag om man håller ner knappen.
const float MOVE_SPEED = 6.0;        // Hur fort figurerna glider mellan rutorna på skärmen.

const int SCREEN_WIDTH = 1920;                  // Bredden på spelfönstret i pixlar (Full HD).
const int SCREEN_HEIGHT = 1080;                  // Höjden på spelfönstret i pixlar (Full HD).
const int UPSCALE_FACTOR = 4;                   // Eftersom vi använder pixel-art så förstorar vi upp allt 4 gånger så det syns tydligt.
// const int CELL_SIZE_PX = 16 * UPSCALE_FACTOR;   // Storleken på en spelruta efter att den förstorats.
const int TILE_SIZE_PX_RAW = 16;                // Den ursprungliga storleken på en grafikbit (16x16 pixlar).
const int TILE_SIZE_PX_SCALED = TILE_SIZE_PX_RAW * UPSCALE_FACTOR; // Slutgiltig storlek på en grafikbit när den ritas ut.



inline void Expand1DTo2D(int flatIndex, int width, int* x, int* y) // Gör om ett nummer i en lång lista till en position med X och Y (bredd och höjd).
{
*x = flatIndex % width;
*y = flatIndex / width;
}
inline void Expand1DTo2D(int flatIndex, int width, float* x, float* y) // Samma som ovan, men för decimaltal.
{
*x = (float)(flatIndex % width);
*y = (float)(flatIndex / width);
}


static const char STOP_CHAR = '\0'; // Ett speciellt tecken som markerar slutet på en textsträng.
inline bool IsStringEmpty(const char* str) // En enkel kontroll för att se om en textrad är tom eller saknas helt.
{
    return str == nullptr || str[0] == STOP_CHAR;
}