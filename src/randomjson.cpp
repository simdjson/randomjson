#include <iostream>
#include <fstream>
#include <random>
#include <stack>
#include <string>

#include <bitset>

#include "randomjson.h"

namespace randomjson {
const char white_spaces[] {' '};
const char opening_brackets[] {'{', '['};
const char closing_brackets[] {'}', ']'};
const int default_size = 100;
std::random_device rd;
std::mt19937 random_generator(rd());
std::uniform_int_distribution<int> boolean_chooser(0, 1);
std::uniform_int_distribution<char> char_chooser(0, 255);
const char digits[] = "0123456789";
std::uniform_int_distribution<int> digit_chooser(0, 9);

int add_number(char* json, int max_size) {
    const int min_size = 1;
    
    if (min_size >= max_size) {
        return 0;
    }

    std::uniform_int_distribution<int> size_chooser(min_size, max_size);
    int size = size_chooser(random_generator);
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
            int dot_position = dot_position_chooser(random_generator);
            json[dot_position] = '.';
        }
    }

    // if dot_position still equals first_digit_position, then there is no dot

    // removing any leading 0
    if (dot_position-first_digit_position != 1 && dot_position != first_digit_position) {
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
    const char escaped_char[] = "\"\\bfnrt"; // '\u' is handled differently
    std::uniform_int_distribution<int> escaped_char_chooser(0, 7);
    int min_size = 3;
    if (min_size >= max_size) {
        return 0;
    }

    std::uniform_int_distribution<int> size_chooser(min_size, max_size);
    int size = size_chooser(random_generator);

    json[0] = '"';
    json[size-1] = '"';

    std::uniform_int_distribution<int> char_size_chooser(1, 4);

    int i = size-2;
    while (i > 0) {
        int char_size;
        if (i > 4) {
            char_size = char_size_chooser(random_generator);
        }
        else {
            char_size = i;
        }

        switch (char_size) {
            case 1:
                do {
                    json[i] = random_generator() & 0b01111111;
                } while (json[i] == '"');
                break;
            case 2:
                json[i-1] = random_generator();
                if (json[i-1] == '\\'){
                    json[i] = escaped_char_chooser(random_generator);
                }
                else {
                    json[i-1] = (json[i-1] | 0b11000000) & 0b11011111;
                    while (static_cast<unsigned char>(json[i-1]) <= static_cast<unsigned char>(0xc2) ){
                        json[i-1] = (random_generator() | 0b11000000) & 0b11011111;
                    }
                    json[i] = (random_generator() | 0b10000000) & 0b10111111;
                }
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
                break;
            /*case 5:
                json[i-5] = '\\';
                json[i-4] = 'u';
                json[i-3] = digits[digit_chooser(random_generator)];
                json[i-2] = digits[digit_chooser(random_generator)];
                json[i-1] = digits[digit_chooser(random_generator)];
                json[i] = digits[digit_chooser(random_generator)];
                break;*/

        }
        i -= char_size;
    }


    return size;
}

void generate_json(std::fstream* file, int size) {
    std::uniform_int_distribution<int> bracket_chooser(0, 1);
    std::uniform_int_distribution<int> value_type_chooser(0, 3);
    std::uniform_int_distribution<int> close_bracket(0, 1);
    char* json = (char*) malloc(size);

    std::stack<char> closing_stack;
    std::stack<bool> use_comma;
    int bracket_index {bracket_chooser(random_generator)};
    json[0] = opening_brackets[bracket_index];
    use_comma.push(false);
    closing_stack.push(closing_brackets[bracket_index]);

    int i = 1;
    while (true) {
        if (i >= size) {
            break;
        }
        int space_left = size-i-closing_stack.size()-5;
        if (size - i - 5 <= closing_stack.size()) {
            // closing everything left
            while (!closing_stack.empty()) {
                json[i] = closing_stack.top();
                closing_stack.pop();
                i++;
            }
            break;
        }
        else if (space_left < 0) {
            // what we do ?
        }

        if (closing_stack.size() > 1 && close_bracket(random_generator)) { 
             json[i] = closing_stack.top();
             closing_stack.pop();
             use_comma.pop();
             i++;
             continue;
        }


        if (use_comma.top()) {
            json[i] = ',';
            i++;
        }
        else {
            use_comma.top() = true;
        }

        switch (value_type_chooser(random_generator)) {
            case 0:
            json[i] = '[';
            closing_stack.push(']');
            use_comma.push(false);
            i++;
            break;
            case 1:
            json[i] = '{';
            closing_stack.push('}');
            use_comma.push(false);
            i++;
            break;
            case 2:
            i += add_number(json+i*sizeof(char), space_left);
            break;
            case 3:
            i += add_string(json+i*sizeof(char), space_left);
            break;
        }
    }

    file->write(json, sizeof(char)*i);
    free(json);
}
}

int main(int argc, char** argv) {
    std::fstream file("test.txt", std::ios::out | std::ios::binary);
    int size = randomjson::default_size;
    if (argc > 1) {
        size = std::stoi(argv[1]);
    }
    randomjson::generate_json(&file, size);
    return 0;
}
