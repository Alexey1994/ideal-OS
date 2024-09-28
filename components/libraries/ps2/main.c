#include <IdealOS.c>


typedef struct {
	Process_Interface* process;	
}
Global;

Global _global = {0};


void main(Process* process);


void start(Process* process)
{
	get_process_address;
	Global* global = get_global(_global);

	global->process = process->get(get_global("process"));

	main(process);
}


typedef enum {
	PS2_OUTPUT_BUFFER_FULL       = 0b00000001,
	PS2_INPUT_BUFFER_FULL        = 0b00000010,
	PS2_INITIALIZED              = 0b00000100,
	PS2_A2_STATE                 = 0b00001000,
	PS2_KEYBOARD_CONNECTED       = 0b00010000,
	PS2_MOUSE_OUTPUT_BUFFER_FULL = 0b00100000,
	PS2_TIMEOUT_ERROR            = 0b01000000,
	PS2_PARITY_ERROR             = 0b10000000
}
PS2_State;


typedef enum {
	PS2_READ_COMMAND_BYTE          = 0x20,
	PS2_WRITE_COMMAND_BYTE         = 0x60,
	PS2_DISABLE_MOUSE_INTERFACE    = 0xA7,
	PS2_ENABLE_MOUSE_INTERFACE     = 0xA8,
	PS2_MOUSE_INTERFACE_TEST       = 0xA9,
	PS2_KEYBOARD_INTERFACE_TEST    = 0xAB,
	PS2_DISABLE_KEYBOARD_INTERFACE = 0xAD,
	PS2_ENABLE_KEYBOARD_INTERFACE  = 0xAE,
	PS2_WRITE_IN_MOUSE_DEVICE      = 0xD4,
}
PS2_Command;


typedef enum {
	PS2_ENABLE_INPUT_BUFFER_FULL_INTERRUPT       = 0b00000001, //IRQ1
	PS2_ENABLE_MOUSE_INPUT_BUFFER_FULL_INTERRUPT = 0b00000010, //IRQ12
	PS2_DISABLE_MOUSE                            = 0b00100000,
}
PS2_Command_Byte;


typedef enum {
	PS2_MOUSE_ENABLE_STREAMING  = 0xF4,
	PS2_MOUSE_DISABLE_STREAMING = 0xF5,
	PS2_MOUSE_SET_DEFAULTS      = 0xF6,
}
PS2_Mouse_Device_Command;


#include <IO.c>

/*
Byte read_data_from_ps2()
{
	Number tries_count;
	
	//wait while input buffer not full
	tries_count = 0;
	while(!(in_8(0x64) & PS2_MOUSE_OUTPUT_BUFFER_FULL)) {
		++tries_count;
		
		if(tries_count > 1000) {
			return 0;
		}
	}
	
	return in_8(0x60);
}*/


Byte read_data_from_ps2()
{
	Number tries_count;
	
	//wait while input buffer not full
	tries_count = 0;
	while(!(in_8(0x64) & PS2_OUTPUT_BUFFER_FULL)) {
		++tries_count;
		
		if(tries_count > 1000) {
			return 0;
		}
	}
	
	return in_8(0x60);
}


Boolean write_data_in_ps2(Byte data)
{
	Number tries_count;
	
	//wait while output buffer not empty
	tries_count = 0;
	while(in_8(0x64) & PS2_OUTPUT_BUFFER_FULL) {
		++tries_count;
		
		if(tries_count > 1000) {
			return 0;
		}
	}
	
	out_8(0x60, data);
	
	return 1;
}

/*
Boolean ps2_mouse_acknowledge()
{
	return read_data_from_ps2() == 0xFA;
}*/


Number32 read_key_state()
{
	Byte     ps2_key_state;
	Number32 key_state;

	if(!(in_8(0x64) & PS2_OUTPUT_BUFFER_FULL)) {
		return 0;
	}

	ps2_key_state = in_8(0x60);

	if(ps2_key_state == 0xE0) {
		ps2_key_state = in_8(0x60);
		key_state = (ps2_key_state & 0b1111111) + 128;
	}
	else {
		key_state = ps2_key_state & 0b1111111;
	}
	
	if(ps2_key_state & 0b10000000) {
		key_state |= 0x80000000;
	}

	return key_state;
}


Keyboard_Interface _interface;


void main(Process* process)
{
	get_process_address;
	Global* global = get_global(_global);


/*
	out_8(0x64, PS2_DISABLE_KEYBOARD_INTERFACE);
	out_8(0x64, PS2_DISABLE_MOUSE_INTERFACE);


	//test for present ps2
	Byte old_config = in_8(0x64);
	out_8(0x64, 0xAA);

	if(read_data_from_ps2() != 0x55) {
		return;
	}

	out_8(0x64, old_config);


	out_8(0x64, PS2_ENABLE_KEYBOARD_INTERFACE);
	out_8(0x64, PS2_ENABLE_MOUSE_INTERFACE);
*/


	Keyboard_Interface* interface = get_global(_interface);

	interface->read_key_state = get_global(read_key_state);

	process->create(get_global("keyboard"), interface);


	for(;;) {
		global->process->wait(0);
	}
}