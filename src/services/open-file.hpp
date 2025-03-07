#pragma once
#include <string>

namespace pachde {

bool openFileDialog(const std::string& folder, const std::string& filters, const std::string& filename, std::string& result);
bool saveFileDialog(const std::string& folder, const std::string& filters, const std::string& filename, std::string& result);

}
