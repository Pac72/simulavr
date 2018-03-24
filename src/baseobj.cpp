#include "baseobj.h"

#include <sstream>
#include <typeinfo>
#include <string>

std::string BaseObj::Type() {
    return typeid(*this).name();
}

std::string BaseObj::Id() {
    std::stringstream ss;
    ss << (void *)this;
    return ss.str();
}

std::string BaseObj::FullId() {
    std::stringstream ss;
    ss << Type() << '#' << Id();
    return ss.str();
}

// EOF
