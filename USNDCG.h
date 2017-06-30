#ifndef USNDCG_h
#define USNDCG_h

class USNDCG : public USra {
public:
    USNDCG() {}
    USNDCG(unsigned _n_voters, unsigned _nsamples) : USra(_n_voters, _nsamples) {}
    double SubmodFunc(std::vector<unsigned> S);
    double SubmodMargin(unsigned item, std::vector<unsigned> S);
    double SubmodDiff(unsigned item_i, std::vector<unsigned> S_i,
                      unsigned item_j, std::vector<unsigned> S_j);
    double ComputeAj(std::vector<unsigned> permutation, unsigned K, unsigned j);
    std::vector<std::vector<unsigned>> GenSset(std::vector<unsigned> permutation);
    std::vector<double> Training(std::string train_fn, double lr);
};


double USNDCG::SubmodFunc(std::vector<unsigned> S) {
    double sum = 0.0;
    for (unsigned i=0; i<S.size(); i++)
        sum += S[i];
    return std::sqrt(sum);
}

double USNDCG::SubmodMargin(unsigned item, std::vector<unsigned> S) {
    std::vector<unsigned> tmpS;
    for (unsigned i=0; i<S.size(); i++) {
        if (S[i] != item)
            tmpS.push_back(S[i]);
    }
    return SubmodFunc(S) - SubmodFunc(tmpS);
}

double USNDCG::SubmodDiff(unsigned item_i, std::vector<unsigned> S_i, unsigned item_j, std::vector<unsigned> S_j) {
    return SubmodMargin(item_i, S_i) - SubmodMargin(item_j, S_j);
}

std::vector<std::vector<unsigned>> USNDCG::GenSset(std::vector<unsigned> permutation) {
    std::vector<std::vector<unsigned>> S;
    std::vector<unsigned> Scur;
    for (unsigned i=0; i<permutation.size(); i++) {
        Scur.push_back(permutation[i]);
        S.push_back(Scur);
    }
    return S;
}

double USNDCG::ComputeAj(std::vector<unsigned> permutation, unsigned K, unsigned j) {
    std::vector<std::vector<unsigned>> sset = GenSset(permutation);
    double A_j = 0.0, tmp;
    for (unsigned l=0; l<K; l++) {
        tmp = SubmodDiff(l, sset[l], j, sset[j]);
        if (tmp > 0)
            A_j = tmp;
        else
            A_j = 0.0;
    }
    return A_j;
}

std::vector<double> USNDCG::Training(std::string train_fn, double lr) {
    ReadData(train_fn, train_data, train_gt, train_order);
    std::map<std::string, std::vector<std::vector<double>>>::iterator iter;
    std::vector<std::vector<double>> _data;
    std::vector<std::vector<double>> x;
    std::vector<double> _x;
    std::vector<double> w_arr;
    
    iter = train_data.begin();
    unsigned count = 0;
    for (; iter != train_data.end(); ++iter) {
        x.clear(); _data.clear(); _x.clear();
        _data = train_data[iter->first];
        std::cout << "Iter: " << count << std::endl;
        count++;
        for (unsigned i=0; i<_data[0].size(); i++) {
            for (unsigned j=0; j<_data.size(); j++)
                _x.push_back(_data[j][i]);
            x.push_back(_x);
            _x.clear();
        }
        std::vector<std::vector<unsigned>> orderArr;
        std::vector<unsigned> curr;
        
        unsigned _nsamples = 0;
        while (_nsamples < nsamples - 1) {
            curr = Sampling((unsigned)_data.size());
            orderArr.push_back(curr);
            _nsamples++;
        }
        w_arr.clear();
        for (unsigned i=0; i<x.size(); i++) {
            // Delta computation
            double delta = 0.0;
            double pir_j = 1.0;
            for (unsigned j=0; j<x[i].size(); j++) {
                double A_j = 0.0;
                if (x[i][j] != 0) {
                    for (unsigned pi = 0; pi < orderArr.size(); pi++)
                        A_j += ComputeAj(orderArr[pi], (unsigned)x[i].size(), j);
                    A_j /= orderArr.size();
                }
                pir_j += A_j;
                delta += x[i][j] / log(1 + pir_j);
            }
            // Z computation
            double Zi = 0.0;
            std::vector<double> x_sorted;
            std::vector<size_t> x_idx;
            sort<double>(x[i], x_sorted, x_idx);
            for (unsigned j=0; j<x_idx.size(); j++)
                Zi += x[i][j] / log(2 + (double)x_idx[j]);
            if (Zi > 0) {
                delta /= Zi;
                double w_update = w[i] * std::exp(-lr * delta);
                w_arr.push_back(w_update);
            }
            else
                w_arr.push_back(w[i]);
        }
        double w_sum = 0.0;
        for (unsigned i = 0; i < w_arr.size(); i++)
            w_sum += w_arr[i];
        for (unsigned i=0; i<w.size(); i++)
            w[i] = w_arr[i] / w_sum;
    }
    return w;
}

#endif /* USNDCG_h */
