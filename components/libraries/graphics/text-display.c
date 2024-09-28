#include <writer.c>


void scroll_down(Frame* frame)
{
	Number x;
	Number y;
	Number32* line1;
	Number32* line2;

	line1 = frame->buffer + y * frame->width;
	line2 = frame->buffer + (y + 16) * frame->width;

	for(y = 0; y < frame->height - 16; ++y) {
		for(x = 0; x < frame->width; ++x) {
			line1[x] = line2[x];
		}

		line1 += frame->width;
		line2 += frame->width;
	}

	for(; y < frame->height; ++y) {
		for(x = 0; x < frame->width; ++x) {
			line1[x] = 0x000000;
		}

		line1 += frame->width;
	}
}


void print_character(Printer* printer, Number character)
{
	switch(character) {
		case '\n': {
			printer->cursor_pos_x = 0;
			++printer->cursor_pos_y;
			break;
		}
		
		case '\r': {
			printer->cursor_pos_x = 0;
			break;
		}
		
		case '\t': {
			print_character(printer, ' ');
			print_character(printer, ' ');
			print_character(printer, ' ');
			print_character(printer, ' ');
			break;
		}
		
		default: {
			draw_character(printer->frame, printer->cursor_pos_x * 8, printer->cursor_pos_y * 16, character, printer->color);
			++printer->cursor_pos_x;
		}
	}
	
	if(printer->cursor_pos_x >= printer->frame->width / 8) {
		printer->cursor_pos_x = 0;
		++printer->cursor_pos_y;
	}
	
	if(printer->cursor_pos_y >= printer->frame->height / 16) {
		--printer->cursor_pos_y;
		
		scroll_down(printer->frame);
	}
}

Signed_Number print_characters(Printer* printer, Byte* characters, Number number_of_characters)
{
	Number i;

	for(i = 0; i < number_of_characters; ++i) {
		print_character(printer, characters[i]);
	}

	return number_of_characters;
}


void print(Printer* printer, Byte* parameters, ...)
{
	get_process_address;


	Writer writer = {
		.source = printer,
		.write_bytes = get_global(print_characters)
	};

	write(&writer, parameters, &parameters + 1);

	//draw_frame(printer->frame, 0, 0);
}