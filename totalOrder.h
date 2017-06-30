#ifndef TOTAL_ORDER_
#define TOTAL_ORDER_

bool pair_greater(std::pair<std::string, int> a,
	std::pair<std::string, int> b) {
	return (a.second > b.second);
};

class totalOrder {
protected:
	std::vector<std::string> _order;

public:
	totalOrder() {
		_order.clear();
	}

	totalOrder(std::map<std::string, int> w) {
		resetOrder(w);
	}

	void resetOrder(std::map<std::string, int> w) {
		std::vector<std::pair<std::string, int>> wp;
		wp.clear();
		std::map<std::string, int>::iterator iter = w.begin();
		for (; iter != w.end(); ++iter) {
			std::pair<std::string, int> myw;
			myw.first = iter->first;
			myw.second = iter->second;
			wp.push_back(myw);
		}
		std::sort(wp.begin(), wp.end(), pair_greater);
		_order.clear();
		for (size_t i = 0; i < static_cast<size_t>(w.size()); i++)
			_order.push_back(wp[i].first);
	}

	void randomizeOrder() {
		std::random_shuffle(_order.begin(), _order.end());
	}

	void push_back(std::string v) {
		_order.push_back(v);
	}

	/* Implementation of access functions */
	std::string order(size_t i) {
		return _order[i];
	}
	size_t size() {
		return _order.size();
	}
	void clear() {
		_order.clear();
	}
	inline std::string operator()(unsigned int i) {
		return _order[i];
	}
	inline std::string operator[](unsigned int i) {
		return _order[i];
	}
};

#endif