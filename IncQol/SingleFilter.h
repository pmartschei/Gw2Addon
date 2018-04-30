#ifndef SINGLE_FILTER_H
#define SINGLE_FILTER_H
#include "GroupFilter.h"

template<class T>
class SingleFilter : public GroupFilter {
protected:
	T value;
public:
	SingleFilter() { supportDrop = false; }
	virtual void SetValue(T value) { this->value = value; name = GetName(); }
	// Geerbt über IFilter
	virtual void RenderInput(T &value) = 0;

	// Geerbt über IFilter
	virtual void RenderContent() override;
	virtual void CustomMenu() override;
};
template<class T>	
void SingleFilter<T>::RenderContent()
{
	if (filterIndex > 0) {
		showNameInput = false;
		GroupFilter::RenderContent();
	}
	else {
		ImGui::Text("Value : ");
		ImGui::SameLine(tabSpace);
		ImGui::PushItemWidth(-1);
		RenderInput(value);
		ImGui::PopItemWidth();
		name = GetName();
	}
}

template<class T>
void SingleFilter<T>::CustomMenu() {

}
#endif