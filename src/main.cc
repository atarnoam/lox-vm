#include "src/compiler/compiler.h"
#include "src/syntactics/token.h"
#include "src/vm/vm.h"

#include <fstream>
#include <iostream>
#include <memory>
#include <optional>
#include <sstream>

int interpret_result_to_exit_code(InterpretResult result) {
    switch (result) {
    case InterpretResult::COMPILE_ERROR:
        return 65;
    case InterpretResult::RUNTIME_ERROR:
        return 70;
    case InterpretResult::OK:
        return 0;
    }
    throw std::runtime_error(std::string(
        "Unknown InterpretResult " + std::to_string(static_cast<int>(result))));
}

int repl() {
    VM vm{};

    std::string s;
    std::cout << "> ";
    while (std::getline(std::cin, s)) {
        interpret(vm, s);
        std::cout << "> ";
    }
    std::cout << std::endl;
    return 0;
}

int run_file(char *filename) {
    VM vm;
    std::ifstream ifs(filename);
    std::string source(std::istreambuf_iterator<char>(ifs), {});
    InterpretResult result = interpret(vm, source);
    return interpret_result_to_exit_code(result);
}

int main(int argc, char **argv) {
    if (argc == 1) {
        return repl();
    } else if (argc == 2) {
        return run_file(argv[1]);
    } else {
        std::cout << "Usage: cpplox [script]" << std::endl;
        return 1;
    }
}