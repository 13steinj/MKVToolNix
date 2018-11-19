/*
   mkvmerge -- utility for splicing together matroska files
   from component media subtypes

   Distributed under the GPL v2
   see the file COPYING for details
   or visit http://www.gnu.org/copyleft/gpl.html

   class definition for memory handling classes

   Written by Moritz Bunkus <moritz@bunkus.org>.
*/

#pragma once

#include "common/common_pch.h"

#include "common/error.h"

namespace mtx {
  namespace mem {
    class exception: public mtx::exception {
    public:
      virtual const char *what() const throw() {
        return "unspecified memory error";
      }
    };

    class lacing_x: public exception {
    protected:
      std::string m_message;
    public:
      lacing_x(const std::string &message) : m_message{message} { }
      virtual ~lacing_x() throw() { }

      virtual const char *what() const throw() {
        return m_message.c_str();
      }
    };
  }
}

inline void
safefree(void *p) {
  if (p)
    free(p);
}

#define safemalloc(s) _safemalloc(s, __FILE__, __LINE__)
unsigned char *_safemalloc(size_t size, const char *file, int line);

#define safememdup(src, size) _safememdup(src, size, __FILE__, __LINE__)
unsigned char *_safememdup(const void *src, size_t size, const char *file, int line);

#define safestrdup(s) _safestrdup(s, __FILE__, __LINE__)
inline char *
_safestrdup(const std::string &s,
            const char *file,
            int line) {
  return reinterpret_cast<char *>(_safememdup(s.c_str(), s.length() + 1, file, line));
}
inline unsigned char *
_safestrdup(const unsigned char *s,
            const char *file,
            int line) {
  return _safememdup(s, strlen(reinterpret_cast<const char *>(s)) + 1, file, line);
}
inline char *
_safestrdup(const char *s,
            const char *file,
            int line) {
  return reinterpret_cast<char *>(_safememdup(s, strlen(s) + 1, file, line));
}

#define saferealloc(mem, size) _saferealloc(mem, size, __FILE__, __LINE__)
unsigned char *_saferealloc(void *mem, size_t size, const char *file, int line);

class memory_c;
using memory_cptr = std::shared_ptr<memory_c>;
using memories_c  = std::vector<memory_cptr>;

class memory_c {
private:
  unsigned char *m_ptr{};
  std::size_t m_size{}, m_offset{};
  bool m_is_owned{};

  explicit memory_c(void *ptr,
                    std::size_t size,
                    bool take_ownership) // allocate a new counter
    : m_ptr{static_cast<unsigned char *>(ptr)}
    , m_size{size}
    , m_is_owned{take_ownership}
  {
  }

public:
  memory_c() {}

  ~memory_c() {
    if (m_is_owned && m_ptr)
      free(m_ptr);
  }

  memory_c(const memory_c &r) = delete;
  memory_c &operator=(const memory_c &r) = delete;

  unsigned char *get_buffer() const {
    return m_ptr ? m_ptr + m_offset : nullptr;
  }

  size_t get_size() const {
    return m_size >= m_offset ? m_size - m_offset: 0;
  }

  void set_size(std::size_t new_size) {
    m_size = new_size;
  }

  void set_offset(std::size_t new_offset) {
    if (new_offset > m_size)
      throw false;
    m_offset = new_offset;
  }

  bool is_allocated() const throw() {
    return m_ptr;
  }

  memory_cptr clone() const {
    return memory_c::clone(get_buffer(), get_size());
  }

  bool is_owned() const {
    return m_is_owned;
  }

  void take_ownership() {
    if (m_is_owned)
      return;

    m_ptr       = static_cast<unsigned char *>(safememdup(get_buffer(), get_size()));
    m_is_owned  = true;
    m_size     -= m_offset;
    m_offset    = 0;
  }

  void lock() {
    m_is_owned = false;
  }

  void resize(std::size_t new_size) throw();
  void add(unsigned char const *new_buffer, std::size_t new_size);
  void add(memory_c const &new_buffer) {
    add(new_buffer.get_buffer(), new_buffer.get_size());
  }
  void add(memory_cptr const &new_buffer) {
    add(*new_buffer);
  }

  void prepend(unsigned char const *new_buffer, std::size_t new_size);
  void prepend(memory_c const &new_buffer) {
    prepend(new_buffer.get_buffer(), new_buffer.get_size());
  }
  void prepend(memory_cptr const &new_buffer) {
    prepend(*new_buffer);
  }

  std::string to_string() const {
    if (!is_allocated() || !get_size())
      return {};
    return { reinterpret_cast<char const *>(get_buffer()), static_cast<std::string::size_type>(get_size()) };
  }

public:
  static inline memory_cptr
  take_ownership(void *buffer, std::size_t length) {
    return memory_cptr{ new memory_c(reinterpret_cast<unsigned char *>(buffer), length, true) };
  }

  static inline memory_cptr
  borrow(void *buffer, std::size_t length) {
    return memory_cptr{ new memory_c(reinterpret_cast<unsigned char *>(buffer), length, false) };
  }

  static inline memory_cptr
  borrow(std::string &buffer) {
    return borrow(&buffer[0], buffer.length());
  }

  static memory_cptr
  alloc(std::size_t size) {
    return take_ownership(safemalloc(size), size);
  };

  static inline memory_cptr
  clone(const void *buffer,
        std::size_t size) {
    return take_ownership(safememdup(buffer, size), size);
  }

  static inline memory_cptr
  clone(std::string const &buffer) {
    return clone(buffer.c_str(), buffer.length());
  }

  static memory_c & splice(memory_c &buffer, std::size_t offset, std::size_t to_remove, boost::optional<memory_c const &> to_insert = boost::none);
};

inline bool
operator ==(memory_c const &a,
            memory_c const &b) {
  return (a.get_size() == b.get_size())
      && ((a.get_buffer() && b.get_buffer()) || (!a.get_buffer() && !b.get_buffer()))
      && (a.get_buffer() ? !std::memcmp(a.get_buffer(), b.get_buffer(), a.get_size()) : true);
}

inline bool
operator !=(memory_c const &a,
            memory_c const &b) {
  return !(a == b);
}

inline bool
operator ==(memory_c const &a,
            char const *b) {
  if (!a.get_buffer() && !b)
    return true;

  if (!a.get_buffer() || !b)
    return false;

  return (a.get_size() == std::strlen(b))
    && !std::memcmp(a.get_buffer(), b, a.get_size());
}

inline bool
operator !=(memory_c const &a,
            char const *b) {
  return !(a == b);
}

memory_cptr lace_memory_xiph(const std::vector<memory_cptr> &blocks);
std::vector<memory_cptr> unlace_memory_xiph(memory_cptr &buffer);
