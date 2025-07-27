/*
 * main.c - A classic scroller demo for a C/SDL2 project.
 *
 * Cross-compiles on Linux for Windows.
 * Creates a 3D starfield, a color-cycling sine-wave text scroller,
 * a raster bar, and plays background music.
 */

#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

// --- Constants ---
#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define NUM_STARS 500
#define STAR_SPREAD 512

// --- Structs ---
typedef struct {
    float x, y, z;
    float speed;
} Star;

// --- Globals ---
SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
TTF_Font* font = NULL;
Mix_Music* music = NULL;

SDL_Color textColor = { 0, 255, 0, 255 }; // Initial color, will be modulated
Star stars[NUM_STARS];
const char* scrollText = "GREETINGS FROM A C AND SDL2 DEMO... NOW WITH MUSIC, RASTER BARS AND COLOR CYCLING TEXT... ENJOY THE SHOW...";
float scrollX;
float time_counter = 0;


// --- Function Prototypes ---
int init_sdl();
int init_font();
int init_audio();
void init_stars();
void cleanup();
void update_stars();
void render_stars();
void render_raster_bar();
void render_scroller(SDL_Texture* textTexture, int textWidth, int textHeight);
SDL_Texture* create_text_texture(const char* text, int* w, int* h);

// --- Main Function ---
int main(int argc, char* argv[]) {
    // --- Initialization ---
    if (init_sdl() != 0) return 1;
    if (init_font() != 0) return 1;
    if (init_audio() != 0) return 1;

    Mix_PlayMusic(music, -1); // Play music, loop forever

    init_stars();
    scrollX = SCREEN_WIDTH;

    // Create texture from the scroll text
    int textW, textH;
    SDL_Texture* textTexture = create_text_texture(scrollText, &textW, &textH);
    if (!textTexture) {
        cleanup();
        return 1;
    }

    // --- Main Loop ---
    int is_running = 1;
    SDL_Event e;
    Uint32 last_tick = SDL_GetTicks();

    while (is_running) {
        // Event handling
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                is_running = 0;
            }
        }

        // --- Update Game Logic ---
        update_stars();
        scrollX -= 1.5f;
        if (scrollX < -textW) {
            scrollX = SCREEN_WIDTH;
        }
        time_counter += 0.05f;

        // --- Drawing ---
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Black
        SDL_RenderClear(renderer);

        // Draw game objects
        render_stars();
        render_raster_bar();
        render_scroller(textTexture, textW, textH);

        SDL_RenderPresent(renderer);

        // Frame rate limiting
        Uint32 current_tick = SDL_GetTicks();
        if (current_tick - last_tick < 16) {
             SDL_Delay(16 - (current_tick - last_tick));
        }
        last_tick = current_tick;
    }

    // --- Cleanup ---
    SDL_DestroyTexture(textTexture);
    cleanup();
    return 0;
}

// --- Function Implementations ---

// Initialize SDL and create a window/renderer
int init_sdl() {
    // We now initialize AUDIO as well as VIDEO
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }
    window = SDL_CreateWindow("C Scroller Demo", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) {
        printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        printf("Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }
    return 0;
}

// Initialize SDL_ttf and load a font
int init_font() {
    if (TTF_Init() == -1) {
        printf("SDL_ttf could not initialize! TTF_Error: %s\n", TTF_GetError());
        return 1;
    }
    font = TTF_OpenFont("font.ttf", 24);
    if (!font) {
        printf("Failed to load font! TTF_Error: %s\n", TTF_GetError());
        printf("Please ensure 'font.ttf' is in the same directory as the executable.\n");
        SDL_Delay(5000); 
        return 1;
    }
    return 0;
}

// Initialize SDL_mixer and load music
int init_audio() {
    // Open audio with standard settings
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        printf("SDL_mixer could not initialize! Mix_Error: %s\n", Mix_GetError());
        return 1;
    }
    // IMPORTANT: You must provide a path to a music file.
    // This example assumes a file named "music.ogg" is in the same directory.
    music = Mix_LoadMUS("music.ogg");
    if (!music) {
        printf("Failed to load music! Mix_Error: %s\n", Mix_GetError());
        printf("Please ensure 'music.ogg' is in the same directory as the executable.\n");
        SDL_Delay(5000);
        return 1;
    }
    return 0;
}


// Create the text texture to be rendered
SDL_Texture* create_text_texture(const char* text, int* w, int* h) {
    SDL_Surface* textSurface = TTF_RenderText_Blended(font, text, textColor);
    if (!textSurface) {
        printf("Unable to render text surface! TTF_Error: %s\n", TTF_GetError());
        return NULL;
    }
    
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, textSurface);
    if (!texture) {
        printf("Unable to create texture from rendered text! SDL_Error: %s\n", SDL_GetError());
    }
    
    *w = textSurface->w;
    *h = textSurface->h;
    
    SDL_FreeSurface(textSurface);
    return texture;
}


// Initialize star positions randomly
void init_stars() {
    srand(time(NULL));
    for (int i = 0; i < NUM_STARS; i++) {
        stars[i].x = (float)(rand() % STAR_SPREAD) - (STAR_SPREAD / 2);
        stars[i].y = (float)(rand() % STAR_SPREAD) - (STAR_SPREAD / 2);
        stars[i].z = (float)(rand() % STAR_SPREAD);
        stars[i].speed = ((float)(rand() % 100) / 200.0f) + 0.2f;
    }
}

// Update star positions to move them towards the camera
void update_stars() {
    for (int i = 0; i < NUM_STARS; i++) {
        stars[i].z -= stars[i].speed;
        if (stars[i].z <= 0) {
            stars[i].x = (float)(rand() % STAR_SPREAD) - (STAR_SPREAD / 2);
            stars[i].y = (float)(rand() % STAR_SPREAD) - (STAR_SPREAD / 2);
            stars[i].z = STAR_SPREAD;
        }
    }
}

// Render the stars using 2D projection
void render_stars() {
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // White
    for (int i = 0; i < NUM_STARS; i++) {
        if (stars[i].z > 0) {
            float k = 128.0f / stars[i].z;
            int px = (int)(stars[i].x * k + SCREEN_WIDTH / 2);
            int py = (int)(stars[i].y * k + SCREEN_HEIGHT / 2);

            if (px >= 0 && px < SCREEN_WIDTH && py >= 0 && py < SCREEN_HEIGHT) {
                float size = (1.0f - (stars[i].z / STAR_SPREAD)) * 3;
                SDL_Rect r = { px, py, (int)size, (int)size };
                SDL_RenderFillRect(renderer, &r);
            }
        }
    }
}

// Render the moving, color-cycling raster bar
void render_raster_bar() {
    // Enable blending for transparency
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    // Calculate color based on time
    Uint8 r = (Uint8)((sin(time_counter * 0.8f) + 1.0f) / 2.0f * 255);
    Uint8 g = (Uint8)((sin(time_counter * 0.8f + 2.0f) + 1.0f) / 2.0f * 255);
    Uint8 b = (Uint8)((sin(time_counter * 0.8f + 4.0f) + 1.0f) / 2.0f * 255);
    
    SDL_SetRenderDrawColor(renderer, r, g, b, 100); // 100 for alpha

    // Calculate position based on time
    SDL_Rect bar;
    bar.x = 0;
    bar.w = SCREEN_WIDTH;
    bar.h = SCREEN_HEIGHT / 8;
    bar.y = (int)((sin(time_counter) + 1.0f) / 2.0f * (SCREEN_HEIGHT - bar.h));

    SDL_RenderFillRect(renderer, &bar);
    
    // Disable blending
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
}


// Render the scrolling text with a sine wave and color cycling
void render_scroller(SDL_Texture* textTexture, int textWidth, int textHeight) {
    // Calculate color modulation based on time
    Uint8 r = (Uint8)((sin(time_counter) + 1.0f) / 2.0f * 255);
    Uint8 g = (Uint8)((sin(time_counter + 2.0f) + 1.0f) / 2.0f * 255);
    Uint8 b = (Uint8)((sin(time_counter + 4.0f) + 1.0f) / 2.0f * 255);
    SDL_SetTextureColorMod(textTexture, r, g, b);

    // Calculate position with sine wave
    SDL_Rect destRect;
    destRect.x = (int)scrollX;
    destRect.y = (int)((SCREEN_HEIGHT / 2) - (textHeight / 2) + (sin(time_counter * 2.0f) * (SCREEN_HEIGHT / 20)));
    destRect.w = textWidth;
    destRect.h = textHeight;

    SDL_RenderCopy(renderer, textTexture, NULL, &destRect);
}


// Clean up all initialized resources
void cleanup() {
    if (music) Mix_FreeMusic(music);
    if (font) TTF_CloseFont(font);
    if (renderer) SDL_DestroyRenderer(renderer);
    if (window) SDL_DestroyWindow(window);
    
    music = NULL;
    font = NULL;
    renderer = NULL;
    window = NULL;
    
    Mix_Quit();
    TTF_Quit();
    SDL_Quit();
}
