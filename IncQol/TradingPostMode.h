#ifndef TRADING_POST_MODE_H
#define TRADING_POST_MODE_H

enum TradingPostType {
	Sell,
	Buy,
	TypesCount
};
enum TradingPostMode {
	Percent,
	Flat,
	ModesCount
};
static const char * TradingTypes[] = { "Sell","Buy" };
static const char * TradingModes[] = { "Percentage","Added" };
#endif