#include "errors.hpp"

InputError::InputError(const std::string& msg) : message(msg) {}

const char* InputError::what() const noexcept { return message.c_str(); }

RuntimeError::RuntimeError(const std::string& msg) : message(msg) {}
const char* RuntimeError::what() const noexcept { return message.c_str(); }

MemoryError::MemoryError(const std::string& msg) : message(msg) {}
const char* MemoryError::what() const noexcept { return message.c_str(); }

NotImplementedError::NotImplementedError(const std::string& msg) : message(msg) {}
const char* NotImplementedError::what() const noexcept { return message.c_str(); }
