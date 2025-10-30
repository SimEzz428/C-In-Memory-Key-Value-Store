#pragma once
#include "wal.h"
#include <string>

bool rebuild_pages(const WalFile& wal, const std::string& out_path, std::string& err);