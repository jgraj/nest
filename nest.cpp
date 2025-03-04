#include <cctype>

#define GAR_ABC
#include <ctk-0.11/ctk.cpp>

void panic(const char* format, ...) {
	va_list args;
	va_start(args, format);
	vfprintf(stderr, format, args);
	va_end(args);
	exit(1);
}

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
	for (size_t i = 0; i < string->len; ++i) {
		if (string->buf[i] == ':' && std::isspace(string->buf[i + 1])) {
			string->remove(i + 1);
			--i;
		}
	}
}

gar<gar<char>> split(gar<char> array, char delimiter) {
	gar<gar<char>> new_array = gar<gar<char>>::create_auto();
	size_t last_index = 0;
	for (size_t i = 0; i <= array.len; ++i) {
		if (i == array.len || array.buf[i] == delimiter) {
			size_t size = i - last_index;
			ar<char> sub_array = ar<char>::create(size);
			std::memcpy(sub_array.buf, &array.buf[last_index], sizeof(char) * size);
			new_array.push(sub_array.to_gar());
			last_index = ++i;
		}
	}
	return new_array;
}

void create_selector_comma(gar<char>* output, gar<gar<char>> selector_stack, size_t current_index) {
	gar<char> selector = selector_stack[current_index];
	gar<gar<char>> split_by_commas = split(selector, ',');
	for (size_t i = 0; i < split_by_commas.len; ++i) {
		gar<gar<char>> new_selector_stack = selector_stack.clone();
		trim_string(&split_by_commas[i]);
		new_selector_stack[current_index] = split_by_commas[i];
		if (current_index + 1 < selector_stack.len) {
			create_selector_comma(output, new_selector_stack, current_index + 1);
		} else {
			for (size_t ii = 0; ii < new_selector_stack.len; ++ii) {
				bool is_ampersand = new_selector_stack[ii][0] == '&';
				if (ii > 0 && !is_ampersand) {
					output->push(' ');
				}
				if (is_ampersand) {
					new_selector_stack[ii].remove(0);
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

void compile(char* input, size_t input_len) {
	gar<char> output = gar<char>::create_auto();
	gar<gar<char>> selector_stack = gar<gar<char>>::create_auto();
	gar<gar<char>> body_stack = gar<gar<char>>::create_auto();
	gar<char> current_buffer = gar<char>::create_auto();
	gar<char> current_body = gar<char>::create_auto();
	bool is_at_rule = false;
	for (size_t i = 0; i < input_len; ++i) {
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
					current_body = gar<char>::create_auto();
					selector_stack.push(current_buffer);
				}
				current_buffer = gar<char>::create_auto();
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
				current_buffer = gar<char>::create(32);
				break;
			}
			default: {
				current_buffer.push(c);
				break;
			}
		}
	}

	output.push('\0');
	fputs(output.buf, stdout);
}

int main(int argc, char** argv) {
	if (argc != 2) {
		panic("expected 1 arg");
	}
	const char* file_path = argv[1];
	FILE* file = std::fopen(file_path, "r");
	if (file == NULL) {
		panic("fopen failed '%s'", file_path);
	}
	std::fseek(file, 0, SEEK_END);
	size_t file_size = std::ftell(file);
	std::fseek(file, 0, SEEK_SET);
	char* file_content = alloc_space<char>(file_size);
	if (std::fread(file_content, 1, file_size, file) != file_size) {
		panic("fread failed");
	}
	compile(file_content, file_size);
}