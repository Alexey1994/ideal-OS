#ifndef PROCESS_INCLUDED
#define PROCESS_INCLUDED


typedef enum {
	PROCESS_LOADED,
	PROCESS_WORKING,
	PROCESS_WAITING,
}
Process_State;


typedef enum {
	//0 - 255 interrupts
	FILE_CHANGE_EVENT= 256,
}
Event;


typedef void(*Process_Start)(struct Process* process);


typedef struct {
	void* (*get)    (Byte* path);
	void  (*create) (Byte* path, void* data);
	
	Byte          command[256];
	Number        number_of_arguments;
	Byte*         arguments[MAX_NUMBER_OF_ARGUMENTS];


	Process_State state;
	Event* wait_events; //null terminated


	Process_Start start;

	struct Process* next;
	
	Number esp;
	Number ebp;
}
Process;


#define get_module_address_by_function(function) \
	Number module_address; \
	asm("call . + 5\n" "pop %0" : "=a"(module_address)); \
	module_address -= (Number)&(function) + 15;

#define global(name) *(void**)((Byte*)&name + module_address)
#define global_ptr(name) ((Byte*)&name + module_address)


typedef struct {
	void     (*create)      (Process** process, Byte* command, Number* parsed_command_size, Process* previous_piping_process);
	void     (*switch_to)   (Process* process);
	void     (*yield)       ();
	void     (*wait)        (Event event, ...); //null terminated
	void     (*stop)        (Process* process, Number code);
	void     (*exit)        (Number code);
	Number   (*execute)     (Byte* command);
	Process* (*get_list)    ();
}
Process_Interface;


#endif//PROCESS_INCLUDED