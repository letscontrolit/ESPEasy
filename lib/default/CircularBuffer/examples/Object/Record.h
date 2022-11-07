#include <Print.h>

#pragma once
class Record {
public:
	Record(unsigned int value, unsigned long time) {
		_value = value;
		_time = time;
	}
	~Record() {};

	void print(Print* out) {
		out->print(_time);
		out->print("  ");
		out->print(_value);
	}
	
	unsigned int value() {
		return _value;
	}
private:
	unsigned int _value;
	unsigned long _time;
};
