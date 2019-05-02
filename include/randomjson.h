#ifndef RANDOMJSON_H
#define RANDOMJSON_H

#include <functional>
#include <fstream>

namespace randomjson {
void generate_json(std::fstream* file, int size);
//int generate_object(char* json, int max_size);
//int generate_array(char* json, int max_size);
int add_number(char* json, int max_size);
int add_string(char* json, int max_size);
//int generate_string(char* json, int max_size);
//int generate_whitespace(char* json, int max_size);
//const std::function<std::string> value_functions[] {generate_object, generate_array, generate_number, generate_string};


}
#endif
