#include "data_access.h"

#include <src/database/dev/database_development.h>
#include <src/database/production/database_production.h>
#include <config.h>


data_access::data_access()
{
}

data_access * data_access::get_default()
{
	static data_access* singleton = nullptr;
	if (!singleton) {
#if TEST_DATABASE
		singleton = new database_development();
#else
		singleton = new database_production();
#endif
	}
	return singleton;
}


