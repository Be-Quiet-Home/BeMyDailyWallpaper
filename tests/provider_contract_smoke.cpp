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
	ProviderResult mutableResult;
	mutableResult.SetImagePath("/boot/home/test-wallpaper.jpg");

	if (mutableResult.ImagePath().Compare(
		"/boot/home/test-wallpaper.jpg") != 0) {
		return Fail("SetImagePath did not preserve the image path");
	}

	if (!mutableResult.HasImagePath())
		return Fail("SetImagePath did not report an image path");

	mutableResult.SetImagePath("");

	if (mutableResult.ImagePath().Length() != 0)
		return Fail("SetImagePath did not clear the image path");

	if (mutableResult.HasImagePath())
		return Fail("cleared image path was still reported");

	WallpaperInfo sourceInfo(
		"Original title",
		"Original description.",
		"Original source",
		"Original attribution.",
		"2026-07-14");
	ProviderResult infoResult;
	infoResult.SetInfo(sourceInfo);

	sourceInfo = WallpaperInfo(
		"Changed title",
		"Changed description.",
		"Changed source",
		"Changed attribution.",
		"2099-12-31");

	if (infoResult.Info().Title().Compare("Original title") != 0)
		return Fail("SetInfo did not preserve the title");

	if (infoResult.Info().Description().Compare(
		"Original description.") != 0) {
		return Fail("SetInfo did not preserve the description");
	}

	if (infoResult.Info().Source().Compare("Original source") != 0)
		return Fail("SetInfo did not preserve the source");

	if (infoResult.Info().Copyright().Compare(
		"Original attribution.") != 0) {
		return Fail("SetInfo did not preserve the attribution");
	}

	if (infoResult.Info().Date().Compare("2026-07-14") != 0)
		return Fail("SetInfo did not preserve the date");

	WallpaperInfo replacementInfo(
		"Replacement title",
		"Replacement description.",
		"Replacement source",
		"Replacement attribution.",
		"2027-01-01");
	infoResult.SetInfo(replacementInfo);

	if (infoResult.Info().Title().Compare("Replacement title") != 0)
		return Fail("SetInfo did not replace the title");

	if (infoResult.Info().Description().Compare(
		"Replacement description.") != 0) {
		return Fail("SetInfo did not replace the description");
	}

	if (infoResult.Info().Source().Compare("Replacement source") != 0)
		return Fail("SetInfo did not replace the source");

	if (infoResult.Info().Copyright().Compare(
		"Replacement attribution.") != 0) {
		return Fail("SetInfo did not replace the attribution");
	}

	if (infoResult.Info().Date().Compare("2027-01-01") != 0)
		return Fail("SetInfo did not replace the date");

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

	if (demoResult.Info().Date().Length() != 0)
		return Fail("DemoProvider returned an unexpected date");

	if (demoResult.ImagePath().Length() != 0)
		return Fail("DemoProvider returned a non-empty image path");

	if (demoResult.HasImagePath())
		return Fail("DemoProvider reported an image path");

	FailingProvider failingProvider;
	ProviderResult failedResult;

	if (!IsNeutralProviderResult(failedResult))
		return Fail("FailingProvider result was not neutral before Fetch");

	if (failingProvider.Fetch(failedResult) != B_ERROR)
		return Fail("FailingProvider did not return B_ERROR");

	printf("BeMyDailyWall provider contract smoke: ok\n");
	return 0;
}
