#include "quant.h"

#include <random>


static inline int find_lower(double target, double* q_space, int len) {
	for (int i = len - 1; i >= 0; --i) {
		if (q_space[i] <= target)
			return i;
	}
	return 0;
}

static inline int find_upper(double target, double* q_space, int len) {
	for (int i = 0; i < len; ++i) {
		if (q_space[i] >= target)
			return i;
	}
	return len - 1;
}

static inline int fast_find_lower(double target, double* q_space, int len) {
	int hi = len - 1, lo = 0;
	int mid = (len >> 1);
	while (hi > lo) {
		if (target > q_space[mid]) {
			lo = mid;
		}
		else if (target < q_space[mid]) {
			hi = mid - 1;
		}
		else {
			return mid;
		}
		mid = (hi + lo + 1) >> 1;
	}
	return lo;
}

static inline int fast_find_upper(double target, double* q_space, int len) {
	int hi = len - 1, lo = 0;
	int mid = (len >> 1);
	while (hi > lo) {
		if (target > q_space[mid]) {
			lo = mid + 1;
		}
		else if (target < q_space[mid]) {
			hi = mid;
		}
		else {
			return mid;
		}
		mid = (hi + lo) >> 1;
	}
	return hi;
}

Quantize::Quantize(const double* q_space, int len) {
	this->q_space = new double[len];
	for (int i = 0; i < len; ++i) {
		this->q_space[i] = q_space[i];
	}
	this->q_space_len = len;
}

Quantize::~Quantize() {
	this->q_space_len = -1;
	delete[] this->q_space;
}

void Quantize::deterministic_quantize(const double* buffer, int len, char* output) {
	for (int i = 0; i < len; ++i) {
		int lower_bound_index = fast_find_lower(buffer[i], q_space, q_space_len);
		if (lower_bound_index == q_space_len || buffer[i] < (q_space[lower_bound_index + 1] + q_space[lower_bound_index]) / 2) {
			output[i] = lower_bound_index;
		}
		else {
			output[i] = lower_bound_index + 1 >= q_space_len ? q_space_len - 1 : lower_bound_index + 1;
		}
	}
}

void Quantize::stochastic_quantize(const double* buffer, int len, char* output) {
	for (int i = 0; i < len; ++i) {
		int lower_bound_index = fast_find_lower(buffer[i], q_space, q_space_len);
		int upper_bound_index = fast_find_upper(buffer[i], q_space, q_space_len);
		if (lower_bound_index == upper_bound_index) {
			output[i] = lower_bound_index;
		}
		else {
			int rand_space = static_cast<int>((q_space[upper_bound_index] - q_space[lower_bound_index]) * 1000);
			int rand_mark = static_cast<int>((buffer[i] - q_space[lower_bound_index]) * 1000);

			int rnd = rand() % (rand_space);
			if (rnd > rand_mark) {
				output[i] = lower_bound_index;
			}
			else {
				output[i] = upper_bound_index;
			}
		}
	}
}

void Quantize::decode_quantized_array(const char* buffer, int len, double* output) {
	for (int i = 0; i < len; ++i) {
		if (buffer[i] >= q_space_len || buffer[i] < 0) {
			throw UnknownCodeException(buffer[i]);
		}
		output[i] = q_space[buffer[i]];
	}
}
