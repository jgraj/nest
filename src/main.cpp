#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <cctype>
#include "gar.cpp"

void trim_string(gar<char>* string) {
	while (string->len && std::isspace((*string)[0])) {
		string->len -= 1;
		string->buf += 1;
	}
	while (string->len && std::isspace((*string)[string->len - 1])) {
		string->len -= 1;
	}
}

void trim_body(gar<char>* string) {
	for (int i = 0; i < string->len; ++i) {
		if (string->buf[i] == ':' && std::isspace(string->buf[i + 1])) {
			string->remove_at(i + 1);
			--i;
		}
	}
}

gar<gar<char>> split(gar<char> array, char delimiter) {
	gar<gar<char>> new_array = gar<gar<char>>::alloc(4);
	int last_index = 0;
	for (int i = 0; i <= array.len; ++i) {
		if (i == array.len || array.buf[i] == delimiter) {
			int size = i - last_index;
			gar<char> sub_array = gar<char>::alloc(size);
			std::memcpy(sub_array.buf, &array.buf[last_index], sizeof(char) * size);
			sub_array.len = size;
			new_array.push(sub_array);
			last_index = ++i;
		}
	}
	return new_array;
}

void create_selector_comma(gar<char>* output, gar<gar<char>> selector_stack, int current_index) {
	gar<char> selector = selector_stack[current_index];
	gar<gar<char>> split_by_commas = split(selector, ',');
	for (int i = 0; i < split_by_commas.len; ++i) {
		gar<gar<char>> new_selector_stack = selector_stack.clone();
		trim_string(&split_by_commas[i]);
		new_selector_stack[current_index] = split_by_commas[i];
		if (current_index + 1 < selector_stack.len) {
			create_selector_comma(output, new_selector_stack, current_index + 1);
		} else {
			for (int ii = 0; ii < new_selector_stack.len; ++ii) {
				bool is_ampersand = new_selector_stack[ii][0] == '&';
				if (ii > 0 && !is_ampersand) {
					output->push(' ');
				}
				if (is_ampersand) {
					new_selector_stack[ii].remove_at(0);
				}
				output->join(new_selector_stack[ii]);
			}
		}
		if (i < split_by_commas.len - 1) {
			output->push(',');
		}
	}
}

void create_selector(gar<char>* output, gar<gar<char>> selector_stack, gar<char> current_body) {
	if (current_body.len == 0) {
		return;
	}
	create_selector_comma(output, selector_stack, 0);
	output->push('{');
	if (current_body[current_body.len - 1] == ';') {
		current_body.pop();
	}
	output->join(current_body);
	output->push('}');
}

void compile(char* input, int input_len) {
	gar<char> output = gar<char>::alloc(1024);
	gar<gar<char>> selector_stack = gar<gar<char>>::alloc(8);
	gar<gar<char>> body_stack = gar<gar<char>>::alloc(8);
	gar<char> current_buffer = gar<char>::alloc(32);
	gar<char> current_body = gar<char>::alloc(1024);
	bool is_at_rule = false;
	for (int i = 0; i < input_len; ++i) {
		char c = input[i];
		switch (c) {
			case '{': {
				trim_string(&current_buffer);
				if (current_buffer[0] == '@') {
					is_at_rule = true;
					output.join(current_buffer);
					output.push('{');
				} else {
					body_stack.push(current_body);
					current_body = gar<char>::alloc(1024);
					selector_stack.push(current_buffer);
				}
				current_buffer = gar<char>::alloc(32);
				break;
			}
			case '}': {
				if (selector_stack.len == 0 && is_at_rule) {
					is_at_rule = false;
					output.push('}');
				} else {
					create_selector(&output, selector_stack, current_body);
					current_body = body_stack.pop();
					selector_stack.pop();
				}
				break;
			}
			case ';': {
				trim_string(&current_buffer);
				trim_body(&current_buffer);
				current_buffer.push(';');
				current_body.join(current_buffer);
				current_buffer = gar<char>::alloc(32);
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
		std::printf("No arguments provided");
		return 1;
	}
	if (argc > 2) {
		std::printf("Too many arguments provided");
		return 1;
	}
	const char* file_path = argv[1];
	FILE* file = std::fopen(file_path, "r");
	if (file == NULL) {
		std::printf("Couldn't open file '%s'", file_path);
		return 1;
	}
	std::fseek(file, 0, SEEK_END);
	long file_size = std::ftell(file);
	std::fseek(file, 0, SEEK_SET);
	char* file_content = (char*)std::malloc(file_size * sizeof(char));
	std::fread(file_content, 1, file_size, file);
	compile(file_content, file_size);
}