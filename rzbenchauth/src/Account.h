#ifndef ACCOUNT_H
#define ACCOUNT_H

#include <string>

class Account
{
public:
	Account(const std::string& name) : name(name) {}

	const std::string& getName() { return name; }

private:
	std::string name;
};

#endif // ACCOUNT_H
