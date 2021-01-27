#pragma once

#ifdef _MSC_VER
#define TEMPLATE ->
#define TEMPLATEV .
#else
#define TEMPLATE ->template
#define TEMPLATEV .template
#endif
