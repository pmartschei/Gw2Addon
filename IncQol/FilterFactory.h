#ifndef FILTER_FACTORY_H
#define FILTER_FACTORY_H
#include <string>
#include <map>

template <class T>
class FilterFactory
{
public:
	T* get(std::string const& id) const;
	void set(std::string const& id, T* exemplar);

private:
	std::map < std::string, T* > types;
};

template<class T>
inline T * FilterFactory<T>::get(std::string const & id) const
{
	if (types.find(id) == types.end()) return nullptr;
	return types.at(id);
}

template<class T>
inline void FilterFactory<T>::set(std::string const & id, T * exemplar)
{
	types[id] = exemplar;
}

#endif
