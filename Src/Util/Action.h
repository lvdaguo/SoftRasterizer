#pragma once

#include "pch.h"

// 事件回调函数的模板
// 支持最多4个输入参数的函数类型（事件的广播不需要返回值）
// 内部存放一个function和一个id
// 全局唯一的id被事件用于定位回调函数

class Action
{
public:
	typedef std::function<void()> func_t;

	Action() : m_func(), m_id(0) { }
	Action(func_t func) : m_func(func)
	{
		m_id = GenerateID();
	}
	Action(const Action& other) : m_func(other.m_func), m_id(other.m_id) { }
	~Action() { }

	Action& operator = (const Action& other)
	{
		m_id = other.m_id;
		m_func = other.m_func;
		return *this;
	}

	void Invoke() { m_func(); }
	void Assign(func_t func) { m_func = func; }

private:
	unsigned int m_id;
	func_t m_func;

public:
	unsigned int GetID() const { return m_id; }
	func_t GetFunction() const { return m_func; }
};

template<typename T>
class Action1
{
public:
	typedef std::function<void(T)> func_t;

	Action1() : m_func(), m_id(0) { }
	Action1(func_t func) : m_func(func), m_id(GenerateID()) { }
	Action1(const Action1<T>& other) : m_func(other.m_func), m_id(other.m_id) { }
	~Action1() { }

	Action1<T>& operator = (const Action1<T>& other)
	{
		m_id = other.m_id;
		m_func = other.m_func;
		return *this;
	}

	void Invoke(T t) { m_func(t); }
	void Assign(func_t func) { m_func = func; }

private:
	unsigned int m_id;
	func_t m_func;

public:
	unsigned int GetID() const { return m_id; }
	func_t GetFunction() const { return m_func; }
};

template<typename T, typename U>
class Action2
{
public:
	typedef std::function<void(T, U)> func_t;

	Action2() : m_func(), m_id(0) { }
	Action2(func_t func) : m_func(func), m_id(GenerateID()) { }
	Action2(const Action2<T, U>& other) : m_func(other.m_func), m_id(other.m_id) { }
	~Action2() { }

	Action2<T, U>& operator = (const Action2<T, U>& other)
	{
		m_id = other.m_id;
		m_func = other.m_func;
		return *this;
	}

	void Invoke(T t, U u) { m_func(t, u); }
	void Assign(func_t func) { m_func = func; }

private:
	unsigned int m_id;
	func_t m_func;

public:
	unsigned int GetID() const { return m_id; }
	func_t GetFunction() const { return m_func; }
};

template<typename T, typename U, typename V>
class Action3
{
public:
	typedef std::function<void(T, U, V)> func_t;

	Action3() : m_func(), m_id(0) { }
	Action3(func_t func) : m_func(func), m_id(GenerateID()) { }
	Action3(const Action3<T, U, V>& other) : m_func(other.m_func), m_id(other.m_id) { }
	~Action3() { }

	Action3<T, U, V>& operator = (const Action3<T, U, V>& other)
	{
		m_id = other.m_id;
		m_func = other.m_func;
		return *this;
	}

	void Invoke(T t, U u, V v) { m_func(t, u, v); }
	void Assign(func_t func) { m_func = func; }

private:
	unsigned int m_id;
	func_t m_func;

public:
	unsigned int GetID() const { return m_id; }
	func_t GetFunction() const { return m_func; }
};

template<typename T, typename U, typename V, typename W>
class Action4
{
public:
	typedef std::function<void(T, U, V, W)> func_t;

	Action4() : m_func(), m_id(0) { }
	Action4(func_t func) : m_func(func), m_id(GenerateID()) { }
	Action4(const Action4<T, U, V, W>& other) : m_func(other.m_func), m_id(other.m_id) { }
	~Action4() { }

	Action4<T, U, V, W>& operator = (const Action4<T, U, V, W>& other)
	{
		m_id = other.m_id;
		m_func = other.m_func;
		return *this;
	}

	void Invoke(T t, U u, V v, W w) { m_func(t, u, v, w); }
	void Assign(func_t func) { m_func = func; }

private:
	unsigned int m_id;
	func_t m_func;

public:
	unsigned int GetID() const { return m_id; }
	func_t GetFunction() const { return m_func; }
};