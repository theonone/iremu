#include "cfgProv.hpp"

VMConfig ConfigProvider::_conf = {};

void ConfigProvider::set(const VMConfig& conf) { _conf = conf; }

const VMConfig& ConfigProvider::get() { return _conf; }
