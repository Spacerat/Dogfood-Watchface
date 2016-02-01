#include <pebble.h>

static Window *s_main_window;
static TextLayer *s_main_text_layer;
static TextLayer *s_outer_text_layer;
static TextLayer *s_date_text_layer;

static Layer *s_watch_layer;
static GPoint s_centerPoint;
static GRect s_bounds;
int s_minute;
int s_hour;
static GFont s_font_hour;

static GColor s_text_color;
static GColor s_background_color;
static GColor s_hand_color;

#define KEY_BG_COL 1
#define KEY_HAND_COL 2
#define KEY_TEXT_COL 3


static void update_time() {
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);
  static char main_buffer[] = "00";
  static char outer_buffer[] = "00";
  static char date_buffer[] = "00/00/0000";
  s_minute = tick_time->tm_min;
  s_hour = tick_time->tm_hour;

  strftime(main_buffer, sizeof("00"), "%H", tick_time);
  strftime(outer_buffer, sizeof("00"), "%M", tick_time);
  strftime(date_buffer, sizeof("00/00/0000"), "%m/%d/%Y", tick_time);
  
  if (s_main_text_layer && s_outer_text_layer) {
    GPoint main_text_pos = (GPoint) {
      .x = (int16_t) (sin_lookup(TRIG_MAX_ANGLE * s_minute/60) * (int32_t)-30 / TRIG_MAX_RATIO) + s_centerPoint.x - 30,
      .y = (int16_t) (-cos_lookup(TRIG_MAX_ANGLE * s_minute/60) * (int32_t)-30 / TRIG_MAX_RATIO) + s_centerPoint.y - 30,
    };
    GPoint outer_text_pos = (GPoint) {
      .x = (int16_t) (sin_lookup(TRIG_MAX_ANGLE * s_minute/60) * (int32_t)(s_bounds.size.w*0.3+15) / TRIG_MAX_RATIO) + s_centerPoint.x - 15,
      .y = (int16_t) (-cos_lookup(TRIG_MAX_ANGLE * s_minute/60) * (int32_t)(s_bounds.size.w*0.3+15) / TRIG_MAX_RATIO) + s_centerPoint.y - 18,
    };

    layer_set_frame((Layer*)s_main_text_layer, (GRect) {
      .origin =  main_text_pos,
      .size = {60, 60}
    });
    text_layer_set_text(s_main_text_layer, main_buffer);
    layer_set_frame((Layer*)s_outer_text_layer, (GRect) {
      .origin =  outer_text_pos,
      .size = {30, 30}
    });
    text_layer_set_text(s_outer_text_layer, outer_buffer);
    text_layer_set_text(s_date_text_layer, date_buffer);
  }
}

// static void set_app_colors(int bg, int hand, int text) {
//   s_background_color = GColorFromHEX(bg);
//   s_text_color = GColorFromHEX(text);
//   s_hand_color = GColorFromHEX(hand);
// }


static void update_colours() {
  text_layer_set_text_color(s_main_text_layer, s_text_color);
  text_layer_set_text_color(s_outer_text_layer, s_text_color);
  text_layer_set_text_color(s_date_text_layer, s_text_color);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
  
  if (s_watch_layer) {
    layer_mark_dirty(s_watch_layer);
  }
}

static void render_watch(Layer *layer, GContext *ctx) {
  int minutes = s_minute;
  GRect bounds = layer_get_bounds(layer);
  graphics_context_set_fill_color(ctx, s_background_color);
  graphics_fill_rect(ctx, bounds, 0, GCornerNone);
  #ifdef PBL_COLOR
  graphics_context_set_stroke_width(ctx, 5);
  graphics_context_set_antialiased(ctx, true);
  #endif
  graphics_context_set_stroke_color(ctx, s_hand_color);
 
  GPoint hand = (GPoint) {
    .x = (int16_t) (sin_lookup(TRIG_MAX_ANGLE * minutes/60) * (int32_t)s_bounds.size.w*0.3 / TRIG_MAX_RATIO) + s_centerPoint.x,
    .y = (int16_t) (-cos_lookup(TRIG_MAX_ANGLE * minutes/60) * (int32_t)s_bounds.size.w*0.3 / TRIG_MAX_RATIO) + s_centerPoint.y,
  };
  graphics_draw_line(ctx, s_centerPoint, hand);
  #ifdef PBL_ROUND
    graphics_draw_arc(ctx, bounds, GOvalScaleModeFillCircle, 0, TRIG_MAX_ANGLE * minutes / 60);
  #endif
}

static void inbox_received_handler(DictionaryIterator *iter, void *context) {
  #ifdef PBL_COLOR
  Tuple *background_color_t = dict_find(iter, KEY_BG_COL);
  Tuple *hand_color_t = dict_find(iter, KEY_HAND_COL);
  Tuple *text_color_t = dict_find(iter, KEY_TEXT_COL);

  if (background_color_t) {
    int background_color = background_color_t->value->int32;
    persist_write_int(KEY_BG_COL, background_color);
    s_background_color = GColorFromHEX(background_color);
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Set back color %d", background_color);
  }
  if (hand_color_t) {
    int hand_color = hand_color_t->value->int32;
    persist_write_int(KEY_HAND_COL, hand_color);
    s_hand_color = GColorFromHEX(hand_color);
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Set hand color %d", hand_color);
  }
  if (text_color_t) {
    int text_color = text_color_t->value->int32;
    persist_write_int(KEY_TEXT_COL, text_color);
    s_text_color = GColorFromHEX(text_color);
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Set text color %d", text_color);
  }
  #endif
  update_colours();
  if (s_watch_layer) {
    layer_mark_dirty(s_watch_layer);
  }

}



static void main_window_load(Window * window) {
  Layer *window_layer = window_get_root_layer(window);
  s_bounds = layer_get_bounds(window_layer);
  s_centerPoint = grect_center_point(&s_bounds);
  
  s_main_text_layer = text_layer_create(GRect(0, 0, 30, 36));
  s_outer_text_layer = text_layer_create(GRect(0, 0, 30, 36));
  s_date_text_layer = text_layer_create(GRect(0, 0, 150, 30));
  
  text_layer_set_background_color(s_main_text_layer, GColorClear);
  s_font_hour = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_OPENSANS_48));
  text_layer_set_font(s_main_text_layer, s_font_hour);
  text_layer_set_font(s_outer_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24));
  text_layer_set_font(s_date_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24));
  text_layer_set_text_alignment(s_main_text_layer, GTextAlignmentCenter);
  text_layer_set_text_alignment(s_outer_text_layer, GTextAlignmentCenter);
  text_layer_set_text_alignment(s_date_text_layer, GTextAlignmentCenter);
  text_layer_set_background_color(s_main_text_layer, GColorClear);
  text_layer_set_background_color(s_outer_text_layer, GColorClear);
  text_layer_set_background_color(s_date_text_layer, GColorClear);
  text_layer_set_text(s_main_text_layer, "00");
  text_layer_set_text(s_outer_text_layer, "00");
  text_layer_set_text(s_date_text_layer, "00/00/0000");
  
  s_watch_layer = layer_create(s_bounds);
  layer_set_update_proc(s_watch_layer, render_watch);

  layer_add_child(window_layer, s_watch_layer);
  layer_add_child(window_layer, text_layer_get_layer(s_main_text_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_outer_text_layer));
//   layer_add_child(window_layer, text_layer_get_layer(s_date_text_layer));
    
  update_colours();
  update_time();
}

static void main_window_unload(Window * window) {
  text_layer_destroy(s_main_text_layer);
  text_layer_destroy(s_date_text_layer);
  text_layer_destroy(s_outer_text_layer);
  layer_destroy(s_watch_layer);
  fonts_unload_custom_font(s_font_hour);
}

static void init() {
  s_background_color = GColorBlack;
  #ifdef PBL_COLOR
  s_hand_color = GColorVeryLightBlue;
  #else
    s_hand_color = GColorWhite;
  #endif
  s_text_color = GColorWhite;
  
  s_main_window = window_create();
    
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });
  
   window_stack_push(s_main_window, true);

  #ifdef PBL_COLOR
  if (persist_read_int(KEY_BG_COL)) {
    int background_color = persist_read_int(KEY_BG_COL);
    s_background_color = GColorFromHEX(background_color);
  }
  if (persist_read_int(KEY_HAND_COL)) {
    int hand_color = persist_read_int(KEY_HAND_COL);
    s_hand_color = GColorFromHEX(hand_color);
  }
  if (persist_read_int(KEY_TEXT_COL)) {
    int text_color = persist_read_int(KEY_TEXT_COL);
    s_text_color = GColorFromHEX(text_color);
  }
  #endif
  update_colours();
  
  app_message_register_inbox_received(inbox_received_handler);
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());

  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
}

static void deinit() {
  window_destroy(s_main_window);
}


int main(void) {
  init();
  app_event_loop();
  deinit();
}





