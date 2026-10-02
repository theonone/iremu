#include "errors.hpp"

InputError::InputError(const std::string& msg) : message(msg) {}

const char* InputError::what() const noexcept { return message.c_str(); }
