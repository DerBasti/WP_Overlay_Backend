#ifndef __VALUE_CHANGE_OBSERVER__
#define __VALUE_CHANGE_OBSERVER__

#include <functional>

template<class _Ty>
struct is_atomic : std::false_type {};

template<class _Ty>
struct is_atomic<std::atomic<_Ty>> : std::true_type {};

template<class _Ty>
struct remove_atomic {
	using type = _Ty;
};

template<class _Ty>
struct remove_atomic<std::atomic<_Ty>> {
	using type = _Ty;
};

template<class _Type>
class ValueChangeObserver {
private:
	std::function<void(const _Type& current, const _Type& newValue)> onValueChanged;
protected:
	std::function<void(const _Type& current, const _Type& newValue)> getOnValueChangedCallback() const {
		return onValueChanged;
	}
	virtual void assignValue(const _Type& otherValue) {
		if (value != otherValue) {
			if constexpr (is_atomic<_Type>::value) {
				_Type previousValue = value.load();
				this->value = otherValue.load();
				onValueChanged(previousValue, otherValue);
			} else {
				_Type previousValue = value;
				this->value = otherValue;
				onValueChanged(previousValue, otherValue);
			}
		}
	}
	_Type value;
public:
	ValueChangeObserver() {
		if constexpr (is_atomic<_Type>::value) {
		}
		else if constexpr (std::is_enum<_Type>::value) {
			this->value = static_cast<_Type>(0);
		}
		else {
			this->value = _Type{ 0 };
		}
		this->onValueChanged = [](const _Type& o, const _Type& e) {};
	}
	ValueChangeObserver(const _Type& initialValue, std::function<void(const _Type& currentValue, const _Type& newValue)> onValueChanged) {
		if constexpr (is_atomic<_Type>::value) {
			this->value = initialValue.load();
		}
		else {
			value = initialValue;
		}
		this->onValueChanged = onValueChanged;
	}
	virtual ~ValueChangeObserver() {

	}
	ValueChangeObserver<_Type>& operator=(const ValueChangeObserver<_Type>& other) {
		if constexpr (is_atomic<_Type>::value) {
			this->value = other.value.load();
		}
		else {
			this->value = other.getValue();
		}
		onValueChanged = other.onValueChanged;
		return *this;
	}
	bool operator==(const ValueChangeObserver<_Type>& other) const {
		return operator==(other.getValue());
	}
	bool operator==(const _Type& other) const {
		return getValue() == other;
	}
	bool operator!=(const ValueChangeObserver<_Type>& other) const {
		return operator!=(other.getValue());
	}
	bool operator!=(const _Type& other) const {
		return !operator==(other);
	}
	virtual ValueChangeObserver<_Type>& operator=(const _Type& other) {
		assignValue(other);
		return *this;
	}
	inline void setValueChangedCallback(std::function<void(const _Type& current, const _Type& newValue)> onValueChanged) {
		this->onValueChanged = onValueChanged;
	}
	inline operator _Type() const {
		return getValue();
	}
	virtual const _Type getValue() const {
		if constexpr(is_atomic<_Type>::value) {
			return value.load();
		}
		else {
			return value;
		}
	}
};

template<class _Type, class = typename std::enable_if<std::is_pointer<_Type>::value>::type>
class PointerChangeObserver {
private:
	std::function<void(_Type current, _Type newValue)> onPointerValueChangedCallback;
protected:
	std::function<void(_Type current, _Type newValue)> getOnValueChangedCallback() const {
		return onPointerValueChangedCallback;
	}
	virtual void assignValue(_Type otherValue) {
		if (value != otherValue) {
			_Type previousValue = value;
			this->value = otherValue;
			onPointerValueChangedCallback(previousValue, otherValue);
		}
	}
	_Type value;
public:
	PointerChangeObserver() {
		this->value = 0;
		this->onPointerValueChangedCallback = [](_Type o, _Type e) {};
	}
	PointerChangeObserver(_Type initialValue, std::function<void(_Type currentValue, _Type newValue)> onValueChanged) {
		value = initialValue;
		this->onPointerValueChangedCallback = onValueChanged;
	}
	virtual ~PointerChangeObserver() {

	}
	PointerChangeObserver<_Type>& operator=(const PointerChangeObserver<_Type>& other) {
		this->value = other.getValue();
		onPointerValueChangedCallback = other.onPointerValueChangedCallback;
		return *this;
	}
	bool operator==(const PointerChangeObserver<_Type>& other) const {
		return operator==(other.getValue());
	}
	bool operator==(const _Type& other) const {
		return getValue() == other;
	}
	bool operator!=(const PointerChangeObserver<_Type>& other) const {
		return operator!=(other.getValue());
	}
	bool operator!=(const _Type& other) const {
		return !operator==(other);
	}
	virtual PointerChangeObserver<_Type>& operator=(const _Type& other) {
		assignValue(other);
		return *this;
	}
	inline _Type operator->() const {
		return getValue();
	}
	inline void setPointerChangedCallback(std::function<void(_Type current, _Type newValue)> onPointerValueChangedCallback) {
		this->onPointerValueChangedCallback = onPointerValueChangedCallback;
	}
	inline operator _Type() const {
		return getValue();
	}
	virtual const _Type getValue() const {
		return value;
	}
};

template<class _Type>
class AtomicValueChangeObserver : public ValueChangeObserver<std::atomic<_Type>> {
protected:
	virtual void assignValue(const _Type& otherValue) {
		if (this->value != otherValue) {
			_Type previousValue = this->value;
			this->value = otherValue;
			this->getOnValueChangedCallback()(previousValue, otherValue);
		}
	}
public:
	AtomicValueChangeObserver() : ValueChangeObserver<std::atomic<_Type>>() {	}
	AtomicValueChangeObserver(const _Type& initialValue, std::function<void(const _Type& currentValue, const _Type& newValue)> onValueChanged) : ValueChangeObserver<std::atomic<_Type>>(initialValue, onValueChanged) {

	}
	virtual ~AtomicValueChangeObserver() {

	}
	virtual AtomicValueChangeObserver<_Type>& operator=(const _Type& other) {
		this->assignValue(other);
		return *this;
	}
};


#endif