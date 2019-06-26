#include <iostream>

#include "randomjson.h"
#include "simdjson.h"
#include "simdjson.cpp"

// Namespace to prevent a conflict with simdjson.cpp because it contains simdutf8checks.
namespace simdutf8check {
#include "simdutf8check.h"
}

void test_utf8(const char* json, int size) {
    assert(simdutf8check::validate_utf8_fast(json, size));
}

void test_parse_simdjson(const char* json, int size) {
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
    int size = 100;
    if (argc > 1) {
        size = std::stoi(argv[1]);
    }

    // doing multiple tests
    for (int i = 0; i < 100; i++)
    {
        randomjson::Settings settings(size);
        randomjson::RandomJson random_json(settings);
        random_json.save("test.json");
        std::cout << "seed " << random_json.get_generation_seed() << std::endl;
        test_utf8(random_json.get_json(), random_json.get_size());
        test_parse_simdjson(random_json.get_json(), random_json.get_size());
    }
    return 0;
}
