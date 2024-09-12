Process* processes_list = 0;
Process* current_process = 0;


Process* get_processes_list()
{
	return processes_list;
}


Process** get_last_process_pointer()
{
	Process** last_process_pointer;
	
	last_process_pointer = &processes_list;
	
	while(*last_process_pointer) {
		last_process_pointer = &(*last_process_pointer)->next;
	}
	
	return last_process_pointer;
}


Number esp;
Number ebp;


void switch_to_process(Process* process)
{
	//save current process context
	if(current_process) {
		asm("mov %%esp, %0":"=a"(current_process->esp));
		asm("mov %%ebp, %0":"=a"(current_process->ebp));
		//print("[save %s %d %d]\n", current_process->command, current_process->start, current_process->esp);
	}
	else {
		asm("mov %%esp, %0":"=a"(esp));
		asm("mov %%ebp, %0":"=a"(ebp));
	}
	
	//switch to process
	current_process = process;

	asm("mov %0, %%esp"::"a"(current_process->esp));
	asm("mov %0, %%ebp"::"a"(current_process->ebp));

	//print("[switch %s %d %d]\n", current_process->command, current_process->start, current_process->esp);
	
	if(current_process->state == PROCESS_LOADED) {
		//print("[start %s %d esp=%d]\n", current_process->command, current_process, current_process->esp);
		current_process->state = PROCESS_WORKING;
		current_process->start(current_process);
		
		asm("exit_address:");
		//print("[stop %s %d]\n", current_process->command, current_process);

		//free process
		Process** process_pointer;

		process_pointer = &processes_list;

		while(*process_pointer != current_process) {
			process_pointer = &(*process_pointer)->next;
		}
			
		*process_pointer = current_process->next;


		//get next process
		Process* next_process;

		next_process = current_process->next;

		if(!next_process) {
			next_process = processes_list;
		}


		//switch to next process
		if(next_process) {
			switch_to_process(next_process);
		}
		else {
			//IdealOS end!!!
			asm("mov %0, %%esp"::"a"(esp));
			asm("mov %0, %%ebp"::"a"(ebp));
		}
	}
}


void exit_from_current_process(Number code)
{
	asm("jmp exit_address");
}


void stop_process(Process* process, Number code)
{
	Process** process_pointer;

	process_pointer = &processes_list;

	while(*process_pointer != process) {
		process_pointer = &(*process_pointer)->next;
	}

	if(!*process_pointer) {
		return;
	}

	*process_pointer = (*process_pointer)->next;


	//free process allocated memory
	Memory_Chunk* current_chunk;

	current_chunk = top_chunk;

	while(current_chunk) {
		if(current_chunk->owner == process) {
			current_chunk->is_busy = 0;
		}

		current_chunk = current_chunk->previous_chunk;
	}

	//defragmentation
	while(!top_chunk->is_busy) {
		heap_top = top_chunk;
		top_chunk = top_chunk->previous_chunk;
	}
}


void initialize_process(
	Process* process,
	Byte* command,
	Process_Start start,
	Process* previous_piping_process,
	Number stack_size
)
{
	process->next = 0;
	process->state = PROCESS_LOADED;
	process->wait_events = 0;

	process->get = &get_interface_data;
	process->create = &create_interface_by_path;

	process->start = start;

	Number i;
	for(i = 0; command[i]; ++i) {
		process->command[i] = command[i];
	}
	process->command[i] = '\0';

	process->esp = allocate_memory(stack_size, 0) + stack_size;//process->start + file->file_size + 64 * 1024;
	process->ebp = process->esp;

	*get_last_process_pointer() = process;
}


void yield()
{
	Process* next_process;

	next_process = current_process->next;

	while(next_process && next_process->state == PROCESS_WAITING) {
		next_process = next_process->next;
	}

	if(next_process) {
		switch_to_process(next_process);
	}
	else {
		switch_to_process(processes_list);
	}
}


void wait_event(Event event, ...) {
	current_process->wait_events = &event;
	current_process->state = PROCESS_WAITING;
	yield();
}


void emit_event(Event event)
{
	Process* process = processes_list;

	while(process) {
		if(process->state == PROCESS_WAITING) {
			Event* e = process->wait_events;

			while(e && *e) {
				if(*e == event) {
					process->state = PROCESS_WORKING;
					break;
				}

				++e;
			}
		}

		process = process->next;
	}
}