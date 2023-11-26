#include <SDL2/SDL.h>
#include <SDL_blendmode.h>
#include <SDL_events.h>
#include <SDL_render.h>
#include <SDL_video.h>
#include <print.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>

#include <string> // for string class

#include "color.h"
#include "imageloader.h"
#include "raycaster.h"

SDL_Window *window;
SDL_Renderer *renderer;

void clear()
{
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
  SDL_RenderClear(renderer);
}

void draw_floor()
{
  // floor color
  SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);
  SDL_Rect rect = {
      0,
      SCREEN_HEIGHT / 2,
      SCREEN_WIDTH,
      SCREEN_HEIGHT / 2};
  SDL_RenderFillRect(renderer, &rect);
}

void draw_ui()
{
  /* int size = 256; */
  /* ImageLoader::render(renderer, "p", SCREEN_WIDTH/2.0f - size/2.0f, SCREEN_HEIGHT - size, size); */
  ImageLoader::render(renderer, "bg", 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
}

int main()
{
  print("hello world");

  SDL_Init(SDL_INIT_VIDEO);
  ImageLoader::init();
  SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

  if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0)
  {
    print("SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError());
    return -1;
  }

  if (TTF_Init() == -1)
  {
    printf("TTF_Init: %s\n", TTF_GetError());
    return -1;
  }

  TTF_Font *font = TTF_OpenFont("assets/font.ttf", 32);
  if (font == NULL)
  {
    printf("TTF_OpenFont: %s\n", TTF_GetError());
    // handle error
  }

  Uint32 startTicks = SDL_GetTicks();
  int frameCount = 0;
  char fpsText[128];

  // Load sound effect
  Mix_Chunk *walkSound = Mix_LoadWAV("assets/walk.wav");
  if (walkSound == NULL)
  {
    print("Failed to load walk sound effect! SDL_mixer Error: %s\n", Mix_GetError());
    return -1;
  }

  Mix_Chunk *winSound = Mix_LoadWAV("assets/dooropen.mp3");
  if (winSound == NULL)
  {
    print("Failed to load win sound effect! SDL_mixer Error: %s\n", Mix_GetError());
    return -1;
  }

  // Load music
  Mix_Music *music = Mix_LoadMUS("assets/musics.mp3");
  if (music == NULL)
  {
    print("Failed to load music! SDL_mixer Error: %s\n", Mix_GetError());
    return -1;
  }

  window = SDL_CreateWindow("DOOM", 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
  renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

  ImageLoader::loadImage("+", "assets/brick2.png");
  ImageLoader::loadImage("-", "assets/brick.png");
  ImageLoader::loadImage("|", "assets/brick.png");
  ImageLoader::loadImage("*", "assets/metal.png");
  ImageLoader::loadImage("g", "assets/brick_door.png");
  /* ImageLoader::loadImage("p", "assets/player.png"); */
  ImageLoader::loadImage("bg", "assets/background.png");
  ImageLoader::loadImage("e1", "assets/sprite1.png");

  Raycaster r = {renderer};

  bool running = true;
  int speed = 10;

  bool isWalkingSoundPlaying = false;

  SDL_Texture *fpsTexture = NULL;
  SDL_Rect fpsRect = {0, 0, 0, 0};

  // Define menu items
  const char *menuItems[] = {"Level", "Cooler Level", "Quit"};
  int selectedItem = 0; // Index of the selected menu item

  std::string level1 = "assets/map.txt";
  std::string level2 = "assets/map2.txt";

  // Menu loop
  bool inMenu = true;
  while (inMenu)
  {
    SDL_Event event;
    // Event handling
    while (SDL_PollEvent(&event))
    {
      if (event.type == SDL_QUIT)
      {
        inMenu = false;
        running = false; // To exit the game completely
        break;
      }
      else if (event.type == SDL_KEYDOWN)
      {
        switch (event.key.keysym.sym)
        {
        case SDLK_UP:
          selectedItem = (selectedItem + 2) % 3; // Move up in the menu
          break;
        case SDLK_DOWN:
          selectedItem = (selectedItem + 1) % 3; // Move down in the menu
          break;
        case SDLK_RETURN:
          if (selectedItem == 0)
          {
            r.load_map(level1);
            inMenu = false; // Start the game
          }
          else if (selectedItem == 1)
          {
            r.load_map(level2);
            inMenu = false; // Start the game
          }
          else if (selectedItem == 2)
          {
            inMenu = false;
            running = false; // Quit the game
          }
          break;
        }
      }
    }

    // Rendering the menu
    clear(); // Clear the screen
    for (int i = 0; i < 3; i++)
    {
      // Set text color to white or red depending on if the item is selected
      SDL_Color textColor = (i == selectedItem) ? SDL_Color{255, 0, 0, 255} : SDL_Color{255, 255, 255, 255};
      SDL_Surface *textSurface = TTF_RenderText_Solid(font, menuItems[i], textColor);
      SDL_Texture *textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
      int text_width = textSurface->w;
      int text_height = textSurface->h;
      SDL_FreeSurface(textSurface);

      // Calculate position to center the text
      int x = (SCREEN_WIDTH - text_width) / 2;
      int y = (SCREEN_HEIGHT / 4) * (i + 1) - (text_height / 2);
      SDL_Rect renderQuad = {x, y, text_width, text_height};

      // Render text to screen
      SDL_RenderCopy(renderer, textTexture, NULL, &renderQuad);
      SDL_DestroyTexture(textTexture);
    }
    SDL_RenderPresent(renderer); // Update the screen
  }

  // Start the music
  Mix_PlayMusic(music, -1);

  // Lower the volume by 20dB
  int currentVolume = Mix_VolumeMusic(-1); // Get the current volume
  int newVolume = currentVolume / 4;       // Decrease the volume by 20dB
  Mix_VolumeMusic(newVolume);

  while (running)
  {
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
      if (event.type == SDL_QUIT)
      {
        running = false;
        break;
      }
      if (event.type == SDL_KEYDOWN)
      {
        switch (event.key.keysym.sym)
        {
        case SDLK_LEFT:
          r.player.a += 3.14 / 24;
          break;
        case SDLK_RIGHT:
          r.player.a -= 3.14 / 24;
          break;
        case SDLK_UP:
          r.player.x += speed * cos(r.player.a);
          r.player.y += speed * sin(r.player.a);
          if (!isWalkingSoundPlaying)
          {
            Mix_PlayChannel(-1, walkSound, 0);
            isWalkingSoundPlaying = true;
          }
          break;
        case SDLK_DOWN:
          r.player.x -= speed * cos(r.player.a);
          r.player.y -= speed * sin(r.player.a);
          if (!isWalkingSoundPlaying)
          {
            Mix_PlayChannel(-1, walkSound, 0);
            isWalkingSoundPlaying = true;
          }
          break;
        default:
          break;
        }
      }
      else if (event.type == SDL_KEYUP)
      {
        switch (event.key.keysym.sym)
        {
        case SDLK_UP:
        case SDLK_DOWN:
          if (isWalkingSoundPlaying)
          {
            Mix_HaltChannel(-1); // This stops the sound from playing
            isWalkingSoundPlaying = false;
          }
          break;
        }
      }

      if (r.player.won == true)
      {
        // Stop the music
        Mix_HaltMusic();

        Mix_PlayChannel(-1, winSound, 0);

        // Clear the screen
        SDL_RenderClear(renderer);

        // Set text color to white
        SDL_Color textColor = {255, 255, 255, 255};

        // Render the text
        SDL_Surface *textSurface = TTF_RenderText_Solid(font, "You escaped the maze", textColor);
        SDL_Texture *textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
        int text_width = textSurface->w;
        int text_height = textSurface->h;
        SDL_FreeSurface(textSurface);

        // Calculate position to center the text
        int x = (SCREEN_WIDTH - text_width) / 2;
        int y = (SCREEN_HEIGHT - text_height) / 2;
        SDL_Rect renderQuad = {x, y, text_width, text_height};

        // Render text to screen
        SDL_RenderCopy(renderer, textTexture, NULL, &renderQuad);
        SDL_DestroyTexture(textTexture);

        // Update the screen
        SDL_RenderPresent(renderer);

        // Wait for a while so the user can read the message
        SDL_Delay(3000);

        running = false;
        break;
      }
    }

    clear();
    draw_floor();

    r.render();

    // draw_ui();
    // render
    frameCount++;
    Uint32 currentTicks = SDL_GetTicks();
    if (currentTicks - startTicks >= 1000)
    {
      float fps = frameCount / ((currentTicks - startTicks) / 1000.0f);
      snprintf(fpsText, sizeof(fpsText), "FPS: %.2f", fps);
      frameCount = 0;
      startTicks = currentTicks;

      // Free the old texture
      if (fpsTexture)
      {
        SDL_DestroyTexture(fpsTexture);
        fpsTexture = NULL;
      }

      SDL_Color textColor = {255, 255, 255, 255}; // White color
      SDL_Surface *textSurface = TTF_RenderText_Solid(font, fpsText, textColor);
      fpsTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
      fpsRect.w = textSurface->w;
      fpsRect.h = textSurface->h;
      SDL_FreeSurface(textSurface);
    }

    if (fpsTexture)
    {
      SDL_RenderCopy(renderer, fpsTexture, NULL, &fpsRect);
    }

    SDL_RenderPresent(renderer);
  }
  Mix_FreeChunk(walkSound);
  Mix_CloseAudio();
  TTF_CloseFont(font);
  TTF_Quit();
  SDL_DestroyWindow(window);
  SDL_Quit();
}
