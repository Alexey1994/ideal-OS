#include <IdealOS.c>


void start(Process* process)
{
	get_process_address;


	Graphics_Interface* graphics = process->get(get_global("graphics"));
	Process_Interface* process_interface = process->get(get_global("process"));


	Frame* frame;

	Byte p1 = 16 * 8;
	Byte p2 = 8 * 8;
	Byte p3 = 0 * 8;

	Number t;

	frame = graphics->create_frame(50, 64);

	for(t = 0; ; ++t) {
		graphics->clean_frame(frame, 0x000000);

		if((t + 16) % 32 < 16) {
			p1 += 8;
		}
		else {
			p1 -= 8;
		}

		if((t + 8) % 32 < 16) {
			p2 += 8;
		}
		else {
			p2 -= 8;
		}

		if((t + 0) % 32 < 16) {
			p3 += 8;
		}
		else {
			p3 -= 8;
		}

		graphics->draw_rectangle(frame, 0, 32 - (8 + p1 / 8), 10, (8 + p1 / 8) * 2, 0xFFFFFF);
		graphics->draw_rectangle(frame, 20, 32 - (8 + p2 / 8), 10, (8 + p2 / 8) * 2, 0xFFFFFF);
		graphics->draw_rectangle(frame, 40, 32 - (8 + p3 / 8), 10, (8 + p3 / 8) * 2, 0xFFFFFF);

		graphics->draw_frame(frame, 320 - frame->width / 2, 240 - frame->height / 2);


		process_interface->wait(32, 0);
	}
}