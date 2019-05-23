#ifndef RANDOMJSON_H
#define RANDOMJSON_H

#include <bitset>
#include <fstream>
#include <random>
#include <stack>

// public api
namespace randomjson {

class RandomJson {
    public:
    RandomJson(int size);
    RandomJson(int size, int seed);
    ~RandomJson();

    void mutate();
    char* get_json();
    int get_size();
    void set_seed(int chosen_seed);
    int get_seed();
    void activate_BOM();
    void deactivate_BOM();
    void save(std::string file_name);

    private:
    void generate_json(char* json, int size);
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

    char* json;
    int size;

    std::random_device rd;
    int seed;
    std::mt19937 random_generator;
    std::uniform_int_distribution<int> boolean_chooser;
    std::uniform_int_distribution<char> char_chooser;
    bool BOM_is_activated = false;
};

RandomJson::RandomJson(int size)
: size(size)
, rd()
, seed(rd())
, random_generator(seed)
, boolean_chooser(0,1)
, char_chooser(0,255)
{
    json = new char[size];
    generate_json(json, size);
}

RandomJson::RandomJson(int size, int seed)
: size(size)
, seed(seed)
, random_generator(seed)
, boolean_chooser(0,1)
, char_chooser(0,255)
{
    json = new char[size];
    generate_json(json, size);
}

RandomJson::~RandomJson()
{
    delete[] json;
}

int RandomJson::randomly_add_BOM(char* json)
{
    int size = 0;
    if (boolean_chooser(random_generator)) {
        json[0] = 0xEF;
        json[1] = 0xBB;
        json[2] = 0xBF;
        size = 3;
    }

    return size;
}

int RandomJson::add_number(char* json, int max_size)
{
    const int min_size = 1;
    int size = 0;
    if (min_size > max_size) {
        return size;
    }

    const char digits[] = "0123456789";
    std::uniform_int_distribution<int> digit_chooser(0, 9);
    std::normal_distribution<float> size_chooser(min_size*2, max_size/8);
    size = std::abs(size_chooser(random_generator));
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

int RandomJson::add_string(char* json, int max_size)
{
    int min_size = 2;
    int size = 0;
    if (min_size > max_size) {
        return size;
    }

    const char hexa_digits[] = "0123456789ABCDEFabcdef";
    const int nb_hexa_digits = 22;
    std::uniform_int_distribution<int> hexa_digit_chooser(0, nb_hexa_digits-1);
    const char escaped_char[] = "\"\\bfnrt"; // '\u' is handled differently
    const int nb_escaped_char = 7;
    std::uniform_int_distribution<int> escaped_char_chooser(0, nb_escaped_char-1);

    std::normal_distribution<float> size_chooser(min_size*2, max_size/8);
    size = std::abs(size_chooser(random_generator));
    if (size < min_size) size = min_size;
    if (size > max_size) size = max_size;

    json[0] = '"';
    json[size-1] = '"';

    // Numbers associated to types are arbitrary
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
        case 1: // One byte character
            do {
                json[i] = random_generator() & 0b01111111;
            } while (json[i] == '"' || json[i] == '\\' || json[i] <= 0x1f);
            char_size = 1;
            break;
        case 2: // Two bytes character or escaped character
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
        case 3: // Three bytes character
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
        case 4: // Four bytes character
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
        case 5: // \uXXXX character
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

int RandomJson::add_array_entry(char* json, std::stack<char>& closing_stack, std::stack<bool>& use_comma, int max_size)
{
    int comma_length = use_comma.top() ? 1: 0;
    int size = 0;
    if (max_size < comma_length) {
        return size;
    }

    int offset = add_randomsized_whitespace(json, max_size);

    if (use_comma.top()) {
        json[offset] = ',';
        offset++;
        offset += add_randomsized_whitespace(&json[offset], max_size-offset);
    }
    
    size = add_value(&json[offset], closing_stack, use_comma, max_size-offset) + offset;
    return size;
}

int RandomJson::add_object_entry(char* json, std::stack<char>& closing_stack, std::stack<bool>& use_comma, int max_size)
{
    const int min_key_size = 2;
    const int colon_size = 1;
    const int min_value_size = 1;
    int comma_length = use_comma.top() ? 1 : 0;
    int min_size = min_key_size + colon_size + min_value_size + comma_length;
    int size = 0;
    if (min_size > max_size) {
        return size;
    }
    // Adding whitespace before key or comma
    int offset = add_randomsized_whitespace(json, max_size - min_size);

    // Adding comma before key if necessary
    if (use_comma.top()) {
        json[offset] = ',';
        offset++;
        min_size -= comma_length;
        // Adding whitespace after comma
        offset += add_randomsized_whitespace(&json[offset], max_size - offset - min_size);
    }
    // Adding key
    min_size -= min_key_size;
    int key_size = add_string(&json[offset], max_size - offset - min_size);
    offset += key_size;
    // Adding space after key and before colon
    offset += add_randomsized_whitespace(&json[offset], max_size - offset - min_size);
    // Adding colon
    json[offset] = ':';
    min_size -= colon_size;
    offset++;
    // Adding whitespace after colon and before value
    offset += add_randomsized_whitespace(&json[offset], max_size - offset - min_size);
    // Adding value
    int value_size = add_value(&json[offset], closing_stack, use_comma, max_size - offset);

    // Quick fix if we failed to write a value
    // This should not happen, but it actually happens frequently at the end of documents.
    if (value_size == 0) {
        return size;
    }

    size = offset + value_size;
    return size;
}

int RandomJson::add_value(char* json, std::stack<char>& closing_stack, std::stack<bool>& use_comma, int max_size)
{
    const int min_size = 1;
    int size = 0;

    if (min_size > max_size) {
        return size;
    }

    // the number associated to the type is arbitrary
    std::uniform_int_distribution<int> value_type_chooser(0, 2);
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

int RandomJson::init_object_or_array(char* json, std::stack<char>& closing_stack, std::stack<bool>& use_comma, int max_size)
{
    const int min_size = 2;
    int size = 0;
    if (min_size > max_size) {
        return size;
    }
    
    if (boolean_chooser(random_generator)) {
        json[0] = '[';
        closing_stack.push(']');
    }
    else {
        json[0] = '{';
        closing_stack.push('}');
    }
    size = 1;
    use_comma.push(false);

    return size;
}

int RandomJson::randomly_close_bracket(char* json, std::stack<char>& closing_stack, std::stack<bool>& use_comma)
{
    const int magic_closer = 0x40000000;
    int size = 0;
    if (closing_stack.size() > 1 && magic_closer >= random_generator()) {
        json[0] = closing_stack.top();
        closing_stack.pop();
        use_comma.pop();
        use_comma.top() = true;
        size = 1;
    }
    return size;
}

int RandomJson::add_randomsized_whitespace(char* json, int max_size)
{
    std::normal_distribution<float> size_chooser(0, max_size/64);
    int size = std::abs(size_chooser(random_generator));
    if (size < 0) size = 0;
    if (size > max_size) size = max_size;
    add_whitespace(json, size);

    return size;
}

void RandomJson::add_whitespace(char* json, int size)
{
    const char whitespaces[] {0x09, 0x0A, 0x0D, 0x20};
    std::uniform_int_distribution<char> char_chooser(0, 3);

    for (int i = 0; i < size; i++) {
        json[i] = whitespaces[char_chooser(random_generator)];
    }
}

void RandomJson::generate_json(char* json, int size)
{
    int offset = 0;
    if (BOM_is_activated) {
        offset = randomly_add_BOM(json);
    }
    std::stack<char> closing_stack; // Used to keep track of the structure we're in
    std::stack<bool> use_comma; // Used to keep track if a comma is necessary or not

    offset += add_randomsized_whitespace(&json[offset], size);
    offset += init_object_or_array(&json[offset], closing_stack,  use_comma, size-offset);
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
            add_whitespace(&json[offset], size-offset);
            break;
        }
        else if (space_left < 0) {
            // There's a problem. What we do ?
        }
        int closing_offset = randomly_close_bracket(&json[offset], closing_stack, use_comma);
        offset += closing_offset;
        space_left -= closing_offset;
        switch (closing_stack.top()) {
        case ']' :
            offset += add_array_entry(&json[offset], closing_stack, use_comma, space_left);
            break;
        case '}':
            offset += add_object_entry(&json[offset], closing_stack, use_comma, space_left);
            break;
        }
    }
}

void RandomJson::mutate() {
    char opening_bracket;
    int opening_bracket_position = -1;
    char closing_bracket;
    int closing_bracket_position = size-1;
    int brackets_to_close = 1;

    // finding a random opening bracket
    std::uniform_int_distribution<int> position_chooser(0, size);
    int position = position_chooser(random_generator);    
    while (position < size-1) {
        switch (json[position]) {
            case '{':
                opening_bracket = '{';
                closing_bracket = '}';
                opening_bracket_position = position;
                break;
            case '[':
                opening_bracket = '[';
                closing_bracket = ']';
                opening_bracket_position = position;
                break;
            default:
                position++;
                break;
        }
        if (opening_bracket_position > -1) {
            break;
        }
    }
    
    // finding the corresponding closing bracket
    while (position < size-1) {
        position++;
        if (json[position] == opening_bracket) {
            brackets_to_close++;
        }
        else if (json[position] == closing_bracket) {
            brackets_to_close--;
            if (brackets_to_close == 0) {
                closing_bracket_position = position;
                break;
            }
        }
    }

    // generating something random
    if (closing_bracket_position < size) {
        int size = closing_bracket_position - opening_bracket_position + 1;
        generate_json(&json[opening_bracket_position], size);
    }
}

char* RandomJson::get_json() {
    return json;
}
int RandomJson::get_size() {
    return size;
}

void RandomJson::save(std::string file_name)
{
    std::fstream file(file_name, std::ios::out | std::ios::binary);
    file.write(json, sizeof(char)*size);
    file.close();
}

void RandomJson::set_seed(int chosen_seed)
{
    seed = chosen_seed;
    random_generator.seed(seed);
}

int RandomJson::get_seed()
{
    return seed;
}

void RandomJson::activate_BOM()
{
    BOM_is_activated = true;
}

void RandomJson::deactivate_BOM()
{
    BOM_is_activated = false;
}

}

#endif