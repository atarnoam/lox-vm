#include "heap_obj.h"

HeapData::HeapData() : HeapData(nullptr) {}

HeapData::HeapData(HeapData *next) : next(next), marked(false) {}