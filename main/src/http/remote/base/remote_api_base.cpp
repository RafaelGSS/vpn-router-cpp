#include "remote_api_base.h"
#include <src/http/remote/dev/local_api.h>
#include <src/http/remote/production/remote_api.h>
#include <config.h>

remote_api_base::remote_api_base()
{
}

remote_api_base * remote_api_base::get_default()
{
	static remote_api_base* singleton = nullptr;
	if (!singleton) {
#if TEST_API
		singleton = new local_api();
#else
		singleton = new remote_api();
#endif
	}
	return singleton;
}

