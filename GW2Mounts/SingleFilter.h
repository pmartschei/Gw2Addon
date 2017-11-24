#ifndef SINGLE_FILTER_H
#define SINGLE_FILTER_H
#include "IFilter.h"

template<class T>
class SingleFilter : public IFilter {
protected:

	virtual std::string GetName() = 0;
public:
	T value;
	// Geerbt über IFilter
	virtual void RenderInput(T &value) = 0;

	// Geerbt über IFilter
	virtual void RenderContent() override;
};
template<class T>
inline void SingleFilter<T>::RenderContent()
{
	ImGui::Text("Value : ");
	ImGui::SameLine(ImGui::GetWindowContentRegionWidth()-ImGui::GetContentRegionAvailWidth()+120);
	ImGui::PushItemWidth(-1);
	RenderInput(value);
	ImGui::PopItemWidth();
	name = GetName();
}
#endif