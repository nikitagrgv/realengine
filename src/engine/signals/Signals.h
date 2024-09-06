#pragma once

#include <algorithm>
#include <cassert>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>

////////////////////////////////////////////////////////////////////////////////////////////////////

template<class... Args>
class Callback
{
public:
    Callback() = default;

    // Functions
    Callback(void (*func)(Args...))
        : f_{std::make_unique<FunctionBased>(func)}
    {}

    // Member functions
    template<class Obj>
    Callback(Obj *obj, void (Obj::*func)(Args...))
        : f_{std::make_unique<MemberFunctionBased<Obj>>(obj, func)}
    {}

    // Const member functions
    template<class Obj>
    Callback(const Obj *obj, void (Obj::*func)(Args...) const)
        : f_{std::make_unique<ConstMemberFunctionBased<Obj>>(obj, func)}
    {}

    // Functors
    template<class F>
    Callback(F functor)
        : f_{std::make_unique<FunctorBased<F>>(std::move(functor))}
    {}

    Callback(const Callback &other)
        : f_(other.f_->clone())
    {}

    Callback(Callback &&other) noexcept
        : f_(std::move(other.f_))
    {}

    Callback &operator=(const Callback &other)
    {
        if (&other != this)
        {
            f_ = other.f_->clone();
        }

        return *this;
    }

    Callback &operator=(Callback &&other) noexcept
    {
        if (&other != this)
        {
            f_ = std::move(other.f_);
        }

        return *this;
    }

    void operator()(Args... args) { call(args...); }

    void call(Args... args)
    {
        assert(isValid());
        f_->operator()(args...);
    }

    [[nodiscard]] bool isValid() const { return f_ != nullptr; }
    [[nodiscard]] bool isEmpty() const { return f_ == nullptr; }

    void clear() { return f_ = nullptr; }

protected:
    class Base
    {
    public:
        virtual ~Base() = default;
        virtual void operator()(Args... args) = 0;

        virtual std::unique_ptr<Base> clone() = 0;
    };

    class FunctionBased : public Base
    {
    public:
        FunctionBased(void (*func)(Args...))
            : func_(func)
        {
            assert(func);
        }

        void operator()(Args... args) override { (*func_)(args...); }

        std::unique_ptr<Base> clone() override { return std::make_unique<FunctionBased>(func_); }

    private:
        void (*func_)(Args...) = nullptr;
    };

    template<class Obj>
    class MemberFunctionBased : public Base
    {
    public:
        MemberFunctionBased(Obj *obj, void (Obj::*func)(Args...))
            : obj_(obj)
            , func_(func)
        {
            assert(obj);
            assert(func);
        }

        void operator()(Args... args) override { (obj_->*func_)(args...); }

        std::unique_ptr<Base> clone() override
        {
            return std::make_unique<MemberFunctionBased>(obj_, func_);
        }

    private:
        Obj *obj_ = nullptr;
        void (Obj::*func_)(Args...) = nullptr;
    };

    template<class Obj>
    class ConstMemberFunctionBased : public Base
    {
    public:
        ConstMemberFunctionBased(const Obj *obj, void (Obj::*func)(Args...) const)
            : obj_(obj)
            , func_(func)
        {
            assert(obj);
            assert(func);
        }

        void operator()(Args... args) override { (obj_->*func_)(args...); }

        std::unique_ptr<Base> clone() override
        {
            return std::make_unique<ConstMemberFunctionBased>(obj_, func_);
        }

    private:
        const Obj *obj_ = nullptr;
        void (Obj::*func_)(Args...) const = nullptr;
    };

    template<class F>
    class FunctorBased : public Base
    {
    public:
        explicit FunctorBased(F functor)
            : functor_(std::move(functor))
        {}

        void operator()(Args... args) override { functor_(args...); }

        std::unique_ptr<Base> clone() override { return std::make_unique<FunctorBased>(functor_); }

    private:
        F functor_;
    };

private:
    std::unique_ptr<Base> f_;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

class Context;

class ISignal
{
public:
    virtual ~ISignal() = default;

protected:
    friend class Context;
    virtual void on_context_destroyed(Context *ctx) = 0;
    virtual void on_context_moved(Context *old_ctx, Context *new_ctx) = 0;
};


template<class... Args>
class Signal : public ISignal
{
public:
    ~Signal() override;

    void operator()(Args... args) { call(args...); }

    void call(Args... args)
    {
        for (auto &it : context_to_callbacks_)
        {
            for (auto &callback : it.second)
            {
                callback(args...);
            }
        }
    }

    void connect(Context &context, Callback<Args...> callback);

private:
    void on_context_destroyed(Context *ctx) override
    {
        assert(ctx);
        context_to_callbacks_.erase(ctx);
    }

    void on_context_moved(Context *old_ctx, Context *new_ctx) override
    {
        assert(old_ctx);
        assert(new_ctx);
        std::vector<Callback<Args...>> callbacks = std::move(context_to_callbacks_[old_ctx]);
        context_to_callbacks_[new_ctx] = std::move(callbacks);
        context_to_callbacks_.erase(old_ctx);
    }

private:
    std::unordered_map<Context *, std::vector<Callback<Args...>>> context_to_callbacks_;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

class Context
{
public:
    Context() = default;

    Context(const Context &) = delete;
    Context &operator=(const Context &) = delete;

    Context(Context &&other)
    {
        if (&other == this)
        {
            return;
        }

        for (auto &s : other.signals_)
        {
            s->on_context_moved(&other, this);
        }
        signals_ = std::move(other.signals_);
    }

    Context &operator=(Context &&other)
    {
        if (&other == this)
        {
            return *this;
        }

        clear();

        for (auto &s : other.signals_)
        {
            s->on_context_moved(&other, this);
        }
        signals_ = std::move(other.signals_);
        return *this;
    }

    void clear()
    {
        for (auto &s : signals_)
        {
            s->on_context_destroyed(this);
        }
    }

    virtual ~Context() { clear(); }

private:
    template<class... Args>
    friend class Signal;

private:
    mutable std::unordered_set<ISignal *> signals_;
};

template<class... Args>
Signal<Args...>::~Signal()
{
    for (auto &it : context_to_callbacks_)
    {
        auto &sigs = it.first->signals_;
        sigs.erase(this);
    }
}

template<class... Args>
void Signal<Args...>::connect(Context &context, Callback<Args...> callback)
{
    context_to_callbacks_[&context].push_back(std::move(callback));
    context.signals_.insert(this);
}
