#ifndef RANDOMJSON_H
#define RANDOMJSON_H


#include <iostream>
#include <cmath>

#include <bitset>
#include <fstream>
#include <random>
#include <stack>
#include <string>

namespace randomjson {
void generate_json(std::fstream* file, int size);
int randomly_add_BOM(char* json);
int init_object_or_array(char* json, std::stack<char>& closing_stack, std::stack<bool>& use_comma, int max_size);
int randomly_close_bracket(char* json, std::stack<char>& closing_stack, std::stack<bool>& use_comma);
int add_value(char* json, std::stack<char>& closing_stack, std::stack<bool>& use_comma, int max_size);
int add_array_entry(char* json, std::stack<char>& closing_stack, std::stack<bool>& use_comma, int max_size);
int add_object_entry(char* json, std::stack<char>& closing_stack, std::stack<bool>& use_comma, int max_size);
int add_number(char* json, int max_size);
int add_string(char* json, int max_size);
void add_whitespace(char* json, int max_size);
int add_randomsized_whitespace(char* json, int max_size);

const int default_size = 100;
std::random_device rd;
//std::mt19937 random_generator(rd());
int seed = rd();
//int seed = -1887426970;
std::mt19937 random_generator(seed);
std::uniform_int_distribution<int> boolean_chooser(0, 1);
std::uniform_int_distribution<char> char_chooser(0, 255);

// not used
int randomly_add_BOM(char* json) {
    if (boolean_chooser(random_generator)) {
        json[0] = 0xEF;
        json[1] = 0xBB;
        json[2] = 0xBF;
        std::cout << "BOM";
        return 3;
    }
    return 0;
}

int add_number(char* json, int max_size) {
    const int min_size = 1;
    if (min_size >= max_size) {
        return 0;
    }

    const char digits[] = "0123456789";
    std::uniform_int_distribution<int> digit_chooser(0, 9);

    std::normal_distribution<float> size_chooser(min_size*2, max_size/8);
    int size = std::abs(size_chooser(random_generator));
    if (size < min_size) size = min_size;
    if (size > max_size) size = max_size;
    for (int i = 0; i < size; i++) {
        json[i] = digits[digit_chooser(random_generator)];
    }

    int first_digit_position = 0;

    // adding a minus sign, or not
    if (size > 1) {
        if (boolean_chooser(random_generator)) {
            json[0] = '-';
            first_digit_position = 1;
        }
    }
    // adding a dot, or not.
    int dot_position = first_digit_position; 
    if (size-first_digit_position > 3) { // whether there is enough space or not
        if (boolean_chooser(random_generator)) { // whether we want it or not
            std::uniform_int_distribution<int> dot_position_chooser(first_digit_position+1, size-2);
            dot_position = dot_position_chooser(random_generator);
            json[dot_position] = '.';
        }
    }

    // if dot_position still equals first_digit_position, then there is no dot

    // removing any leading 0
    if (dot_position-first_digit_position != 1) {
        while (json[first_digit_position] == '0') {
            json[first_digit_position] = digits[digit_chooser(random_generator)];
        }
    }

    // adding an exponent, or not
    if (size-dot_position > 3) { // whether there is enough space or not
        if (boolean_chooser(random_generator)) { // whether we want it or not
            std::uniform_int_distribution<int> exponent_position_chooser(dot_position+2, size-2);
            int exponent_position = exponent_position_chooser(random_generator);
            // adding the exponent
            switch (boolean_chooser(random_generator)) {
                case 0:
                    json[exponent_position] = 'e';
                    break;
                case 1:
                    json[exponent_position] = 'E';
            }
            // if there is enough place to add a sign
            if (size-exponent_position > 2) {
                std::uniform_int_distribution<int> sign_chooser(0, 2);
                switch (sign_chooser(random_generator)) {
                case 0:
                    json[exponent_position+1] = '+';
                    break;
                case 1:
                    json[exponent_position+1] = '-';
                    break;
                default:
                    // no sign
                    break;
                }
            }
        }
    }
    return size;
}

int add_string(char* json, int max_size) {
    int min_size = 2;
    if (min_size >= max_size) {
        return 0;
    }

    const char hexa_digits[] = "0123456789ABCDEFabcdef";
    const int nb_hexa_digits = 22;
    std::uniform_int_distribution<int> hexa_digit_chooser(0, nb_hexa_digits-1);
    const char escaped_char[] = "\"\\bfnrt"; // '\u' is handled differently
    const int nb_escaped_char = 7;
    std::uniform_int_distribution<int> escaped_char_chooser(0, nb_escaped_char-1);

    std::normal_distribution<float> size_chooser(min_size*2, max_size/8);
    int size = std::abs(size_chooser(random_generator));
    if (size < min_size) size = min_size;
    if (size > max_size) size = max_size;

    json[0] = '"';
    json[size-1] = '"';

    std::uniform_int_distribution<int> char_type_chooser(1, 5);

    int i = size-min_size;
    while (i > 0) {
        int char_size;
        int char_type;
        if (i > 6) {
            char_type = char_type_chooser(random_generator);
        }
        else if (i > 4) {
            std::uniform_int_distribution<int> char_size_chooser(1, 4);
            char_type = char_size_chooser(random_generator);
        }
        else {
            char_type = i;
        }
        switch (char_type) {
            case 1:
                do {
                    json[i] = random_generator() & 0b01111111;
                } while (json[i] == '"' || json[i] == '\\' || json[i] <= 0x1f);
                char_size = 1;
                break;
            case 2:
                json[i-1] = random_generator();
                if (json[i-1] == '\\') {
                    json[i] = escaped_char[escaped_char_chooser(random_generator)];
                }
                else {
                    json[i-1] = (json[i-1] | 0b11000000) & 0b11011111;
                    while (static_cast<unsigned char>(json[i-1]) <= static_cast<unsigned char>(0xc2)){
                        json[i-1] = (random_generator() | 0b11000000) & 0b11011111;
                    }
                    json[i] = (random_generator() | 0b10000000) & 0b10111111;
                }
                char_size = 2;
                break;
            case 3:
                json[i-2] = (random_generator() | 0b11100000) & 0b11101111;
                json[i-1] = (random_generator() | 0b10000000) & 0b10111111;
                json[i] = (random_generator() | 0b10000000) & 0b10111111;
                if (static_cast<unsigned char>(json[i-2]) == static_cast<unsigned char>(0xe0)) {
                    while (static_cast<unsigned char>(json[i-1]) < static_cast<unsigned char>(0xa0)) {
                        json[i-1] = (random_generator() | 0b10000000) & 0b10111111;
                    }
                }
                else if (static_cast<unsigned char>(json[i-2]) == static_cast<unsigned char>(0xed)) {
                    while (static_cast<unsigned char>(json[i-1]) > static_cast<unsigned char>(0x9f)) {
                        json[i-1] = (random_generator() | 0b10000000) & 0b10111111;
                    }
                }
                char_size = 3;
                break;
            case 4:
                json[i-3] = 0b11110000 + random_generator()%4;
                json[i-2] = (random_generator() | 0b10000000) & 0b10111111;
                json[i-1] = (random_generator() | 0b10000000) & 0b10111111;
                json[i] = (random_generator() | 0b10000000) & 0b10111111;
                if (static_cast<unsigned char>(json[i-3]) == static_cast<unsigned char>(0xf0)) {
                    while (static_cast<unsigned char>(json[i-2]) < static_cast<unsigned char>(0x90)) {
                        json[i-2] = (random_generator() | 0b10000000) & 0b10111111;
                    }
                }
                else if (static_cast<unsigned char>(json[i-3]) == static_cast<unsigned char>(0xf4)) {
                    while (static_cast<unsigned char>(json[i-2]) > static_cast<unsigned char>(0x8f)) {
                        json[i-2] = (random_generator() | 0b10000000) & 0b10111111;
                    }
                }
                char_size = 4;
                break;
            case 5: // '\u' case
                json[i-5] = '\\';
                json[i-4] = 'u';
                json[i-3] = hexa_digits[hexa_digit_chooser(random_generator)];
                json[i-2] = hexa_digits[hexa_digit_chooser(random_generator)];
                json[i-1] = hexa_digits[hexa_digit_chooser(random_generator)];
                json[i] = hexa_digits[hexa_digit_chooser(random_generator)];
                char_size = 6;
                break;
        }

        i -= char_size;
    }

    return size;
}

int add_array_entry(char* json, std::stack<char>& closing_stack, std::stack<bool>& use_comma, int max_size) {
    int comma_length = use_comma.top() ? 1: 0;
    if (max_size < comma_length) {
        return 0;
    }

    int offset = add_randomsized_whitespace(json, max_size);

    if (use_comma.top()) {
        json[offset] = ',';
        offset++;
        offset += add_randomsized_whitespace(json + offset*sizeof(char), max_size-offset);
    }
    
    return add_value(json + offset*sizeof(char), closing_stack, use_comma, max_size-offset) + offset;
}

int add_object_entry(char* json, std::stack<char>& closing_stack, std::stack<bool>& use_comma, int max_size) {
    const int min_key_size = 2;
    const int colon_size = 1;
    const int min_value_size = 1;
    int comma_length = use_comma.top() ? 1 : 0;
    int min_size = min_key_size + colon_size + min_value_size + comma_length;

    if (min_size >= max_size) {
        return 0;
    }

    int offset = add_randomsized_whitespace(json, max_size);

    if (use_comma.top()) {
        json[offset] = ',';
        offset++;
        offset += add_randomsized_whitespace(json + offset*sizeof(char), max_size-offset);
    }

    int key_size = add_string(json + offset*sizeof(char), max_size - offset - colon_size - min_value_size);
    offset += key_size;
    offset += add_randomsized_whitespace(json + offset*sizeof(char), max_size-offset);
    json[offset] = ':';
    offset++;
    offset += add_randomsized_whitespace(json + offset*sizeof(char), max_size-offset);
    int value_size = add_value(json + offset*sizeof(char), closing_stack, use_comma, max_size-offset);

    // we failed to write a value
    // This should not happen, but it does happen frequently at the end of documents
    if (value_size == 0) {
        return 0;
    }

    offset += value_size;

    return offset;
}

int add_value(char* json, std::stack<char>& closing_stack, std::stack<bool>& use_comma, int max_size) {
    const int min_size = 1;

    if (min_size >= max_size) {
        return 0;
    }

    std::uniform_int_distribution<int> value_type_chooser(0, 2);
    int size;
    switch (value_type_chooser(random_generator)) {
        case 0:
            size = init_object_or_array(json, closing_stack, use_comma, max_size);
            break;
        case 1:
            size = add_string(json, max_size);
            if (size != 0) {
                use_comma.top() = true;
            }
        break;
        case 2:
            size = add_number(json, max_size);
            if (size != 0) {
                use_comma.top() = true;
            }
            break;
        default:
        size = 0;
        break;
    }

    return size;
}

int init_object_or_array(char* json, std::stack<char>& closing_stack, std::stack<bool>& use_comma, int max_size) {
    const int min_size = 2;
    if (min_size >= max_size) {
        return 0;
    }
    
    if (boolean_chooser(random_generator)) {
        json[0] = '[';
        closing_stack.push(']');
    }
    else {
        json[0] = '{';
        closing_stack.push('}');
    }
    use_comma.push(false);

    return 1;
}

int randomly_close_bracket(char* json, std::stack<char>& closing_stack, std::stack<bool>& use_comma) {
    const int magic_closer = 0x40000000;
    if (closing_stack.size() > 1 && magic_closer >= random_generator()) {
        json[0] = closing_stack.top();
        closing_stack.pop();
        use_comma.pop();
        use_comma.top() = true;
        return 1;
    }
    return 0;
}

int add_randomsized_whitespace(char* json, int max_size) {
    std::normal_distribution<float> size_chooser(0, (max_size)/64);
    int size = std::abs(size_chooser(random_generator));
    if (size > max_size) size = max_size;
    add_whitespace(json, size);

    return size;
}

void add_whitespace(char* json, int size) {
    const char whitespaces[] {0x09, 0x0A, 0x0D, 0x20};
    std::uniform_int_distribution<char> char_chooser(0, 3);

    for (int i = 0; i < size; i++) {
        json[i] = whitespaces[char_chooser(random_generator)];
    }
}

void generate_json(std::fstream& file, int size) {
    char* json = (char*) malloc(size);
    //int offset = randomly_add_BOM(json);
    int offset = 0;
    std::stack<char> closing_stack;
    std::stack<bool> use_comma;

    offset += add_randomsized_whitespace(json+offset*sizeof(char), size);
    offset += init_object_or_array(json + offset*sizeof(char), closing_stack,  use_comma, size-offset);
    
    while (true) {
        if (offset >= size) {
            break;
        }
        int space_left = size-offset-closing_stack.size();
        
        if (space_left-5 <= 0) {
            // closing everything left
            while (!closing_stack.empty()) {
                json[offset] = closing_stack.top();
                closing_stack.pop();
                offset++;
            }
            add_whitespace(json+offset*sizeof(char), size-offset);
            break;
        }
        else if (space_left < 0) {
            // There's a problem. What we do ?
        }

        int closing_offset = randomly_close_bracket(json+offset*sizeof(char), closing_stack, use_comma);
        offset += closing_offset;
        space_left -= closing_offset;
        if (closing_stack.top() == ']') {
            offset += add_array_entry(json+offset*sizeof(char), closing_stack, use_comma, space_left);
        }
        else {
            offset += add_object_entry(json+offset*sizeof(char), closing_stack, use_comma, space_left);
        }
    }

    file.write(json, sizeof(char)*size);
    free(json);
}
}

#endif
