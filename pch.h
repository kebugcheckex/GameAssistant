#pragma once

#include <codecvt>
#include <iostream>
#include <future>
#include <sstream>

#include <fmt/core.h>

#include <Windows.h>

#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Storage.h>
#include <winrt/Windows.Graphics.Capture.h>
#include <windows.graphics.capture.interop.h>
#include <windows.graphics.capture.h>

#include <wil/resource.h>

#include <gflags/gflags.h>

#define GLOG_NO_ABBREVIATED_SEVERITIES
#include <glog/logging.h>

#include "robmikh.common/d3dHelpers.h"
#include "robmikh.common/d3dHelpers.desktop.h"
#include "robmikh.common/direct3d11.interop.h"
#include "robmikh.common/capture.desktop.interop.h"
