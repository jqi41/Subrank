#include <algorithm>
#include <assert.h>
#include <cctype>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <math.h>
#include <numeric>
#include <sstream>
#include <stdlib.h>
#include <string>
#include <unordered_set>
#include <vector>

#include "sort.h"
#include "Utils.h"
#include "totalOrder.h"
#include "USra.h"
#include "NestedUSra.h"
#include "Parse-Options.h"
#include "ULARA.h"
#include "USNDCG.h"


enum LBDTrainKind {
    _Linear = 0,
    _Nested = 1,
    _USNDCG = 2
};

struct USra_Options {
public:
    std::string train_fn;
    std::string dev_fn;
    std::string test_fn;
    unsigned nVoters;
    unsigned nHidden;
    unsigned USraKind;
    unsigned nsamples;
    unsigned topK;
    double learningRate;
    bool compNDCG;

    explicit USra_Options()
        : nVoters(46), nHidden(10), topK(10), learningRate(0.1), USraKind(0),
          compNDCG(false) {}

    void Register(ParseOptions *po) {
        po->Register("nVoters", &nVoters, "Number of ranking lists");
        po->Register("learningRate", &learningRate, "Learning rate for params update");
        po->Register("topK", &topK, "Observe top-K ranking elements");
        po->Register("USraKind", &USraKind, "Training type of the unsupervised submodular rank aggregation");
        po->Register("nHidden", &nHidden, "Number of elements in the hidden layer");
        po->Register("compNDCG", &compNDCG, "Whether or not NDCG is computed ?");
    }
};

int main(int argc, char *argv[]) {
    const char *usage =
        "Supervised/Unsupervised Submodular Ranking Aggregation\n"
        "Usage1: USra [options...] train_path dev_path test_path\n"
        "Usage2: USra [options...] train_path test_path\n";
    ParseOptions po(usage);
    USra_Options usra_opt;
    usra_opt.Register(&po);
    po.Read(argc, argv);
    short numArg = po.NumArgs();

    if (numArg != 2) {
        po.PrintUsage();
        exit(1);
    }
    std::vector<double> NDCG_set;
    if (numArg == 2) {
        usra_opt.train_fn = po.GetArg(1);
        usra_opt.test_fn = po.GetArg(2);
        if (usra_opt.USraKind == 0) {
            USra alg(usra_opt.nVoters, Sigmoid);
            alg.Training(usra_opt.train_fn, usra_opt.learningRate);
            NDCG_set =
                alg.Inference(usra_opt.test_fn, usra_opt.topK, usra_opt.compNDCG);
        }
        else if (usra_opt.USraKind == 1) {
            NestedUSra alg(usra_opt.nVoters, usra_opt.nHidden, Sigmoid);
            alg.Training(usra_opt.train_fn, usra_opt.learningRate);
            NDCG_set = alg.Inference(usra_opt.test_fn, usra_opt.topK, usra_opt.compNDCG);
        }
        else if (usra_opt.USraKind == 2){
            ULARA alg(usra_opt.nVoters);
            alg.Training(usra_opt.train_fn, usra_opt.learningRate);
            NDCG_set = alg.Inference(usra_opt.test_fn, usra_opt.topK, usra_opt.compNDCG);
        }
    }
    
    if (usra_opt.compNDCG) {
        for (size_t i = 0; i < NDCG_set.size(); i++)
            std::cout << "NDCG " << i + 1 << ": " << NDCG_set[i] << std::endl;
    }

    return 0;
}
