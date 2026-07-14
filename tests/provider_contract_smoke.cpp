#include "DailyImageProvider.h"
#include "DemoProvider.h"
#include "ProviderResult.h"

#include <Errors.h>

#include <stdio.h>
#include <string.h>


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


static bool
IsNeutralProviderResult(const ProviderResult& result)
{
	return result.Info().Title().Length() == 0
		&& result.Info().Description().Length() == 0
		&& result.Info().Source().Length() == 0
		&& result.Info().Copyright().Length() == 0
		&& result.Info().Date().Length() == 0
		&& result.ImagePath().Length() == 0
		&& !result.HasImagePath();
}


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

	if (!IsNeutralProviderResult(demoResult))
		return Fail("DemoProvider result was not neutral before Fetch");

	if (demoProvider.Fetch(demoResult) != B_OK)
		return Fail("DemoProvider did not return B_OK");

	if (demoProvider.Name() == NULL
		|| strcmp(demoProvider.Name(), "Demo provider") != 0) {
		return Fail("DemoProvider returned an unexpected stable name");
	}

	if (demoResult.Info().Title().Compare("Somewhere else") != 0)
		return Fail("DemoProvider returned an unexpected title");

	if (demoResult.Info().Description().Compare(
		"Your daily window to somewhere else.") != 0) {
		return Fail("DemoProvider returned an unexpected description");
	}

	if (demoResult.Info().Source().Compare(demoProvider.Name()) != 0)
		return Fail("DemoProvider returned an unexpected source");

	if (demoResult.Info().Copyright().Compare(
		"Not an affiliated provider.") != 0) {
		return Fail("DemoProvider returned an unexpected attribution");
	}

	if (demoResult.HasImagePath())
		return Fail("DemoProvider unexpectedly returned an image path");

	FailingProvider failingProvider;
	ProviderResult failedResult;

	if (!IsNeutralProviderResult(failedResult))
		return Fail("FailingProvider result was not neutral before Fetch");

	if (failingProvider.Fetch(failedResult) != B_ERROR)
		return Fail("FailingProvider did not return B_ERROR");

	printf("BeMyDailyWall provider contract smoke: ok\n");
	return 0;
}
