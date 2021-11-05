// remopts_common.h

#include <dhcp/option.h>
#include <dhcp/pkt4.h>
#include <iostream>
#include <string>

namespace isc {
namespace remopts {

std::string gethexOptionPtr(const isc::dhcp::OptionPtr opt);
std::string unhexOptionPtr(const isc::dhcp::OptionPtr opt);
std::string sstrim(const std::stringstream &ss);
bool make_curl_request(const std::string &request, std::stringstream &ss);

} // namespace remopts
} // namespace isc

extern "C" {}
