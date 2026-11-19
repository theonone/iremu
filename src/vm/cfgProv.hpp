#pragma once
#include "../common/config.hpp"

class ConfigProvider {
   private:
    static VMConfig _conf;

   public:
    static void set(const VMConfig& conf);
    static const VMConfig& get();
};