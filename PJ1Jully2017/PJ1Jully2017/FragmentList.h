#pragma once
#include <algorithm>
#include <tuple>
template <typename T>
class FragmentList
{
	static_assert(std::is_arithmetic<T>::value, "Template parameter is not an arithmetic type!"); //onemogucava kompajliranje ako tip T nije aritmeticki
protected:
	T ** kolekcija;
	int broj_elemenata;
	int broj_fragmenata;
	int *kapacitet_fragmenata;
	int max_broj_elemenata;

	//methods
	std::tuple<int, int> nadjiIndexElementa(int) const noexcept; //noexcept because we already checked if the index is out of range
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

	void allocate(int n); //dodaje tacno 1 fragment
	virtual void addElement(const T) noexcept(false);

	virtual constexpr std::tuple<int, int> broj_elemenataIPraznihElemenata() const noexcept final;

	virtual T& operator[](int n) noexcept(false);
};

template <typename T>
FragmentList<T>::FragmentList() noexcept : kolekcija(nullptr), broj_elemenata(0), broj_fragmenata(0), kapacitet_fragmenata(nullptr) {};

template <typename T>
void FragmentList<T>::allocate(int n) 
{
	T ** tmp = new T*[broj_fragmenata + 1];
	int *capacity_tmp = new int[broj_fragmenata + 1]; //increase in size
	std::copy(kolekcija, kolekcija + broj_fragmenata, tmp);
	std::copy(kapacitet_fragmenata, kapacitet_fragmenata + broj_fragmenata, capacity_tmp);
	kolekcija = tmp;
	kapacitet_fragmenata = capacity_tmp;
	kolekcija[broj_fragmenata] = new T[n]; //zbog toga sto index ide od 0 // acutall allocation //index of the "first" element is actually zero
	max_broj_elemenata += n;
	kapacitet_fragmenata[broj_fragmenata] = n;
	++broj_fragmenata;
	std::fill(tmp, tmp + broj_fragmenata, nullptr);
	std::fill(capacity_tmp, capacity_tmp + broj_fragmenata, nullptr); //because of reasons, maybe redundant, avoid destruction of things that are pointed to
}

template<typename T>
std::tuple<int, int> FragmentList<T>::nadjiIndexElementa(int n) const noexcept
{
	int i = n; //brojac
	int fragment = 0;
	std::for_each(kapacitet_fragmenata, kapacitet_fragmenata + broj_fragmenata,
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
	if (broj_elemenata == max_broj_elemenata)
		throw (new std::exception("The list is full. The club is packed, sorry. "));
	else
	{
		std::tuple<int, int> tmp = nadjiIndexElementa(broj_elemenata);
		int fragment = tmp[0];
		int index = tmp[1];
		kolekcija[fragment][index] = new_element;
	}
}

template<typename T>
T& FragmentList<T>::operator[](int n) noexcept(false)
{
	if (n<0 || n>broj_elemenata)
		throw(new std::exception("Index out of range"));
	else
	{
		std::tuple<int, int> tmp = nadjiIndexElementa(n);
		return kolekcija[tmp[0]][tmp[1]];
	}
}

template <typename T>
constexpr std::tuple<int, int> FragmentList<T>::broj_elemenataIPraznihElemenata() const noexcept
{
	return std::make_tuple<int, int>(broj_elemenata, max_broj_elemenata - broj_elemenata);
}

template <typename T>
FragmentList<T>::~FragmentList()
{
	destroy();
}

template <typename T>
void FragmentList<T>::destroy() noexcept
{
	delete[] kapacitet_fragmenata;
	std::for_each(kolekcija, kolekcija + broj_fragmenata, [](T*& arr_ptr) {delete[] arr_ptr; }); //umjesto T*& arr_ptr moze auto& arr_ptr
	delete[] kolekcija;
}

template <typename T>
void FragmentList<T>::copy(const FragmentList<T>& other)
{
	broj_elemenata = other.broj_elemenata;
	broj_fragmenata = other.broj_fragmenata;
	kapacitet_fragmenata = new int[broj_fragmenata];
	std::copy(other.kapacitet_fragmenata, other.kapacitet_fragmenata + broj_fragmenata, kapacitet_fragmenata);
	kolekcija = new T**[broj_fragmenata];
	int index = 0;
	std::for_each(kolekcija, kolekcija + broj_fragmenata, [](T*& arr_ptr)
	{
		arr_ptr = new T[kapacitet_fragmenata[index]];
		std::copy(other.kolekcija[index], other.kolekcija[index] + kapacitet_fragmenata[index++], arr_ptr);
	});
}

template <typename T>
void FragmentList<T>::move(FragmentList<T>&& other) noexcept
{
	kapacitet_fragmenata = other.kapacitet_fragmenata;
	kolekcija = other.kolekcija;
	broj_elemenata = other.broj_elemenata;
	broj_fragmenata = other.broj_fragmenata;
	max_broj_elemenata = other.max_broj_elemenata;
	other.kapacitet_fragmenata = nullptr;
	other.kolekcija = nullptr;
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
	destroy(); //da ne bude memory leak, curenje memorije
	move(std::move(other));
	return *this;
}

template <typename T>
FragmentList<T>& FragmentList<T>::operator=(const FragmentList& other)
{
	if (this != &other) //provjera da se objekat ne dodjeljuje sam sebi
	{
		destroy();
		copy(other);
	}
	return *this;
}