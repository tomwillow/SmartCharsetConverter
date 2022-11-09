
#include <unordered_map>
#include <cassert>

// 双向map，支持双向查表操作
template <typename T1, typename T2>
class doublemap
{
public:
	doublemap(std::initializer_list<std::pair<T1, T2>> &&lst)
	{
		for (std::pair<T1, T2> pr : lst)
		{
			insert(std::forward<std::pair<T1, T2>>(pr));
		}
	}

	bool has(const T1 &t1)const
	{
		return t1ToT2.find(t1) != t1ToT2.end();
	}

	bool has(const T2 &t2)const
	{
		return t2ToT1.find(t2) != t2ToT1.end();
	}

	void insert(std::pair<T1, T2> &&pr)
	{
		assert(has(pr.first) == false);
		assert(has(pr.second) == false);
		t1ToT2.insert(pr);
		t2ToT1.insert(std::pair<T2, T1>{pr.second, pr.first});
	}

	const T2 &operator[](const T1 &t1) const
	{
		return t1ToT2.at(t1);
	}

	const T1 &operator[](const T2 &t2) const
	{
		return t2ToT1.at(t2);
	}

private:
	std::unordered_map<T1, T2> t1ToT2;
	std::unordered_map<T2, T1> t2ToT1;
};