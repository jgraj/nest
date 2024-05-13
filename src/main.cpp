#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include "array.cpp"

void trim_string(DynArr<char>* string) {
	while (string->len && isspace((*string)[0])) {
		string->len -= 1;
		string->buf += 1;
	}
	while (string->len && isspace((*string)[string->len - 1])) {
		string->len -= 1;
	}
}

void trim_body(DynArr<char>* string) {
	for (int i = 0; i < string->len; ++i) {
		if (string->buf[i] == ':' && isspace(string->buf[i + 1])) {
			string->remove_at(i + 1);
			--i;
		}
	}
}

void create_selector_comma(DynArr<char>* output, DynArr<DynArr<char>> selector_stack, int current_index) {
	DynArr<char> selector = selector_stack[current_index];
	DynArr<DynArr<char>> split_by_commas = selector.split(',');
	for (int i = 0; i < split_by_commas.len; ++i) {
		DynArr<DynArr<char>> new_selector_stack = selector_stack.clone();
		trim_string(&split_by_commas[i]);
		new_selector_stack[current_index] = split_by_commas[i];
		if (current_index + 1 < selector_stack.len) {
			create_selector_comma(output, new_selector_stack, current_index + 1);
		} else {
			for (int ii = 0; ii < new_selector_stack.len; ++ii) {
				output->join(new_selector_stack[ii]);
				if (ii < new_selector_stack.len - 1) {
					output->push(' ');
				}
			}
		}
		if (i < split_by_commas.len - 1) {
			output->push(',');
		}
	}
}

void create_selector(DynArr<char>* output, DynArr<DynArr<char>> selector_stack, DynArr<char> current_body) {
	if (current_body.len == 0) {
		return;
	}
	bool is_at_rule = false;
	if (selector_stack[0][0] == '@') {
		is_at_rule = true;
		output->join(selector_stack.pop_front());
		output->push('{');
	}
	create_selector_comma(output, selector_stack, 0);
	output->push('{');
	if (current_body[current_body.len - 1] == ';') {
		current_body.pop();
	}
	output->join(current_body);
	output->push('}');
	if (is_at_rule) {
		output->push('}');
	}
}

void compile(char* input, int input_len) {
	DynArr<char> output = DynArr<char>::new_with_cap(1024);
	DynArr<DynArr<char>> selector_stack = DynArr<DynArr<char>>::new_with_cap(8);
	DynArr<DynArr<char>> body_stack = DynArr<DynArr<char>>::new_with_cap(8);
	DynArr<char> current_buffer = DynArr<char>::new_with_cap(32);
	DynArr<char> current_body = DynArr<char>::new_with_cap(1024);
	for (int i = 0; i < input_len; ++i) {
		char c = input[i];
		switch (c) {
			case '{': {
				body_stack.push(current_body);
				current_body = DynArr<char>::new_with_cap(1024);
				trim_string(&current_buffer);
				selector_stack.push(current_buffer);
				current_buffer = DynArr<char>::new_with_cap(32);
				break;
			}
			case '}': {
				create_selector(&output, selector_stack, current_body);
				current_body = body_stack.pop();
				selector_stack.pop();
				break;
			}
			case ';': {
				trim_string(&current_buffer);
				trim_body(&current_buffer);
				current_buffer.push(';');
				current_body.join(current_buffer);
				current_buffer = DynArr<char>::new_with_cap(32);
				break;
			}
			default: {
				current_buffer.push(c);
				break;
			}
		}
	}

	output[output.len] = 0;
	fputs(output.buf, stdout);
}

int main(int argc, char** argv) {
	if (argc < 2) {
		printf("No arguments provided");
		return 1;
	}
	if (argc > 2) {
		printf("Too many arguments provided");
		return 1;
	}
	const char* file_path = argv[1];
	FILE* file = fopen(file_path, "r");
	if (file == NULL) {
		printf("Couldn't open file '%s'", file_path);
		return 1;
	}
	fseek(file, 0, SEEK_END);
	long file_size = ftell(file);
	fseek(file, 0, SEEK_SET);
	char* file_content = (char*)malloc(file_size * sizeof(char));
	fread(file_content, 1, file_size, file);
	compile(file_content, file_size);
}