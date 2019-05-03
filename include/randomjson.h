#ifndef RANDOMJSON_H
#define RANDOMJSON_H

#include <functional>
#include <fstream>

namespace randomjson {
void generate_json(std::fstream* file, int size);
int randomly_add_BOM(char* json);
int randomly_close_bracket(char* json, std::stack<char>& closing_stack, std::stack<bool>& use_comma);
int init_object_or_array(char* json, std::stack<char>& closing_stack, std::stack<bool>& use_comma, int max_size);
int add_value(char* json, std::stack<char>& closing_stack, std::stack<bool>& use_comma, int max_size);
int add_array_entry(char* json, std::stack<char>& closing_stack, std::stack<bool>& use_comma, int max_size);
int add_object_entry(char* json, std::stack<char>& closing_stack, std::stack<bool>& use_comma, int max_size);
int add_number(char* json, int max_size);
int add_string(char* json, int max_size);

//int add_whitespace(char* json, int max_size);

}
#endif
