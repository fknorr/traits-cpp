#include "traits.h"


class Shape
{
    public:
        virtual void Draw(traits::ConstSelfPtr self) const = 0;
};


template<typename T>
class traits::Impl<Shape, T>
    : public ImplBase<Shape, T>
{
    public:
        virtual void Draw(ConstSelfPtr self) const override {
            std::cout << this->Instance(self) << "\n";
        }
};



void foo(traits::ConstRef<Shape> shape) {
    shape.Call(&Shape::Draw);
}

int main() {
    Call(&Shape::Draw, 42);
    foo(42);
}

