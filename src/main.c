#include <SDL.h>
#include <SDL_image.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 1000
#define FPS_CAP 60
#define PLAYER_WIDTH 70
#define PLAYER_HEIGHT 60
#define PLAYER_ACC 30
#define TOWER_COUNT 10
#define TOWER_WIDTH 100
#define TOWER_HEIGHT 600
#define GAP_HEIGHT 200
#define GAP_DISTANCE 500
#define FLOOR_WIDTH WINDOW_WIDTH * 2
#define TOWER_SPEED -4
#define GRAVITY 13

typedef struct {
  float x, y;
} v2_t;

typedef struct {
  v2_t pos;
  int inactive;
} tower_t;

typedef struct {
  v2_t pos;
} floor_t;

static struct {
  bool quit;
  bool menu;
  bool game_over;
  SDL_Renderer *renderer;
  SDL_Window *window;
  SDL_Texture *sprite;
  int score;
  int highscore;
  struct {
    v2_t pos;
    float acc;
  } player;
  tower_t towers[TOWER_COUNT];
  floor_t floors[2];
} state;

SDL_Rect nums[10] = {
    {496, 60, 12, 18},  {134, 455, 12, 18}, {292, 160, 12, 18},
    {306, 160, 12, 18}, {320, 160, 12, 18}, {334, 160, 12, 18},
    {292, 184, 12, 18}, {306, 184, 12, 18}, {320, 184, 12, 18},
    {334, 184, 12, 18},
};

SDL_Rect medals[3] = {
    {121, 258, 22, 22},
    {121, 282, 22, 22},
    {112, 453, 22, 22},
};

void init_player() {
  state.player.pos =
      (v2_t){(float)WINDOW_WIDTH / 2 - (float)PLAYER_WIDTH / 2,
             (float)WINDOW_HEIGHT / 2 - (float)PLAYER_HEIGHT - 2};
}

void init_towers(tower_t towers[]) {
  for (int i = 0; i < TOWER_COUNT; i++) {
    towers[i].pos.x = 200 + WINDOW_WIDTH + (i * GAP_DISTANCE);
    towers[i].pos.y =
        ((float)rand() / (float)RAND_MAX) * ((float)WINDOW_HEIGHT / 3) +
        ((float)WINDOW_HEIGHT / 3) / 3;
  }
}

void init_floors(floor_t floors[]) {
  for (int i = 0; i < 2; i++) {
    floors[i].pos.x = i * FLOOR_WIDTH;
    floors[i].pos.y = WINDOW_HEIGHT - ((float)WINDOW_HEIGHT / 3) + 100;
  }
}

static void handle_fps(const uint64_t start, uint64_t end) {

  end = SDL_GetPerformanceCounter();

  float deltams =
      (end - start) / (float)SDL_GetPerformanceFrequency() * 1000.0f;
  const float delayms = (1000.0f / FPS_CAP) - deltams;
  SDL_Delay(delayms < 0 ? 0 : delayms);

  end = SDL_GetPerformanceCounter();
  deltams = (end - start) / (float)SDL_GetPerformanceFrequency() * 1000.0f;

  float fps = 1.0f / (deltams / 1000.0f);
  // printf("fps: %f\n", fps);
}

int count_digits(int num) {
  int ret = 1;
  while (num /= 10)
    ret++;
  return ret;
}

void score_to_array(int score, int digit_count, int arr[]) {
  for (int i = digit_count - 1; i >= 0; i--) {
    arr[i] = score % 10;
    score /= 10;
  }
}

static void handle_event(SDL_Event event) {
  while (SDL_PollEvent(&event)) {
    switch (event.type) {
    case SDL_QUIT:
      state.quit = true;
      return;
    case SDL_KEYDOWN:
      switch (event.key.keysym.scancode) {
      case SDL_SCANCODE_SPACE:
        if (state.menu) {
          state.menu = false;
        }
        state.player.acc = PLAYER_ACC;
        break;
      default:
        break;
      }
    case SDL_MOUSEBUTTONDOWN:
      switch (event.button.button)
      case SDL_BUTTON_LEFT:
        if (state.menu) {
          state.menu = false;
        }
      state.player.acc = PLAYER_ACC;
    default:
      break;
    }
  }
}

void collission_event() {
  state.menu = true;
  state.game_over = true;
  if (state.score > state.highscore) {
    state.highscore = state.score;
  }
}

static void update() {
  if (!state.menu) {
    if (state.game_over) {
      init_player();
      init_towers(state.towers);
      state.score = 0;
      state.game_over = false;
    }

    // EDGE COLLISION
    if (state.player.pos.y < 0 ||
        state.player.pos.y + PLAYER_WIDTH > state.floors[0].pos.y) {
      collission_event();
    }

    // TOWER COLLISION
    for (int i = 0; i < TOWER_COUNT; i++) {

      // UPPER TOWER COLLISION
      if (state.player.pos.x < state.towers[i].pos.x + TOWER_WIDTH &&
          state.player.pos.x + PLAYER_WIDTH > state.towers[i].pos.x &&
          state.player.pos.y < state.towers[i].pos.y &&
          state.player.pos.y + PLAYER_HEIGHT >
              state.towers[i].pos.y - TOWER_HEIGHT) {
        collission_event();
      }

      // LOWER TOWER COLLISION
      if (state.player.pos.x < state.towers[i].pos.x + TOWER_WIDTH &&
          state.player.pos.x + PLAYER_WIDTH - 10 > state.towers[i].pos.x &&
          state.player.pos.y <
              state.towers[i].pos.y + GAP_HEIGHT + TOWER_HEIGHT &&
          state.player.pos.y + PLAYER_HEIGHT >
              GAP_HEIGHT + state.towers[i].pos.y) {
        collission_event();
      }

      // MOVING TOWERS
      if (state.towers[i].pos.x <= -TOWER_WIDTH - 10) {
        state.towers[i].pos.x = TOWER_COUNT * GAP_DISTANCE - TOWER_WIDTH;
        state.towers[i].pos.y =
            ((float)rand() / (float)RAND_MAX) * ((float)WINDOW_HEIGHT / 3) +
            ((float)WINDOW_HEIGHT / 3) / 2;
      }
      state.towers[i].pos.x += TOWER_SPEED;

      // UPDATING SCORE
      if (state.player.pos.x > state.towers[i].pos.x + TOWER_WIDTH &&
          state.towers[i].inactive != 1) {
        state.score++;
        state.towers[i].inactive = 1;
      }
    }

    // MOVING PLAYER
    state.player.pos.y += GRAVITY - state.player.acc;
    state.player.acc *= 0.93;

    // MOVING FLOOR
    for (int i = 0; i < 2; i++) {
      if (state.floors[i].pos.x <= -FLOOR_WIDTH - ((float)FLOOR_WIDTH / 2)) {
        state.floors[i].pos.x += 2 * FLOOR_WIDTH;
      }
      state.floors[i].pos.x += TOWER_SPEED;
    }
  }
}

static void render() {
  // SDL_SetRenderDrawColor(state.renderer, 255, 255, 255, 255);
  SDL_RenderClear(state.renderer);

  // RENDER BACKGROUND
  const SDL_Rect bg_src = {0, 0, 144, 256};
  const SDL_Rect bg_dst = {0, 0, WINDOW_WIDTH, state.floors[0].pos.y};
  SDL_RenderCopy(state.renderer, state.sprite, &bg_src, &bg_dst);

  // RENDER PLAYER
  const SDL_Rect player_src = {3, 489, 17, 17};
  const SDL_Rect player_dst = {(int)state.player.pos.x, (int)state.player.pos.y,
                               PLAYER_WIDTH, PLAYER_HEIGHT};
  SDL_RenderCopy(state.renderer, state.sprite, &player_src, &player_dst);

  // RENDER TOWERS
  int rendered_towers = 0;
  for (int i = 0; i < TOWER_COUNT; i++) {
    if (state.towers[i].pos.x > WINDOW_WIDTH) {
      state.towers[i].inactive = 0;
      continue;
    }
    if (rendered_towers == 2)
      break;
    rendered_towers++;

    const SDL_Rect tower_upper_src = {56, 323, 26, 160};
    const SDL_Rect tower_lower_src = {84, 323, 26, 160};
    const SDL_Rect tower_upper_dst = {state.towers[i].pos.x,
                                      state.towers[i].pos.y - TOWER_HEIGHT,
                                      TOWER_WIDTH, TOWER_HEIGHT};
    const SDL_Rect tower_lower_dst = {state.towers[i].pos.x,
                                      GAP_HEIGHT + state.towers[i].pos.y,
                                      TOWER_WIDTH, TOWER_HEIGHT};

    SDL_RenderCopy(state.renderer, state.sprite, &tower_upper_src,
                   &tower_upper_dst);
    SDL_RenderCopy(state.renderer, state.sprite, &tower_lower_src,
                   &tower_lower_dst);
  }

  // RENDER SCORE
  if (!state.menu) {
    int score_dig_count = count_digits(state.score);
    int score_digits[score_dig_count];
    score_to_array(state.score, score_dig_count, score_digits);
    for (int i = 0; i <= score_dig_count - 1; i++) {
      const SDL_Rect dst = {(int)(WINDOW_WIDTH / 2) -
                                (int)((score_dig_count * 50) / 2) + (i * 50),
                            (int)(WINDOW_HEIGHT / 6), 40, 60};
      int number = score_digits[i];
      SDL_RenderCopy(state.renderer, state.sprite, &nums[score_digits[i]],
                     &dst);
    }
  }

  // RENDER FLOOR
  const SDL_Rect floor_src = {292, 0, 168, 57};
  for (int i = 0; i < 2; i++) {
    SDL_Rect floor_dst = {state.floors[i].pos.x, state.floors[i].pos.y,
                          FLOOR_WIDTH, ((float)WINDOW_HEIGHT / 3)};
    SDL_RenderCopy(state.renderer, state.sprite, &floor_src, &floor_dst);
  }

  // RENDER MENU
  if (state.menu) {
    if (!state.game_over) {
      // FLOPSYBIRD TITLE
      {
        SDL_Rect src = {351, 91, 89, 24};
        SDL_Rect dst = {WINDOW_WIDTH / 2 - 300, WINDOW_HEIGHT / 2 - 500, 600,
                        162};
        SDL_RenderCopy(state.renderer, state.sprite, &src, &dst);
      }

      // NORMAL MENU
      {
        SDL_Rect src = {295, 59, 92, 25};
        SDL_Rect dst = {WINDOW_WIDTH / 2 - 200, WINDOW_HEIGHT / 2 - 300, 400,
                        108};
        SDL_RenderCopy(state.renderer, state.sprite, &src, &dst);
      }
    } else {
      // GAMEOVER TITLE
      {
        SDL_Rect src = {395, 59, 96, 21};
        SDL_Rect dst = {WINDOW_WIDTH / 2 - 200, WINDOW_HEIGHT / 2 - 400, 400,
                        108};
        SDL_RenderCopy(state.renderer, state.sprite, &src, &dst);
      }

      // GAMEOVER MENU
      {
        SDL_Rect src = {3, 259, 113, 57};
        SDL_Rect dst = {WINDOW_WIDTH / 2 - 250, WINDOW_HEIGHT / 2 - 200, 500,
                        252};
        SDL_RenderCopy(state.renderer, state.sprite, &src, &dst);
      }

      // RENDER SCORE
      {
        int score_dig_count = count_digits(state.score);
        int score_digits[score_dig_count];
        score_to_array(state.score, score_dig_count, score_digits);
        for (int i = 0; i <= score_dig_count - 1; i++) {
          const SDL_Rect dst = {i * 40 + (WINDOW_WIDTH / 2) + 160 -
                                    ((score_dig_count * 40) / 2),
                                (WINDOW_HEIGHT / 2) - 103 - 25, 35, 50};
          int number = score_digits[i];
          SDL_RenderCopy(state.renderer, state.sprite, &nums[score_digits[i]],
                         &dst);
        }
      }
      // RENDER HIGHSCORE
      {
        int score_dig_count = count_digits(state.highscore);
        int score_digits[score_dig_count];
        score_to_array(state.highscore, score_dig_count, score_digits);
        for (int i = 0; i <= score_dig_count - 1; i++) {
          const SDL_Rect dst = {i * 40 + (WINDOW_WIDTH / 2) + 160 -
                                    ((score_dig_count * 40) / 2),
                                (WINDOW_HEIGHT / 2) - 10 - 25, 35, 50};
          int number = score_digits[i];
          SDL_RenderCopy(state.renderer, state.sprite, &nums[score_digits[i]],
                         &dst);
        }
      }
      // RENDER MEDAL
      if (state.score >= 40) {
        const SDL_Rect dst = {WINDOW_WIDTH / 2 - 192, WINDOW_HEIGHT / 2 - 107,
                              97, 97};
        SDL_RenderCopy(state.renderer, state.sprite, &medals[2], &dst);
      } else if (state.score >= 30) {
        const SDL_Rect dst = {WINDOW_WIDTH / 2 - 192, WINDOW_HEIGHT / 2 - 107,
                              97, 97};
        SDL_RenderCopy(state.renderer, state.sprite, &medals[1], &dst);
      } else if (state.score >= 20) {
        const SDL_Rect dst = {WINDOW_WIDTH / 2 - 192, WINDOW_HEIGHT / 2 - 107,
                              97, 97};
        SDL_RenderCopy(state.renderer, state.sprite, &medals[0], &dst);
      }
    }
  }

  SDL_RenderPresent(state.renderer);
}

int main() {
  srand(time(NULL));

  // INITIALIZATION
  if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
    goto CLEANUP;

  state.window = SDL_CreateWindow("Flopsy Bird", -10, SDL_WINDOWPOS_CENTERED,
                                  WINDOW_WIDTH, WINDOW_HEIGHT, 0);
  if (!state.window)
    goto CLEANUP;

  state.renderer =
      SDL_CreateRenderer(state.window, -1, SDL_RENDERER_ACCELERATED);
  if (!state.renderer)
    goto CLEANUP;

  // SPRITE LOADING
  state.sprite =
      IMG_LoadTexture(state.renderer, "/Users/mfischbach/Developer/projects/"
                                      "flopsy-bird/src/res/img/sprite.png");
  // INIT PLAYER
  init_player();

  // INIT TOWERS
  init_towers(state.towers);

  // INIT FLOOR
  init_floors(state.floors);

  state.quit = false;
  state.game_over = false;
  state.menu = true;
  state.score = 0;
  uint64_t start, end;
  SDL_Event event;
  while (!state.quit) {
    start = SDL_GetPerformanceCounter();

    handle_event(event);

    update();

    render();

    handle_fps(start, end);
  }

  // CLEANUP
CLEANUP:;
  if (strcmp(SDL_GetError(), "") != 0) {
    fprintf(stderr, "Error: %s\n", SDL_GetError());
  }
  SDL_DestroyWindow(state.window);
  SDL_DestroyRenderer(state.renderer);
  SDL_DestroyTexture(state.sprite);
  SDL_Quit();
  printf("Cleanup successfull!\n");

  return 0;
}
