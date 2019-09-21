class Reset {
    public:
        virtual ~Reset() = default;
        virtual void reset() const = 0;
};

class Repeat {
    public:
        virtual ~Repeat() = default;
        virtual void repeat(unsigned n) const = 0;
};

class Interface
    : public Reset
    , public Repeat
{
};

template<typename T>
class Implementation
    : public Interface
{
    public:
        Implementation(T *ptr)
            : _ptr(ptr)
        {
        }

        void reset() const override {
            *_ptr = T{};
        }

        void repeat(unsigned n) const override {
            for (unsigned i = 0; i < n; ++i) {
                *_ptr += *_ptr;
            }
        }

    private:
        T *_ptr;
};


void test(const Interface &ref) {
    ref.reset();
    ref.repeat(2);
}


int main() {
    int integer = 42;
    double real = 13.37;
    test(Implementation<int>(&integer));
    test(Implementation<double>(&real));
}


