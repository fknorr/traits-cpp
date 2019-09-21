#include <tuple>


namespace traits {

    template<typename Interface>
    class Self;

    template<typename Interface>
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
    Return call(Return (Interface::*method)(ConstSelf<Interface>, Params...) const, Object &self,
            Args &&...args);

    template<typename Object, typename Interface, typename Return, typename ...Params,
        typename ...Args>
    Return call(Return (Interface::*method)(ConstSelf<Interface>, Params...) const,
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


template<typename Interface>
class traits::Self
{
    private:
        Self(void *ptr)
            : _ptr(ptr)
        {
        }

        void *_ptr;

        template<typename Object>
        friend class ConstSelf;

        template<typename, typename>
        friend class ImplBase;

        template<typename... Interfaces>
        friend class Ref;

        template<typename Object, typename Return, typename ...Params,
            typename ...Args>
        friend Return call(Return (Object::*)(Self<Interface>, Params...) const, Object &self,
                Args &&...args);
};


template<typename Interface>
class traits::ConstSelf
{
    public:
        ConstSelf(Self<Interface> self)
            : _ptr(self._ptr)
        {
        }

    private:
        ConstSelf(const void *ptr)
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

        template<typename Object, typename Interf, typename Return, typename ...Params,
            typename ...Args>
        friend Return call(Return (Interf::*method)(ConstSelf<Interf>, Params...) const,
                const Object &self, Args &&...args);
};


template<typename Interface, typename Object>
class traits::ImplBase
    : public Interface
{
    protected:
        Object &instance(Self<Interface> self) const
        {
            return *static_cast<Object*>(self._ptr);
        }

        const Object &instance(ConstSelf<Interface> self) const
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
        Return call(Return (Interface::*method)(Self<Interface>, Params...) const,
                Args &&...args) const
        {
            return _itable->dispatch(method, Self<Interface>(_obj), std::forward<Args>(args)...);
        }

        template<typename Interface, typename Return, typename ...Params, typename ...Args>
        Return call(Return (Interface::*method)(ConstSelf<Interface>, Params...) const,
                Args &&...args) const
        {
            return _itable->dispatch(method, ConstSelf<Interface>(_obj),
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
        Return call(Return (Interface::*method)(ConstSelf<Interface>, Params...) const,
                Args &&...args) const
        {
            return _itable->dispatch(method, ConstSelf<Interface>(_obj),
                    std::forward<Args>(args)...);
        }

    private:
        const void *_obj;
        const detail::InterfaceTable<Interfaces...> *_itable;
};


template<typename Object, typename Interface, typename Return, typename ...Params,
    typename ...Args>
Return traits::call(Return (Interface::*method)(ConstSelf<Interface>, Params...) const,
        Object &self, Args &&...args)
{
    return (Impl<Interface, Object>{}.*method)(Self<Interface>(&self),
            std::forward<Args>(args)...);
}


template<typename Object, typename Interface, typename Return, typename ...Params,
    typename ...Args>
Return traits::call(Return (Interface::*method)(ConstSelf<Interface>, Params...) const,
        const Object &self, Args &&...args)
{
    return (Impl<Interface, Object>{}.*method)(ConstSelf<Interface>(&self),
            std::forward<Args>(args)...);
}

