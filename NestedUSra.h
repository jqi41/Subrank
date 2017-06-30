#ifndef NESTEDUSRA_H_
#define NESTEDUSRA_H_

inline double SigmoidFunc(double x) {
	return 1 / (1 + exp(-x));
}

class NestedUSra : public USra {
private:
	unsigned nhidden;						/* number of hidden layers */
	std::vector<std::vector<double>> W1;	/* Weights1 for ranking lists */
	std::vector<double> W2;					/* Weigths2 for ranking lists */

public:
	NestedUSra() {}
	NestedUSra(unsigned _n_voters, unsigned _nhidden, FuncKind _kind);
	void Training(std::string train_fn, double lr);
	std::vector<double> Inference(std::string test_fn, unsigned topK, bool compNDCG);
	void PrintParamW1();
	void PrintParamW2();
};

NestedUSra::NestedUSra(unsigned _n_voters, unsigned _nhidden, FuncKind _kind)
	: USra(_n_voters, _kind) {
	nhidden = _nhidden;
	for (size_t i = 0; i < nhidden; i++) {
		W1.push_back(std::vector<double>());
		for (size_t j = 0; j < n_voters; j++)
			W1[i].push_back(1.0 / double(n_voters * nhidden));
	}
	for (size_t i = 0; i < nhidden; i++)
		W2.push_back(1.0 / nhidden);
}

void NestedUSra::Training(std::string train_fn, double lr) {
	ReadData(train_fn, train_data, train_gt, train_order);
	std::map<std::string, std::vector<std::vector<double>>>::iterator iter;
	std::vector<std::vector<double>> _data;
	std::vector<std::vector<double>> x;
	std::vector<double> _x;
	std::vector<unsigned> prev, curr;
	std::vector<std::vector<unsigned>> orderArr;

	size_t count = 0;
	for (size_t _iteration = 0; _iteration < 1; _iteration++) {
		iter = train_data.begin();
		for (; iter != train_data.end(); iter++) {
			_data.clear(); orderArr.clear();
			_x.clear();	x.clear();
			orderArr.clear();
			_data = train_data[iter->first];
			for (size_t i = 0; i < _data[0].size(); i++) {
				for (size_t j = 0; j < _data.size(); j++)
					_x.push_back(_data[j][i]);
				x.push_back(_x);
				_x.clear();
			}
			prev = Sampling((unsigned)_data.size());
			std::cout << "Iter: " << count << std::endl;
			count++;

			// Sampling of permutations
			for (size_t k = 0; k < nsamples; k++) {
				curr = Sampling((unsigned)_data.size());
				if (IsValidSmp(x, curr, prev)) {
					orderArr.push_back(curr);
					prev = curr;
				}
			}

			// The update for params of layer 1
			std::vector<double> nabla1_vec;
            double delta1 = 0;
            double nabla1 = 0;
			for (unsigned i = 0; i < nhidden; i++) {
                delta1 = 0;
				for (unsigned j = 0; j < n_voters; j++)
					for (unsigned t = 0; t < orderArr.size(); t++)
						delta1 += W1[i][j] * ComputeLBD1(GetRList(x, j), orderArr[t], kind);
                delta1 /= orderArr.size();
                for (unsigned j = 0; j < n_voters; j++) {
                    nabla1 = 0.0;
                    for (unsigned t = 0; t < orderArr.size(); t++)
                        nabla1 += SigmoidFunc(delta1) * (1 - SigmoidFunc(delta1)) *
                            ComputeLBD1(GetRList(x, j), orderArr[t], kind);
                    nabla1 /= orderArr.size();
                    nabla1_vec.push_back(nabla1);
                }
                double Z = 0.0;
                for (unsigned j = 0; j < n_voters; j++)
                    Z += W1[i][j] * std::exp(-lr*nabla1_vec[j]);
                for (unsigned j = 0; j < n_voters; j++)
                    W1[i][j] *= std::exp(-lr*nabla1_vec[j]) / Z;
			}

			// The update for params of layer 2
			std::vector<double> nabla2_vec;
            double delta2 = 0, nabla2 = 0;
			for (unsigned i = 0; i < nhidden; i++) {
				delta1 = 0, delta2 = 0, nabla2 = 0;
                for (unsigned j = 0; j < n_voters; j++)
					for (unsigned t = 0; t < orderArr.size(); t++)
						delta1 += W1[i][j] * ComputeLBD1(GetRList(x, j), orderArr[t], kind);
				delta1 /= orderArr.size();
				for (unsigned j = 0; j < nhidden; j++)
					delta2 += W2[j] * SigmoidFunc(delta1);
				nabla2 = SigmoidFunc(delta2) * (1 - SigmoidFunc(delta2)) * SigmoidFunc(delta1);
				nabla2_vec.push_back(nabla2);
			}
			for (unsigned i = 0; i < nhidden; i++) {
				double Z = 0;
				for (size_t j = 0; j < nhidden; j++)
					Z += W2[j] * std::exp(-lr*nabla2_vec[j]);
				W2[i] *= std::exp(-lr*nabla2_vec[i]) / Z;
			}
		}
	}
}

std::vector<double> NestedUSra::Inference(std::string test_fn, unsigned topK, bool compNDCG) {
	std::map<std::string, std::map<size_t, std::string>> trans =
		ReadData(test_fn, test_data, test_gt, test_order);
	std::map<std::string, std::vector<std::string>> rx;
	std::map<size_t, std::string> _gt;
	totalOrder _order;
	std::vector<double> results;
    std::vector<double> map_results;
	std::vector<std::vector<double>> x;
	std::vector<double> _x, _Rx, _rx;
    std::vector<double> _map_Rx, _map_rx;
	std::vector<size_t> idx;
	std::vector<std::string> str_idx;
	std::map<std::string, std::vector<std::vector<double>>>::iterator
		iter = test_data.begin();

	if (compNDCG) {
		for (; iter != test_data.end(); iter++) {
			_Rx.clear(); _rx.clear(); idx.clear(); str_idx.clear();
			x = test_data[iter->first];
			_gt = trans[iter->first];
			for (size_t i = 0; i < x.size(); i++) {
				double f1 = 0, f2 = 0;
				for (size_t n = 0; n < nhidden; n++) {
					f1 = 0;
					for (size_t j = 0; j < n_voters; j++)
						f1 += x[i][j] * W1[n][j];
					f1 = SigmoidFunc(f1);
					f2 += f1 * W2[n];
				}
				_Rx.push_back(SigmoidFunc(f2));
			}
			sort<double>(_Rx, _rx, idx);
			for (size_t i = 0; i < idx.size(); i++)
				str_idx.push_back(_gt[idx[i]]);
			rx[iter->first] = str_idx;
		}
        for (unsigned i = 1; i <= topK; i++) {
			results.push_back(GetNDCG(rx, test_gt, i));
            map_results.push_back(GetMAP(rx, test_gt, i));
        }
	}
	else {
		std::ofstream out("boost.txt");
		std::map<int, std::vector<double>> repr;
		for (; iter != test_data.end(); iter++) {
			_Rx.clear(); _rx.clear(); idx.clear();
			x = test_data[iter->first];
			idx.clear();
			for (size_t i = 0; i < x.size(); i++) {
				double f1 = 0, f2 = 0;
				for (size_t n = 0; n < nhidden; n++) {
                    f1 = 0;
					for (size_t j = 0; j < n_voters; j++)
						f1 += x[i][j] * W1[n][j];
					f1 = SigmoidFunc(f1);
					f2 += f1 * W2[n];
				}
				f2 = SigmoidFunc(f2);
				_Rx.push_back(f2);
			}
			repr[std::atoi(iter->first.c_str())] = _Rx;
		}
		for (unsigned i = 0; i < repr.size(); i++) {
			std::vector<double> tmp = repr[i];
			for (unsigned j = 0; j < tmp.size(); j++)
				out << tmp[j] << " ";
			out << std::endl;
		}
		out.close();
	}

	return results;
}

#endif 
