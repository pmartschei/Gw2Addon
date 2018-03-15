#ifndef TRADING_POST_MODUS_H
#define TRADING_POST_MODUS_H

enum TradingPostType {
	Sell,
	Buy,
	TypesCount
};
enum TradingPostModus {
	Percent,
	Flat,
	ModusCount
};
static const char * TradingTypes[] = { "Sell","Buy" };
static const char * TradingModus[] = { "Percent","Flat" };
#endif