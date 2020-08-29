#include <Mlib/Math/Naive_Bayes_Regression.hpp>

using namespace Mlib;

void test_naive_bayes_regression_1d() {
    Array<float> predictors;
    Array<float> responses;

    predictors.resize[3](5);
    responses.resize(5);
    randomize_array(predictors);
    responses = predictors[0] + float(100);

    NaiveBayesRegression<float> r;
    r.train(predictors, responses);
    {
        Array<float> predictors1;
        predictors1.resize(3);
        predictors1(0) = 5;
        predictors1(1) = 15;
        predictors1(2) = 20;
        assert_isclose<float>(r.predicted(predictors1), 105.000877);
    }
}

int main(int argc, const char** argv) {
    test_naive_bayes_regression_1d();
    return 0;
}
