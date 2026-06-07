#ifndef DAILY_IMAGE_PROVIDER_H
#define DAILY_IMAGE_PROVIDER_H

#include "ProviderResult.h"


class DailyImageProvider {
public:
	virtual ~DailyImageProvider();

	virtual const char* Name() const = 0;
	virtual bool Fetch(ProviderResult& result) = 0;
};

#endif
