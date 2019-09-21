#include <tuple>


namespace traits {

    class Self;

    class ConstSelf;

    template<typename Interface, typename Object>
    class ImplBase;

    template<typename Interface, typename Object>
    class Impl;

    template<typename ...Interfaces>
    class ConstRef;

    template<typename ...Interfaces>
    class Ref;

    template<typename Object, typename Interface, typename Return, typename ...Params,
        typename ...Args>
    Return call(Return (Interface::*method)(ConstSelf, Params...) const, Object &self,
            Args &&...args);

    template<typename Object, typename Interface, typename Return, typename ...Params,
        typename ...Args>
    Return call(Return (Interface::*method)(ConstSelf, Params...) const,
            const Object &self, Args &&...args);

    namespace detail {
        template<typename ...Interfaces>
        struct InterfaceTable;

        template<typename Interface, typename Object>
        const Interface &global_impl_instance();

        template<typename Object, typename ...Interfaces>
        const InterfaceTable<Interfaces...> &global_impl_table_instance();
    }
}


class traits::Self
{
    private:
        explicit Self(void *ptr)
            : _ptr(ptr)
        {
        }

        void *_ptr;

        friend class ConstSelf;

        template<typename, typename>
        friend class ImplBase;

        template<typename... Interfaces>
        friend class Ref;

        template<typename Object, typename Interface, typename Return, typename ...Params,
            typename ...Args>
        friend Return call(Return (Interface::*method)(Self, Params...) const,
                const Object &self, Args &&...args);
};


class traits::ConstSelf
{
    public:
        ConstSelf(Self self)
            : _ptr(self._ptr)
        {
        }

    private:
        explicit ConstSelf(const void *ptr)
            : _ptr(ptr)
        {
        }

        const void *_ptr;

        template<typename, typename>
        friend class ImplBase;

        template<typename...>
        friend class Ref;

        template<typename...>
        friend class ConstRef;

        template<typename Object, typename Interface, typename Return, typename ...Params,
            typename ...Args>
        friend Return call(Return (Interface::*method)(ConstSelf, Params...) const,
                const Object &self, Args &&...args);
};


template<typename Interface, typename Object>
class traits::ImplBase
    : public Interface
{
    protected:
        Object &instance(Self self) const
        {
            return *static_cast<Object*>(self._ptr);
        }

        const Object &instance(ConstSelf self) const
        {
            return *static_cast<const Object*>(self._ptr);
        }
};


template<typename ...Interfaces>
class traits::detail::InterfaceTable {
    public:
        InterfaceTable(const Interfaces *... pointers)
            : _pointers(pointers...) {
        }

        template<typename Return, typename Base, typename ...Params, typename ...Args>
        Return dispatch(Return (Base::*method)(Params...) const, Args &&...args) const {
            return (std::get<const Base *>(_pointers)->*method)(std::forward<Args>(args)...);
        }

        template<typename ...SubsetOfInterfaces>
        auto subset() {
            return InterfaceTable<SubsetOfInterfaces...>(
                    std::get<const SubsetOfInterfaces *>(_pointers)...);
        }

    private:
        std::tuple<const Interfaces *...> _pointers;
};


template<typename Interface, typename Object>
const Interface &traits::detail::global_impl_instance()
{
    static const Impl<Interface, Object> impl;
    return impl;
}


template<typename Object, typename ...Interfaces>
const traits::detail::InterfaceTable<Interfaces...> &traits::detail::global_impl_table_instance()
{
    static const InterfaceTable<Interfaces...> table(
            &global_impl_instance<Interfaces, Object>()...);
    return table;
}


template<typename ...Interfaces>
class traits::Ref {
    public:
        template<typename Object>
        Ref(Object &obj)
            : _obj(&obj)
            , _itable(&detail::global_impl_table_instance<Object, Interfaces...>())
        {
        }

        template<typename Interface, typename Return, typename ...Params, typename ...Args>
        Return call(Return (Interface::*method)(Self, Params...) const,
                Args &&...args) const
        {
            return _itable->dispatch(method, Self(_obj), std::forward<Args>(args)...);
        }

        template<typename Interface, typename Return, typename ...Params, typename ...Args>
        Return call(Return (Interface::*method)(ConstSelf, Params...) const,
                Args &&...args) const
        {
            return _itable->dispatch(method, ConstSelf(_obj),
                    std::forward<Args>(args)...);
        }

    private:
        void *_obj;
        const detail::InterfaceTable<Interfaces...> *_itable;

        friend class ConstRef<Interfaces...>;
};


template<typename ...Interfaces>
class traits::ConstRef {
    public:
        template<typename Object>
        ConstRef(const Object &obj)
            : _obj(&obj)
            , _itable(&detail::global_impl_table_instance<Object, Interfaces...>())
        {
        }

        template<typename Interface, typename Return, typename ...Params, typename ...Args>
        Return call(Return (Interface::*method)(ConstSelf, Params...) const,
                Args &&...args) const
        {
            return _itable->dispatch(method, ConstSelf(_obj),
                    std::forward<Args>(args)...);
        }

    private:
        const void *_obj;
        const detail::InterfaceTable<Interfaces...> *_itable;
};


template<typename Object, typename Interface, typename Return, typename ...Params,
    typename ...Args>
Return traits::call(Return (Interface::*method)(ConstSelf, Params...) const,
        Object &self, Args &&...args)
{
    return (Impl<Interface, Object>{}.*method)(Self(&self),
            std::forward<Args>(args)...);
}


template<typename Object, typename Interface, typename Return, typename ...Params,
    typename ...Args>
Return traits::call(Return (Interface::*method)(ConstSelf, Params...) const,
        const Object &self, Args &&...args)
{
    return (Impl<Interface, Object>{}.*method)(ConstSelf(&self),
            std::forward<Args>(args)...);
}

