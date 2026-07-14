#include "DailyImageProvider.h"
#include "DemoProvider.h"
#include "ProviderResult.h"

#include <Errors.h>

#include <stdio.h>


class FailingProvider : public DailyImageProvider {
public:
	virtual const char* Name() const
	{
		return "Failing provider";
	}

	virtual status_t Fetch(ProviderResult& result)
	{
		(void)result;
		return B_ERROR;
	}
};


static int
Fail(const char* message)
{
	fprintf(stderr, "BeMyDailyWall provider contract smoke: %s\n", message);
	return 1;
}


int
main()
{
	DemoProvider demoProvider;
	ProviderResult demoResult;

	if (demoProvider.Fetch(demoResult) != B_OK)
		return Fail("DemoProvider did not return B_OK");

	if (demoResult.Info().Title().Compare("Somewhere else") != 0)
		return Fail("DemoProvider returned an unexpected title");

	if (demoResult.Info().Source().Compare(demoProvider.Name()) != 0)
		return Fail("DemoProvider returned an unexpected source");

	if (demoResult.HasImagePath())
		return Fail("DemoProvider unexpectedly returned an image path");

	FailingProvider failingProvider;
	ProviderResult failedResult;

	if (failingProvider.Fetch(failedResult) != B_ERROR)
		return Fail("FailingProvider did not return B_ERROR");

	printf("BeMyDailyWall provider contract smoke: ok\n");
	return 0;
}
