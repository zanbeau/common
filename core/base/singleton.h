#pragma once

// 简单的 CRTP 单例基类：class Foo : public Singleton<Foo> {};
template <typename T>
class Singleton
{
public:
    static T *instance()
    {
        static T inst;
        return &inst;
    }

protected:
    Singleton() = default;
    ~Singleton() = default;
    Singleton(const Singleton &) = delete;
    Singleton &operator=(const Singleton &) = delete;
};
