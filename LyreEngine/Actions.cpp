#include "stdafx.h"

#include "Actions.h"

std::istream & operator>>(std::istream & is, Action & action) {
	int val;
	is >> val;
	action = static_cast<Action>(val);
	return is;
}
