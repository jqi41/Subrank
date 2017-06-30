#ifndef USRA_H_
#define USRA_H_

bool myfunction(double i, double j) { return (i > j); }

double dcg_scores(double rel) {
    if (rel == 0.0)
        return 0.0;
    else if (rel == 1.0)
        return 1.0;
    else if (rel == 2.0)
        return 3.0;
    else if (rel == 3.0)
        return 7.0;
    else
        return 15.0;
}

class USra {
protected:
  std::vector<double> w; /* Weights1 for ranking list */

protected:
    unsigned nsamples = 100; /* the max sampling number (default 100) */
    unsigned n_voters;       /* Number of voters */
    FuncKind kind;

    /* Training data */
    std::map<std::string, std::vector<std::vector<double>>> train_data;
    std::map<std::string, std::map<std::string, int>> train_gt;
    std::map<std::string, totalOrder> train_order;

    /* Developing data */
    std::map<std::string, std::vector<std::vector<double>>> dev_data;
    std::map<std::string, std::map<std::string, int>> dev_gt;
    std::map<std::string, totalOrder> dev_order;

    /* Testing data */
    std::map<std::string, std::vector<std::vector<double>>> test_data;
    std::map<std::string, std::map<std::string, int>> test_gt;
    std::map<std::string, totalOrder> test_order;

public:
    USra() {}
    USra(unsigned _n_voters) : n_voters(_n_voters) {
        for (size_t i = 0; i < _n_voters; i++)
            w.push_back(1.0 / n_voters);
    }
    USra(unsigned _n_voters, unsigned _nsamples)
        : n_voters(_n_voters), nsamples(_nsamples) {
            for (size_t i=0; i<_n_voters; i++)
                w.push_back(1.0 / n_voters);
    }
    USra(unsigned _n_voters, FuncKind _kind) : n_voters(_n_voters), kind(_kind) {
        for (size_t i = 0; i < _n_voters; i++)
            w.push_back(1.0 / n_voters);
    }
    USra(unsigned _n_voters, unsigned _nsamples, FuncKind _kind)
        : n_voters(_n_voters), nsamples(_nsamples), kind(_kind) {
        for (size_t i = 0; i < _n_voters; i++)
            w.push_back(1.0 / n_voters);
    }
    std::map<std::string, std::map<size_t, std::string>> ReadData(std::string fn,
           std::map<std::string, std::vector<std::vector<double>>> &data,
           std::map<std::string, std::map<std::string, int>> &gt,
           std::map<std::string, totalOrder> &order);

    std::vector<unsigned> Sampling(unsigned nmax);
    bool IsValidSmp(std::vector<std::vector<double>> x, std::vector<unsigned> cur,
                    std::vector<unsigned> last);
    std::vector<double> GetRList(std::vector<std::vector<double>> _data,
                                 size_t i);
    double ComputeNDCG(std::vector<double> sol_score, unsigned topK);
    double ComputeMAP(std::vector<double> sol_score, unsigned topK);
    double ComputeLBD1(std::vector<double> x, std::vector<unsigned> order,
                       FuncKind _kind);
    std::vector<double> Training(std::string train_fn, double lr);
    std::vector<double> Inference(std::string fn, size_t topK, bool compNDCG);
    double GetNDCG(std::map<std::string, std::vector<std::string>> _sol,
                   std::map<std::string, std::map<std::string, int>> _gt, unsigned topK);
    double GetMAP(std::map<std::string, std::vector<std::string>> _sol,
                  std::map<std::string, std::map<std::string, int>> _gt, unsigned topK);
};

std::map<std::string, std::map<size_t, std::string>> USra::ReadData(std::string fn,
               std::map<std::string, std::vector<std::vector<double>>> &data,
               std::map<std::string, std::map<std::string, int>> &gt,
               std::map<std::string, totalOrder> &order) {
    const int READMAX = 1024;
    std::ifstream In(fn.c_str());
    char buf[READMAX] = "";
    std::map<std::string, std::map<size_t, std::string>> MapStr;

    unsigned _idx = 0;
    while (In.good()) {
        In.getline(buf, READMAX);
        std::string str_buf(buf);
        if (str_buf == "")
            break;
        std::stringstream a;
        a.str(buf);
        bool title = true;
        std::string qid, doc, relevance, value;
        std::vector<double> subdata;
        unsigned count = 1;
        while (a) {
            if (title) {
                a >> qid;
                a >> doc;
                a >> relevance;
                if (data.find(qid) == data.end())
                    data[qid] = std::vector<std::vector<double>>();
                if (MapStr.find(qid) == MapStr.end()) {
                    MapStr[qid] = std::map<size_t, std::string>();
                    _idx = 0;
                }
                if (gt.find(qid) == gt.end())
                    gt[qid] = std::map<std::string, int>();
                if (order.find(qid) == order.end())
                    order[qid].push_back(relevance);
                gt[qid][doc] = std::atoi(relevance.c_str());
                MapStr[qid][_idx] = doc;
                title = false;
                _idx++;
            } else {
                count++;
                a >> value;
                double tmp = std::atof(value.c_str());
                subdata.push_back(tmp);
            }
            if (count > n_voters)
                break;
        }
        data[qid].push_back(subdata);
        subdata.clear();
        title = true;
    }
    return MapStr;
}

double USra::ComputeLBD1(std::vector<double> x, std::vector<unsigned> order,
                         FuncKind _kind) {
    std::vector<double> sorted_order;
    std::vector<size_t> sorted_idx;
    sort<double>(x, sorted_order, sorted_idx);
    double delta;

    double sum_x = 0.0;
    double sum_sigma = 0.0;
    for (unsigned i = 1; i <= sorted_idx.size(); i++) {
        switch (_kind) {
            case Sqrt:
                delta = std::sqrt(i) - std::sqrt(i - 1);
                break;
            case Sigmoid:
                delta = 1 / (1 + std::exp(-double(i))) - 1 / (1 + std::exp(-double(i - 1)));
                break;
            case Log:
                delta = log(1 + i) - log(i);
                break;
        }
        sum_x += x[sorted_idx[i - 1]] * delta;
        sum_sigma += x[order[i - 1] - 1] * delta;
    }
    return sum_x - sum_sigma;
}

std::vector<unsigned> USra::Sampling(unsigned nmax) {
    std::vector<unsigned> order;
    for (unsigned i = 1; i <= nmax; i++)
        order.push_back(i);
    std::random_shuffle(order.begin(), order.end(), myrandom);

    return order;
}

bool USra::IsValidSmp(std::vector<std::vector<double>> x,
                      std::vector<unsigned> curOrder,
                      std::vector<unsigned> lastOrder) {
    std::vector<double> d_vec1, d_vec2;
    for (size_t i = 0; i < n_voters; i++) {
        d_vec1.push_back(ComputeLBD1(x[i], curOrder, kind));
        d_vec2.push_back(ComputeLBD1(x[i], lastOrder, kind));
    }
    double alpha1 = std::exp(-1 * InnerProd(w, d_vec1) / w.size());
    double alpha2 = std::exp(-1 * InnerProd(w, d_vec2) / w.size());
    double alpha = alpha1 / alpha2;

    if (alpha > 0.9)
        return true;
    else
        return false;
}

std::vector<double> USra::GetRList(std::vector<std::vector<double>> _data, size_t i) {
    assert(i < _data.size());
    return _data[i];
}

std::vector<double> USra::Training(std::string train_fn, double lr) {
    ReadData(train_fn, train_data, train_gt, train_order);
    std::map<std::string, std::vector<std::vector<double>>>::iterator iter;
    std::vector<std::vector<double>> _data;
    std::vector<std::vector<double>> x;
    std::vector<double> _x;

    int count = 0;
    for (size_t _iterations = 0; _iterations < 1; _iterations++) {
        iter = train_data.begin();
        for (; iter != train_data.end(); ++iter) {
            x.clear();
            _data.clear();
            _x.clear();
            _data = train_data[iter->first];
            std::cout << "Iter: " << count << std::endl;
            count++;
            for (size_t i = 0; i < _data[0].size(); i++) {
                for (size_t j = 0; j < _data.size(); j++)
                    _x.push_back(_data[j][i]);
                x.push_back(_x);
                _x.clear();
            }
            std::vector<std::vector<unsigned>> orderArr;
            std::vector<unsigned> curr, prev;
            prev = Sampling((unsigned int)_data.size());
            orderArr.push_back(prev);

            unsigned _nsamples = 0;
            while (_nsamples < nsamples - 1) {
                curr = Sampling((unsigned int)_data.size());
                if (IsValidSmp(x, curr, prev)) {
                    orderArr.push_back(curr);
                    prev = curr;
                    _nsamples++;
                }
            }
            double delta_w = 0.0;
            double sum_w = 0.0;
            for (size_t i = 0; i < w.size(); i++) {
                delta_w = 0.0;
                for (size_t t = 0; t < nsamples; t++)
                    delta_w += ComputeLBD1(GetRList(x, i), orderArr[t], kind);
                delta_w /= nsamples;
                w[i] *= std::exp(-lr * delta_w);
                sum_w += w[i];
            }
            for (size_t i = 0; i < w.size(); i++)
                w[i] /= sum_w;
        }
    }
//  for (size_t i = 0; i < w.size(); i++)
//    std::cout << w[i] << std::endl;
    return w;
}

double USra::ComputeNDCG(std::vector<double> sol_score, unsigned topK) {
    if (sol_score.size() == 0)
        return 0.0;
    std::vector<double> gt_score = sol_score;
    std::sort(gt_score.begin(), gt_score.end(), myfunction);

    double score_sol_total = std::pow(2.0, dcg_scores(sol_score[0])) - 1;
    double score_gt_total = std::pow(2.0, dcg_scores(gt_score[0])) - 1;
    double de;
    for (size_t idx = 2; idx <= topK; idx++) {
        de = std::log2(double(idx));
        score_sol_total += (std::pow(2.0, dcg_scores(sol_score[idx - 1])) - 1) / de;
        score_gt_total += (std::pow(2.0, dcg_scores(gt_score[idx - 1])) - 1) / de;
    }
    if (score_gt_total == 0)
        return 0.15;
    else
        return score_sol_total / score_gt_total;
}

double USra::ComputeMAP(std::vector<double> sol_score, unsigned topK) {
    if (sol_score.size() == 0)
        return 0.0;
    double score_sol_total = 0.0;
    for (size_t r = 1; r<= topK; r++)
        score_sol_total += sol_score[r - 1] * std::log(topK / r);
    score_sol_total /= topK;
    
    return score_sol_total;
}

double USra::GetNDCG(std::map<std::string, std::vector<std::string>> _sol,
                     std::map<std::string, std::map<std::string, int>> _gt,
                     unsigned topK) {
    std::map<std::string, int> sub_gt;
    std::vector<std::string> sub_sol;
    std::vector<double> eval_score;
    std::map<std::string, std::vector<std::string>>::iterator iter = _sol.begin();
    std::vector<double> final;
    double _final;

    for (; iter != _sol.end(); ++iter) {
        eval_score.clear();
        sub_gt = _gt[iter->first];
        sub_sol = _sol[iter->first];
        for (size_t i = 0; i < sub_sol.size(); i++)
            eval_score.push_back(sub_gt[sub_sol[i]]);
        if (topK <= sub_sol.size()) {
            _final = ComputeNDCG(eval_score, topK);
            if (_final != -1)
                final.push_back(_final);
        }
    }
    double _sum = 0.0;
    for (size_t i = 0; i < final.size(); i++)
        _sum += final[i];
    return _sum / final.size();
}

double USra::GetMAP(std::map<std::string, std::vector<std::string>> _sol,
                    std::map<std::string, std::map<std::string, int>> _gt,
                    unsigned topK) {
    std::map<std::string, int> sub_gt;
    std::vector<std::string> sub_sol;
    std::vector<double> eval_score;
    std::map<std::string, std::vector<std::string>>::iterator iter = _sol.begin();
    std::vector<double> final;
    double _final;
    
    for (; iter != _sol.end(); ++iter) {
        eval_score.clear();
        sub_gt = _gt[iter->first];
        sub_sol = _sol[iter->first];
        for (size_t i=0; i<sub_sol.size(); i++)
            eval_score.push_back(sub_gt[sub_sol[i]]);
        if (topK <= sub_sol.size()) {
            _final = ComputeMAP(eval_score, topK);
            final.push_back(_final);
        }
    }
    double _sum = 0.0;
    for (size_t i=0; i<final.size(); i++)
        _sum += final[i];
    return _sum / final.size();
}

std::vector<double> USra::Inference(std::string test_fn, size_t topK,
                                    bool compNDCG) {
    std::map<std::string, std::map<size_t, std::string>> trans =
        ReadData(test_fn, test_data, test_gt, test_order);
    std::map<std::string, std::vector<std::string>> rx;
    std::map<size_t, std::string> _gt;
    totalOrder _order;
    std::vector<std::vector<double>> x;
    std::vector<double> _x, _Rx, _rx;
    std::vector<double> _map_Rx, _map_rx;
    std::vector<size_t> idx;
    std::vector<std::string> str_idx;
    std::map<std::string, std::vector<std::vector<double>>>::iterator iter = test_data.begin();
    std::vector<double> results;
    std::vector<double> map_results;

    if (compNDCG) {
        for (; iter != test_data.end(); iter++) {
            _Rx.clear();    _map_Rx.clear();
            _rx.clear();    _map_rx.clear();
            idx.clear();
            str_idx.clear();
            x = test_data[iter->first];
            _gt = trans[iter->first];
            idx.clear();
            for (size_t i = 0; i < x.size(); i++) {
                double sum = 0.0;
                for (size_t j = 0; j < n_voters; j++)
                    sum += x[i][j] * w[j];
                _Rx.push_back(sum);
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
            _Rx.clear();
            _rx.clear();
            idx.clear();
            x = test_data[iter->first];
            _gt = trans[iter->first];
            idx.clear();
            for (size_t i = 0; i < x.size(); i++) {
                double sum = 0.0;
                for (size_t j = 0; j < n_voters; j++)
                    sum += x[i][j] * w[j];
                _Rx.push_back(sum);
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
