#include <stdio.h>
#include "raylib.h"
#include "raymath.h"

#define UT_IMPLEMENTATION
#include "utils.h"

#define BACKGROUND ((Color){0x18, 0x18, 0x18, 0x18})

typedef struct {
  Vector2 startPos;
  Vector2 endPos;
  float length;
  float thick;
  bool checked;
} Toothpick;

typedef ut_da_declare(Toothpick) Toothpicks;

void scale_toothpicks(Toothpicks* ts)
{
  float factor = 0.97f;

  Vector2 center = {
    GetScreenWidth() / 2.0f,
    GetScreenHeight() / 2.0f
  };

  for (size_t i = 0; i < ts->count; i++) {
    Toothpick* t = &ts->items[i];

    t->startPos = Vector2Add(
        center,
        Vector2Scale(Vector2Subtract(t->startPos, center), factor)
        );

    t->endPos = Vector2Add(
        center,
        Vector2Scale(Vector2Subtract(t->endPos, center), factor)
        );

    t->length *= factor;
    t->thick  *= 0.99f;
  }
}

void draw_toothpick(Toothpicks* ts, Toothpicks* ts_temp, Toothpick* t)
{
  Vector2* pos = NULL;
  bool collides_start = false;
  for (size_t i = 0; i < ts->count; i++) {
    if (&ts->items[i] == t) continue;
    if (Vector2Equals(t->startPos, ts->items[i].startPos) || Vector2Equals(t->startPos, ts->items[i].endPos)) {
      collides_start = true;
      break;
    }
  }
  bool collides_end = false;
  for (size_t i = 0; i < ts->count; i++) {
    if (&ts->items[i] == t) continue;
    if (Vector2Equals(t->endPos, ts->items[i].startPos) || Vector2Equals(t->endPos, ts->items[i].endPos)) {
      collides_end = true;
      break;
    }
  }

  if (collides_start && collides_end) return;
  else if (!collides_start && !collides_end) {
    Toothpick t1 = {0};
    Toothpick t2 = {0};

    // Vertical:   t->startPos.y - t->endPos.y < 0
    if (t->startPos.y - t->endPos.y < 0) {
      t1.startPos = Vector2Add(t->startPos, (Vector2){.x = - t->length / 2, .y = 0});
      t1.endPos   = Vector2Add(t->startPos, (Vector2){.x =   t->length / 2, .y = 0});
    } else {
      t1.startPos = Vector2Add(t->startPos, (Vector2){.x = 0, .y = - t->length / 2});
      t1.endPos   = Vector2Add(t->startPos, (Vector2){.x = 0, .y =   t->length / 2});
    }
    t1.length = t->length;
    t1.thick  = t->thick;
    ut_da_push(ts_temp, t1);

    if (t->startPos.y - t->endPos.y < 0) {
      t2.startPos = Vector2Add(t->endPos, (Vector2){.x = - t->length / 2, .y = 0});
      t2.endPos   = Vector2Add(t->endPos, (Vector2){.x =   t->length / 2, .y = 0});
    } else {
      t2.startPos = Vector2Add(t->endPos, (Vector2){.x = 0, .y = - t->length / 2});
      t2.endPos   = Vector2Add(t->endPos, (Vector2){.x = 0, .y =   t->length / 2});
    }
    t2.length = t->length;
    t2.thick  = t->thick;
    ut_da_push(ts_temp, t2);
  } else {
    if (collides_start) pos = &t->endPos;
    else if (collides_end) pos = &t->startPos;
    else abort();

    Toothpick tt = {0};

    // Vertical:   t->startPos.y - t->endPos.y < 0
    if (t->startPos.y - t->endPos.y < 0) {
      tt.startPos = Vector2Add(*pos, (Vector2){.x = - t->length / 2, .y = 0});
      tt.endPos   = Vector2Add(*pos, (Vector2){.x =   t->length / 2, .y = 0});
    } else {
      tt.startPos = Vector2Add(*pos, (Vector2){.x = 0, .y = - t->length / 2});
      tt.endPos   = Vector2Add(*pos, (Vector2){.x = 0, .y =   t->length / 2});
    }
    tt.length = t->length;
    tt.thick  = t->thick;
    ut_da_push(ts_temp, tt);
  }
}

int main(void)
{
  InitWindow(800, 600, "Terrific toothpick patterns");
  SetTargetFPS(60);

  Toothpicks da_toothpicks = {0};

  float thick  = GetScreenWidth() * 0.005f;
  float length = GetScreenHeight() * 0.15f;
  Toothpick origin = {
    .startPos = {
      .x = GetScreenWidth()  / 2.0f,
      .y = GetScreenHeight() / 2.0f - length / 2,
    },
    .endPos = {
      .x = GetScreenWidth()  / 2.0f,
      .y = GetScreenHeight() / 2.0f + length / 2,
    },
    .length  = length,
    .thick   = thick,
    .checked = false,
  };
  ut_da_push(&da_toothpicks, origin);

  while(!WindowShouldClose()) {
    if (IsKeyDown(KEY_RIGHT)) {
      Toothpicks da_temp = {0};
      for (size_t i = 0; i < da_toothpicks.count; i++) {
        if (!da_toothpicks.items[i].checked) {
          draw_toothpick(&da_toothpicks, &da_temp, &da_toothpicks.items[i]);
          da_toothpicks.items[i].checked = true;
        }
      }
      for (size_t i = 0; i < da_temp.count; i++) {
        ut_da_push(&da_toothpicks, da_temp.items[i]);
      }
      ut_da_free(&da_temp);
      scale_toothpicks(&da_toothpicks);
    }
    BeginDrawing();
    {
      ClearBackground(BACKGROUND);
      for (size_t i = 0; i < da_toothpicks.count; i++)
        DrawLineEx(da_toothpicks.items[i].startPos, da_toothpicks.items[i].endPos, da_toothpicks.items[i].thick, RAYWHITE);
    }
    EndDrawing();
  }
  CloseWindow();
  ut_da_free(&da_toothpicks);
  return 0;
}
