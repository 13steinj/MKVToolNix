/*
   mkvmerge -- utility for splicing together matroska files
   from component media subtypes

   Distributed under the GPL
   see the file COPYING for details
   or visit http://www.gnu.org/copyleft/gpl.html

   OS dependant file system & system helper functions

   Written by Moritz Bunkus <moritz@bunkus.org>
*/

#include "common/common.h"

#include "common/error.h"
#include "common/fs_sys_helpers.h"
#include "common/strings/editing.h"
#include "common/strings/utf8.h"

#if defined(SYS_WINDOWS)

# include <io.h>
# include <windows.h>
# include <winreg.h>
# include <direct.h>
# include <shlobj.h>
# include <sys/timeb.h>

HANDLE
CreateFileUtf8(LPCSTR lpFileName,
               DWORD dwDesiredAccess,
               DWORD dwShareMode,
               LPSECURITY_ATTRIBUTES lpSecurityAttributes,
               DWORD dwCreationDisposition,
               DWORD dwFlagsAndAttributes,
               HANDLE hTemplateFile) {
  // convert the name to wide chars
  wchar_t *wbuffer = win32_utf8_to_utf16(lpFileName);
  HANDLE ret       = CreateFileW(wbuffer, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
  delete []wbuffer;

  return ret;
}

void
create_directory(const char *path) {
  wchar_t *wbuffer = win32_utf8_to_utf16(path);
  int result       = _wmkdir(wbuffer);
  delete []wbuffer;

  if (0 != result)
    throw error_c(boost::format(Y("mkdir(%1%) failed; errno = %2% (%3%)")) % path % errno % strerror(errno));
}

int
fs_entry_exists(const char *path) {
  struct _stat s;

  wchar_t *wbuffer = win32_utf8_to_utf16(path);
  int result       = _wstat(wbuffer, &s);
  delete []wbuffer;

  return 0 == result;
}

int64_t
get_current_time_millis() {
  struct _timeb tb;
  _ftime(&tb);

  return (int64_t)tb.time * 1000 + tb.millitm;
}

bool
get_registry_key_value(const std::string &key,
                       const std::string &value_name,
                       std::string &value) {
  std::vector<std::string> key_parts = split(key, "\\", 2);
  HKEY hkey;
  HKEY hkey_base = key_parts[0] == "HKEY_CURRENT_USER" ? HKEY_CURRENT_USER : HKEY_LOCAL_MACHINE;
  DWORD error    = RegOpenKeyExA(hkey_base, key_parts[1].c_str(), 0, KEY_READ, &hkey);

  if (ERROR_SUCCESS != error)
    return false;

  bool ok        = false;
  DWORD data_len = 0;
  DWORD dwDisp;
  if (ERROR_SUCCESS == RegQueryValueExA(hkey, value_name.c_str(), NULL, &dwDisp, NULL, &data_len)) {
    char *data = new char[data_len + 1];
    memset(data, 0, data_len + 1);

    if (ERROR_SUCCESS == RegQueryValueExA(hkey, value_name.c_str(), NULL, &dwDisp, (BYTE *)data, &data_len)) {
      value = data;
      ok    = true;
    }

    delete []data;
  }

  RegCloseKey(hkey);

  return ok;
}

std::string
get_current_exe_path() {
  char file_name[4000];
  memset(file_name, 0, sizeof(file_name));
  if (!GetModuleFileNameA(NULL, file_name, 3999))
    return "";

  std::string path           = file_name;
  std::string::size_type pos = path.rfind('\\');
  if (std::string::npos == pos)
    return "";

  path.erase(pos);

  return path;
}

std::string
get_installation_path() {
  std::string path;

  if (get_registry_key_value("HKEY_LOCAL_MACHINE\\Software\\mkvmergeGUI\\GUI", "installation_path", path) && !path.empty())
    return path;

  if (get_registry_key_value("HKEY_CURRENT_USER\\Software\\mkvmergeGUI\\GUI", "installation_path", path) && !path.empty())
    return path;

  return get_current_exe_path();
}

void
set_environment_variable(const std::string &key,
                         const std::string &value) {
  SetEnvironmentVariableA(key.c_str(), value.c_str());
  std::string env_buf = (boost::format("%1%=%2%") % key % value).str();
  _putenv(env_buf.c_str());
}

std::string
get_environment_variable(const std::string &key) {
  char buffer[100];
  memset(buffer, 0, 100);

  if (0 == GetEnvironmentVariableA(key.c_str(), buffer, 99))
    return "";

  return buffer;
}

unsigned int
get_windows_version() {
  OSVERSIONINFO os_version_info;

  memset(&os_version_info, 0, sizeof(OSVERSIONINFO));
  os_version_info.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

  if (!GetVersionEx(&os_version_info))
    return WINDOWS_VERSION_UNKNOWN;

  return (os_version_info.dwMajorVersion << 16) | os_version_info.dwMinorVersion;
}

std::string
get_application_data_folder() {
  wchar_t szPath[MAX_PATH];

  if (SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_APPDATA | CSIDL_FLAG_CREATE, NULL, 0, szPath)))
    return to_utf8(std::wstring(szPath)) + "\\mkvtoolnix";

  return "";
}

#else // SYS_WINDOWS

# include <errno.h>
# include <stdlib.h>
# include <sys/stat.h>
# include <sys/time.h>
# include <sys/types.h>
# include <unistd.h>

void
create_directory(const char *path) {
  std::string local_path = g_cc_local_utf8->native(path);
  if (0 != mkdir(local_path.c_str(), 0777))
    throw error_c(boost::format(Y("mkdir(%1%) failed; errno = %2% (%3%)")) % path % errno % strerror(errno));
}

int
fs_entry_exists(const char *path) {
  std::string local_path = g_cc_local_utf8->native(path);
  struct stat s;
  return 0 == stat(local_path.c_str(), &s);
}

int64_t
get_current_time_millis() {
  struct timeval tv;
  if (0 != gettimeofday(&tv, NULL))
    return -1;

  return (int64_t)tv.tv_sec * 1000 + (int64_t)tv.tv_usec / 1000;
}

std::string
get_application_data_folder() {
  const char *home = getenv("HOME");
  if (NULL == home)
    return "";

  return std::string(home) + "/.mkvtoolnix";
}

#endif // SYS_WINDOWS
