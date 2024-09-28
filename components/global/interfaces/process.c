#ifndef PROCESS_INCLUDED
#define PROCESS_INCLUDED


#define get_process_address \
	int process_address; \
	asm("call . + 5\n" "pop %%eax\n" "sub $. - 1, %%eax\n" "mov %%eax, %0\n" : "=a"(process_address));

#define get_global(name) ((Byte*)&(name) + process_address)


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