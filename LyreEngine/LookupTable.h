#pragma once

template <typename T>
struct LookupTableArgDesc {
	T from;
	T to;
	int nSteps;
};

template <typename T, typename Arg1T, typename Arg2T>
class LookupTable2D {
protected:
	std::vector<T> m_table;
	using func_type = std::function<T(Arg1T, Arg2T)>;
public:
	LookupTable2D(func_type func, LookupTableArgDesc<Arg1T> desc1, LookupTableArgDesc<Arg2T> desc2) {
		Arg1T x1 = desc1.from;  
		Arg2T x2;
		Arg1T step1 = (desc1.to - desc1.from) / desc1.nSteps;
		Arg2T step2 = (desc2.to - desc2.from) / desc2.nSteps;
		for (int i = 0; i <= desc1.nSteps; i++, x1 += step1) {
			x2 = desc2.from;
			for (int j = 0; j <= desc2.nSteps; j++, x2 += step2) {
				m_table.push_back(func(x1, x2));
			}
		}
	}
	const T* buffer() const {
		return m_table.data();
	}
	const size_t size() const {
		return m_table.size();
	}
	virtual ~LookupTable2D() {}
};

template <typename T, typename ArgT>
class LookupTable final
	: public LookupTable2D<T, int, ArgT> {
	using func_type = std::function<T(ArgT)>;
public:
	LookupTable(func_type func, LookupTableArgDesc<ArgT> desc)
		: LookupTable2D([func](int unused, ArgT value) { return func(value); }, { 0, 0, 1 }, desc)
	{}
};

