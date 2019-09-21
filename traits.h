#include <iostream>
#include <memory>


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
    Return Call(Return (Interface::*method)(ConstSelf<Interface>, Params...) const, Object &self,
            Args &&...args);

    template<typename Object, typename Interface, typename Return, typename ...Params,
        typename ...Args>
    Return Call(Return (Interface::*method)(ConstSelf<Interface>, Params...) const,
            const Object &self, Args &&...args);

    namespace detail {
        template<typename ...Interfaces>
        struct Combined;
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
        friend Return Call(Return (Object::*)(Self<Interface>, Params...) const, Object &self,
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
        friend class ConstRef;

        template<typename Object, typename Interf, typename Return, typename ...Params,
            typename ...Args>
        friend Return Call(Return (Interf::*method)(ConstSelf<Interf>, Params...) const,
                const Object &self, Args &&...args);
};


template<typename Interface, typename Object>
class traits::ImplBase
    : public Interface
{
    protected:
        Object &Instance(Self<Interface> self) const
        {
            return *static_cast<Object*>(self._ptr);
        }

        const Object &Instance(ConstSelf<Interface> self) const
        {
            return *static_cast<const Object*>(self._ptr);
        }
};


template<typename ...Interfaces>
struct traits::detail::Combined {
    class Interface: public virtual Interfaces... {};

    template<typename Object>
    class Implementation final
        : public Interface
        , public Impl<Interfaces, Object>... {
    };

    template<typename Object>
    static Implementation<Object> &Instance()
    {
        static Implementation<Object> inst;
        return inst;
    }
};


template<typename Only>
struct traits::detail::Combined<Only> {
    using Interface = Only;

    template<typename Object>
    using Implementation = Impl<Only, Object>;

    template<typename Object>
    static Implementation<Object> &Instance()
    {
        static Implementation<Object> inst;
        return inst;
    }
};


template<typename ...Interfaces>
class traits::Ref {
    public:
        template<typename Object>
        Ref(Object &obj)
            : _obj(&obj)
            , _impl(&detail::Combined<Interfaces...>::template Instance<Object>())
        {
        }

        template<typename Interface, typename Return, typename ...Params, typename ...Args>
        Return Call(Return (Interface::*method)(Self<Interface>, Params...) const, Args &&...args)
        {
            return (_impl->*method)(Self<Interface>(_obj), std::forward<Args>(args)...);
        }

    private:
        void *_obj;
        const typename detail::Combined<Interfaces...>::Interface *_impl;

        friend class ConstRef<Interfaces...>;
};


template<typename ...Interfaces>
class traits::ConstRef {
    public:
        template<typename Object>
        ConstRef(const Object &obj)
            : _obj(&obj)
            , _impl(&detail::Combined<Interfaces...>::template Instance<Object>())
        {
        }

        ConstRef(Ref<Interfaces...> ref)
            : _obj(ref._obj)
            , _impl(ref._impl)
        {
        }

        ConstRef(const ConstRef &) = default;

        template<typename Interface, typename Return, typename ...Params, typename ...Args>
        Return Call(Return (Interface::*method)(ConstSelf<Interface>, Params...) const,
                Args &&...args)
        {
            return (_impl->*method)(ConstSelf<Interface>(_obj), std::forward<Args>(args)...);
        }

    private:
        const void *_obj;
        const typename detail::Combined<Interfaces...>::Interface *_impl;
};


template<typename Object, typename Interface, typename Return, typename ...Params,
    typename ...Args>
Return traits::Call(Return (Interface::*method)(ConstSelf<Interface>, Params...) const,
        Object &self, Args &&...args)
{
    return (Impl<Interface, Object>{}.*method)(Self<Interface>(&self),
            std::forward<Args>(args)...);
}

template<typename Object, typename Interface, typename Return, typename ...Params,
    typename ...Args>
Return traits::Call(Return (Interface::*method)(ConstSelf<Interface>, Params...) const,
        const Object &self, Args &&...args)
{
    return (Impl<Interface, Object>{}.*method)(ConstSelf<Interface>(&self),
            std::forward<Args>(args)...);
}

