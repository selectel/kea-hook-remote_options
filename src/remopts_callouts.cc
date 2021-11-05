// load_unload.cc
#include "remopts_callouts.h"
#include "remopts_log.h"
#include <cc/data.h>
#include <config.h>
#include <exceptions/exceptions.h>
#include <hooks/hooks.h>

using namespace isc;
using namespace isc::hooks;
using namespace isc::log;
using namespace isc::remopts;
using namespace isc::data;

// Kea has reversed boolean values...
int KEA_SUCCESS = 0;
int KEA_FAILURE = 1;

std::string conf_url;
std::string conf_user_class;
std::string conf_machine_guid;

extern "C" {
int load(LibraryHandle &handle) {
  try {
    ConstElementPtr param_url = handle.getParameter("url");
    ConstElementPtr param_user_class = handle.getParameter("user_class");
    ConstElementPtr param_machine_guid = handle.getParameter("machine_guid");

    if (param_url) {
      if (param_url->getType() != Element::string) {
        isc_throw(BadValue, "Parameter 'url' should be a string!");
      }
      conf_url = param_url->stringValue();
      LOG_DEBUG(remopts_logger, DBGLVL_TRACE_BASIC, REMOPTS_PARAM_VALUE)
          .arg("url")
          .arg(conf_url);
    } else {
      // parameter is mandatory
      isc_throw(NotFound, "Parameter 'url' is mandatory!");
    }

    if (param_user_class) {
      if (param_user_class->getType() != Element::string) {
        isc_throw(BadValue, "Parameter 'user_class' should be a string!");
      }
      conf_user_class = param_user_class->stringValue();
      LOG_DEBUG(remopts_logger, DBGLVL_TRACE_BASIC, REMOPTS_PARAM_VALUE)
          .arg("user_class")
          .arg(conf_user_class);
    }

    if (param_machine_guid) {
      if (param_machine_guid->getType() != Element::string) {
        isc_throw(BadValue, "Parameter 'machine_guid' should be a string!");
      }
      conf_machine_guid = param_machine_guid->stringValue();
      LOG_DEBUG(remopts_logger, DBGLVL_TRACE_BASIC, REMOPTS_PARAM_VALUE)
          .arg("machine_guid")
          .arg(conf_machine_guid);
    }

    LOG_INFO(remopts_logger, REMOPTS_LOAD);
    return KEA_SUCCESS;

  } catch (const std::exception &ex) {

    LOG_ERROR(remopts_logger, REMOPTS_LOAD_FAIL).arg(ex.what());
    return KEA_FAILURE;
  }
}

int unload() {
  LOG_INFO(remopts_logger, REMOPTS_UNLOAD);
  return (0);
}

// hook doesn't support multi-threading by now
int multi_threading_compatible() {
  // 1 - compatible
  // 0 - non compatible
  return (0);
}
}
