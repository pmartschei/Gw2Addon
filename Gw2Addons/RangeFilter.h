#ifndef RANGE_FILTER_H
#define RANGE_FILTER_H
#include "IFilter.h"

template<class T>
class RangeFilter : public IFilter {
protected:
	virtual std::string GetName() = 0;
public:
	T minValue;
	T maxValue;
	virtual void RenderInput(T &value,std::string s) = 0;
	// Geerbt über IFilter
	virtual void RenderContent() override;
};
template<class T>
inline void RangeFilter<T>::RenderContent()
{
	ImGui::Text("1st Boundary : ");
	ImGui::SameLine(tabSpace);
	ImGui::PushItemWidth(-1);
	RenderInput(minValue,"first");
	ImGui::PopItemWidth();
	ImGui::Text("2nd Boundary : ");
	ImGui::SameLine(tabSpace);
	ImGui::PushItemWidth(-1);
	RenderInput(maxValue,"second");
	ImGui::PopItemWidth();
	name = GetName();
}
#endif