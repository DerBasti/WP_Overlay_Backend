#ifndef __ROSE_RANDOMIZER__
#define __ROSE_RANDOMIZER__

#include <random>
#include <type_traits>


template<class _NumericType>
class NumericRandomizer {
private:
	std::piecewise_linear_distribution<double> distribution;
	std::default_random_engine generator;
	_NumericType minimum;
	_NumericType maximum;
protected:
	__inline void setNewDistributor(std::piecewise_linear_distribution<double> distributor) {
		this->distribution = distributor;
	}
	__inline void setMinimum(_NumericType minimum) {
		this->minimum = minimum;
	}
	__inline void setMaximum(_NumericType maximum) {
		this->maximum = maximum;
	}
	virtual void onNewMinimumAndMaximum() {
		std::vector<_NumericType> interval{ this->getMinimumBoundry(), this->getMaximumBoundry() };
		std::vector<double> weights{ 1.0, 1.0 };

		setNewDistributor(std::piecewise_linear_distribution<double>(interval.begin(), interval.end(), weights.begin()));
	}
public:
	NumericRandomizer() : NumericRandomizer(_NumericType(0), _NumericType(1)) {

	}
	NumericRandomizer(_NumericType minimum, _NumericType maximum) {
		this->minimum = minimum;
		this->maximum = maximum;

		std::vector<_NumericType> interval{ minimum, maximum };
		std::vector<_NumericType> weights{ (_NumericType)1.0, (_NumericType)1.0 }; 
		generator = std::default_random_engine(std::random_device{}());
		setNewDistributor(std::piecewise_linear_distribution<double>(interval.begin(), interval.end(), weights.begin()));
	}

	virtual ~NumericRandomizer() {

	}
	_NumericType operator()() {
		return generateRandomValue();
	}
	_NumericType generateRandomValue() {
		return generateRandomValue(generator);
	}
	template<class _Engine>
	_NumericType operator()(_Engine& engine) {
		return generateRandomValue(engine);
	}
	template<class _Engine>
	_NumericType generateRandomValue(_Engine& engine) {
		return static_cast<_NumericType>(distribution(engine) + 0.002);
	}
	__inline _NumericType getMinimumBoundry() const {
		return minimum;
	}
	__inline _NumericType getMaximumBoundry() const {
		return maximum;
	}
	__inline void setNewBoundries(const _NumericType newMinimum, const _NumericType newMaximum) {
		if (newMinimum == getMinimumBoundry() && newMaximum == getMaximumBoundry()) {
			return;
		}
		this->minimum = newMinimum;
		this->maximum = newMaximum;
		this->onNewMinimumAndMaximum();
	}
};

template <class _NumericType>
class WeightedNumericRandomizer : public NumericRandomizer<_NumericType> {
protected:
	std::vector<double> weights;
	virtual void onNewMinimumAndMaximum() {
		std::vector<double> interval;
		_NumericType difference = this->getMaximumBoundry() - this->getMinimumBoundry();
		if (weights.size() > 1 && weights.size() > interval.size()) {
			for (uint32_t i = 0; i < weights.size(); i++) {
				double offset = static_cast<double>(difference * i / static_cast<double>(weights.size() - 1));
				interval.push_back(offset + (double)this->getMinimumBoundry());
			}
		}
		else if(weights.size() == 1 && interval.size() > weights.size()) {
			weights.push_back(weights.at(0));
			interval.push_back(this->getMinimumBoundry());
			interval.push_back(this->getMaximumBoundry());
		}

		this->setNewDistributor(std::piecewise_linear_distribution<double>(interval.begin(), interval.end(), weights.begin()));
	}
public:
	WeightedNumericRandomizer() : WeightedNumericRandomizer(_NumericType(0), _NumericType(1), std::vector<double>{1.0, 1.0}) {

	}
	WeightedNumericRandomizer(_NumericType minimum, _NumericType maximum) : WeightedNumericRandomizer(minimum, maximum, std::vector<double>{1.0f, 1.0f}) {

	}
	WeightedNumericRandomizer(_NumericType minimum, _NumericType maximum, std::vector<double> weightsForMinimumAndMaximum) : NumericRandomizer<_NumericType>(minimum, maximum) {
		this->weights = weightsForMinimumAndMaximum;
		this->onNewMinimumAndMaximum();
	}
	virtual ~WeightedNumericRandomizer() {

	}
	__inline std::vector<_NumericType> getWeights() const {
		return weights;
	}
	__inline void setBoundriesAndWeightDistribution(_NumericType minimum, _NumericType maximum, std::vector<double> newWeights) {
		this->setMinimum(minimum);
		this->setMaximum(maximum);
		setNewWeightDistribution(newWeights);
	}
	__inline void setNewWeightDistribution(std::vector<double> newWeights) {
		this->weights = newWeights;
		this->onNewMinimumAndMaximum();
	}
};

#endif //__ROSE_RANDOMIZER__