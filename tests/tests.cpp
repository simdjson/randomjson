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
    if (res != simdjson::SUCCESS) {
        std::cout << "simdjson error: " << res << std::endl;
    }
    assert(res == simdjson::SUCCESS);
}

int main(int argc, char** argv) {
    const int default_size = 1000;
    int size = default_size;
    if (argc > 1) {
        size = std::stoi(argv[1]);
    }

    randomjson::RandomJson random_json(64);
    std::cout << "seed: " << random_json.get_seed() << std::endl;
    random_json.save("test.json");
    test_utf8(random_json.get_json(), random_json.get_size());
    test_parse_simdjson(random_json.get_json(), random_json.get_size());
    /*random_json.mutate();
    test_utf8(random_json.get_json(), random_json.get_size());
    test_parse_simdjson(random_json.get_json(), random_json.get_size());
    random_json.save("mutate.json");*/
    return 0;
}
