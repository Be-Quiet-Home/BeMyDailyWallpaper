#ifndef DEMO_PROVIDER_H
#define DEMO_PROVIDER_H

#include "DailyImageProvider.h"


class DemoProvider : public DailyImageProvider {
public:
	virtual const char* Name() const;
	virtual status_t Fetch(ProviderResult& result);
};

#endif
