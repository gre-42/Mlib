namespace Mlib {

/**
 * From: https://stackoverflow.com/questions/23400693/is-iterable-like-behavior-in-c-attainable
 */
template<class Iterator>
class Iterable
{
public:
    Iterable(const Iterator& begin, const Iterator& end)
    : begin_(begin), end_(end)
    {}

    const Iterator& begin() const
    {
        return begin_;
    }

    const Iterator& end() const
    {
        return end_;
    }
private:
    const Iterator& begin_;
    const Iterator& end_;
};

}
