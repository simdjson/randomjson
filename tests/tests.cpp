#include <iostream>
#include <cassert>

#include "randomjson.h"
//#include "simdjson.h"
//#include "simdjson.cpp"
#include "simdutf8check.h"

void test_utf8(char* json, int size) {
    assert(validate_utf8_fast(json, size));
}

void test_parse_simdjson(std::string json_filename, int size) {  
    //padded_string p = get_corpus("test.json"); 
    //padded_string p = get_corpus("canada.json");
    /*ParsedJson pj = build_parsed_json(p); // do the parsing
    if( ! pj.isValid() ) {
        // something went wrong
    }*/
    /*ParsedJson pj;
    pj.allocateCapacity(p.size());
    const int res = json_parse(p, pj);
    //assert(res != 0);
    if (res != 0) {
        std::cout << "test";
    }*/
}

int main(int argc, char** argv) {
    const int default_size = 1000;
    std::cout << "seed: " << randomjson::get_seed() << std::endl;
    int size = default_size;
    if (argc > 1) {
        size = std::stoi(argv[1]);
    }
    //randomjson::generate_json_file("test.json", size);

    char* json = (char*) malloc(size);
    randomjson::generate_json(json, size);
    test_utf8(json, size);
    char json[] = {};
    test_parse_simdjson({}, size);
    free(json);

    return 0;
}
