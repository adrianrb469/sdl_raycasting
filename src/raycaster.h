#pragma once

#include <print.h>
#include <iostream>
#include <fstream>
#include <SDL_render.h>
#include <string>
#include <vector>
#include <cmath>
#include <SDL2/SDL.h>
#include <unordered_map>
#include "color.h"
#include "imageloader.h"

const Color B = {0, 0, 0};
const Color W = {255, 255, 255};

const int WIDTH = 16;
const int HEIGHT = 11;
const int BLOCK = 50;
const int SCREEN_WIDTH = WIDTH * BLOCK;
const int SCREEN_HEIGHT = HEIGHT * BLOCK;

// Define a minimum distance to avoid division by zero
const float MIN_DISTANCE = 0.1f;

struct Player
{
  int x;
  int y;
  float a;
  float fov;
  bool won = false;
};

struct Impact
{
  float d;
  std::string mapHit; // + | -
  int tx;
};

struct Enemy
{
  int x;
  int y;
  std::string texture;
};

std::vector<Enemy> enemies;

class Raycaster
{
public:
  Raycaster(SDL_Renderer *renderer)
      : renderer(renderer)
  {

    player.x = BLOCK + BLOCK / 2;
    player.y = BLOCK + BLOCK / 2;

    player.a = M_PI / 4.0f;
    player.fov = M_PI / 3.0f;

    scale = 50;
    tsize = 128;

    enemies = {Enemy{BLOCK * 5, BLOCK, "e1"}};
  }

  void load_map(const std::string &filename)
  {
    std::ifstream file(filename);
    std::string line;
    while (getline(file, line))
    {
      map.push_back(line);
    }
    file.close();
  }

  void point(int x, int y, Color c)
  {
    SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, c.a);
    SDL_RenderDrawPoint(renderer, x, y);
  }

  void rect(int x, int y, const std::string &mapHit)
  {
    for (int cx = x; cx < x + BLOCK; cx++)
    {
      for (int cy = y; cy < y + BLOCK; cy++)
      {
        int tx = ((cx - x) * tsize) / BLOCK;
        int ty = ((cy - y) * tsize) / BLOCK;

        Color c = ImageLoader::getPixelColor(mapHit, tx, ty);
        SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, 255);
        SDL_RenderDrawPoint(renderer, cx, cy);
      }
    }
  }

  void rect2(int x, int y, int width, int height, const std::string &mapHit)
  {
    for (int cx = x; cx < x + width; cx++)
    {
      for (int cy = y; cy < y + height; cy++)
      {
        int tx = ((cx - x) * tsize) / width;
        int ty = ((cy - y) * tsize) / height;

        Color c = ImageLoader::getPixelColor(mapHit, tx, ty);
        SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, 255);
        SDL_RenderDrawPoint(renderer, cx, cy);
      }
    }
  }

  void draw_enemy(Enemy enemy)
  {
    float enemy_a = atan2(enemy.y - player.y, enemy.x - player.x);
    float enemy_d = sqrt(pow(player.x - enemy.x, 2) + pow(player.y - enemy.y, 2));
    int enemy_size = (SCREEN_HEIGHT / enemy_d) * scale;

    int enemy_x = (enemy_a - player.a) * (SCREEN_WIDTH / player.fov) + SCREEN_WIDTH / 2.0f - enemy_size / 2.0f;
    int enemy_y = (SCREEN_HEIGHT / 2.0f) - enemy_size / 2.0f;

    for (int x = enemy_x; x < enemy_x + enemy_size; x++)
    {
      for (int y = enemy_y; y < enemy_y + enemy_size; y++)
      {
        int tx = (x - enemy_x) * tsize / enemy_size;
        int ty = (y - enemy_y) * tsize / enemy_size;

        Color c = ImageLoader::getPixelColor(enemy.texture, tx, ty);
        if (c.r != 152 && c.g != 0 && c.b != 136)
        {
          SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, c.a);
          SDL_RenderDrawPoint(renderer, x, y);
        }
      }
    }
  }

  Impact cast_ray(float a)
  {
    float d = 0;
    std::string mapHit;
    int tx;

    while (true)
    {
      int x = static_cast<int>(player.x + d * cos(a));
      int y = static_cast<int>(player.y + d * sin(a));

      int i = static_cast<int>(x / BLOCK);
      int j = static_cast<int>(y / BLOCK);

      if (map[j][i] != ' ')
      {
        mapHit = map[j][i];

        int hitx = x - i * BLOCK;
        int hity = y - j * BLOCK;
        int maxhit;

        if (hitx == 0 || hitx == BLOCK - 1)
        {
          maxhit = hity;
        }
        else
        {
          maxhit = hitx;
        }

        tx = maxhit * tsize / BLOCK;

        break;
      }

      /* point(x, y, W); */
      if (d < MIN_DISTANCE)
      {
        d = MIN_DISTANCE;
      }

      d += 1;
    }
    return Impact{d, mapHit, tx};
  }

  void draw_stake(int x, float h, Impact i)
  {
    float start = SCREEN_HEIGHT / 2.0f - h / 2.0f;
    float end = start + h;

    for (int y = start; y < end; y++)
    {
      int ty = (y - start) * tsize / h;
      Color c = ImageLoader::getPixelColor(i.mapHit, i.tx, ty);
      SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, c.a);

      SDL_RenderDrawPoint(renderer, x, y);
    }
  }

  void minimap()
  {
    ImageLoader::render(renderer, "bg", 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    // Define the new size of each block in the minimap
    const int MINIMAP_BLOCK_SIZE = BLOCK / 4; // half the size of the original BLOCK

    // Calculate the starting position of the minimap (top right corner)
    const int MINIMAP_OFFSET_X = SCREEN_WIDTH - WIDTH * MINIMAP_BLOCK_SIZE;
    const int MINIMAP_OFFSET_Y = 0; // start at the top of the screen

    for (int x = 0; x < WIDTH * BLOCK; x += BLOCK)
    {
      for (int y = 0; y < HEIGHT * BLOCK; y += BLOCK)
      {
        int i = static_cast<int>(x / BLOCK);
        int j = static_cast<int>(y / BLOCK);

        if (map[j][i] != ' ')
        {
          std::string mapHit(1, map[j][i]);
          // Scale and offset the position of each block
          int minimap_x = MINIMAP_OFFSET_X + (x / BLOCK) * MINIMAP_BLOCK_SIZE;
          int minimap_y = MINIMAP_OFFSET_Y + (y / BLOCK) * MINIMAP_BLOCK_SIZE;

          // Use the new smaller block size for rendering
          rect2(minimap_x, minimap_y, MINIMAP_BLOCK_SIZE, MINIMAP_BLOCK_SIZE, mapHit);
        }
        else
        {
          int minimap_x = MINIMAP_OFFSET_X + (x / BLOCK) * MINIMAP_BLOCK_SIZE;
          int minimap_y = MINIMAP_OFFSET_Y + (y / BLOCK) * MINIMAP_BLOCK_SIZE;
          SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
          SDL_Rect rect = {
              minimap_x,
              minimap_y,
              MINIMAP_BLOCK_SIZE,
              MINIMAP_BLOCK_SIZE};
          SDL_RenderFillRect(renderer, &rect);
        }
      }

      // Calculate the scaled player position for the minimap
      int minimapPlayerX = MINIMAP_OFFSET_X + (player.x / BLOCK) * MINIMAP_BLOCK_SIZE;
      int minimapPlayerY = MINIMAP_OFFSET_Y + (player.y / BLOCK) * MINIMAP_BLOCK_SIZE;

      SDL_Rect playerRect = {
          minimapPlayerX,
          minimapPlayerY,
          4,
          4};

      // Set the draw color for the player's marker (e.g., red)
      SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);

      // Draw the player's marker
      SDL_RenderFillRect(renderer, &playerRect);
    }
  }

  void render()
  {
    std::cout << "playerX: " << player.x << " playerY: " << player.y << std::endl;

    // draw right side of the screen

    for (int i = 0; i < SCREEN_WIDTH; i++)
    {
      double a = player.a + player.fov / 2.0 - player.fov * i / SCREEN_WIDTH;
      Impact impact = cast_ray(a);
      float d = impact.d;
      Color c = Color(255, 0, 0);

      if (impact.mapHit == "+")
      {
        c = Color(255, 255, 255);
      }
      else if (impact.mapHit == "-")
      {
        c = Color(0, 255, 0);
      }

      // std::cout << "SCREEN_WIDTH - 100: " << SCREEN_WIDTH - 100 << std::endl;
      // std::cout << "SCREEN_HEIGHT - 100: " << SCREEN_HEIGHT - 100 << std::endl;

      if (player.x > SCREEN_WIDTH - 140 && player.y > SCREEN_HEIGHT - 140)
      {
        player.won = true;
      }

      if (d == 0)
      {
        player.x = 85;
        player.y = 85;
      }
      int x = i;
      float h = static_cast<float>(SCREEN_HEIGHT) / static_cast<float>(d * cos(a - player.a)) * static_cast<float>(scale);
      draw_stake(x, h, impact);
    }

    minimap();
  }

  Player player;

private:
  int scale;
  SDL_Renderer *renderer;
  std::vector<std::string> map;
  int tsize;
};
