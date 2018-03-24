#ifndef BASEOBJ_H_
#define BASEOBJ_H_

#include <string>

class BaseObj {
    public:
        virtual std::string Type();
        virtual std::string Id();
        virtual std::string FullId();
};

#endif /* BASEOBJ_H_ */
