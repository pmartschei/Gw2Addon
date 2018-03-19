#ifndef FOREIGN_FUNCTION_H
#define FOREIGN_FUNCTION_H

#define CALL_FN(conv) ((T(conv*)(Ts...))m_ptr)(args...)
enum CallingConvention {
	CALL_CONV_FASTCALL,
	CALL_CONV_CDECL,
	CALL_CONV_STDCALL,
	CALL_CONV_THISCALL,
	CALL_CONV_VECTORCALL
};

template <typename T, CallingConvention cv = CALL_CONV_CDECL>
class ForeignFunction {
public:
	ForeignFunction() : m_ptr(nullptr) {}
	ForeignFunction(void *ptr) : m_ptr(ptr) {}
	ForeignFunction(uintptr_t ptr) { m_ptr = (void*)ptr; }
	void* data() {
		return m_ptr;
	}

	template <typename... Ts>
	T operator()(Ts... args) {
		if (!m_ptr) throw EXCEPTION_ACCESS_VIOLATION;

		switch (cv) {
		case CALL_CONV_FASTCALL:
			return CALL_FN(__fastcall);
			break;

		case CALL_CONV_CDECL:
			return CALL_FN(__cdecl);
			break;

		case CALL_CONV_STDCALL:
			return CALL_FN(__stdcall);
			break;

		case CALL_CONV_THISCALL:
			return CALL_FN(__thiscall);
			break;

		case CALL_CONV_VECTORCALL:
			return CALL_FN(__vectorcall);
			break;

		default:
			throw EXCEPTION_ACCESS_VIOLATION;
		}
	}

	explicit operator bool() const {
		return !!m_ptr;
	}

private:
	void *m_ptr;

};
#endif