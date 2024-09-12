#include <IdealOS.c>


#define INTERRUPT_QUEUE_SIZE 256

typedef struct {
	Number start;
	Number end;
	Byte   data[INTERRUPT_QUEUE_SIZE];
}
Interrupt_Queue;


typedef struct {
	Number32 address_low;
	Number32 address_high;
	Number32 size_low;
	Number32 size_high;
	Number32 type; //1 - normal, 2 - reserved, other - not used
}
Memory_Region;


typedef struct {
	Interrupt_Queue* interrupt_queue;
	
	//sector address is 0x500
	void (*read_sector)(Number32 sector_number);
	void (*write_sector)(Number32 sector_number);

	//VGA_Interface  VGA;
	VESA_Mode*     VESA;
	Number32       size_of_memory_regions;
	Memory_Region* memory_regions;
	void* t;
}
Loader_Api;


Loader_Api* loader_api;


void main();


void _start(Loader_Api api)
{
	loader_api = &api;
	main();
}


#include <IO.c>
#include <memory.c>
#include <string.c>

#include "interfaces/heap.c"
#include "interface.c"
#include "interfaces/file.c"
#include "interfaces/process.c"

#include "shell.c"


void interrupt_handler(Process* process)
{
	Interrupt_Queue* interrupt_queue;
	Number           start_index;
	Number           interrupt_number;
	Process*         next_process;
	
	interrupt_queue = loader_api->interrupt_queue;

	for(;;) {
		for(;;) {
			start_index = interrupt_queue->start;

			if(start_index == interrupt_queue->end) {
				break;
			}

			interrupt_number = interrupt_queue->data[start_index];

			emit_event(interrupt_number);

			++start_index;
			
			if(start_index >= INTERRUPT_QUEUE_SIZE) {
				start_index = 0;
			}
			
			interrupt_queue->start = start_index;
		}


		next_process = process->next;

		if(!next_process) {
			break;
		}

		while(next_process && next_process->state == PROCESS_WAITING) {
			next_process = next_process->next;
		}

		if(next_process) {
			switch_to_process(next_process);
		}
		else {
			asm("hlt");
		}
	}
}


void init(Process* process)
{
	execute_command("graphics");


	Process* bootscreen_process;

	if(create_program_process(&bootscreen_process, "bootscreen", 0, 0)) {
		return;
	}

	//switch_to_process(bootscreen_process);

	execute_command("ps2");


	Number i;

	for(i = 0; i < 18 * 4; ++i) {
		wait_event(32, 0);
	}
	

	stop_process(bootscreen_process, 0);


	execute_command("shell");
}


void main()
{
	if(!initialize_heap()) {
		return;
	}


	create_interface_by_path("VESA", loader_api->VESA);
	create_interface_by_path("file", &file_interface);
	create_interface_by_path("heap", &heap_interface);
	create_interface_by_path("process", &process_interface);


	Process interrupt_handler_process;
	Process init_process;

	initialize_process(&interrupt_handler_process, "interrupt_handler", &interrupt_handler, 0, 1024);
	initialize_process(&init_process, "init", &init, 0, 65536);

	switch_to_process(&interrupt_handler_process);
}