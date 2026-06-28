#include "ui.h"

#define DISPLAY_WIDTH        144
#define DISPLAY_HEIGHT       168
#define NUM_ZONES            2
#define ZONE_HEIGHT          (DISPLAY_HEIGHT / NUM_ZONES)
#define PROGRESS_BAR_HEIGHT  16

#define ICON_INSET           3
#define PLAY_W               16    /* ~30% bigger than original 12 */
#define PLAY_H               16
#define PAUSE_BAR_W          4
#define PAUSE_BAR_GAP        3

#define VIBE_ICON_W          32
#define VIBE_ICON_H          20
#define VIBE_CIRCLE_R        5
#define VIBE_ARC_INNER_R     9
#define VIBE_ARC_OUTER_R     13
#define VIBE_ARC_HALF_SPAN   45    /* degrees of arc on each side */

static UIContext *s_ui = NULL;

static void draw_play_icon(GContext *ctx, int x, int y) {
  GPoint points[3] = {
    GPoint(x, y),
    GPoint(x, y + PLAY_H - 1),
    GPoint(x + PLAY_W - 1, y + (PLAY_H - 1) / 2)
  };
  GPathInfo info = { .num_points = 3, .points = points };
  GPath *path = gpath_create(&info);
  gpath_draw_filled(ctx, path);
  gpath_destroy(path);
}

static void draw_pause_icon(GContext *ctx, int x, int y) {
  GRect bar1 = GRect(x, y, PAUSE_BAR_W, PLAY_H - 1);
  GRect bar2 = GRect(x + PAUSE_BAR_W + PAUSE_BAR_GAP, y, PAUSE_BAR_W, PLAY_H - 1);
  graphics_fill_rect(ctx, bar1, 0, GCornerNone);
  graphics_fill_rect(ctx, bar2, 0, GCornerNone);
}

static void draw_vibe_icon(GContext *ctx, int zone_y, bool is_assigned) {
  int vibe_x = DISPLAY_WIDTH - ICON_INSET - VIBE_ICON_W;
  int vibe_y = zone_y + ICON_INSET;
  int cx = vibe_x + VIBE_ICON_W / 2;
  int cy = vibe_y + VIBE_ICON_H / 2;

  graphics_context_set_stroke_color(ctx, GColorBlack);
  graphics_context_set_stroke_width(ctx, 2);
  graphics_draw_circle(ctx, GPoint(cx, cy), VIBE_CIRCLE_R);

  if (is_assigned) {
    int radii[2] = { VIBE_ARC_INNER_R, VIBE_ARC_OUTER_R };
    for (int i = 0; i < 2; i++) {
      int r = radii[i];
      GRect arc_rect = GRect(cx - r, cy - r, 2 * r, 2 * r);
      graphics_draw_arc(ctx, arc_rect, GOvalScaleModeFillCircle,
                        DEG_TO_TRIGANGLE(270 - VIBE_ARC_HALF_SPAN),
                        DEG_TO_TRIGANGLE(270 + VIBE_ARC_HALF_SPAN));
      graphics_draw_arc(ctx, arc_rect, GOvalScaleModeFillCircle,
                        DEG_TO_TRIGANGLE(90 - VIBE_ARC_HALF_SPAN),
                        DEG_TO_TRIGANGLE(90 + VIBE_ARC_HALF_SPAN));
    }
  }
  graphics_context_set_stroke_width(ctx, 1);
}

static void root_update_proc(Layer *layer, GContext *ctx) {
  (void)layer;
  if (s_ui == NULL) return;

  GRect bounds = layer_get_bounds(layer);
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_rect(ctx, bounds, 0, GCornerNone);

  graphics_context_set_stroke_color(ctx, GColorBlack);
  graphics_draw_line(ctx,
                     GPoint(0, ZONE_HEIGHT),
                     GPoint(DISPLAY_WIDTH - 1, ZONE_HEIGHT));

  VibeAssignment va = *s_ui->vibe_assignment;

  for (int i = 0; i < NUM_ZONES; i++) {
    int zone_y = i * ZONE_HEIGHT;
    TimerState *t = s_ui->timers[i];
    graphics_context_set_fill_color(ctx, GColorBlack);

    uint32_t bar_width = timer_seconds_bar_width(t->elapsed_seconds, DISPLAY_WIDTH);
    if (bar_width > 0) {
      GRect bar_rect = GRect(0,
                             zone_y + ZONE_HEIGHT - PROGRESS_BAR_HEIGHT,
                             bar_width,
                             PROGRESS_BAR_HEIGHT);
      graphics_fill_rect(ctx, bar_rect, 0, GCornerNone);
    }

    int icon_x = ICON_INSET;
    int icon_y = zone_y + ICON_INSET;
    if (t->elapsed_seconds == 0) {
      /* no play/pause indicator when timer is at zero */
    } else if (t->running) {
      draw_play_icon(ctx, icon_x, icon_y);
    } else {
      draw_pause_icon(ctx, icon_x, icon_y);
    }

    bool is_assigned = ((i == 0) && (va == VIBE_TIMER1)) ||
                       ((i == 1) && (va == VIBE_TIMER2));
    draw_vibe_icon(ctx, zone_y, is_assigned);
  }
}

void ui_init(UIContext *ctx,
             TimerState *timer1, TimerState *timer2,
             VibeAssignment *vibe_assignment) {
  s_ui = ctx;

  ctx->timers[0] = timer1;
  ctx->timers[1] = timer2;
  ctx->vibe_assignment = vibe_assignment;

  ctx->main_window = window_create();
  ctx->root_layer = window_get_root_layer(ctx->main_window);
  layer_set_update_proc(ctx->root_layer, root_update_proc);

  GFont font = fonts_get_system_font(FONT_KEY_ROBOTO_BOLD_SUBSET_49);

  for (int i = 0; i < 2; i++) {
    int zone_y = i * ZONE_HEIGHT;
    GRect frame = GRect(0,
                        zone_y + 8,
                        DISPLAY_WIDTH,
                        ZONE_HEIGHT - PROGRESS_BAR_HEIGHT - 16);
    ctx->minute_text[i] = text_layer_create(frame);
    text_layer_set_font(ctx->minute_text[i], font);
    text_layer_set_text_alignment(ctx->minute_text[i], GTextAlignmentCenter);
    text_layer_set_background_color(ctx->minute_text[i], GColorClear);
    text_layer_set_text_color(ctx->minute_text[i], GColorBlack);
    text_layer_set_text(ctx->minute_text[i], "0");
    layer_add_child(ctx->root_layer, text_layer_get_layer(ctx->minute_text[i]));
  }
}

void ui_push(UIContext *ctx) {
  window_stack_push(ctx->main_window, true);
}

void ui_refresh(UIContext *ctx) {
  for (int i = 0; i < 2; i++) {
    uint32_t m = timer_minutes(ctx->timers[i]);
    snprintf(ctx->minute_buf[i], sizeof(ctx->minute_buf[i]), "%u", (unsigned)m);
    text_layer_set_text(ctx->minute_text[i], ctx->minute_buf[i]);
  }
  layer_mark_dirty(ctx->root_layer);
}

void ui_destroy(UIContext *ctx) {
  for (int i = 0; i < 2; i++) {
    if (ctx->minute_text[i]) {
      text_layer_destroy(ctx->minute_text[i]);
      ctx->minute_text[i] = NULL;
    }
  }
  if (ctx->main_window) {
    window_destroy(ctx->main_window);
    ctx->main_window = NULL;
  }
  s_ui = NULL;
}
