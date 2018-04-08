// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#include <stdio.h>
#include <tchar.h>

#include <windows.h>

#include <map>
#include <vector>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <mutex>
#include <condition_variable>
#include <filesystem>

#include "..\common\inc\errors.h"
#include "..\common\inc\application.h"
#include "..\common\inc\concurrentqueue.h"

#include "file_helpers.h"
#include "dicom_file.h"
#include "operation.h"
// TODO: reference additional headers your program requires here
