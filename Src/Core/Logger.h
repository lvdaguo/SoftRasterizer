#pragma once

class Logger : public Singleton<Logger>
{
public:
	Logger() { TIPS(L"Logger Con"); }
	void Init();
	~Logger();
};
