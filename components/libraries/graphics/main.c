#include <IdealOS.c>


typedef struct {
	VESA_Mode*         VESA;
	Heap_Interface*    heap;
	Process_Interface* process;
}
Module;

Module _module = {0};


void main(Process* process);


void start(Process* process)
{
	get_module_address_by_function(start);
	Module* module = global_ptr(_module);

	module->VESA = process->get(global_ptr("VESA"));
	module->heap = process->get(global_ptr("heap"));
	module->process = process->get(global_ptr("process"));

	main(process);
}


Frame* create_frame(Number width, Number height)
{
	get_module_address_by_function(create_frame);
	Module* module = global_ptr(_module);


	Frame* frame;

	frame = module->heap->allocate(sizeof(Frame) + width * height * sizeof(frame->buffer[0]), 0);

	frame->scroll_x = 0;
	frame->scroll_y = 0;
	frame->width    = width;
	frame->height   = height;

	return frame;
}


void clean_frame(Frame* frame, Number32 color)
{
	Number    x;
	Number    y;
	Number32* line;

	line = frame->buffer;

	for(y = 0; y < frame->height; ++y) {
		for(x = 0; x < frame->width; ++x) {
			line[x] = color;
		}

		line += frame->width;
	}
}


void draw_rectangle(Frame* frame, Number x, Number y, Number width, Number height, Number32 color)
{
	Number    i;
	Number    j;
	Number32* line;

	if(x >= frame->width || y >= frame->height) {
		return;
	}

	if(x + width >= frame->width) {
		width = frame->width - x;
	}

	if(y + height >= frame->height) {
		height = frame->height - y;
	}

	line = frame->buffer + y * frame->width + x;

	for(i = 0; i < height; ++i) {
		for(j = 0; j < width; ++j) {
			line[j] = color;
		}

		line += frame->width;
	}
}


void draw_frame_in_frame(Frame* destination_frame, Frame *source_frame, Number x, Number y)
{
	Number    i;
	Number    j;
	Number32* source_line;
	Number32* destination_line;

	Number width;
	Number height;

	if(x >= destination_frame->width || y >= destination_frame->height) {
		return;
	}

	width = source_frame->width;
	height = source_frame->height;

	if(x + width >= destination_frame->width) {
		width = destination_frame->width - x;
	}

	if(y + height >= destination_frame->height) {
		height = destination_frame->height - y;
	}

	source_line = source_frame->buffer;
	destination_line = destination_frame->buffer + y * destination_frame->width + x;

	for(i = 0; i < height; ++i) {
		for(j = 0; j < width; ++j) {
			destination_line[j] = source_line[j];
		}

		destination_line += destination_frame->width;
		source_line += source_frame->width;
	}
}


void draw_frame(Frame* frame, Number x, Number y)
{
	get_module_address_by_function(draw_frame);
	Module* module = global_ptr(_module);


	Number    i;
	Number    j;
	Number32* source_line;

	Number width;
	Number height;

	if(x >= 640 || y >= 480) {
		return;
	}

	width = frame->width;
	height = frame->height;

	if(x + width >= 640) {
		width = 640 - x;
	}

	if(y + height >= 480) {
		height = 480 - y;
	}

	source_line = frame->buffer;

	if(module->VESA->bpp == 24) {
		Byte* destination_line;

		destination_line = module->VESA->framebuffer + y * module->VESA->pitch + x * 3;

		for(i = 0; i < height; ++i) {
			for(j = 0; j < width; ++j) {
				*(Number32*)(destination_line + j * 3) = source_line[j];
			}

			source_line += frame->width;
			destination_line += module->VESA->pitch;
		}
	}
	else if(module->VESA->bpp == 32) {
		Byte* destination_line;

		destination_line = module->VESA->framebuffer + y * module->VESA->pitch + x * 4;

		for(i = 0; i < height; ++i) {
			for(j = 0; j < width; ++j) {
				*(Number32*)(destination_line + j * 4) = source_line[j];
			}

			source_line += frame->width;
			destination_line += module->VESA->pitch;
		}
	}
}


#include "font.c"


void draw_character(Frame* frame, Number x, Number y, Number character, Number32 color)
{
	get_module_address_by_function(draw_character);


	Number i;
	Number j;
	Byte*  glyph;
	Byte   glyph_color;
	Number r;
	Number g;
	Number b;

	Number32* line;
	Number    width;
	Number    height;

	if(x >= frame->width || y >= frame->height) {
		return;
	}

	width = 8;

	if(x + width >= frame->width) {
		width = frame->width - x;
	}

	height = 16;

	if(y + height >= frame->height) {
		height = frame->height - y;
	}

	b = ((color >> 0) & 0xFF) + 1;
	g = ((color >> 8) & 0xFF) + 1;
	r = ((color >> 16) & 0xFF) + 1;
	
	//Byte(*glyphs)[95][128] = global_ptr(glyphs_8_16);
	//glyph = (*glyphs)[character - ' '];
	glyph = (*(Byte(*)[95][128])global_ptr(glyphs_8_16))[character - ' '];

	line = frame->buffer + y * frame->width + x;

	for(i = 0; i < height; ++i) {
		for(j = 0; j < width; ++j) {
			glyph_color = glyph[i * 8 + j];
			//line[j] = rgb(glyph_color, glyph_color, glyph_color);//rgb(glyph_color * r, glyph_color * g, glyph_color * b);
			if(glyph_color) {
				line[j] = rgb((glyph_color * r) / 256, (glyph_color * g) / 256, (glyph_color * b) / 256);
			}
		}

		line += frame->width;
	}
}


#include "text-display.c"


Graphics_Interface _interface;


void main(Process* process)
{
	get_module_address_by_function(main);
	Module* module = global_ptr(_module);


	//if(process->get(global_ptr("graphics"))) {
	//	return;
	//}


	Graphics_Interface* interface = global_ptr(_interface);

	
	interface->create_frame = global_ptr(create_frame);
	interface->clean_frame = global_ptr(clean_frame);
	interface->draw_frame = global_ptr(draw_frame);
	interface->draw_frame_in_frame = global_ptr(draw_frame_in_frame);
	interface->draw_rectangle = global_ptr(draw_rectangle);
	interface->draw_character = global_ptr(draw_character);
	interface->print = global_ptr(print);

	process->create(global_ptr("graphics"), interface);

	for(;;) {
		module->process->wait(0);
	}
}