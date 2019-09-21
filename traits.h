#include <iostream>
#include <memory>


namespace traits {

    class SelfPtr;
    class ConstSelfPtr;

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
    Return Call(Return (Interface::*method)(ConstSelfPtr, Params...) const, Object &self,
            Args &&...args);

    template<typename Object, typename Interface, typename Return, typename ...Params,
        typename ...Args>
    Return Call(Return (Interface::*method)(ConstSelfPtr, Params...) const, const Object &self,
            Args &&...args);

    namespace detail {
        template<typename ...Interfaces>
        struct Combined;
    }
}


class traits::SelfPtr
{
    private:
        SelfPtr(void *ptr)
            : _ptr(ptr)
        {
        }

        void *_ptr;

        friend class ConstSelfPtr;

        template<typename Interface, typename Object>
        friend class ImplBase;

        template<typename... Interfaces>
        friend class Ref;

        template<typename Object, typename Return, typename ...Params,
            typename ...Args>
        friend Return Call(Return (Object::*)(SelfPtr, Params...) const, Object &self,
                Args &&...args);
};


class traits::ConstSelfPtr
{
    public:
        ConstSelfPtr(SelfPtr self)
            : _ptr(self._ptr)
        {
        }

    private:
        ConstSelfPtr(const void *ptr)
            : _ptr(ptr)
        {
        }

        const void *_ptr;

        template<typename Interface, typename Object>
        friend class ImplBase;

        template<typename... Interfaces>
        friend class ConstRef;

        template<typename Object, typename Interface, typename Return, typename ...Params,
            typename ...Args>
        friend Return Call(Return (Interface::*method)(ConstSelfPtr, Params...) const,
                const Object &self, Args &&...args);
};


template<typename Interface, typename Object>
class traits::ImplBase
    : public Interface
{
    protected:
        Object &Instance(SelfPtr self) const
        {
            return *static_cast<Object*>(self._ptr);
        }

        const Object &Instance(ConstSelfPtr self) const
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
        Return Call(Return (Interface::*method)(SelfPtr, Params...) const, Args &&...args)
        {
            return (_impl->*method)(_obj, std::forward<Args>(args)...);
        }

    private:
        SelfPtr _obj;
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
        Return Call(Return (Interface::*method)(SelfPtr, Params...) const, Args &&...args)
        {
            return (_impl->*method)(_obj, std::forward<Args>(args)...);
        }

        template<typename Interface, typename Return, typename ...Params, typename ...Args>
        Return Call(Return (Interface::*method)(ConstSelfPtr, Params...) const, Args &&...args)
        {
            return (_impl->*method)(_obj, std::forward<Args>(args)...);
        }

    private:
        ConstSelfPtr _obj;
        const typename detail::Combined<Interfaces...>::Interface *_impl;
};


template<typename Object, typename Interface, typename Return, typename ...Params,
    typename ...Args>
Return traits::Call(Return (Interface::*method)(ConstSelfPtr, Params...) const, Object &self,
        Args &&...args)
{
    return (Impl<Interface, Object>{}.*method)(SelfPtr(&self), std::forward<Args>(args)...);
}

template<typename Object, typename Interface, typename Return, typename ...Params,
    typename ...Args>
Return traits::Call(Return (Interface::*method)(ConstSelfPtr, Params...) const, const Object &self,
        Args &&...args)
{
    return (Impl<Interface, Object>{}.*method)(ConstSelfPtr(&self), std::forward<Args>(args)...);
}

