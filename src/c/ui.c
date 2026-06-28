#include "ui.h"

#define DISPLAY_WIDTH        144
#define DISPLAY_HEIGHT       168
#define NUM_ZONES            2
#define ZONE_HEIGHT          (DISPLAY_HEIGHT / NUM_ZONES)
#define PROGRESS_BAR_HEIGHT  16

#define ICON_SIZE            24   /* all icons are 24x24 px */
#define ICON_INSET           3

static UIContext *s_ui = NULL;

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

    /* Progress bar */
    uint32_t bar_width = timer_seconds_bar_width(t->elapsed_seconds, DISPLAY_WIDTH);
    if (bar_width > 0) {
      graphics_context_set_fill_color(ctx, GColorBlack);
      graphics_fill_rect(ctx,
                         GRect(0, zone_y + ZONE_HEIGHT - PROGRESS_BAR_HEIGHT,
                               bar_width, PROGRESS_BAR_HEIGHT),
                         0, GCornerNone);
    }

    /* Play / pause icon (top-left; blank when timer is at 0:00) */
    GRect play_rect = GRect(ICON_INSET, zone_y + ICON_INSET, ICON_SIZE, ICON_SIZE);
    if (t->elapsed_seconds > 0) {
      GBitmap *bmp = t->running ? s_ui->bmp_play : s_ui->bmp_pause;
      graphics_draw_bitmap_in_rect(ctx, bmp, play_rect);
    }

    /* Vibe icon (top-right) */
    bool is_assigned = ((i == 0) && (va == VIBE_TIMER1)) ||
                       ((i == 1) && (va == VIBE_TIMER2));
    GBitmap *vibe_bmp = is_assigned ? s_ui->bmp_vibe_on : s_ui->bmp_vibe_off;
    GRect vibe_rect = GRect(DISPLAY_WIDTH - ICON_INSET - ICON_SIZE,
                            zone_y + ICON_INSET,
                            ICON_SIZE, ICON_SIZE);
    graphics_draw_bitmap_in_rect(ctx, vibe_bmp, vibe_rect);
  }
}

void ui_init(UIContext *ctx,
             TimerState *timer1, TimerState *timer2,
             VibeAssignment *vibe_assignment) {
  s_ui = ctx;

  ctx->timers[0] = timer1;
  ctx->timers[1] = timer2;
  ctx->vibe_assignment = vibe_assignment;

  ctx->bmp_play     = gbitmap_create_with_resource(RESOURCE_ID_ICON_PLAY);
  ctx->bmp_pause    = gbitmap_create_with_resource(RESOURCE_ID_ICON_PAUSE);
  ctx->bmp_vibe_on  = gbitmap_create_with_resource(RESOURCE_ID_ICON_VIBE_ON);
  ctx->bmp_vibe_off = gbitmap_create_with_resource(RESOURCE_ID_ICON_VIBE_OFF);

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
  if (ctx->bmp_play)     { gbitmap_destroy(ctx->bmp_play);     ctx->bmp_play     = NULL; }
  if (ctx->bmp_pause)    { gbitmap_destroy(ctx->bmp_pause);    ctx->bmp_pause    = NULL; }
  if (ctx->bmp_vibe_on)  { gbitmap_destroy(ctx->bmp_vibe_on);  ctx->bmp_vibe_on  = NULL; }
  if (ctx->bmp_vibe_off) { gbitmap_destroy(ctx->bmp_vibe_off); ctx->bmp_vibe_off = NULL; }

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
