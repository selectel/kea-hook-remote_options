// pkt4_send.cc
#include "log/macros.h"
#include "remopts_callouts.h"
#include "remopts_common.h"
#include "remopts_log.h"
#include <boost/algorithm/string.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <dhcp/option.h>
#include <dhcp/pkt4.h>
#include <exceptions/exceptions.h>
#include <hooks/hooks.h>
#include <iostream>
#include <stdbool.h>
#include <string>

using namespace std;
using namespace isc;
using namespace isc::hooks;
using namespace isc::remopts;
using namespace isc::log;
using namespace isc::dhcp;
using namespace boost::system;

namespace pt = boost::property_tree;

extern "C" {
int pkt4_send(CalloutHandle &handle) {
  Pkt4Ptr response4_ptr;
  handle.getArgument("response4", response4_ptr);

  string user_class_id;
  bool userclass_match = false;
  try {
    handle.getContext("user_class_id", user_class_id);
    if (user_class_id.compare(conf_user_class) == 0)
      userclass_match = true;
    LOG_DEBUG(remopts_logger, DBGLVL_TRACE_BASIC, REMOPTS_PARAM_VALUE)
        .arg("userclass_match: ")
        .arg(userclass_match);
  } catch (const std::exception &ex) {
    LOG_INFO(remopts_logger, REMOPTS_MSG).arg("user_class_id is missing");
  }

  string guid_id;
  bool guid_match = false;
  try {
    handle.getContext("guid_id", guid_id);
    if (boost::algorithm::contains(guid_id, conf_machine_guid))
      guid_match = true;
    LOG_DEBUG(remopts_logger, DBGLVL_TRACE_BASIC, REMOPTS_PARAM_VALUE)
        .arg("guid_match: ")
        .arg(guid_match);
  } catch (const std::exception &ex) {
    LOG_INFO(remopts_logger, REMOPTS_MSG).arg("guid_id is missing");
  }

  OptionPtr option82;
  option82 = response4_ptr->getOption(82);

  string final_url;
  if (option82 and (userclass_match or guid_match)) {
    OptionPtr option82sub1_ptr = option82->getOption(1);
    OptionPtr option82sub2_ptr = option82->getOption(2);
    OptionPtr option82sub9_ptr = option82->getOption(9);

    if (option82sub1_ptr) {
      final_url = final_url + "?sub82_1=" + gethexOptionPtr(option82sub1_ptr);
      LOG_DEBUG(remopts_logger, DBGLVL_TRACE_BASIC, REMOPTS_URL)
          .arg("final_url: " + final_url);
    }

    if (option82sub2_ptr) {
      final_url = final_url + "&sub82_2=" + gethexOptionPtr(option82sub2_ptr);
      LOG_DEBUG(remopts_logger, DBGLVL_TRACE_BASIC, REMOPTS_URL)
          .arg("final_url: " + final_url);
    }

    if (option82sub9_ptr) {
      final_url = final_url + "&sub82_9=" + gethexOptionPtr(option82sub9_ptr);
      LOG_DEBUG(remopts_logger, DBGLVL_TRACE_BASIC, REMOPTS_URL)
          .arg("final_url: " + final_url);
    }

    final_url = conf_url + final_url;
    LOG_DEBUG(remopts_logger, DBGLVL_TRACE_BASIC, REMOPTS_URL)
        .arg("final_url: " + final_url);
  } else {
    LOG_DEBUG(remopts_logger, DBGLVL_TRACE_BASIC, REMOPTS_URL)
        .arg("No option82 or user_class/guid_id match");
    return (0); // we already can't get data from opt82
  }

  // get response from curl request
  stringstream ss;
  if (make_curl_request(final_url, ss)) {
    LOG_INFO(remopts_logger, REMOPTS_MSG)
        .arg("curl OK response: \"" + sstrim(ss) + "\"");
    // parse json response
    pt::ptree root;
    try {
      pt::read_json(ss, root);
    } catch (const exception &ex) {
      LOG_ERROR(remopts_logger, REMOPTS_MSG)
          .arg("curl ERR invalid json response: \"" + sstrim(ss) + "\"");
    }

    for (auto &node : root) {
      string first_ = node.first;          // json key
      string second_ = node.second.data(); // json value
      LOG_DEBUG(remopts_logger, DBGLVL_TRACE_BASIC, REMOPTS_MSG)
          .arg("json_parser: first->" + first_ + "; second->" + second_);

      OptionPtr option = response4_ptr->getOption(stoi(first_));

      // check if option exists and update or create it
      if (option) {
        LOG_DEBUG(remopts_logger, DBGLVL_TRACE_BASIC, REMOPTS_MSG)
            .arg("Updating existing option " + first_);

        option->setData(second_.cbegin(), second_.cend());
      } else {
        LOG_DEBUG(remopts_logger, DBGLVL_TRACE_BASIC, REMOPTS_MSG)
            .arg("No preexisting option " + first_ + ", adding..");

        // create option and add it to response
        OptionBuffer buffer;
        // TODO: pack suboptions of option, now only string directly
        buffer.assign(second_.cbegin(), second_.cend());
        option.reset(new Option(Option::V4, stoi(first_), buffer));
        response4_ptr->addOption(option);
        response4_ptr->pack();
      }
      LOG_DEBUG(remopts_logger, DBGLVL_TRACE_BASIC, REMOPTS_MSG)
          .arg(response4_ptr->toText());
    }
    return (0);

  } else {
    LOG_ERROR(remopts_logger, REMOPTS_MSG)
        .arg("ERR response: " + ss.str());
    return (1);
  };
}
}
