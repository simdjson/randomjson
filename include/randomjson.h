#ifndef RANDOMJSON_H
#define RANDOMJSON_H

#include <bitset>
#include <fstream>
#include <random>
#include <stack>
#include <stdint.h>

namespace randomjson {

class RandomEngine {
    public:
    void seed(int new_seed) {
        seed_ = new_seed;
        wyhash64_x_ = new_seed;
    };
    uint64_t next() {
        // Adaptated from https://github.com/wangyi-fudan/wyhash/blob/master/wyhash.h
        // Inspired from https://github.com/lemire/testingRNG/blob/master/source/wyhash.h
        wyhash64_x_ += UINT64_C(0x60bee2bee120fc15);
        __uint128_t tmp;
        tmp = (__uint128_t) wyhash64_x_ * UINT64_C(0xa3b195354a39b70d);
        uint64_t m1 = (tmp >> 64) ^ tmp;
        tmp = (__uint128_t) m1 * UINT64_C(0x1b03738712fad5c9);
        uint64_t m2 = (tmp >> 64) ^ tmp;
        return m2;
    }
    bool next_bool() { return (next() & 1) == 1; }
    int next_int() { return static_cast<int>(next()); }
    char next_char() { return static_cast<char>(next()); }
    double next_double() { return static_cast<double>(next()); }
    int next_ranged_int(int min, int max) {
            if (min == max) {
            return min;
        }
        int range = max-min+1;
        uint64_t random_number = next();
        return random_number%range + min;
    }

    private:
    uint64_t seed_;
    uint64_t wyhash64_x_;
};

struct Settings {
    // If filepath is different than an empty string, RandomJson will load from the corresponding file.
    // That means the json document won't be randomly generated.
    // That implies the generation_seed won't be used and the size will be adjusted to the file's size.
    // The other options will be used for the mutations, but they will not correspond to the original document.
    std::string filepath;
    int size = 0;
    int generation_seed = std::random_device{}();
    int mutation_seed = std::random_device{}();
    int number_of_mutations = 0;
    std::vector<int> skipped_mutations; // we want to skip mutations that causes errors
    bool bom = false;
    int max_number_range = 308; // Numbers will be smaller than 10^range
    int max_number_size; // in bytes and in length
    int max_string_size = 2048; // in bytes
    int max_whitespace_size = 24; // in bytes and in length
    int max_depth = 1024;
    // These are other option ideas that are not currently implemented.
    /*float chances_have_BOM = 0;
    float chances_over_max_number_range = 0;
    float chances_over_max_number_size = 0;
    float chances_over_max_string_size = 0;
    float chances_over_max_whitespace_size = 0;
    float chances_over_max_depth = 0;
    float chances_invalid_comma = 0;
    float chances_invalid_utf8 = 0;
    float chances_invalid_string = 0;
    float chances_invalid_codepoint = 0;
    float chances_invalid_number = 0;
    float chances_generating_number = 0;
    float chances_generating_string = 0;
    float chances_generating_object = 0;
    float chances_generating_array = 0;
    float chances_generating_null = 0;
    float chances_generating_false = 0;
    float chances_generating_true = 0;*/

    Settings() {}

    Settings(int size)
    : size(size)
    {}

    Settings(int size, int mutation_seed)
    : size(size)
    , mutation_seed(mutation_seed)
    {}

    Settings(std::string filepath)
    : filepath(filepath)
    {}
};


class RandomJson {
    public:
    RandomJson(const Settings& settings);
    ~RandomJson();

    // Randomly modify bytes
    void mutate();
    // Reverse the modifications of the last mutation. Only the last one.
    void reverse_mutation();
    void save(std::string file_name);
    void load_settings(const Settings& new_settings);

    // getters
    const char* get_json();
    int get_size();
    int get_generation_seed();
    int get_mutation_seed();
    int get_number_of_mutations();
    bool is_from_file();
    std::string get_filepath();

    private:
    // Generates an entire json document
    void generate();
    // Loads a json document from a file
    void load_file(const std::string& filepath);
    // Generates a valid json value taking exactly a given size (in bytes) on a given position.
    // Function's name is poorly chosen.
    void generate_json(char* json, int size, RandomEngine& random_generator);
    // Inserts a BOM at the beginning of the json document.
    int insert_BOM(char* json);
    // Randomly inserts "{" or "[" in the document.
    int init_object_or_array(char* json, std::stack<char>& closing_stack, std::stack<bool>& use_comma, int max_size, RandomEngine& random_generator);
    // Randomly chooses to close or not to close the current container.
    int randomly_close_bracket(char* json, std::stack<char>& closing_stack, std::stack<bool>& use_comma, RandomEngine& random_generator);
    // Randomly inserts any json value
    int insert_value(char* json, std::stack<char>& closing_stack, std::stack<bool>& use_comma, int max_size, RandomEngine& random_generator);
    // Inserts a random array entry
    int insert_array_entry(char* json, std::stack<char>& closing_stack, std::stack<bool>& use_comma, int max_size, RandomEngine& random_generator);
    // Inserts a random key followed by a random value.
    int insert_object_entry(char* json, std::stack<char>& closing_stack, std::stack<bool>& use_comma, int max_size, RandomEngine& random_generator);
    // Randomly chooses to insert a random integer or a random float
    int insert_number(char* json, int max_size, RandomEngine& random_generator);
    // Inserts a random integer
    int insert_integer(char* json, int max_size, RandomEngine& random_generator);
    // Inserts a random float
    int insert_float(char* json, int max_size, RandomEngine& random_generator);
    // inserts a random string
    int insert_string(char* json, int max_size, RandomEngine& random_generator);
    // Inserts a random sequence of whitespaces for a random size of bytes
    int insert_whitespace(char* json, int max_size, RandomEngine& random_generator);
    // Inserts a random sequences of whitespaces for a given size of bytes.
    void insert_givensized_whitespace(char* json, int size, RandomEngine& random_generator);

    char* json;

    RandomEngine generation_random;
    RandomEngine mutation_random;

    // To save value of a mutated byte
    struct SavedByte {
        int position;
        char value;
    };
    std::vector<SavedByte> saved_bytes;

    Settings settings;
};

RandomJson::RandomJson(const Settings& settings)
: settings(settings)
{
    generation_random.seed(settings.generation_seed);
    mutation_random.seed(settings.mutation_seed);
    if (settings.filepath != "") {
        load_file(settings.filepath);
    }
    else {
        generate();
    }

    for (int i = 0; i < settings.number_of_mutations; i++) {
        mutate();
    }
}

RandomJson::~RandomJson()
{
    delete[] json;
}

int RandomJson::insert_BOM(char* json)
{
    int size = 3;
    json[0] = 0xEF;
    json[1] = 0xBB;
    json[2] = 0xBF;

    return size;
}

int RandomJson::insert_integer(char* json, int max_size, RandomEngine& random_generator)
{
    const int min_size = 1;
    int size = 0;
    if (min_size > max_size) {
        return size;
    }

    int number = random_generator.next_int();

    // preventing single minus sign
    if (max_size == min_size && number < 0) {
        number = -number;
    }

    // Inserting the most digits we can
    std::string string_number = std::to_string(number);
    size = std::min(static_cast<int>(string_number.size()), max_size);
    for (int i = 0; i < size; i++) {
        json[i] = string_number.at(i);
    }
    return size;
}

int RandomJson::insert_float(char* json, int max_size, RandomEngine& random_generator)
{
    const int min_size = 3; // single digit + dot + single digit
    int size = 0;
    if (min_size > max_size) {
        return size;
    }

    int offset = 0;

    // trying to insert a minus sign    
    const int required_space_for_sign = 1 + min_size;
    if (max_size >= required_space_for_sign && random_generator.next_bool()) {
        json[offset] = '-';
        offset++;
    }

    uint64_t significant = random_generator.next();
    std::string string_significant = std::to_string(significant);

    // trying to insert a dot
    int dot_position = 0;
    bool dot_inserted = false;
    int max_dot_position = std::min(static_cast<int>(string_significant.size()), max_size - offset) - 1;
    dot_position = random_generator.next_ranged_int(0, max_dot_position);
    if (dot_position < max_dot_position-1) { // A dot can't end a float. We leave a chance to not insert a dot.
        if (dot_position == 0) {
            string_significant.at(0) = '0';
            string_significant.at(1) = '.';
        }
        else {
            string_significant.at(dot_position) = '.';
        }
        dot_inserted = true;
    }
    // inserting all we can from the significant
    int space_for_significant = std::min(static_cast<int>(string_significant.size()), max_size - offset);
    if (!dot_inserted) {
        space_for_significant -= 2;
    }
    for (int i = 0; i < space_for_significant; i++) {
        json[offset] = string_significant.at(i);
        offset++;
    }

    int exponent = random_generator.next_ranged_int(0, settings.max_number_range-dot_position);
    std::string string_exponent = std::to_string(exponent);

    // trying to insert the exponent
    const int required_space_for_exponent = 2; // e + one digit
    // If there is no dot, then there must be an exponent, otherwise it won't be a float
    // If we still have no dot, then we still have space for a an exponent
    if (!dot_inserted || max_size - offset >= required_space_for_exponent) {
        switch (random_generator.next_bool()) {
            case true: json[offset] = 'e'; break;
            case false: json[offset] = 'E'; break;
        }
        offset++;

        // trying to insert a sign
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

        // Inserting the most digits we can for the exponent
        int space_for_exponent = std::min(static_cast<int>(string_exponent.size()), max_size - offset);
        for (int i = 0; i < space_for_exponent; i++) {
            json[offset] = string_exponent.at(i);
            offset++;
        }
    }
    
    size = offset;

    return size;
}

int RandomJson::insert_number(char* json, int max_size, RandomEngine& random_generator)
{
    const int min_size = 1;
    int size = 0;
    if (min_size > max_size) {
        return size;
    }

    // boolean used to choose between two cases. No truthness involved.
    switch (random_generator.next_bool()) {
        case true:
            size = insert_float(json, max_size, random_generator);
            break;
        case false:
            size = insert_integer(json, max_size, random_generator);
            break;
    }

    return size;
}

int insert_escaped_codepoint(char* json, int max_size, RandomEngine& random_generator) {
    const char hexa_digits[] = "0123456789ABCDEF0123456789abcdef";
    const int min_size = 4;
    int size = 0;
    if (min_size > max_size) {
        return size;
    }

    // multi purpose random bits
    uint64_t random_bits = random_generator.next();
    uint16_t wanabe_codepoint = random_bits & 0xffff;

    bool is_low_surrogate = false;
    bool is_high_surrogate = false;

    // will be use if the first codepoint is the part of a surrogate pair
    uint16_t second_wanabe_codepoint;

    // checking if we have a surrogate pair
    if (0xd800 <= wanabe_codepoint && wanabe_codepoint <= 0xdbff) {
        is_high_surrogate = true;
    }
    else if (0xdc00 <= wanabe_codepoint && wanabe_codepoint <= 0xdfff) {
        is_low_surrogate = true;
        second_wanabe_codepoint = wanabe_codepoint;
    }

    // checking if we have enough place for the surrogate we may have
    const int surrogate_pair_size = 10;
    bool surrogate_pair_allowed = (max_size >= surrogate_pair_size) ? true : false;
    if (!surrogate_pair_allowed && (is_high_surrogate || is_low_surrogate)) {
        // don't have enough space for a surrogate pair
        return size;
    }

    // if we have a part of a surrogate pair, we generate the other part
    if (is_high_surrogate) {
        second_wanabe_codepoint = random_generator.next_ranged_int(0xdc00, 0xdfff);
    }
    else if (is_low_surrogate) {
        wanabe_codepoint = random_generator.next_ranged_int(0xd800, 0xdbff);
    }

    // will be used to randomly chose between capital or minuscule hexa digit
    random_bits >>= 16;

    // inserting codepoint
    json[3] = hexa_digits[(wanabe_codepoint & 0xf) + (random_bits & 0x10)];
    wanabe_codepoint >>= 4;
    random_bits >>= 1;
    json[2] = hexa_digits[(wanabe_codepoint & 0xf) + (random_bits & 0x10)];
    wanabe_codepoint >>= 4;
    random_bits >>= 1;
    json[1] = hexa_digits[(wanabe_codepoint & 0xf) + (random_bits & 0x10)];
    wanabe_codepoint >>= 4;
    random_bits >>= 1;
    json[0] = hexa_digits[(wanabe_codepoint & 0xf) + (random_bits & 0x10)];

    // inserting second surrogate or not
    if (is_high_surrogate || is_low_surrogate) {
        json[4] = '\\';
        json[5] = 'u';
        json[9] = hexa_digits[(second_wanabe_codepoint & 0xf) + (random_bits & 0x10)];
        second_wanabe_codepoint >>= 4;
        random_bits >>= 1;
        json[8] = hexa_digits[(second_wanabe_codepoint & 0xf) + (random_bits & 0x10)];
        second_wanabe_codepoint >>= 4;
        random_bits >>= 1;
        json[7] = hexa_digits[(second_wanabe_codepoint & 0xf) + (random_bits & 0x10)];
        second_wanabe_codepoint >>= 4;
        random_bits >>= 1;
        json[6] = hexa_digits[(second_wanabe_codepoint & 0xf) + (random_bits & 0x10)];
        
        size = surrogate_pair_size;
    }
    else {
        size = min_size;
    }

    return size;
}

int RandomJson::insert_string(char* json, int max_size, RandomEngine& random_generator) {
    int min_size = 2;
    int size = 0;
    if (min_size > max_size) {
        return size;
    }

    max_size = std::min(max_size, settings.max_string_size);

    int offset = 0;
    json[offset] = '"';
    offset++;

    bool s = false;
    bool closed = false;
    int remaining_size;
    unsigned char* ujson = reinterpret_cast<unsigned char*>(json); // won't have to cast all the time
    while ((remaining_size = max_size - offset -1) > 0) {
        json[offset] = random_generator.next_char();

        // Closing quote. The string is closing by itself.
        if (json[offset] == '"') {
            offset++;
            closed = true;
            break;
        }

        // trying to insert escaped stuff
        if (json[offset] == '\\') {
            const int min_escaped_size = 2;
            if (remaining_size < min_escaped_size) {
                continue;
            }
            const char escaped_char[] = "\"\\bfnrtu";
            const int nb_escaped_char = 8;
            int position = random_generator.next_ranged_int(0, nb_escaped_char-1);
            json[offset+1] = escaped_char[position];

            if (json[offset+1] == 'u') {
                int size = insert_escaped_codepoint(&json[offset+min_escaped_size], remaining_size-2, random_generator);
                if (size == 0) {
                    continue;
                }
                offset += size + min_escaped_size;
            }
            else {
                offset += min_escaped_size;
            }
            continue;
            
        }

        // one byte character
        if (0x20 <= ujson[offset] && ujson[offset] <= 0x7f) {
            offset++;
            continue;
        }

        // two bytes character
        if (0xc2 <= ujson[offset] && ujson[offset] <= 0xdf) {
            const int char_size = 2;
            if (remaining_size < char_size) {
                continue;
            }
            json[offset+1] = random_generator.next_ranged_int(0x80, 0xbf);
            offset += char_size;
            continue;
        }

        // three bytes character
        if (0xec <= ujson[offset] && ujson[offset] <= 0xef) {
            const int char_size = 3;
            if (remaining_size < char_size) {
                continue;
            }
            if (ujson[offset] == 0xed) {
                json[offset+1] = random_generator.next_ranged_int(0x80, 0x9f);
            }
            else {
                json[offset+1] = random_generator.next_ranged_int(0x80, 0xbf);
            }
            json[offset+2] = random_generator.next_ranged_int(0x80, 0xbf);

            offset += char_size;
            continue;
        }

        // four bytes character
        if (0xf0 <= ujson[offset] && ujson[offset] <= 0xf4) {
            const int char_size = 4;
            if (remaining_size < char_size) {
                continue;
            }

            if (ujson[offset] == 0xf0) {
                json[offset+1] = random_generator.next_ranged_int(0x90, 0xbf);
            }
            else if (ujson[offset] == 0xf4) {
                json[offset+1] = random_generator.next_ranged_int(0x80, 0x8f);
            }
            else { // 0xf1 >= ujson[offset] <= 0xf3
                json[offset+1] = random_generator.next_ranged_int(0x80, 0xbf);
            }

            json[offset+2] = random_generator.next_ranged_int(0x80, 0xbf);
            json[offset+3] = random_generator.next_ranged_int(0x80, 0xbf);
            
            offset += char_size;
            continue;
        }
    }

    // If the string has not randomly close by itself, we close it.
    if (!closed) {
        json[offset] = '"';
        offset++;
    }

    size = offset;

    return offset;
}

int RandomJson::insert_array_entry(char* json, std::stack<char>& closing_stack, std::stack<bool>& use_comma, int max_size, RandomEngine& random_generator)
{
    int comma_length = use_comma.top() ? 1: 0;
    int size = 0;
    if (max_size < comma_length) {
        return size;
    }

    int offset = insert_whitespace(json, max_size, random_generator);

    if (use_comma.top()) {
        json[offset] = ',';
        offset++;
        offset += insert_whitespace(&json[offset], max_size-offset, random_generator);
    }
    
    int value_size = insert_value(&json[offset], closing_stack, use_comma, max_size-offset, random_generator);

    // We make sure a value has been successfully written
    // Otherwise, we make sure to overwrite the invalid comma we might have inserted
    if (value_size == 0) {
        return size;
    }

    size = value_size + offset;
    return size;
}

int RandomJson::insert_object_entry(char* json, std::stack<char>& closing_stack, std::stack<bool>& use_comma, int max_size, RandomEngine& random_generator)
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
    // Inserting whitespace before key or comma
    int offset = insert_whitespace(json, max_size - min_size, random_generator);

    // Inserting comma before key if necessary
    if (use_comma.top()) {
        json[offset] = ',';
        offset++;
        min_size -= comma_length;
        // Inserting whitespace after comma
        offset += insert_whitespace(&json[offset], max_size - offset - min_size, random_generator);
    }
    // Inserting key
    min_size -= min_key_size;
    int key_size = insert_string(&json[offset], max_size - offset - min_size, random_generator);
    offset += key_size;
    // Inserting space after key and before colon
    offset += insert_whitespace(&json[offset], max_size - offset - min_size, random_generator);
    // Inserting colon
    json[offset] = ':';
    min_size -= colon_size;
    offset++;
    // Inserting whitespace after colon and before value
    offset += insert_whitespace(&json[offset], max_size - offset - min_size, random_generator);
    // Inserting value
    int value_size = insert_value(&json[offset], closing_stack, use_comma, max_size - offset, random_generator);

    // Quick fix if we failed to write a value
    // This should not happen, but it actually happens frequently at the end of documents.
    if (value_size == 0) {
        return size;
    }

    size = offset + value_size;
    return size;
}

int RandomJson::insert_value(char* json, std::stack<char>& closing_stack, std::stack<bool>& use_comma, int max_size, RandomEngine& random_generator)
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
        size = insert_string(json, max_size, random_generator);
        if (size != 0) {
            use_comma.top() = true;
        }
    break;
    case 2:
        size = insert_number(json, max_size, random_generator);
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

    if (closing_stack.size() >= settings.max_depth-2) {
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

int RandomJson::insert_whitespace(char* json, int max_size, RandomEngine& random_generator)
{
    const int min_size = 0;
    int size = 0;
    if (max_size < min_size) {
        return size;
    }

    max_size = std::min(max_size, settings.max_whitespace_size);
    size = random_generator.next_ranged_int(min_size, max_size);
    insert_givensized_whitespace(json, size, random_generator);

    return size;
}

void RandomJson::insert_givensized_whitespace(char* json, int size, RandomEngine& random_generator)
{
    const char whitespaces[] {0x09, 0x0A, 0x0D, 0x20};

    for (int i = 0; i < size; i++) {
        json[i] = whitespaces[random_generator.next_ranged_int(0, 3)];
    }
}

void RandomJson::generate() {
    json = new char[settings.size];
    int offset = 0;
    if (settings.bom) {
        offset = insert_BOM(json);
    }
    generate_json(&json[offset], settings.size-offset, generation_random);
    settings.filepath = "";
}

void RandomJson::load_file(const std::string& filepath) {
    settings.filepath = filepath;
    std::ifstream file (filepath, std::ios::in | std::ios::binary | std::ios::ate);
    settings.size = file.tellg();
    json = new char[settings.size];
    file.seekg(0, std::ios::beg);
    file.read(json, settings.size);
    file.close();
}

void RandomJson::generate_json(char* json, int size, RandomEngine& random_generator)
{
    int offset = 0;
    std::stack<char> closing_stack; // Used to keep track of the structure we're in
    std::stack<bool> use_comma; // Used to keep track if a comma is necessary or not

    offset += insert_whitespace(&json[offset], size, random_generator);
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
            insert_givensized_whitespace(&json[offset], size-offset, random_generator);
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
            offset += insert_array_entry(&json[offset], closing_stack, use_comma, space_left, random_generator);
            break;
        case '}':
            offset += insert_object_entry(&json[offset], closing_stack, use_comma, space_left, random_generator);
            break;
        }
    }
}

void RandomJson::mutate() {
    const int bytes_to_change = 1;

    saved_bytes.clear();
    for (int i = 0; i < bytes_to_change; i++) {
        int random_position = mutation_random.next_ranged_int(0, settings.size-1);
        char previous_value = json[random_position];
        json[random_position] = mutation_random.next_char();
        SavedByte saved_byte { random_position, previous_value};
        saved_bytes.push_back(saved_byte);
    }
    settings.number_of_mutations++;
}

void RandomJson::reverse_mutation() {
    for (SavedByte saved_byte : saved_bytes) {
        json[saved_byte.position] = saved_byte.value;
    }
    saved_bytes.clear();
}

void RandomJson::save(std::string file_name)
{
    std::fstream file(file_name, std::ios::out | std::ios::binary);
    file.write(json, sizeof(char)*settings.size);
    file.close();
}

void RandomJson::load_settings(const Settings& new_settings) {
    settings = new_settings;
    // TODO: Find an intelligent way to reallocate memory only if necessary
    delete[] json;
    generation_random.seed(settings.generation_seed);
    mutation_random.seed(settings.mutation_seed);
    if (settings.filepath != "") {
        load_file(settings.filepath);
    }
    else {
        generate();
    }

    for (int i = 0; i < settings.number_of_mutations; i++) {
        mutate();
    }
}

/*
** Getters
*/
const char* RandomJson::get_json()
{
    return json;
}

int RandomJson::get_size()
{
    return settings.size;
}

int RandomJson::get_number_of_mutations()
{
    return settings.number_of_mutations;
}

// The generation seed has no meaning if the json is from a file
int RandomJson::get_generation_seed()
{
    return settings.generation_seed;
}

int RandomJson::get_mutation_seed()
{
    return settings.mutation_seed;
}

bool RandomJson::is_from_file()
{
    return settings.filepath != "";
}

std::string RandomJson::get_filepath()
{
    return settings.filepath;
}

}

#endif