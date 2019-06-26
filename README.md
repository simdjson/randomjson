# RandomJson
RandomJson generates valid json documents randomly. It is used with [fuzzyjson](https://github.com/ioioioio/fuzzyjson) to find errors in json parsers.

## Usage
Simply include include/randomjson.h.
```C
#include "randomjson.h"
```

## Generation
The first step is to create a randomjson::Settings object. randomjson::Settings has four overloaded constructors that have been found to be convenient.
```C
// Default constructor
// size = 0 and random seeds are randomly selected
randomjson::Settings settings_default;

// Given size
int size = 100;
randomjson::Settings settings_100bytes(100);

// Given size an generation seed
int size2 = 100;
int generation_seed = 1;
randomjson::Settings settings_100bytes_seed1(size, generation_seed);

// From a file
std::string filepath("json.json");
RandomJsonSettings settings_from_file(filepath);
```

Once the randomsjson::settings object is created, all its options can be modified.
```C
randomjson::Settings settings;
settings.size = 200;
settings.max_number_size = 5;
```

The size has to be chosen by the user. RandomJson do not want to be responsible if a too big document is generated. The json document will be exactly the size chosen by the user.

Once the settings are proprely chosen, it is time to generate the document.
```C
randomjson::RandomJson random_json(settings);
```

The document will be immediately generated. 

It is worth noting that if the document is "generated" from a file, then all the other options will be ignored.

## Mutation
Currently, RandomJson modifies one single random byte when mutation() is called. 
```C
random_json.mutate();
```

The last mutation (and only the last) can be reverted anytime
```C
reverse_mutation();
```

The mutation_seed and generation_seed are two independent options. It is believed that it gives more possibilities, but it has not been proven.
```C
settings.generation_seed = 1;
settings.mutation_seed = 2;
```

It is possible to select a number of mutations on creation
```C
settings.number_of_mutations = 10;
```
However, there is currently no way to know which mutations to skip if we do not want to generate an invalid document.

The generation functions (such insert_string() or insert_object_entry(), for example) have been made in a way it would be easy to replace a value on a given position by other random values. Commit [4f93f06](https://github.com/ioioioio/randomjson/commit/4f93f06110ebc7295d29d221a977b12672d42fdb) shows a way it could be possible (and inefficient and buggy). However, to keep track of values' position is not easy. For that reason, it does not work for now.

## Save
Once the document is created, it can be saved anytime.
```C
std::string filepath2("json2.json")
random_json.save(filepath2);
```

## Tests
```
mkdir build
cd build
cmake ..
make
./test/test
```
A hundred 100 bytes documents are generated. For each of them, the utf8 validity is tested with [fastvalidate-utf-8
](https://github.com/lemire/fastvalidate-utf-8), then [simdjson](https://github.com/lemire/simdjson) sees if it can parse it. Every used seed is printed in the console so the user have a way to recreate any error.

The user can chose a custom size for the json documents to be tested:
```
./test/test 1000000
```