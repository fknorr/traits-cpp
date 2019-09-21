#include "traits.h"


class Reset {
    public:
        virtual void reset(traits::Self self) const = 0;
};

template<typename T>
class traits::Impl<Reset, T>
    : public ImplBase<Reset, T>
{
    public:
        virtual void reset(traits::Self self) const final {
            this->instance(self) = T{};
        }
};

class Repeat {
    public:
        virtual void repeat(traits::Self self, unsigned n) const = 0;
};

template<typename T>
class traits::Impl<Repeat, T>
    : public ImplBase<Repeat, T>
{
    public:
        virtual void repeat(traits::Self self, unsigned n) const final {
            auto &inst = this->instance(self);
            for (unsigned i = 0; i < n; ++i) {
                inst += inst;
            }
        }
};


void test(traits::Ref<Reset, Repeat> ref) {
    ref.call(&Reset::reset);
    ref.call(&Repeat::repeat, 2);
}


int main() {
    int integer = 42;
    double real = 13.37;
    test(integer);
    test(real);
}

