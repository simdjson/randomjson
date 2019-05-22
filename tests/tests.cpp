#include <iostream>

#include "randomjson.h"
#include "simdjson.h"
#include "simdjson.cpp"

// There is a conflict with simdjson.cpp because it contains simdutf8checks.
namespace simdutf8check {
#include "simdutf8check.h"
}

void test_utf8(char* json, int size) {
    assert(simdutf8check::validate_utf8_fast(json, size));
}

void test_parse_simdjson(char* json, int size) {
    ParsedJson pj;
    bool allocation_is_successful = pj.allocateCapacity(size);
    assert(allocation_is_successful);
    const int res = json_parse(json, size, pj);
    assert(res != 0);
}

int main(int argc, char** argv) {
    const int default_size = 1000;
    int size = default_size;
    if (argc > 1) {
        size = std::stoi(argv[1]);
    }

    // Failing seeds:
    // -1042029749
    // -581267614
    // -149936094
    // 113722174
    // 113722175
    // 511262440
    randomjson::RandomJson random_json(size);
    random_json.generate();
    std::cout << "seed: " << random_json.get_seed() << std::endl;
    test_utf8(random_json.get_json(), random_json.get_size());
    test_parse_simdjson(random_json.get_json(), random_json.get_size());
    return 0;
}
