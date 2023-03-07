#pragma once
#include <Mlib/Math/Ols.hpp>

namespace Mlib {

template <class TData>
class NaiveBayesRegression {
private:
    Array<Ols<TData>> ols_;
public:
    void train(const Array<TData>& predictors, const Array<TData>& responses)
    {
        assert(predictors.ndim() == 2);
        assert(responses.ndim() == 1);
        assert(predictors.shape(1) == responses.shape(0));
        ols_.resize(predictors.shape(0));
        for (size_t i=0; i<ols_.length(); i++) {
            ols_(i).train(predictors[i], responses, (TData)1e-6);
        }
    }
    TData predicted(const Array<TData>& predictors) const {
        assert(all(predictors.shape() == ols_.shape()));
        TData weighted_predicted = 0;
        TData sum_confidences = 0;
        for (size_t i=0; i<ols_.length(); i++) {
            const TData confidence = ols_(i).confidence(predictors(i));
            sum_confidences += confidence;
            weighted_predicted += confidence * ols_(i).predicted(predictors(i));
        }
        return weighted_predicted / sum_confidences;
    }
};

}