#pragma once


class Quantize {
private:
	double* q_space;
	int q_space_len;
public:
	Quantize(double* q_space, int len);
	~Quantize();
	// stochastically quantize an float array into byte buffer.
	// returns pointer of char array with same length as input.
	void stochastic_quantize(double* source, int len, char* target);
	// deterministic quantize an float array into byte buffer.
	// returns pointer of char array with same length as input.
	void deterministic_quantize(double* source, int len, char* target);
	// decode bytes buffer into float array.
	// returns pointer of float array with same length as input.
	void decode_quantized_array(char* source, int len, double* target);
};

class UnknownCodeException {
private:
	int code_input;
public:
	UnknownCodeException(int code_input) : code_input(code_input) {}
};