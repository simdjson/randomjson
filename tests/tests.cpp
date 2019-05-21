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
    const int res = json_parse(json,size, pj);
    assert(res != 0);
}

int main(int argc, char** argv) {
    const int default_size = 1000;
    std::cout << "seed: " << randomjson::get_seed() << std::endl;
    int size = default_size;
    if (argc > 1) {
        size = std::stoi(argv[1]);
    }

    char* json = (char*) malloc(size);
    randomjson::generate_json(json, size);
    test_parse_simdjson(json, size);
    free(json);

    return 0;
}
