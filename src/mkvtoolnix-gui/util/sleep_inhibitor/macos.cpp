#include "common/common_pch.h"

#include <IOKit/pwr_mgt/IOPMLib.h>

#include "mkvtoolnix-gui/util/sleep_inhibitor_p.h"
#include "mkvtoolnix-gui/util/sleep_inhibitor/macos.h"

// See https://developer.apple.com/library/archive/qa/qa1340/_index.html

namespace mtx::gui::Util {

class MacOSSleepInhibitorPrivate: public BasicSleepInhibitorPrivate {
public:
  std::optional<IOPMAssertionID> m_assertionID;
};

MacOSSleepInhibitor::MacOSSleepInhibitor()
  : BasicSleepInhibitor(*new MacOSSleepInhibitorPrivate)
{
}

MacOSSleepInhibitor::~MacOSSleepInhibitor() {
}

bool
MacOSSleepInhibitor::inhibit() {
  auto p = p_func();

  mxdebug_if(p->ms_debug, fmt::format("macOS sleep inhibitor: starting\n"));

  if (p->m_assertionID) {
    mxdebug_if(p->ms_debug, fmt::format("macOS sleep inhibitor: already inhibited\n"));
    return true;
  }

  IOPMAssertionID assertionID{};
  auto reason = CFSTR("MKVToolNix GUI job queue running");
  auto result = IOPMAssertionCreateWithName(kIOPMAssertionTypeNoDisplaySleep, kIOPMAssertionLevelOn, reason, &assertionID);

  if (result != kIOReturnSuccess) {
    mxdebug_if(p->ms_debug, fmt::format("macOS sleep inhibitor: error: IOPM assertion could not be created\n"));
    return false;
  }

  p->m_assertionID = assertionID;
  mxdebug_if(p->ms_debug, fmt::format("macOS sleep inhibitor: success: assertion ID: {0}\n", *p->m_assertionID));

  return true;
}

void
MacOSSleepInhibitor::uninhibit() {
  auto p = p_func();

  mxdebug_if(p->ms_debug, fmt::format("macOS sleep inhibitor: uninhibiting: {0}\n", p->m_assertionID ? fmt::format("assertion ID {0}", *p->m_assertionID) : "nothing to do"));

  if (!p->m_assertionID)
    return;

  IOPMAssertionRelease(*p->m_assertionID);
  p->m_assertionID.reset();
}

bool
MacOSSleepInhibitor::isInhibited()
  const {
  return !!p_func()->m_assertionID;
}

}
