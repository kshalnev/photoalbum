#pragma once

#ifndef WINVER
#define WINVER 0x0600
#endif

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0600
#endif

#ifndef _WIN32_WINDOWS
#define _WIN32_WINDOWS 0x0410
#endif

#ifndef _WIN32_IE
#define _WIN32_IE 0x0700
#endif

#define WIN32_LEAN_AND_MEAN

#include <windows.h>

#include <objbase.h>

#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

#include <gdiplus.h>
#pragma comment(lib, "gdiplus.lib")

#include <string>
#include <list>
#include <memory>

using namespace std;
using namespace Gdiplus;

#ifndef ASSERT
#include <crtdbg.h>
#define ASSERT _ASSERTE
#endif

#include <atlbase.h>
#include <atlwin.h>
