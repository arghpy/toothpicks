#include <stdio.h>
#include "raylib.h"
#include "raymath.h"

#define UT_IMPLEMENTATION
#include "utils.h"

#define BACKGROUND ((Color){0x18, 0x18, 0x18, 0x18})

typedef struct {
  Vector2 startPos;
  Vector2 endPos;
  bool checked;
} Toothpick;

typedef struct {
  Vector2 startPos;
  Vector2 endPos;
} Line;

typedef enum {
  UP,
  DOWN,
} Scaling;

float length = 0;
float generations = 0;

typedef ut_da_declare(Toothpick) Toothpicks;
typedef ut_da_declare(Line) Lines;
ut_ll_declare(Line_ll, Lines);

void scale_lines(Line_ll *ll_head, float factor)
{
  Line_ll *ll_index = ll_head;
  Vector2 center = {
    GetScreenWidth() / 2.0f,
    GetScreenHeight() / 2.0f
  };

  while (ll_index != NULL) {
    for (size_t i = 0; i < ll_index->value.count; i++) {
      Line *l = &ll_index->value.items[i];

      l->startPos = Vector2Add(
          center,
          Vector2Scale(Vector2Subtract(l->startPos, center), factor)
          );

      l->endPos = Vector2Add(
          center,
          Vector2Scale(Vector2Subtract(l->endPos, center), factor)
          );
    }
    ll_index = ll_index->next;
  }
}

void resize_lines(Line_ll* ll_curr, Line_ll* ll_head, Scaling scale)
{
  bool resize;
  float factor;

  switch(scale) {
    case UP:
      resize = false;
      factor = 0.99f;
      while (!resize) {
        for (size_t i = 0; i < ll_curr->value.count; i++) {
          if (ll_curr->value.items[i].startPos.y < length) {
            resize = true;
            break;
          }
        }
        if (resize) {
          scale_lines(ll_head, factor);
          length *= factor;
          resize = false;
        } else break;
      }
      break;
    case DOWN:
      resize = true;
      factor = 1.01f;
      while (resize) {
        for (size_t i = 0; i < ll_curr->value.count; i++) {
          if (ll_curr->value.items[i].startPos.y <= length) {
            resize = false;
            break;
          }
          if (ll_curr->value.items[i].startPos.y > length) {
            scale_lines(ll_head, factor);
            length *= factor;
          }
        }
      }
      break;
    default:
      UNREACHABLE("Switch case on scale");
  }
}

#define MY_EPSILON 0.1f

bool PointsEqual(Vector2 a, Vector2 b)
{
    return fabsf(a.x - b.x) < MY_EPSILON &&
           fabsf(a.y - b.y) < MY_EPSILON;
}

bool is_power_of_2(float n)
{
  int q   = (int)log2f(n);
  float p = powf(2, q);
  return (p == n);
}

void add_lines(Line* l, Lines* ls_temp, Line_ll* ll_tail, Line_ll* ll_limit)
{
  bool collides_start = false;
  bool collides_end   = false;

  Line_ll *ll_index = ll_tail;
  if (generations > 2 && is_power_of_2(generations)) {
    assert(ll_tail->prev->prev->prev != NULL);
    ll_limit = ll_tail->prev->prev->prev;
  }

  while(ll_index != NULL && ll_index != ll_limit) {
    for (size_t i = 0; i < ll_index->value.count; i++) {
      if (&ll_index->value.items[i] == l) continue;
      if (!CheckCollisionPointCircle(ll_index->value.items[i].startPos, l->startPos, length * 2) &&
          !CheckCollisionPointCircle(ll_index->value.items[i].endPos,   l->startPos, length * 2) &&
          !CheckCollisionPointCircle(ll_index->value.items[i].startPos, l->endPos,   length * 2) &&
          !CheckCollisionPointCircle(ll_index->value.items[i].endPos,   l->endPos,   length * 2)) continue;

      if (PointsEqual(l->startPos, ll_index->value.items[i].startPos) ||
          PointsEqual(l->startPos, ll_index->value.items[i].endPos))
        collides_start = true;

      if (PointsEqual(l->endPos, ll_index->value.items[i].startPos) ||
          PointsEqual(l->endPos, ll_index->value.items[i].endPos))
        collides_end = true;
    }
    ll_index = ll_index->prev;
  }

  if (collides_start && collides_end) return;
  else if (!collides_start && !collides_end) {
    Line l1 = {0};
    Line l2 = {0};

    // Vertical:   l->startPos.y - l->endPos.y < 0
    if (l->startPos.y - l->endPos.y < 0) {
      l1.startPos = Vector2Add(l->startPos, (Vector2){.x = - length / 2, .y = 0});
      l1.endPos   = Vector2Add(l->startPos, (Vector2){.x =   length / 2, .y = 0});
      l2.startPos = Vector2Add(l->endPos,   (Vector2){.x = - length / 2, .y = 0});
      l2.endPos   = Vector2Add(l->endPos,   (Vector2){.x =   length / 2, .y = 0});
    } else {
      l1.startPos = Vector2Add(l->startPos, (Vector2){.x = 0, .y = - length / 2});
      l1.endPos   = Vector2Add(l->startPos, (Vector2){.x = 0, .y =   length / 2});
      l2.startPos = Vector2Add(l->endPos,   (Vector2){.x = 0, .y = - length / 2});
      l2.endPos   = Vector2Add(l->endPos,   (Vector2){.x = 0, .y =   length / 2});
    }
    ut_da_push(ls_temp, l1);
    ut_da_push(ls_temp, l2);
  } else {
    Vector2 pos = {0};
    if      (collides_start) pos = l->endPos;
    else if (collides_end)   pos = l->startPos;
    else abort();

    Line ll = {0};

    // Vertical:   l->startPos.y - l->endPos.y < 0
    if (l->startPos.y - l->endPos.y < 0) {
      ll.startPos = Vector2Add(pos, (Vector2){.x = - length / 2, .y = 0});
      ll.endPos   = Vector2Add(pos, (Vector2){.x =   length / 2, .y = 0});
    } else {
      ll.startPos = Vector2Add(pos, (Vector2){.x = 0, .y = - length / 2});
      ll.endPos   = Vector2Add(pos, (Vector2){.x = 0, .y =   length / 2});
    }
    ut_da_push(ls_temp, ll);
  }
}

int main(void)
{
  SetTraceLogLevel(LOG_ERROR);
  InitWindow(800, 600, "Terrific toothpick patterns");
  SetTargetFPS(60);

  char *font_path_regular = "./assets/fonts/JetBrainsMono-Regular.ttf";
  Font font_regular = LoadFont(font_path_regular);
  if (!IsFontValid(font_regular)) return 1;

  Line_ll *ll_head = NULL;
  Line_ll *ll_curr = NULL;
  Line_ll *ll_tail = NULL;
  Line_ll *ll_limit = NULL;
  Lines lines = {0};

  length = GetScreenHeight() * 0.15f;

  Line l = {
    .startPos = {
      .x = GetScreenWidth()  / 2.0f,
      .y = GetScreenHeight() / 2.0f - length / 2,
    },
    .endPos = {
      .x = GetScreenWidth()  / 2.0f,
      .y = GetScreenHeight() / 2.0f + length / 2,
    },
  };
  ut_da_push(&lines, l);
  ut_ll_push(Line_ll, ll_head, lines, ll_tail);
  ll_curr = ll_head;
  generations++;

  while(!WindowShouldClose()) {
    if (IsKeyDown(KEY_RIGHT)) {
      if (ll_curr == ll_tail) {
        Lines ls = {0};
        for (size_t i = 0; i < ll_curr->value.count; i++)
          add_lines(&ll_curr->value.items[i], &ls, ll_tail, ll_limit);

        ut_ll_push(Line_ll, ll_head, ls, ll_tail);
        generations++;
      }
      ll_curr = ll_curr->next;
      resize_lines(ll_curr, ll_head, UP);
    }
    if (IsKeyDown(KEY_LEFT)) {
      if (ll_curr != ll_head)
        ll_curr = ll_curr->prev;
      resize_lines(ll_curr, ll_head, DOWN);
    }
    BeginDrawing();
    {
      ClearBackground(BACKGROUND);
      Line_ll *ll_index = ll_head;
      while (ll_index != NULL && ll_index->prev != ll_curr) {
        for (size_t i = 0; i < ll_index->value.count; i++) {
          if (ll_index != ll_curr)
            DrawLineV(ll_index->value.items[i].startPos, ll_index->value.items[i].endPos, RAYWHITE);
          else
            DrawLineV(ll_index->value.items[i].startPos, ll_index->value.items[i].endPos, BLUE);
        }
        ll_index = ll_index->next;
      }
      DrawTextEx(font_regular, TextFormat("Gen:\n%d", (int)generations), (Vector2){0, 0}, font_regular.baseSize, 0, WHITE);
    }
    EndDrawing();
  }
  UnloadFont(font_regular);

  Line_ll* ll_index = ll_head;
  while(ll_index != NULL) {
    ut_da_free(&ll_index->value);
    ll_index = ll_index->next;
  }
  ut_ll_free(Line_ll, ll_head);
  CloseWindow();
  return 0;
}
