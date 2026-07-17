#include "ProviderResolver.h"

#include "AppSettings.h"
#include "DailyImageProvider.h"
#include "DemoProvider.h"
#include "LocalFolderProvider.h"

#include <Errors.h>

#include <new>


status_t
ProviderResolver::Create(const AppSettings& settings,
	DailyImageProvider*& provider)
{
	if (provider != NULL)
		return B_BAD_VALUE;

	if (settings.ProviderName().Compare("Demo provider") == 0) {
		provider = new(std::nothrow) DemoProvider();
	} else if (settings.ProviderName().Compare("Local folder") == 0) {
		provider = new(std::nothrow) LocalFolderProvider(
			settings.LocalFolderPath().String(),
			settings.LastImagePath().String());
	} else {
		return B_NAME_NOT_FOUND;
	}

	if (provider == NULL)
		return B_NO_MEMORY;

	return B_OK;
}
