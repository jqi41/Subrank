#ifndef _ULARA_H_
#define _ULARA_H_

class ULARA : public USra{
private:
	std::vector<double> w;

public:
	ULARA() {}
	ULARA(unsigned _n_voters) : USra(_n_voters) {
		for (size_t i = 0; i < _n_voters; i++)
			w.push_back(1.0 / _n_voters);
	}
	std::vector<double> Training(std::string train_fn, double lr);
};

std::vector<double> ULARA::Training(std::string train_fn, double lr) {
	ReadData(train_fn, train_data, train_gt, train_order);
	std::vector<std::vector<double>> _data;
	std::map<std::string, std::vector<std::vector<double>>>::iterator iter;
	std::vector<std::vector<double>> x;
	std::vector<double> _x;
	int count = 0;

	for (size_t _iteration = 0; _iteration < 1; _iteration++) {
		iter = train_data.begin();
		for (; iter != train_data.end(); iter++) {
			_data.clear();
			_data = train_data[iter->first];
			std::cout << "Iter: " << count << std::endl;
			count++;

			double mu = 0;
			for (unsigned i = 0; i < _data.size(); i++) {
				double delta = 0, Z = 0;
				for (unsigned j = 0; j < _data[i].size(); j++)
					mu += _data[i][j];
				mu /= double(_data[i].size());
				std::vector<double> delta_vec;
				for (unsigned j = 0; j < _data[i].size(); j++) {
					delta = pow((_data[i][j] - mu), 2.0);
					Z += w[j] * std::exp(-lr*delta);
					delta_vec.push_back(delta);
				}
				for (size_t j = 0; j < n_voters; j++)
					w[j] = w[j] * std::exp(-lr*delta_vec[j]) / Z;
			}
		}
	}
	return w;
}

#endif
