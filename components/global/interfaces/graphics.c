#define rgb(r, g, b) ((r << 16) | (g << 8) | (b))


typedef struct {
	Number   scroll_x;
	Number   scroll_y;
	Number   width;
	Number   height;
	Number32 buffer[];
}
Frame;


typedef struct {
	Frame*   frame;
	Number   cursor_pos_x;
	Number   cursor_pos_y;
	Number32 color;
}
Printer;


typedef struct {
	Number16 attributes;
	Number8  window_a;
	Number8  window_b;
	Number16 granularity;
	Number16 window_size;
	Number16 segment_a;
	Number16 segment_b;
	Number32 win_func_ptr;
	Number16 pitch;
	Number16 width;
	Number16 height;
	Number8  w_char;
	Number8  y_char;
	Number8  planes;
	Number8  bpp;
	Number8  banks;
	Number8  memory_model;
	Number8  bank_size;
	Number8  image_pages;
	Number8  reserved0;
 
	Number8  red_mask;
	Number8  red_position;
	Number8  green_mask;
	Number8  green_position;
	Number8  blue_mask;
	Number8  blue_position;
	Number8  reserved_mask;
	Number8  reserved_position;
	Number8  direct_color_attributes;
 
	Number32 framebuffer;
	Number32 off_screen_mem_off;
	Number16 off_screen_mem_size;
	Number8  reserved1[206];
}
VESA_Mode;


typedef struct {
	Frame* (*create_frame)        (Number width, Number height);
	void   (*clean_frame)         (Frame* frame, Number32 color);
	void   (*draw_rectangle)      (Frame* frame, Number x, Number y, Number width, Number height, Number32 color);
	void   (*draw_frame_in_frame) (Frame* destination_frame, Frame *source_frame, Number x, Number y);
	void   (*draw_frame)          (Frame* frame, Number x, Number y);

	void   (*draw_character)      (Frame* frame, Number x, Number y, Number character, Number32 color);
	void   (*print)               (Printer* printer, Byte* parameters, ...);
}
Graphics_Interface;