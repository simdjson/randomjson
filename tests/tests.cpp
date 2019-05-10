#include "randomjson.h"

#include <iostream>

int main(int argc, char** argv) {
    const int default_size = 1000;
    std::cout << "seed: " << randomjson::get_seed() << std::endl;
    std::fstream file("test.txt", std::ios::out | std::ios::binary);
    int size = default_size;
    if (argc > 1) {
        size = std::stoi(argv[1]);
    }
    randomjson::generate_json(file, size);
    file.close();
    return 0;
}