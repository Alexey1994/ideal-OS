#ifndef WRITER_INCLUDED
#define WRITER_INCLUDED


#include <types.c>


typedef struct {
	void* source;
	Signed_Number (*write_bytes)(void* source, Byte* bytes, Number number_of_bytes);
}
Writer;


Signed_Number write_bytes(Writer* writer, Byte* bytes, Number number_of_bytes)
{
	return writer->write_bytes(writer->source, bytes, number_of_bytes);
}


Signed_Number write_Byte(Writer* writer, Byte byte)
{
	return writer->write_bytes(writer->source, &byte, sizeof(byte));
}


Signed_Number write_string(Writer* writer, Byte* string)
{
	Signed_Number total_bytes_writed;
	Signed_Number bytes_writed;

	total_bytes_writed = 0;

	for(; *string; ++string) {
		bytes_writed = write_Byte(writer, *string);

		if(bytes_writed <= 0) {
			break;
		}

		total_bytes_writed += bytes_writed;
	}

	return total_bytes_writed;
}


Signed_Number write_Number(Writer* writer, Number number)
{
	Signed_Number bytes_writed = 0;
	
	Number next;
	Byte   byte;
	
	next = number / 10;
	
	if(next) {
		bytes_writed = write_Number(writer, next);
	}

	bytes_writed += write_Byte(writer, number % 10 + '0');

	return bytes_writed;
}


Signed_Number write_Number_triplets(Writer* writer, Number number, Number level)
{
	Signed_Number bytes_writed = 0;

	Number next;
	
	next = number / 10;
	
	if(next) {
		bytes_writed = write_Number_triplets(writer, next, level + 1);
	}
	
	bytes_writed += write_Byte(writer, number % 10 + '0');
	
	if(level && !(level % 3)) {
		bytes_writed += write_Byte(writer, ' ');
	}

	return bytes_writed;
}


Signed_Number write_Signed_Number(Writer* writer, Signed_Number number)
{
	Signed_Number bytes_writed = 0;

	if(number < 0) {
		bytes_writed = write_Byte(writer, '-');
		number = -number;
	}
	
	bytes_writed += write_Number(writer, number);

	return bytes_writed;
}

/*
Signed_Number write_Real_Number(Writer* writer, Real_Number number)
{
	Signed_Number bytes_writed = 0;

	Number      integer_number;
	Real_Number decimal;
	
	if(number < 0) {
		bytes_writed += write_Byte(writer, '-');
		number = -number;
	}

	integer_number = number;
	bytes_writed += write_Number(writer, integer_number);
	
	decimal = number - integer_number;
	
	if(decimal > 0) {
		bytes_writed += write_Byte(writer, '.');
	}
	
	while(decimal > 0) {
		decimal *= 10;
		bytes_writed += write_Byte(writer, (Number)decimal % 10 + '0');
		decimal = decimal - (Number)decimal;
	}

	return bytes_writed;
}*/


Signed_Number write_hex_character(Writer* writer, Byte character)
{
	Signed_Number bytes_writed = 0;

	if(character < 10) {
		bytes_writed += write_Byte(writer, character + '0');
	}
	else {
		bytes_writed += write_Byte(writer, character - 10 + 'A');
	}

	return bytes_writed;
}


Signed_Number write_hex_Byte(Writer* writer, Byte byte)
{
	Signed_Number bytes_writed = 0;

	if(byte < 16) {
		bytes_writed += write_Byte(writer, '0');
		bytes_writed += write_hex_character(writer, byte);
	}
	else {
		bytes_writed += write_hex_character(writer, byte >> 4);
		bytes_writed += write_hex_character(writer, byte & 0b00001111);
	}

	return bytes_writed;
}


Signed_Number write(Writer* writer, Byte* format, Byte** values)
{
	Signed_Number bytes_writed;
	Byte          character;

	bytes_writed = 0;

	for(;;) {
		character = *format;
		
		if(!character) {
			break;
		}
		
		++format;

		if(character == '%') {
			character = *format;
			++format;

			switch(character) {
				case 'c':
					bytes_writed += write_Byte(writer, *values);
					++values;
					break;

				case 'x':
					bytes_writed += write_hex_Byte(writer, *values);
					++values;
					break;

				case 'u':
					bytes_writed += write_Number(writer, *values);
					++values;
					break;

				case 'd':
					bytes_writed += write_Signed_Number(writer, *(Signed_Number32*)values);
					++values;
					break;
				
				case 't':
					bytes_writed += write_Number_triplets(writer, *values, 0);
					++values;
					break;

				//case 'f':
				//	bytes_writed += write_Real_Number(writer, *(Real_Number*)values);
				//	++values;
				//	break;

				case 's':
					bytes_writed += write_string(writer, *values);
					++values;
					break;

				default:
					bytes_writed += write_Byte(writer, character);
			}
		}
		else {
			bytes_writed += write_Byte(writer, character);
		}
	}

	return bytes_writed;
}


Signed_Number write_bytes_in_string(Byte** string_pointer, Byte* bytes, Number number_of_bytes)
{
	Number i;
	Byte*  string;

	string = *string_pointer;

	for(i = 0; i < number_of_bytes; ++i) {
		string[i] = bytes[i];
	}

	*string_pointer += number_of_bytes;

	return number_of_bytes;
}


Signed_Number print_in_string(Byte* string, Byte* format, ...)
{
	Writer string_writer = {&string, &write_bytes_in_string};
	
	return write(&string_writer, format, &format + 1)
		+ write_Byte(&string_writer, '\0');	
}


#endif//WRITER_INCLUDED