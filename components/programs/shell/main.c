#include <IdealOS.c>


typedef struct {
	struct File_Node* next;
	File              file;
}
File_Node;


typedef struct {
	Graphics_Interface* graphics;
	File_Interface*     file;
	Heap_Interface*     heap;
	Keyboard_Interface* keyboard;
	Process_Interface*  process;

	File_Node* files_list;
	Number     number_of_files;
	Byte       sector[500];

	Number     number_of_processes;
}
Global;

Global _global = {0};


void main(Process* process);


void start(Process* process)
{
	get_process_address;
	Global* global = get_global(_global);


	global->graphics = process->get(get_global("graphics"));
	global->file     = process->get(get_global("file"));
	global->heap     = process->get(get_global("heap"));
	global->keyboard = process->get(get_global("keyboard"));
	global->process  = process->get(get_global("process"));

	main(process);
}


#include <memory.c>


void update_files_list()
{
	get_process_address;
	Global* global = get_global(_global);


	Number32    f_index;
	File_Node** f_pointer;
	File_Node*  f;

	f = global->files_list;

	while(f) {
		File_Node* next = f->next;
		global->heap->free(f);
		f = next;
	}

	f_index = 1;
	f_pointer = &global->files_list;
	global->number_of_files = 0;

	while(f_index) {
		*f_pointer = global->heap->allocate(sizeof(File_Node), 0);
		(*f_pointer)->next = 0;

		if(!global->file->read(&(*f_pointer)->file, f_index)) {
			global->heap->free(*f_pointer);
			*f_pointer = 0;
			break;
		}

		f_index = (*f_pointer)->file.file_node.next;
		f_pointer = &(*f_pointer)->next;
		++global->number_of_files;
	}
}


void redraw(Frame* frame, Number cursor_pos_y)
{
	get_process_address;
	Global* global = get_global(_global);


	Printer printer = {
		.frame = frame,
		.cursor_pos_x = 0,
		.cursor_pos_y = 0,
		.color = 0xFFFFFF//0xAAAAAA//0x22CC22
	};


	global->graphics->clean_frame(frame, 0x002244);
	global->graphics->draw_rectangle(frame, 0, cursor_pos_y * 16, 640, 16, 0x888888);


	File_Node* f;

	f = global->files_list;

	while(f) {
		global->graphics->print(&printer, get_global("%s\n"), f->file.file_node.name);

		f = f->next;
	}


	Process* current_process = global->process->get_list();

	global->number_of_processes = 0;
	global->graphics->print(&printer, get_global("\n\n"));

	while(current_process) {
		global->graphics->print(&printer, get_global("%x%x%x%x[%d] state:%d %s\n"),
			(Number)current_process->start >> 24,
			(Number)current_process->start >> 16,
			(Number)current_process->start >> 8,
			current_process->start,
			current_process->esp,
			current_process->state,
			current_process->command
		);
		
		++global->number_of_processes;
		current_process = current_process->next;
	}


	global->graphics->print(&printer, get_global("\n\n%t bytes free\n\n"), global->heap->get_free_size());


	global->graphics->draw_frame(frame, 0, 0);
}


void main(Process* process)
{
	get_process_address;
	Global* global = get_global(_global);


	Frame* frame;
	Frame* text_frame;
	Number cursor_pos_y;

	cursor_pos_y = 0;
	
	frame = global->graphics->create_frame(640, 480);
	//global->graphics->clean_frame(frame, 0x002244);

	text_frame = global->graphics->create_frame(320, 480);
	global->graphics->clean_frame(text_frame, 0x000000);


	update_files_list();
	redraw(frame, cursor_pos_y);


	for(;;) {
		Number32 key_state;
		Byte key_code;

		key_state = global->keyboard->read_key_state();
		key_code = key_state & 0xFF;

		if(!(key_state & 0x80000000)) {
			switch(key_code) {
				case KEY_ARROW_DOWN:
				case KEY_NUMPAD_2: {
					if(global->number_of_files && cursor_pos_y < global->number_of_files - 1) {
						++cursor_pos_y;
						redraw(frame, cursor_pos_y);
					}
					else if(cursor_pos_y == global->number_of_files - 1) {
						cursor_pos_y = global->number_of_files - 1 + 3;
						redraw(frame, cursor_pos_y);
					}
					else if(cursor_pos_y - 1 - global->number_of_files < global->number_of_processes) {
						++cursor_pos_y;
						redraw(frame, cursor_pos_y);
					}
					break;
				}

				case KEY_ARROW_UP:
				case KEY_NUMPAD_8: {
					if(cursor_pos_y) {
						if(cursor_pos_y == global->number_of_files - 1 + 3) {
							if(global->number_of_files) {
								cursor_pos_y = global->number_of_files - 1;
							}
						}
						else {
							--cursor_pos_y;
						}

						redraw(frame, cursor_pos_y);
					}
					break;
				}

				case KEY_DELETE:
				case KEY_NUMPAD_DOT: {
					if(cursor_pos_y < global->number_of_files) {
					
					}
					else {
						Number i;
						Number process_number = cursor_pos_y - 2 - global->number_of_files;
						Process* target_process = global->process->get_list();

						for(i = 0; i < process_number; ++i) {
							target_process = target_process->next;
						}

						global->process->stop(target_process, 0);

						redraw(frame, cursor_pos_y);
					}

					break;
				}

				case KEY_ENTER: {
					if(cursor_pos_y < global->number_of_files) {
					Number i;
					File_Node* target_file = global->files_list;

					for(i = 0; i < cursor_pos_y; ++i) {
						target_file = target_file->next;
					}

					global->process->execute(target_file->file.file_node.name);

					redraw(frame, cursor_pos_y);
					}

					break;
				}

				/*case KEY_ENTER: {
					Printer printer = {
						.frame = text_frame,
						.cursor_pos_x = 0,
						.cursor_pos_y = 0,
						.color = 0xFFFFFF//0xAAAAAA//0x22CC22
					};


					Number i;
					File_Node* target_file = global->files_list;

					for(i = 0; i < cursor_pos_y; ++i) {
						target_file = target_file->next;
					}

					File f;

					copy_bytes(&f, &target_file->file, sizeof(File));

					global->file->read_bytes(&f, global->sector, 500);
					global->graphics->clean_frame(text_frame, 0x000000);

					for(i = 0; i < 500 && printer.cursor_pos_y < 30; ++i) {
						global->graphics->print(&printer, get_global("%c"), global->sector[i]);
					}

					global->graphics->draw_frame(text_frame, 320, 0);

					break;
				}*/
			}
		}

		global->process->wait(32, 33, 0);
	}
}