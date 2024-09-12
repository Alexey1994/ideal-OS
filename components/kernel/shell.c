Number get_program_name_extension_index(Byte program_name[64], Number program_name_size)
{
	Number extension_index;
	
	for(extension_index = program_name_size; extension_index; --extension_index) {
		if(program_name[extension_index - 1] == '.') {
			break;
		}
	}
	
	return extension_index;
}


Number parse_program_name(Byte* command, Byte program_name[64])
{
	Number i;
	
	for(
		i = 0;
		i < 63 &&
			command[i] &&
			command[i] != ' ' &&
			command[i] != '|';
		++i
	) {
		program_name[i] = command[i];
	}
	
	program_name[i] = '\0';
	
	return i;
}


Number parse_arguments(Byte* command, Byte** arguments, Number* number_of_arguments)
{
	Number arguments_size;
	
	arguments_size = 0;
	*number_of_arguments = 0;
	
	while(command[arguments_size] && *number_of_arguments < MAX_NUMBER_OF_ARGUMENTS && command[arguments_size] != '|') {
		arguments[*number_of_arguments] = command + arguments_size;
		++*number_of_arguments;
		
		while(command[arguments_size] && command[arguments_size] != ' ' && command[arguments_size] != '|') {
			++arguments_size;
		}
		
		
		if(command[arguments_size] == ' ') {
			command[arguments_size] = '\0';
			++arguments_size;
		}
		else if(command[arguments_size] == '|') {
			Number i;
			for(i = 255; i > arguments_size; --i) {
				command[i] = command[i - 1];
			}
			
			command[arguments_size] = '\0';
			++arguments_size;
		}
		
		
		while(command[arguments_size] && command[arguments_size] == ' ') {
			++arguments_size;
		}
	}
	
	return arguments_size;
}


Boolean load_COM_program(Process* process, File* file)
{
	Byte* program;

	program = allocate_memory(500, 0);
	process->start = program;

	Memory_Chunk* chunk = (Byte*)process - sizeof(Memory_Chunk);
	chunk->owner = process;

	while(read_bytes_from_file(file, program, 500) > 0) {
		//program = allocate_memory(500);
		program = heap_top;
		heap_top += 500;
	}

	//free_memory(program);
	heap_top -= 500;

	return 1;
}


Number load_program(Process* process, Byte* command)
{
	Byte   program_name[64];
	Number program_name_size;
	Number extension_index;
	File   file;

	program_name_size = parse_program_name(command, program_name);
	extension_index = get_program_name_extension_index(program_name, program_name_size);

	if(!extension_index) {
		if(program_name_size >= 64 - 5) {
			goto not_found_error;
		}
		
		program_name[program_name_size++] = '.';
		extension_index = program_name_size;
		program_name[program_name_size++] = 'c';
		program_name[program_name_size++] = 'o';
		program_name[program_name_size++] = 'm';
		program_name[program_name_size] = '\0';
	}

	if(find_file(&file, program_name)) {
		if(!load_COM_program(process, &file)) {
			goto execution_error;
		}
		
		return 0;
	}
	else {
		goto not_found_error;
	}

	not_found_error: {
		//print(
		//	"%s not found.\n"
		//	"Enter \"load dir\" for show all commands",
		//	program_name
		//);
		
		return 1;
	}
	
	execution_error: {
		//print(
		//	"%s not executed.",
		//	program_name
		//);
		
		return 2;
	}
}


Number create_program_process(Process** process, Byte* command, Number* parsed_command_size, Process* previous_piping_process)
{
	Number load_program_status;

	*process = allocate_memory(sizeof(Process), 0);

	load_program_status = load_program(*process, command);

	if(load_program_status) {
		free_memory(*process);
		return load_program_status;
	}

	initialize_process(*process, command, (*process)->start, previous_piping_process, 65536);

	if(!parsed_command_size) {
		parsed_command_size = &load_program_status;
	}

	*parsed_command_size = parse_arguments(
		(*process)->command,
		&(*process)->arguments,
		&(*process)->number_of_arguments
	);

	return 0;
}


Number execute_command(Byte* command)
{
	Process*  process;
	Process*  previous_piping_process;
	Number    parsed_command_size;
	Number    create_programm_process_status;
	
	previous_piping_process = 0;

	for(;;) {
		create_programm_process_status = create_program_process(&process, command, &parsed_command_size, previous_piping_process);
		
		if(create_programm_process_status) {
			return create_programm_process_status;
		}

		if(command[parsed_command_size] != '|') {
			break;
		}
		
		command += parsed_command_size + 1;
		
		while(*command == ' ') {
			++command;
		}
	}
	
	switch_to_process(process);
	
	return 0;
}


Process_Interface process_interface = {
	.create = &create_program_process,
	.switch_to = &switch_to_process,
	.yield = &yield,
	.wait = &wait_event,
	.stop = &stop_process,
	.exit = &exit_from_current_process,
	.execute = &execute_command,
	.get_list = &get_processes_list
};