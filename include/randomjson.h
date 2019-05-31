#ifndef RANDOMJSON_H
#define RANDOMJSON_H

#include <bitset>
#include <fstream>
#include <random>
#include <stack>
#include <stdint.h>

namespace randomjson {
enum class ProvenanceType {
    file,
    seed
};

class RandomEngine {
    public:
    void seed(int new_seed);
    uint64_t next();
    bool next_bool() { return static_cast<bool>(next() & 1); }
    int next_int() { return static_cast<int>(next()); }
    char next_char() { return static_cast<char>(next()); }
    double next_double() { return static_cast<double>(next()); }
    char next_char_digit();
    char next_escaped_char();
    char next_hexa_digit();
    int next_ranged_int(int min, int max);

    private:
    uint64_t seed_;
    uint64_t wyhash64_x_;
};

class RandomJson {
    public:
    RandomJson(); // must be configurated munually
    RandomJson(int size); // automatically generates json
    RandomJson(int size, int seed); // automatically generates json
    RandomJson(std::string file); // from file for mutation
    ~RandomJson();

    void generate();
    void mutate();
    void save(std::string file_name);

    // getters and setters
    char* const get_json();
    int get_size();
    void set_size(int new_size);
    void set_generation_seed(int new_generation_seed);
    int get_generation_seed();
    void set_mutation_seed(int new_mutation_seed);
    int get_mutation_seed();
    int get_number_of_mutations();
    bool infos_correspond_to_json();
    ProvenanceType get_provenance_type();
    std::string get_filename();

    private:
    void generate_json(char* json, int size, RandomEngine& random_generator);
    int add_BOM(char* json);
    int init_object_or_array(char* json, std::stack<char>& closing_stack, std::stack<bool>& use_comma, int max_size, RandomEngine& random_generator);
    int randomly_close_bracket(char* json, std::stack<char>& closing_stack, std::stack<bool>& use_comma, RandomEngine& random_generator);
    int add_value(char* json, std::stack<char>& closing_stack, std::stack<bool>& use_comma, int max_size, RandomEngine& random_generator);
    int add_array_entry(char* json, std::stack<char>& closing_stack, std::stack<bool>& use_comma, int max_size, RandomEngine& random_generator);
    int add_object_entry(char* json, std::stack<char>& closing_stack, std::stack<bool>& use_comma, int max_size, RandomEngine& random_generator);
    int add_number(char* json, int max_size, RandomEngine& random_generator);
    int add_integer(char* json, int max_size, RandomEngine& random_generator);
    int add_float(char* json, int max_size, RandomEngine& random_generator);
    int add_string(char* json, int max_size, RandomEngine& random_generator);
    void add_whitespace(char* json, int max_size, RandomEngine& random_generator);
    int add_randomsized_whitespace(char* json, int max_size, RandomEngine& random_generator);

    ProvenanceType provenance_type;
    char* json;
    int size;
    int number_of_mutations;
    int generation_seed;
    int mutation_seed;
    std::string filename;
    bool bom;
    bool attributes_changed;

    RandomEngine generation_random;
    RandomEngine mutation_random;

    // options
    int max_number_range; // Numbers will be smaller than 10^range
    int max_number_size; // in bytes and in length
    int max_string_size; // in bytes
    int max_whitespace_size; // in bytes and in length
    int max_depth;
    float chances_have_BOM;
    float chances_over_max_number_range;
    float chances_over_max_number_size;
    float chances_over_max_string_size;
    float chances_over_max_whitespace_size;
    float chances_over_max_depth;
    float chances_invalid_comma;
    float chances_invalid_utf8;
    float chances_invalid_string;
    float chances_invalid_codepoint;
    float chances_invalid_number;
    float chances_generating_number;
    float chances_generating_string;
    float chances_generating_object;
    float chances_generating_array;
    float chances_generating_null;
    float chances_generating_false;
    float chances_generating_true;

};

void RandomEngine::seed(int new_seed) 
{
    seed_ = new_seed;
    wyhash64_x_ = new_seed;
}

uint64_t RandomEngine::next()
{
    wyhash64_x_ += UINT64_C(0x60bee2bee120fc15);
    __uint128_t tmp;
    tmp = (__uint128_t) wyhash64_x_ * UINT64_C(0xa3b195354a39b70d);
    uint64_t m1 = (tmp >> 64) ^ tmp;
    tmp = (__uint128_t) m1 * UINT64_C(0x1b03738712fad5c9);
    uint64_t m2 = (tmp >> 64) ^ tmp;
    return m2;
}

char RandomEngine::next_char_digit()
{
    const char digits[] = "0123456789";
    int position = next_ranged_int(0, 9);
    return digits[position];
}

char RandomEngine::next_escaped_char()
{
    const char escaped_char[] = "\"\\bfnrt";
    const int nb_escaped_char = 7;
    int position = next_ranged_int(0, nb_escaped_char-1);
    return escaped_char[position];
}

char RandomEngine::next_hexa_digit()
{
    const char hexa_digits[] = "0123456789ABCDEFabcdef";
    const int nb_hexa_digits = 22;
    int position = next_ranged_int(0, nb_hexa_digits-1);
    return hexa_digits[position];
}

int RandomEngine::next_ranged_int(int min, int max)
{
    if (min == max) {
        return min;
    }
    int range = max-min+1;
    uint64_t random_number = next();
    return random_number%range + min;
}

RandomJson::RandomJson()
: json(nullptr)
, size(0)
, number_of_mutations(0)
, generation_seed(0)
, mutation_seed(0)
, bom(false)
, attributes_changed(true)
, max_number_range(308)
, max_number_size(5)
, max_string_size(24)
, max_whitespace_size(24)
, max_depth(1024)
, chances_have_BOM(0)
, chances_over_max_number_range(0)
, chances_over_max_number_size(0)
, chances_over_max_string_size(0)
, chances_over_max_whitespace_size(0)
, chances_over_max_depth(0)
, chances_invalid_comma(0)
, chances_invalid_utf8(0)
, chances_invalid_string(0)
, chances_invalid_codepoint(0)
, chances_invalid_number(0)
, chances_generating_number(0)
, chances_generating_string(0)
, chances_generating_object(0)
, chances_generating_array(0)
, chances_generating_null(0)
, chances_generating_false(0)
, chances_generating_true(0)
{
}

RandomJson::RandomJson(int size)
: RandomJson::RandomJson(size, std::random_device{}())
{
}

RandomJson::RandomJson(int size, int generation_seed)
: RandomJson::RandomJson()
{
    this->size = size;
    this->generation_seed = generation_seed;
    mutation_seed = std::random_device{}();
    generation_random.seed(generation_seed);
    mutation_random.seed(mutation_seed);
    json = new char[size];
    generate();
}

RandomJson::RandomJson(std::string filename)
: RandomJson::RandomJson()
{
    this->filename = filename;
    provenance_type = ProvenanceType::file;
    generation_seed = std::random_device{}();
    mutation_seed = std::random_device{}();
    generation_random.seed(generation_seed);
    mutation_random.seed(mutation_seed);
    std::ifstream file (filename, std::ios::in | std::ios::binary | std::ios::ate);
    size = file.tellg();
    json = new char[size];
    file.seekg(0, std::ios::beg);
    file.read(json, size);
    file.close();
    attributes_changed = false;
}

RandomJson::~RandomJson()
{
    delete[] json;
}

int RandomJson::add_BOM(char* json)
{
    int size = 3;
    json[0] = 0xEF;
    json[1] = 0xBB;
    json[2] = 0xBF;

    return size;
}

int RandomJson::add_integer(char* json, int max_size, RandomEngine& random_generator)
{
    const int min_size = 1;
    int size = 0;
    if (min_size > max_size) {
        return size;
    }

    int number = random_generator.next_int();
    std::string string_number = std::to_string(number);
    size = std::min(static_cast<int>(string_number.size()), max_size);
    for (int i = 0; i < size; i++) {
        json[i] = string_number.at(i);
    }
    return size;
}

int RandomJson::add_float(char* json, int max_size, RandomEngine& random_generator)
{
    const int min_size = 3;
    int size = 0;
    if (min_size > max_size) {
        return size;
    }

    uint64_t significant = random_generator.next();
    int offset = 0;

    // trying to add a minus sign    
    const int required_space_for_sign = 2; // minus + digit
    if (max_size >= required_space_for_sign && random_generator.next_bool()) {
        json[offset] = '-';
        offset++;
    }

    std::string string_significant = std::to_string(significant);

    // trying to add a dot
    int dot_position = 0;
    const int required_space_for_dot = 3; // one digit + dot + one digit
    if (max_size - offset >= required_space_for_dot) {
        dot_position = random_generator.next_ranged_int(0, string_significant.size()-1);
        if (dot_position < string_significant.size()-1) { // so there's a change we don't add a dot
            if (dot_position == 0) {
                string_significant.at(0) = '0';
                string_significant.at(1) = '.';
            }
            else {
                string_significant.at(dot_position) = '.';
            }
        }
    }

    // adding all we can add of the significant
    int space_for_significant = std::min(static_cast<int>(string_significant.size()), max_size - offset);
    for (int i = 0; i < space_for_significant; i++) {
        json[offset] = string_significant.at(i);
        offset++;
    }

    int exponent = random_generator.next_ranged_int(0, max_number_range-dot_position);
    std::string string_exponent = std::to_string(exponent);

    // trying to add the exponent
    const int required_space_for_exponent = 2; // e + one digit
    if (max_size - offset >= required_space_for_exponent) {
        switch (random_generator.next_bool()) {
            case true: json[offset] = 'e'; break;
            case false: json[offset] = 'E'; break;
        }
        offset++;

        // trying to add a sign
        if (max_size - offset >= required_space_for_sign) {
            switch (random_generator.next_ranged_int(0,2)) {
                case 0:
                    break;
                case 1:
                    json[offset] = '-';
                    offset++;
                    break;
                case 2:
                    json[offset] = '+';
                    offset++;
                    break;
            }
        }

        int space_for_exponent = std::min(static_cast<int>(string_exponent.size()), max_size - offset);
        for (int i = 0; i < space_for_exponent; i++) {
            json[offset] = string_exponent.at(i);
            offset++;
        }
    }
    
    size = offset;

    return size;
}

int RandomJson::add_number(char* json, int max_size, RandomEngine& random_generator)
{
    const int min_size = 1;
    int size = 0;
    if (min_size > max_size) {
        return size;
    }

    // boolean used to choose between to cases. No truthness involved.
    switch (random_generator.next_bool()) {
        case true:
            size = add_float(json, max_size, random_generator);
            break;
        case false:
            size = add_integer(json, max_size, random_generator);
            break;
    }

    return size;
}

int RandomJson::add_string(char* json, int max_size, RandomEngine& random_generator)
{
    int min_size = 2;
    int size = 0;
    if (min_size > max_size) {
        return size;
    }

    max_size = std::min(max_size, max_string_size);

    size = random_generator.next_ranged_int(min_size, max_size);

    json[0] = '"';
    json[size-1] = '"';

    // Numbers associated to types are arbitrary

    int i = size-min_size;
    while (i > 0) {
        int char_size;
        int char_type;
        if (i > 6) {
            //char_type = random_generator.next_ranged_int(1, 5);
            char_type = random_generator.next_ranged_int(1, 4);
        }
        else if (i > 4) {
            char_type = random_generator.next_ranged_int(1, 4);
        }
        else {
            char_type = i;
        }
        switch (char_type) {
        case 1: // One byte character
            do {
                json[i] = random_generator.next_char() & 0b01111111;
            } while (json[i] == '"' || json[i] == '\\' || json[i] <= 0x1f);
            char_size = 1;
            break;
        case 2: // Two bytes character or escaped character
            json[i-1] = random_generator.next_char();
            if (json[i-1] == '\\') {
                json[i] = random_generator.next_escaped_char();
            }
            else {
                json[i-1] = (json[i-1] | 0b11000000) & 0b11011111;
                while (static_cast<unsigned char>(json[i-1]) <= static_cast<unsigned char>(0xc2)){
                    json[i-1] = (random_generator.next_char() | 0b11000000) & 0b11011111;
                }
                json[i] = (random_generator.next_char() | 0b10000000) & 0b10111111;
            }
            char_size = 2;
            break;
        case 3: // Three bytes character
            json[i-2] = (random_generator.next_char() | 0b11100000) & 0b11101111;
            json[i-1] = (random_generator.next_char() | 0b10000000) & 0b10111111;
            json[i] = (random_generator.next_char() | 0b10000000) & 0b10111111;
            if (static_cast<unsigned char>(json[i-2]) == static_cast<unsigned char>(0xe0)) {
                while (static_cast<unsigned char>(json[i-1]) < static_cast<unsigned char>(0xa0)) {
                    json[i-1] = (random_generator.next_char() | 0b10000000) & 0b10111111;
                }
            }
            else if (static_cast<unsigned char>(json[i-2]) == static_cast<unsigned char>(0xed)) {
                while (static_cast<unsigned char>(json[i-1]) > static_cast<unsigned char>(0x9f)) {
                    json[i-1] = (random_generator.next_char() | 0b10000000) & 0b10111111;
                }
            }
            char_size = 3;
            break;
        case 4: // Four bytes character
            json[i-3] = 0b11110000 | random_generator.next_ranged_int(0, 4);
            json[i-2] = (random_generator.next_char() | 0b10000000) & 0b10111111;
            json[i-1] = (random_generator.next_char() | 0b10000000) & 0b10111111;
            json[i] = (random_generator.next_char() | 0b10000000) & 0b10111111;
            if (static_cast<unsigned char>(json[i-3]) == static_cast<unsigned char>(0xf0)) {
                while (static_cast<unsigned char>(json[i-2]) < static_cast<unsigned char>(0x90)) {
                    json[i-2] = (random_generator.next_char() | 0b10000000) & 0b10111111;
                }
            }
            else if (static_cast<unsigned char>(json[i-3]) == static_cast<unsigned char>(0xf4)) {
                while (static_cast<unsigned char>(json[i-2]) > static_cast<unsigned char>(0x8f)) {
                    json[i-2] = (random_generator.next_char() | 0b10000000) & 0b10111111;
                }
            }
            char_size = 4;
            break;
        case 5: // codepoint
            json[i-5] = '\\';
            json[i-4] = 'u';
            json[i-3] = random_generator.next_hexa_digit();
            json[i-2] = random_generator.next_hexa_digit();
            json[i-1] = random_generator.next_hexa_digit();
            json[i] = random_generator.next_hexa_digit();
            char_size = 6;
            break;
        }

        i -= char_size;
    }

    return size;
}

int RandomJson::add_array_entry(char* json, std::stack<char>& closing_stack, std::stack<bool>& use_comma, int max_size, RandomEngine& random_generator)
{
    int comma_length = use_comma.top() ? 1: 0;
    int size = 0;
    if (max_size < comma_length) {
        return size;
    }

    int offset = add_randomsized_whitespace(json, max_size, random_generator);

    if (use_comma.top()) {
        json[offset] = ',';
        offset++;
        offset += add_randomsized_whitespace(&json[offset], max_size-offset, random_generator);
    }
    
    int value_size = add_value(&json[offset], closing_stack, use_comma, max_size-offset, random_generator);

    // we make sure a value has been written
    if (value_size == 0) {
        return size;
    }

    size = value_size + offset;
    return size;
}

int RandomJson::add_object_entry(char* json, std::stack<char>& closing_stack, std::stack<bool>& use_comma, int max_size, RandomEngine& random_generator)
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
    int offset = add_randomsized_whitespace(json, max_size - min_size, random_generator);

    // Adding comma before key if necessary
    if (use_comma.top()) {
        json[offset] = ',';
        offset++;
        min_size -= comma_length;
        // Adding whitespace after comma
        offset += add_randomsized_whitespace(&json[offset], max_size - offset - min_size, random_generator);
    }
    // Adding key
    min_size -= min_key_size;
    int key_size = add_string(&json[offset], max_size - offset - min_size, random_generator);
    offset += key_size;
    // Adding space after key and before colon
    offset += add_randomsized_whitespace(&json[offset], max_size - offset - min_size, random_generator);
    // Adding colon
    json[offset] = ':';
    min_size -= colon_size;
    offset++;
    // Adding whitespace after colon and before value
    offset += add_randomsized_whitespace(&json[offset], max_size - offset - min_size, random_generator);
    // Adding value
    int value_size = add_value(&json[offset], closing_stack, use_comma, max_size - offset, random_generator);

    // Quick fix if we failed to write a value
    // This should not happen, but it actually happens frequently at the end of documents.
    if (value_size == 0) {
        return size;
    }

    size = offset + value_size;
    return size;
}

int RandomJson::add_value(char* json, std::stack<char>& closing_stack, std::stack<bool>& use_comma, int max_size, RandomEngine& random_generator)
{
    const int min_size = 1;
    int size = 0;

    if (min_size > max_size) {
        return size;
    }

    // the number associated to the type is arbitrary
    switch (random_generator.next_ranged_int(0, 2)) {
    case 0:
        size = init_object_or_array(json, closing_stack, use_comma, max_size, random_generator);
        break;
    case 1:
        size = add_string(json, max_size, random_generator);
        if (size != 0) {
            use_comma.top() = true;
        }
    break;
    case 2:
        size = add_number(json, max_size, random_generator);
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

int RandomJson::init_object_or_array(char* json, std::stack<char>& closing_stack, std::stack<bool>& use_comma, int max_size, RandomEngine& random_generator)
{
    const int min_size = 2;
    int size = 0;
    if (min_size > max_size) {
        return size;
    }

    if (closing_stack.size() >= max_depth-2) {
        return size;
    }
    
    if (random_generator.next_bool()) {
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

int RandomJson::randomly_close_bracket(char* json, std::stack<char>& closing_stack, std::stack<bool>& use_comma, RandomEngine& random_generator)
{
    const uint32_t magic_closer = 0x40000000;
    int size = 0;
    if (closing_stack.size() > 1 && magic_closer >= static_cast<uint32_t>(random_generator.next_int())) {
        json[0] = closing_stack.top();
        closing_stack.pop();
        use_comma.pop();
        use_comma.top() = true;
        size = 1;
    }
    return size;
}

int RandomJson::add_randomsized_whitespace(char* json, int max_size, RandomEngine& random_generator)
{
    const int min_size = 0;
    int size = 0;
    if (max_size < min_size) {
        return size;
    }

    max_size = std::min(max_size, max_whitespace_size);
    size = random_generator.next_ranged_int(min_size, max_size);
    add_whitespace(json, size, random_generator);

    return size;
}

void RandomJson::add_whitespace(char* json, int size, RandomEngine& random_generator)
{
    const char whitespaces[] {0x09, 0x0A, 0x0D, 0x20};

    for (int i = 0; i < size; i++) {
        json[i] = whitespaces[random_generator.next_ranged_int(0, 3)];
    }
}

void RandomJson::generate() {
    int offset = 0;
    if (bom) {
        offset = add_BOM(json);
    }
    generate_json(&json[offset], size-offset, generation_random);
    mutation_random.seed(mutation_seed);
    number_of_mutations = 0;
    provenance_type = ProvenanceType::seed;
    attributes_changed = false;
}

void RandomJson::generate_json(char* json, int size, RandomEngine& random_generator)
{
    int offset = 0;
    std::stack<char> closing_stack; // Used to keep track of the structure we're in
    std::stack<bool> use_comma; // Used to keep track if a comma is necessary or not

    offset += add_randomsized_whitespace(&json[offset], size, random_generator);
    offset += init_object_or_array(&json[offset], closing_stack,  use_comma, size-offset, random_generator);
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
            add_whitespace(&json[offset], size-offset, random_generator);
            break;
        }
        else if (space_left < 0) {
            // There's a problem. What we do ?
        }
        int closing_offset = randomly_close_bracket(&json[offset], closing_stack, use_comma, random_generator);
        offset += closing_offset;
        space_left -= closing_offset;
        switch (closing_stack.top()) {
        case ']' :
            offset += add_array_entry(&json[offset], closing_stack, use_comma, space_left, random_generator);
            break;
        case '}':
            offset += add_object_entry(&json[offset], closing_stack, use_comma, space_left, random_generator);
            break;
        }
    }
}

void RandomJson::mutate() {
    number_of_mutations++;
    char opening_bracket;
    int opening_bracket_position = -1;
    char closing_bracket;
    int closing_bracket_position = size-1;
    int brackets_to_close = 1;

    // finding a random opening bracket
    int position = mutation_random.next_ranged_int(0, size);    
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
        generate_json(&json[opening_bracket_position], size, mutation_random);
    }
}

void RandomJson::save(std::string file_name)
{
    std::fstream file(file_name, std::ios::out | std::ios::binary);
    file.write(json, sizeof(char)*size);
    file.close();
}

/*
** Getters and setters 
*/
char* const RandomJson::get_json()
{
    return json;
}

int RandomJson::get_size()
{
    return size;
}

void RandomJson::set_size(int new_size)
{
    size = new_size;
    json = new char[size];
    attributes_changed = true;
}

int RandomJson::get_number_of_mutations()
{
    return number_of_mutations;
}

void RandomJson::set_generation_seed(int new_generation_seed)
{
    generation_seed = new_generation_seed;
    generation_random.seed(new_generation_seed);
    attributes_changed = true;
}

// The generation seed has no meaning if the json is from a file
int RandomJson::get_generation_seed()
{
    return generation_seed;
}

void RandomJson::set_mutation_seed(int new_mutation_seed)
{
    mutation_seed = new_mutation_seed;
    mutation_random.seed(new_mutation_seed);
    attributes_changed = true;
}

int RandomJson::get_mutation_seed()
{
    return mutation_seed;
}

bool RandomJson::infos_correspond_to_json()
{
    return attributes_changed;
}

ProvenanceType RandomJson::get_provenance_type()
{
    return provenance_type;
}

std::string RandomJson::get_filename()
{
    return filename;
}

}

#endif