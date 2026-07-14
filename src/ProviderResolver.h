#ifndef PROVIDER_RESOLVER_H
#define PROVIDER_RESOLVER_H

#include <SupportDefs.h>


class AppSettings;
class DailyImageProvider;


class ProviderResolver {
public:
	static status_t Create(const AppSettings& settings,
		DailyImageProvider*& provider);
};


#endif
