#ifndef SINGLETON_H
#define SINGLETON_H
template<class T>
class Singleton {
public:
	static T* GetInstance() {
		static T* instance = new T();
		return instance;
	}
private:
	Singleton(Singleton const&) {};
	void operator=(Singleton const&) {};
protected:
	Singleton<T>() {};
};
#endif