#pragma once
#include <algorithm>
#include <tuple>
template <typename T>
class FragmentList
{
	static_assert(std::is_arithmetic<T>::value, "Template parameter is not an arithmetic type!");
protected:
	T ** collection;
	int number_of_elements;
	int number_of_fragments;
	int *fragment_capacity;
	int max_number_of_elements;

	//methods
	std::tuple<int, int> findElementIndex(int) const noexcept; //noexcept because we already checked if the index is out of range
	virtual void destroy() noexcept;
	virtual void copy(const FragmentList<T>&);
	virtual void move(FragmentList<T>&&) noexcept;
public:
	FragmentList() noexcept;
	FragmentList(const FragmentList<T>&);
	FragmentList(FragmentList<T>&&) noexcept;
	virtual ~FragmentList() noexcept;

	FragmentList<T>& operator=(const FragmentList<T>&);
	FragmentList<T>& operator=(FragmentList<T>&&) noexcept;

	void allocate(int n);
	virtual void addElement(const T) noexcept(false);

	virtual constexpr std::tuple<int, int> no_of_elements_and_empty_elements() const noexcept final;

	virtual T& operator[](int n) noexcept(false);
};

template <typename T>
FragmentList<T>::FragmentList() noexcept : collection(nullptr), number_of_elements(0), number_of_fragments(0), fragment_capacity(nullptr) {};

template <typename T>
void FragmentList<T>::allocate(int n)
{
	T ** tmp = new T*[number_of_fragments + 1];
	int *capacity_tmp = new int[number_of_fragments + 1]; //increase in size
	std::copy(collection, collection + number_of_fragments, tmp);
	std::copy(fragment_capacity, fragment_capacity + number_of_fragments, capacity_tmp);
	collection = tmp;
	fragment_capacity = capacity_tmp;
	collection[number_of_fragments] = new T[n]; // acutall allocation //index of the "first" element is actually zero
	max_number_of_elements += n;
	fragment_capacity[number_of_fragments] = n;
	++number_of_fragments;
	std::fill(tmp, tmp + number_of_fragments, nullptr);
	std::fill(capacity_tmp, capacity_tmp + number_of_fragments, nullptr); //because of reasons, maybe redundant, avoid destruction of things that are pointed to
}

template<typename T>
std::tuple<int, int> FragmentList<T>::findElementIndex(int n) const noexcept
{
	int i = n;
	int fragment = 0;
	std::for_each(fragment_capacity, fragment_capacity + number_of_fragments,
		[&i](int number)
	{
		if (i > number)
		{
			i -= number;
			++fragment;
		}
	});
	return std::make_tuple<int, int>(fragment, i);
}

template<typename T>
void FragmentList<T>::addElement(const T new_element) noexcept(false)
{
	if (number_of_elements == max_number_of_elements)
		throw (new std::exception("The list is full. The club is packed, sorry. "));
	else
	{
		std::tuple<int, int> tmp = findElementIndex(number_of_elements);
		int fragment = tmp[0];
		int index = tmp[1];
		collection[fragment][index] = new_element;
	}
}

template<typename T>
T& FragmentList<T>::operator[](int n) noexcept(false)
{
	if (n<0 || n>number_of_elements)
		throw(new std::exception("Index out of range"));
	else
	{
		std::tuple<int, int> tmp = findElementIndex(n);
		return collection[tmp[0]][tmp[1]];
	}
}

template <typename T>
constexpr std::tuple<int, int> FragmentList<T>::no_of_elements_and_empty_elements() const noexcept
{
	return std::make_tuple<int, int>(number_of_elements, max_number_of_elements - number_of_elements);
}

template <typename T>
FragmentList<T>::~FragmentList()
{
	destroy();
}

template <typename T>
void FragmentList<T>::destroy() noexcept
{
	delete[] fragment_capacity;
	std::for_each(collection, collection + number_of_fragments, [](T*& arr_ptr) {delete[] arr_ptr; });
	delete[] collection;
}

template <typename T>
void FragmentList<T>::copy(const FragmentList<T>& other)
{
	number_of_elements = other.number_of_elements;
	number_of_fragments = other.number_of_fragments;
	fragment_capacity = new int[number_of_fragments];
	std::copy(other.fragment_capacity, other.fragment_capacity + number_of_fragments, fragment_capacity);
	collection = new T**[number_of_fragments];
	int index = 0;
	std::for_each(collection, collection + number_of_fragments, [](T*& arr_ptr)
	{
		arr_ptr = new T[fragment_capacity[index]];
		std::copy(other.collection[index], other.collection[index] + fragment_capacity[index++], arr_ptr);
	});
}

template <typename T>
void FragmentList<T>::move(FragmentList<T>&& other) noexcept
{
	fragment_capacity = other.fragment_capacity;
	collection = other.collection;
	number_of_elements = other.number_of_elements;
	number_of_fragments = other.number_of_fragments;
	max_number_of_elements = other.max_number_of_elements;
	other.fragment_capacity = nullptr;
	other.collection = nullptr;
}

template <typename T>
FragmentList<T>::FragmentList(const FragmentList& other)
{
	copy(other);
}

template <typename T>
FragmentList<T>::FragmentList(FragmentList&& other) noexcept
{
	move(std::move(other));
}

template <typename T>
FragmentList<T>& FragmentList<T>::operator=(FragmentList&& other) noexcept
{
	destroy();
	move(std::move(other));
	return *this;
}

template <typename T>
FragmentList<T>& FragmentList<T>::operator=(const FragmentList& other)
{
	if (this != &other)
	{
		destroy();
		copy(other);
	}
	return *this;
}