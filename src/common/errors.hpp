#pragma once

#include <exception>
#include <string>

class InputError : public std::exception {
   private:
    std::string message;

   public:
    explicit InputError(const std::string& msg);

    // Override what() to return the error message
    const char* what() const noexcept override;
};
