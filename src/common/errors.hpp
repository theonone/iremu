#pragma once

#include <exception>
#include <string>

class InputError : public std::exception {
   private:
    std::string message;

   public:
    explicit InputError(const std::string& msg);

    const char* what() const noexcept override;
};

class RuntimeError : public std::exception {
   private:
    std::string message;

   public:
    explicit RuntimeError(const std::string& msg);

    const char* what() const noexcept override;
};

class MemoryError : public std::exception {
   private:
    std::string message;

   public:
    explicit MemoryError(const std::string& msg);

    const char* what() const noexcept override;
};

class NotImplementedError : public std::exception {
   private:
    std::string message;

   public:
    explicit NotImplementedError(const std::string& msg);

    const char* what() const noexcept override;
};