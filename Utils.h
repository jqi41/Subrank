#ifndef UTILS_H_
#define UTILS_H_

int myrandom(int i) {
	unsigned seed = (unsigned)time(NULL);
	srand(seed);
	return std::rand() % i;
}

enum FuncKind {
	Sqrt = 0,
	Sigmoid = 1,
	Log = 2
};

double InnerProd(std::vector<double> x1, std::vector<double> x2) {
	assert(x1.size() == x2.size());
	double res = 0.0;
	for (unsigned i = 0; i < x1.size(); i++)
		res += x1[i] * x2[i];
	return res;
}

#endif 