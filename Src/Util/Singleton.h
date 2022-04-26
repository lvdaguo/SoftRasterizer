#pragma once

template <typename T>
class Singleton
{
protected:
    Singleton() {}

public:
    static T& Instance()
    {
        static T instance;
        return instance;
    }
    virtual ~Singleton() {}

    Singleton(const Singleton&) = delete;
    Singleton& operator=(const Singleton&) = delete;
};