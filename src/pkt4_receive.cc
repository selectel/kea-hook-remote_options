// pkg4_receive.cc
#include "remopts_callouts.h"
#include "remopts_common.h"
#include "remopts_log.h"
#include <cc/data.h>
#include <config.h>
#include <dhcp/dhcp4.h>
#include <dhcp/pkt4.h>
#include <exceptions/exceptions.h>
#include <hooks/hooks.h>
#include <string>

using namespace std;
using namespace isc;
using namespace isc::hooks;
using namespace isc::remopts;
using namespace isc::log;
using namespace isc::dhcp;

extern "C" {

int pkt4_receive(CalloutHandle &handle) {
  try {
    Pkt4Ptr query4_ptr;
    handle.getArgument("query4", query4_ptr);

    OptionPtr user_class_ptr;
    OptionPtr uuid_guid_ptr;
    user_class_ptr = query4_ptr->getOption(77);
    uuid_guid_ptr = query4_ptr->getOption(97);

    if (user_class_ptr) {
      string user_class_id;
      user_class_id = unhexOptionPtr(user_class_ptr);
      LOG_DEBUG(remopts_logger, DBGLVL_TRACE_BASIC, REMOPTS_PKT4_RCV)
          .arg("user_class_id")
          .arg(user_class_id);

      handle.setContext("user_class_id", user_class_id);

    } else {
      LOG_DEBUG(remopts_logger, DBGLVL_TRACE_BASIC, REMOPTS_PKT4_RCV)
          .arg("user_class_id")
          .arg("no user-class");
    }

    if (uuid_guid_ptr) {
      string guid_id;

      // very dirty hack
      // machine_id match substring should be coded in hex
      string guid_str = gethexOptionPtr(uuid_guid_ptr);
      LOG_DEBUG(remopts_logger, DBGLVL_TRACE_BASIC, REMOPTS_PKT4_RCV)
          .arg("guid_str")
          .arg(guid_str);

      handle.setContext("guid_id", guid_str);

    } else {
      LOG_DEBUG(remopts_logger, DBGLVL_TRACE_BASIC, REMOPTS_PKT4_RCV)
          .arg("uuid_guid_ptr")
          .arg("no GUID (Client Machine Identifier)");
    }

    return (0);

  } catch (const exception &ex) {
    LOG_DEBUG(remopts_logger, DBGLVL_TRACE_BASIC, REMOPTS_PKT4_RCV)
        .arg("exception")
        .arg(ex.what());
    return (1);
  }
}
}
