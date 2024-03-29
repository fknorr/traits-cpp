#include "traits.h"

#include <typeinfo>
#include <string>
#include <iostream>
#include <sstream>


#define STRINGIFY2(x) #x
#define STRINGIFY(x) STRINGIFY2(x)
#define TEST_EQUAL(a, b) do { if (!((a) == (b))) { \
    std::cerr << "Test failed: " #a " != " #b "\n"; \
    std::exit(1); \
}} while(0)


class Print
{
    public:
        virtual std::string print(traits::ConstSelf self) const = 0;
};

class GetTypeInfo
{
    public:
        virtual const std::type_info &get_type_info(traits::ConstSelf self) const = 0;
};

class Reset
{
    public:
        virtual void reset(traits::Self self) const = 0;
};


template<typename T>
class traits::Impl<Print, T>
    : public ImplBase<Print, T>
{
    public:
        virtual std::string print(traits::ConstSelf self) const override {
            std::ostringstream oss;
            oss << this->instance(self);
            return oss.str();
        }
};

template<typename T>
class traits::Impl<GetTypeInfo, T>
    : public ImplBase<GetTypeInfo, T>
{
    public:
        virtual const std::type_info &get_type_info(traits::ConstSelf self) const override {
            return typeid(this->instance(self));
        }
};

template<typename T>
class traits::Impl<Reset, T>
    : public ImplBase<Reset, T>
{
    public:
        virtual void reset(traits::Self self) const override {
            this->instance(self) = T{};
        }
};


void test_print() {
    int integer = 1;
    traits::Ref<Print> int_ref(integer);
    traits::ConstRef<Print> const_int_ref(2);

    TEST_EQUAL(int_ref.call(&Print::print), "1");
    TEST_EQUAL(const_int_ref.call(&Print::print), "2");

    std::string string = "mut";
    traits::Ref<Print> string_ref(string);
    traits::ConstRef<Print> const_cstring_ref("const c");

    TEST_EQUAL(string_ref.call(&Print::print), "mut");
    TEST_EQUAL(const_cstring_ref.call(&Print::print), "const c");
}


void test_get_type_info() {
    std::string string = "mut";
    traits::Ref<GetTypeInfo> string_ref(string);

    TEST_EQUAL(string_ref.call(&GetTypeInfo::get_type_info), typeid(std::string));
}


void test_reset() {
    int integer = 1;
    traits::Ref<Print, Reset> int_ref(integer);

    TEST_EQUAL(int_ref.call(&Print::print), "1");
    int_ref.call(&Reset::reset);
    TEST_EQUAL(int_ref.call(&Print::print), "0");

    std::string string = "mut";
    traits::Ref<Print, Reset> string_ref(string);

    TEST_EQUAL(string_ref.call(&Print::print), "mut");
    string_ref.call(&Reset::reset);
    TEST_EQUAL(string_ref.call(&Print::print), "");
}


void test_box() {
    traits::Box<Print> box(std::make_unique<std::string>("Hello World!"));
    TEST_EQUAL(box.call(&Print::print), "Hello World!");
    auto move = std::move(box);
    TEST_EQUAL(move.call(&Print::print), "Hello World!");

    struct DeleteTracer {
        DeleteTracer(bool &b): deleted(&b) {}
        ~DeleteTracer() { *deleted = true; }
        bool *deleted;
    };

    bool deleted = false;
    {
        traits::Box<> trace = std::make_unique<DeleteTracer>(deleted);
        TEST_EQUAL(deleted, false);
    }
    TEST_EQUAL(deleted, true);
}


int main() {
    test_print();
    test_get_type_info();
    test_reset();
    test_box();
}
