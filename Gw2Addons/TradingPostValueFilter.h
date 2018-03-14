#ifndef TRADING_POST_VALUE_FILTER_H
#define TRADING_POST_VALUE_FILTER_H

#include "SingleFilter.h"
#include "TradingPostModus.h"

class TradingPostValueFilter : public SingleFilter<int> {
protected:
	TradingPostModus modus = TradingPostModus::Sell;
	virtual IFilter * CreateNew() override;
	virtual void SerializeContent(tinyxml2::XMLPrinter &printer) override;
	virtual void DeserializeContent(tinyxml2::XMLElement* element) override;
public:

	TradingPostValueFilter();
	// Geerbt über SingleFilter
	virtual char * GetSerializeName() override;
	virtual std::string GetName() override;
	virtual bool IsFiltered(FilterData data) override;

	virtual void RenderInput(int& value) override;

	virtual void RenderContent() override;

};

#endif