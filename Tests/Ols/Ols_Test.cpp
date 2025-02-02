#include <Mlib/Math/Ols.hpp>

using namespace Mlib;

void test_ols() {
    Array<float> predictors = random_array<float>(ArrayShape{5});
    Array<float> responses = predictors + float(100);

    {
        Ols<float> r;
        r.train(predictors, responses);
        assert_isclose<float>(r.predicted(5), 105.f);
        assert_isclose(std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity());
        assert_isclose<float>(r.confidence(5), std::numeric_limits<float>::infinity());
        //lerr() << "p " << r.predicted(5);
        //lerr() << "c " << r.confidence_probability(5);
    }
    {
        Ols<float> r;
        r.train(predictors, -responses);
        assert_isclose<float>(r.predicted(5), -105.f, (float)1e-4);
        assert_isclose(std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity());
        assert_isclose<float>(r.confidence(5), std::numeric_limits<float>::infinity());
    }

    responses(4) += 100;
    responses(3) -= 200;
    {
        Ols<float> r;
        r.train(predictors, responses);
        assert_isclose<float>(r.predicted(5), 98.134155f);
        assert_isclose<float>(r.confidence(5), 11.770093f);
        //lerr() << "p " << r.predicted(5);
        //lerr() << "c " << r.confidence_probability(5);
    }
    {
        Ols<float> r;
        r.train(predictors, -responses);
        assert_isclose<float>(r.predicted(5), -98.134155f);
        assert_isclose<float>(r.confidence(5), 11.770093f);
    }
}

int main(int argc, const char** argv) {
    test_ols();
    return 0;
}
