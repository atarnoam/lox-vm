#include "chunk.h"

int Chunk::add_constant(Value value) {
    constants.push_back(value);
    return constants.size() - 1;
}
