/*
   mkvmerge -- utility for splicing together matroska files
   from component media subtypes

   Distributed under the GPL
   see the file COPYING for details
   or visit http://www.gnu.org/copyleft/gpl.html

   helper functions, common variables

   Written by Moritz Bunkus <moritz@bunkus.org>.
*/

#include "common/common_pch.h"

#include <stdlib.h>
#ifdef SYS_WINDOWS
# include <windows.h>
#endif

#include <matroska/KaxVersion.h>
#include <matroska/FileKax.h>

#include "common/mm_io.h"
#include "common/random.h"
#include "common/strings/editing.h"
#include "common/translation.h"
#include "common/xml/element_mapping.h"

#if !defined(LIBMATROSKA_VERSION) || (LIBMATROSKA_VERSION <= 0x000801)
#define matroska_init()
#define matroska_done()
#endif

// Global and static variables

int verbose = 1;

extern bool g_warning_issued;

// Functions

void
mxexit(int code) {
  matroska_done();
  if (code != -1)
    exit(code);

  if (g_warning_issued)
    exit(1);

  exit(0);
}

/** \brief Sets the priority mkvmerge runs with

   Depending on the OS different functions are used. On Unix like systems
   the process is being nice'd if priority is negative ( = less important).
   Only the super user can increase the priority, but you shouldn't do
   such work as root anyway.
   On Windows SetPriorityClass is used.

   \param priority A value between -2 (lowest priority) and 2 (highest
     priority)
 */
void
set_process_priority(int priority) {
#if defined(SYS_WINDOWS)
  static const struct {
    int priority_class, thread_priority;
  } s_priority_classes[5] = {
    { IDLE_PRIORITY_CLASS,         THREAD_PRIORITY_IDLE         },
    { BELOW_NORMAL_PRIORITY_CLASS, THREAD_PRIORITY_BELOW_NORMAL },
    { NORMAL_PRIORITY_CLASS,       THREAD_PRIORITY_NORMAL       },
    { ABOVE_NORMAL_PRIORITY_CLASS, THREAD_PRIORITY_ABOVE_NORMAL },
    { HIGH_PRIORITY_CLASS,         THREAD_PRIORITY_HIGHEST      },
  };

  SetPriorityClass(GetCurrentProcess(), s_priority_classes[priority + 2].priority_class);
  SetThreadPriority(GetCurrentThread(), s_priority_classes[priority + 2].thread_priority);

#else
  static const int s_nice_levels[5] = { 19, 2, 0, -2, -5 };

  // Avoid a compiler warning due to glibc having flagged 'nice' with
  // 'warn if return value is ignored'.
  if (!nice(s_nice_levels[priority + 2])) {
  }
#endif
}

static void
mtx_common_cleanup() {
  random_c::cleanup();
  mm_file_io_c::cleanup();
}

void
mtx_common_init() {
  matroska_init();

  atexit(mtx_common_cleanup);

  srand(time(NULL));

  init_debugging();

  init_locales();

  mm_file_io_c::setup();
  g_cc_local_utf8 = charset_converter_c::init("");
  init_cc_stdio();

  xml_element_map_init();
}

