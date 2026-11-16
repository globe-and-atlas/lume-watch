#include <pebble.h>
#include "config.h"
#include "policy.h"
#include "digits.h"
enum{P_PALETTE=1,P_BG,P_FG,P_ACCENT,P_TIME,P_DATE,P_SECONDS,P_DIGITAL,P_BATTERY};
typedef struct{int palette,bg,fg,accent,time,date,seconds,digital,battery;}Settings;
static Settings s={0,0,0xFFFFAA,0xAAAAAA,0,1,0,0,1};
static const uint32_t PALETTES[6][3]={{0,0xFFFFAA,0xAAAAAA},{0xFFFFFF,0,0x555555},{0x005500,0xFFFFAA,0xAAFFAA},{0xFFFFAA,0,0xAA0000},{0x000055,0xFFFFFF,0x55FFFF},{0x550000,0xFFFFAA,0xFFAA55}};
static Window *window;static Layer *base,*seconds_layer,*hands;static struct tm current;static int charge;
#if LUME_FIXTURE
static int fh=LUME_HOUR,fm=LUME_MINUTE,fs=LUME_SECOND,fb=LUME_BATTERY;static Calendar fd={LUME_YEAR,LUME_MONTH,LUME_DAY};
#endif
static uint32_t color(int i){return s.palette<6?PALETTES[s.palette][i]:(uint32_t)(i==0?s.bg:i==1?s.fg:s.accent);}
static uint32_t dim(void){return mix_color(color(0),color(1),1);}
static GPoint radial(int x,int y,int angle,int radius){int32_t a=TRIG_MAX_ANGLE*angle/3600;return GPoint(x+(int32_t)sin_lookup(a)*radius/TRIG_MAX_RATIO,y-(int32_t)cos_lookup(a)*radius/TRIG_MAX_RATIO);}
static void rotated(GContext *ctx,const GPoint *local,int count,int x,int y,int angle,uint32_t fill){
 int32_t a=TRIG_MAX_ANGLE*angle/3600, sn=sin_lookup(a),cs=cos_lookup(a);GPoint world[9];
 for(int i=0;i<count;i++){int32_t rx=(int32_t)local[i].x*cs-(int32_t)local[i].y*sn,ry=(int32_t)local[i].x*sn+(int32_t)local[i].y*cs;world[i]=GPoint(x+rx/TRIG_MAX_RATIO,y+ry/TRIG_MAX_RATIO);}
 GPathInfo info={count,world};GPath *path=gpath_create(&info);graphics_context_set_fill_color(ctx,GColorFromHEX(fill));gpath_draw_filled(ctx,path);gpath_destroy(path);
}
static void label(GContext *ctx,const char *value,GRect box,const char *font,GTextAlignment alignment,uint32_t ink){graphics_context_set_text_color(ctx,GColorFromHEX(ink));graphics_draw_text(ctx,value,fonts_get_system_font(font),box,GTextOverflowModeFill,alignment,NULL);}
static void digit(GContext *ctx,int n,int x,int y,uint32_t ink){graphics_context_set_fill_color(ctx,GColorFromHEX(ink));for(int row=0;row<DIGIT_H;row++)for(int col=0;col<DIGIT_W;col++)if(DIGITS[n][row] & (1u<<col))graphics_fill_rect(ctx,GRect(x+col,y+row,1,1),0,GCornerNone);}
static void numeral(GContext *ctx,int n,int x,int y){digit(ctx,n,x+1,y+1,dim());digit(ctx,n,x,y,color(1));}
#if defined(PBL_ROUND)
#include "round_analog.h"
static void round_numeral(GContext *ctx,int value,int x,int y,int w,int h){int ww=rr(w),hh=rr(h);for(int row=0;row<hh;row++)for(int col=0;col<ww;col++)if(DIGITS[value][row*DIGIT_H/hh] & (1u<<(col*DIGIT_W/ww))){rcheck(rr(x)+col,rr(y)+row,1,1);graphics_fill_rect(ctx,GRect(rr(x)+col,rr(y)+row,1,1),0,GCornerNone);}}
static RoundData round_state(void){RoundData d={0};d.now=current;d.bg=color(0);d.fg=color(1);d.accent=color(2);d.battery=charge;d.time_mode=s.time;d.date_mode=s.date;d.seconds=s.seconds;d.digital=s.digital;d.show_battery=s.battery;return d;}
#endif
static void draw_base(Layer *own,GContext *ctx){
#if defined(PBL_ROUND)
 {(void)own;RoundData d=round_state();round_analog_base(ctx,&d,2,round_numeral);rproof_marker(ctx);return;}
#endif

 graphics_context_set_antialiased(ctx,false);graphics_context_set_fill_color(ctx,GColorFromHEX(color(0)));graphics_fill_rect(ctx,layer_get_bounds(own),0,GCornerNone);
 const GPoint marker[]={{-3,-94},{3,-94},{3,-77},{-3,-77}};
 for(int h=0;h<12;h++)if(h%3)rotated(ctx,marker,4,100,106,h*300,color(1));
 numeral(ctx,1,73,8);numeral(ctx,2,101,8);numeral(ctx,9,11,88);numeral(ctx,3,161,88);numeral(ctx,6,86,168);
 if(s.date==1){char day[8];snprintf(day,sizeof(day),"%02d",current.tm_mday);graphics_context_set_fill_color(ctx,GColorFromHEX(color(1)));graphics_fill_rect(ctx,GRect(154,153,29,25),1,GCornersAll);label(ctx,day,GRect(154,153,29,25),FONT_KEY_GOTHIC_24_BOLD,GTextAlignmentCenter,color(0));}
 char date[32]="",digital[24]="";
 if(s.date==2)snprintf(date,sizeof(date),"%04d-%02d-%02d",current.tm_year+1900,current.tm_mon+1,current.tm_mday);
 if(s.date==3)snprintf(date,sizeof(date),"%02d/%02d/%04d",current.tm_mon+1,current.tm_mday,current.tm_year+1900);
 if(s.date==4)snprintf(date,sizeof(date),"%02d/%02d/%04d",current.tm_mday,current.tm_mon+1,current.tm_year+1900);
 if(s.digital){int h=display_hour(current.tm_hour,s.time,clock_is_24h_style());snprintf(digital,sizeof(digital),uses24(s.time,clock_is_24h_style())?"%02d:%02d":"%02d:%02d%s",h,current.tm_min,current.tm_hour>=12?"PM":"AM");}
 label(ctx,digital,GRect(1,200,84,16),FONT_KEY_GOTHIC_14_BOLD,GTextAlignmentCenter,color(1));label(ctx,date,GRect(s.digital?85:35,200,s.digital?114:130,16),FONT_KEY_GOTHIC_14_BOLD,GTextAlignmentCenter,color(1));
 if(s.battery){char value[12];snprintf(value,sizeof(value),"%d%%",charge);graphics_context_set_stroke_color(ctx,GColorFromHEX(dim()));graphics_draw_round_rect(ctx,GRect(76,219,16,7),1);graphics_context_set_fill_color(ctx,GColorFromHEX(dim()));graphics_fill_rect(ctx,GRect(78,221,12*charge/100,3),0,GCornerNone);label(ctx,value,GRect(94,214,34,14),FONT_KEY_GOTHIC_14,GTextAlignmentLeft,dim());}
}
static void position_seconds(void){
#if defined(PBL_ROUND)
 {GPoint p=rpolar(90,90,current.tm_sec*60,63);int cx=rr(90),cy=rr(90),left=p.x<cx?p.x-2:cx-2,top=p.y<cy?p.y-2:cy-2,right=p.x>cx?p.x+3:cx+3,bottom=p.y>cy?p.y+3:cy+3;layer_set_frame(seconds_layer,GRect(left,top,right-left,bottom-top));return;}
#endif
GPoint tip=radial(100,106,second_angle(current.tm_sec),76);int x=tip.x<96?tip.x-2:96,y=tip.y<102?tip.y-2:102;int right=tip.x>104?tip.x+3:104,bottom=tip.y>110?tip.y+3:110;layer_set_frame(seconds_layer,GRect(x,y,right-x,bottom-y));}
static void draw_seconds(Layer *own,GContext *ctx){
#if defined(PBL_ROUND)
 {(void)own;RoundData d=round_state();GRect f=layer_get_frame(own);GPoint tip=rpolar(90,90,d.now.tm_sec*60,63);graphics_context_set_stroke_color(ctx,GColorFromHEX(d.accent));graphics_context_set_stroke_width(ctx,1);graphics_draw_line(ctx,GPoint(rr(90)-f.origin.x,rr(90)-f.origin.y),GPoint(tip.x-f.origin.x,tip.y-f.origin.y));return;}
#endif
GRect frame=layer_get_frame(own);GPoint tip=radial(100,106,second_angle(current.tm_sec),76);graphics_context_set_antialiased(ctx,false);graphics_context_set_stroke_color(ctx,GColorFromHEX(color(2)));graphics_context_set_stroke_width(ctx,1);graphics_draw_line(ctx,GPoint(100-frame.origin.x,106-frame.origin.y),GPoint(tip.x-frame.origin.x,tip.y-frame.origin.y));}
static void draw_hand(GContext *ctx,int angle,int length,int width){
 GPoint outer[]={{0,-length},{width,-length+9},{width,5},{0,10},{-width,5},{-width,-length+9}};rotated(ctx,outer,6,100,106,angle,dim());
 GPoint inner[]={{0,-length+2},{width-1,-length+10},{width-1,5},{0,8},{1-width,5},{1-width,-length+10}};rotated(ctx,inner,6,100,106,angle,color(1));
}
static void draw_hands(Layer *own,GContext *ctx){
#if defined(PBL_ROUND)
 {(void)own;RoundData d=round_state();round_analog_hands(ctx,&d,2);rproof_marker(ctx);return;}
#endif
(void)own;graphics_context_set_antialiased(ctx,false);draw_hand(ctx,hour_angle(current.tm_hour,current.tm_min),58,6);draw_hand(ctx,minute_angle(current.tm_min),82,4);graphics_context_set_fill_color(ctx,GColorFromHEX(color(1)));graphics_fill_circle(ctx,GPoint(100,106),7);graphics_context_set_fill_color(ctx,GColorFromHEX(color(0)));graphics_fill_circle(ctx,GPoint(100,106),2);}
static void tick(struct tm *now,TimeUnits changed){current=*now;
#if LUME_FIXTURE
 current.tm_hour=fh;current.tm_min=fm;current.tm_sec=fs;current.tm_year=fd.year-1900;current.tm_mon=fd.month-1;current.tm_mday=fd.day;
#endif
 if(s.seconds){position_seconds();layer_mark_dirty(seconds_layer);}
 if(changed&MINUTE_UNIT){layer_mark_dirty(base);layer_mark_dirty(hands);}
}
static void battery_changed(BatteryChargeState state){charge=state.charge_percent;
#if LUME_FIXTURE
 charge=fb;
#endif
 layer_mark_dirty(base);
}
static void subscription(void){tick_timer_service_unsubscribe();tick_timer_service_subscribe(s.seconds?SECOND_UNIT:MINUTE_UNIT,tick);}
static void setting(DictionaryIterator *iter,uint32_t key,int persistent,int maximum,int *target){Tuple *t=dict_find(iter,key);if(t && (t->type==TUPLE_INT || t->type==TUPLE_UINT)){int v=t->value->int32;if(v>=0 && v<=maximum){*target=v;persist_write_int(persistent,v);}}}
#if LUME_FIXTURE
static void fixture(DictionaryIterator *iter,int key,int maximum,int *target){Tuple *t=dict_find(iter,key);if(t && t->type==TUPLE_INT && t->value->int32>=0 && t->value->int32<=maximum)*target=t->value->int32;}
#endif
static void inbox(DictionaryIterator *iter,void *context){
#if defined(PBL_ROUND) && (LUME_FIXTURE)
 Tuple *round_proof=dict_find(iter,11022);if(round_proof && round_proof->type==TUPLE_INT){r_proof=round_proof->value->int32==1;layer_mark_dirty(base);}
#endif
(void)context;int old=s.seconds;
 setting(iter,MESSAGE_KEY_Palette,P_PALETTE,6,&s.palette);setting(iter,MESSAGE_KEY_Background,P_BG,0xFFFFFF,&s.bg);setting(iter,MESSAGE_KEY_Foreground,P_FG,0xFFFFFF,&s.fg);setting(iter,MESSAGE_KEY_Accent,P_ACCENT,0xFFFFFF,&s.accent);setting(iter,MESSAGE_KEY_TimeFormat,P_TIME,2,&s.time);setting(iter,MESSAGE_KEY_DateFormat,P_DATE,4,&s.date);setting(iter,MESSAGE_KEY_ShowSeconds,P_SECONDS,1,&s.seconds);setting(iter,MESSAGE_KEY_DigitalTime,P_DIGITAL,1,&s.digital);setting(iter,MESSAGE_KEY_ShowBattery,P_BATTERY,1,&s.battery);
#if LUME_FIXTURE
 fixture(iter,11000,23,&fh);fixture(iter,11001,59,&fm);fixture(iter,11002,59,&fs);fixture(iter,11004,100,&fb);Calendar candidate=fd;fixture(iter,11005,2099,&candidate.year);fixture(iter,11006,12,&candidate.month);fixture(iter,11007,31,&candidate.day);if(valid_date(candidate))fd=candidate;charge=fb;
#endif
 if(old!=s.seconds)subscription();
 layer_set_hidden(seconds_layer,!s.seconds);time_t now=time(NULL);tick(localtime(&now),MINUTE_UNIT);layer_mark_dirty(hands);
}
static void restore(int key,int maximum,int *target){if(persist_exists(key)){int value=persist_read_int(key);if(value>=0 && value<=maximum)*target=value;}}
static void init(void){restore(P_PALETTE,6,&s.palette);restore(P_BG,0xFFFFFF,&s.bg);restore(P_FG,0xFFFFFF,&s.fg);restore(P_ACCENT,0xFFFFFF,&s.accent);restore(P_TIME,2,&s.time);restore(P_DATE,4,&s.date);restore(P_SECONDS,1,&s.seconds);restore(P_DIGITAL,1,&s.digital);restore(P_BATTERY,1,&s.battery);
 window=window_create();Layer *root=window_get_root_layer(window);base=layer_create(PBL_IF_ROUND_ELSE(GRect(0,0,PBL_DISPLAY_WIDTH,PBL_DISPLAY_HEIGHT),GRect(0,0,200,228)));layer_set_update_proc(base,draw_base);layer_add_child(root,base);seconds_layer=layer_create(PBL_IF_ROUND_ELSE(GRect(0,0,PBL_DISPLAY_WIDTH,PBL_DISPLAY_HEIGHT),GRect(96,102,8,8)));layer_set_update_proc(seconds_layer,draw_seconds);layer_add_child(root,seconds_layer);layer_set_hidden(seconds_layer,!s.seconds);hands=layer_create(PBL_IF_ROUND_ELSE(GRect(0,0,PBL_DISPLAY_WIDTH,PBL_DISPLAY_HEIGHT),GRect(0,0,200,228)));layer_set_update_proc(hands,draw_hands);layer_add_child(root,hands);window_stack_push(window,true);time_t now=time(NULL);tick(localtime(&now),MINUTE_UNIT);battery_changed(battery_state_service_peek());subscription();battery_state_service_subscribe(battery_changed);app_message_register_inbox_received(inbox);app_message_open(384,64);
}
static void deinit(void){tick_timer_service_unsubscribe();battery_state_service_unsubscribe();app_message_deregister_callbacks();layer_destroy(hands);layer_destroy(seconds_layer);layer_destroy(base);window_destroy(window);}
int main(void){init();app_event_loop();deinit();}
