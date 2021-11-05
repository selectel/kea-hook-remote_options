// remopts_common.cc
#include "remopts_common.h"
#include "remopts_log.h"
#include "remopts_messages.h"
#include <boost/algorithm/hex.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/trim_all.hpp>
#include <curl/curl.h>
#include <stdbool.h>

using namespace isc::log;

namespace isc {
namespace remopts {

std::string gethexOptionPtr(const isc::dhcp::OptionPtr opt) {
  std::string hex_str = opt->toHexString();
  boost::algorithm::erase_head(hex_str, 2); // get rid of '0x' header

  return hex_str;
}

std::string unhexOptionPtr(const isc::dhcp::OptionPtr opt) {
  std::string hex_str = gethexOptionPtr(opt);
  std::string str = boost::algorithm::unhex(hex_str);

  return str;
}

std::string sstrim(const std::stringstream &ss) {
  std::string s = ss.str();
  boost::algorithm::erase_all(s, "\n");
  boost::algorithm::trim_all(s);
  return s;
}

bool make_curl_request(const std::string &request, std::stringstream &ss) {
  // Variables related to curl (there are "C" variables)
  CURL *curl;
  CURLcode res;
  struct curl_slist *list = NULL;
  int curl_opt_res = 0;

  // "C" varaibles.
  char *bp;
  size_t size;
  FILE *response_memfile;

  curl_global_init(CURL_GLOBAL_DEFAULT);
  curl = curl_easy_init();
  if (!curl) {
    curl_global_cleanup();
    LOG_ERROR(remopts_logger, REMOPTS_CURL)
        .arg("Could not initialize curl");
    return false;
  }

  list = curl_slist_append(list, "Accept: application/json");
  if (list == NULL) {
    curl_easy_cleanup(curl);
    curl_global_cleanup();
    LOG_ERROR(remopts_logger, REMOPTS_CURL)
        .arg("Could not create curl slist.");
    return false;
  }

  response_memfile = open_memstream(&bp, &size);
  if (response_memfile == NULL) {
    curl_easy_cleanup(curl);
    curl_global_cleanup();
    LOG_ERROR(remopts_logger, REMOPTS_CURL)
        .arg("Could not create memfile.");
    return false;
  }

  // If we don't set a timeout, curl will try for 300 seconds by default.
  curl_opt_res += curl_easy_setopt(curl, CURLOPT_TIMEOUT, 1L);
  curl_opt_res += curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 1L);
  // libcurl's docs say to cast as void, don't blame me.
  curl_opt_res +=
      curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)response_memfile);
  // CURLOPT_URL takes a char*
  curl_opt_res += curl_easy_setopt(curl, CURLOPT_URL, (request).c_str());
  curl_opt_res += curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);

#ifdef SKIP_PEER_VERIFICATION
  curl_opt_res += curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
#endif

#ifdef SKIP_HOSTNAME_VERIFICATION
  curl_opt_res += curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
#endif

  if (curl_opt_res > 0) {
    fclose(response_memfile);
    free(bp);
    curl_easy_cleanup(curl);
    curl_global_cleanup();
    LOG_ERROR(remopts_logger, REMOPTS_CURL)
        .arg("Error setting curl options.");
    return false;
  }

  res = curl_easy_perform(curl);
  // Check for errors
  if (res != CURLE_OK) {
    fclose(response_memfile);
    free(bp);
    curl_easy_cleanup(curl);
    curl_global_cleanup();
    return false;
  }

  // make bp available for reading.
  fclose(response_memfile);
  curl_easy_cleanup(curl);
  curl_global_cleanup();
  ss << bp;
  free(bp);
  return true;
}

} // namespace remopts
} // namespace isc
