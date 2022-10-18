// The MIT License (MIT)
//
// Copyright (C) 2016 hfuj13@gmail.com
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#pragma once

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/tcp.h>
#include <regex.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include <cassert>
#include <cerrno>
#include <cstring>
#include <cstdio>
#include <cstdlib>

#include <algorithm>
#include <array>
#include <chrono>
#include <exception>
#include <functional>
#include <iomanip>
#include <ios>
#include <iostream>
#include <memory>
#include <random>
#include <sstream>
#include <string>
#include <system_error>
#include <thread>
#include <tuple>
#include <type_traits>
#include <vector>

namespace lwsock {

// var string: variable name
// func_name string: function name
// param string: parameter
#define DECLARE_CALLEE(var, func_name, param) \
std::ostringstream var; \
{ var << func_name << param; }


constexpr char Version[] = "v1.4.1";
constexpr char B64chs[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
constexpr char GUID[] = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
constexpr char EOL[] = "\r\n"; // end of line
constexpr uint8_t Magic[] = {49, 49, 104, 102, 117, 106, 49, 51};

/// WebSocket(RFC6455) Opcode enum
enum class Opcode {
  CONTINUE = 0x0,
  TEXT = 0x1,
  BINARY = 0x2,
  CLOSE = 0x8,
  PING = 0x9,
  PONG = 0xa,
};

/// @brief lwsock error code
///
/// negative value. (positive value is system error)
/// @todo readjust errorcode
enum class LwsockErrc: int32_t {
  NO_ERROR = 0,
  COULD_NOT_OPEN_AVAILABLE_SOCKET = -102,
  SOCKET_CLOSED = -103,
  INVALID_HANDSHAKE = -104,
  FRAME_ERROR = -106,
  INVALID_PARAM = -107,
  INVALID_AF = -108,
  INVALID_MODE = -109,
  BAD_MESSAGE = -110,
  TIMED_OUT = -111,
};

enum class LogLevel: int32_t {
#if 0
  DEBUG = 1,
  DEBUG,
  INFO,
  WARNING,
  ERROR,
#else
    UNDER_LVL = 0,
    VERBOSE,
    DEBUG,
    INFO,
    WARNING,
    ERROR,
    SILENT,
    OVER_LVL
#endif
};

/// @brief get emum class value as int
///
/// @param [in] value
/// @retval int value
template<typename T> auto as_int(const T value) -> typename std::underlying_type<T>::type
{
  return static_cast<typename std::underlying_type<T>::type>(value);
}

/// @brief transform the adress family to string
///
/// @pram [in] af: AF_INET, AF_INET6, AF_UNSPEC
/// @retval string
inline std::string af2str(int af)
{
  std::string str;
  switch (af) {
  case AF_INET:
    str = "AF_INET";
    break;
  case AF_INET6:
    str = "AF_INET6";
    break;
  case AF_UNSPEC:
    str = "AF_UNSPEC";
    break;
  default:
    str = std::to_string(af);
    break;
  }
  return str;
}

/// @brief get now timestamp. UTC only yet
///
/// @param [in] parenthesis: ture: output with '[' and ']' <br>
//      false : output with raw
/// @retval string transformed timestamp (e.g. [2016-12-11T13:24:13.058] or 2016-12-11T13:24:13.058 etc.). the output format is like the ISO8601 (that is it include milliseconds)
/// @todo correspond TIME ZONE
inline std::string now_timestamp(bool parenthesis)
{
  std::chrono::time_point<std::chrono::system_clock> tp = std::chrono::system_clock::now();
  //std::chrono::nanoseconds nsec_since_epoch = std::chrono::duration_cast<std::chrono::nanoseconds>(tp.time_since_epoch());
  std::chrono::milliseconds msec_since_epoch = std::chrono::duration_cast<std::chrono::milliseconds>(tp.time_since_epoch());
  //std::chrono::milliseconds msec_since_epoch = std::chrono::duration_cast<std::chrono::milliseconds>(nsec_since_epoch);
  std::chrono::seconds sec = std::chrono::duration_cast<std::chrono::seconds>(msec_since_epoch);

  std::time_t tt = sec.count();
  std::size_t msec = msec_since_epoch.count() % 1000;
  //std::size_t nsec = nsec_since_epoch.count() % (1000 * 1000); 

  struct tm stm = {0};
  tzset();
  gmtime_r(&tt, &stm);

  std::ostringstream oss;

  oss << (stm.tm_year+1900) << '-'
      << std::setw(2) << std::setfill('0') << (stm.tm_mon+1) << '-'
      << std::setw(2) << std::setfill('0') << stm.tm_mday
      << 'T'
      << std::setw(2) << std::setfill('0') << stm.tm_hour
      << ':'
      << std::setw(2) << std::setfill('0') << stm.tm_min
      << ':'
      << std::setw(2) << std::setfill('0') << stm.tm_sec
      << '.' << std::setw(3) << std::setfill('0') << msec
      //<< std::setw(3) << std::setfill('0') << nsec
      ;

  std::string str;
  if (parenthesis) {
    str += "[" + oss.str() + "]";
  }
  else {
    str += oss.str();
  }

  return str;
}

/// @brief get timestamp by cloed parenthesis
///
/// @retval string transformed timestamp (e.g. [2016-12-11T13:24:13.058])
inline std::string now_timestamp()
{
  return now_timestamp(true);
}

/// @brief trim specified characters
///
/// @param [in] str: string
/// @param [in] charset: character set (specified by a string) what you want to delete
/// @retval trimed string
inline std::string trim(const std::string& str, const std::string& charset)
{
  std::string::size_type p0 = str.find_first_not_of(charset);
  if (p0 == std::string::npos) {
    p0 = 0;
  }
  std::string::size_type p1 = str.find_last_not_of(charset);

  std::string result = str.substr(p0, p1 - p0 + 1);
  return result;
}

/// @brief trim white spaces
///
/// @param [in] str: string
/// @retval trimed string
inline std::string trim(const std::string& str)
{
  return trim(str, " \t\v\r\n");
}

/// @brief transform the string to the lowercase string
///
/// @param [in] str: string
/// @retval lower case string
inline std::string str2lower(const std::string& str)
{
  std::string result;
  std::transform(std::begin(str), std::end(str), std::back_inserter(result), ::tolower);
  return result;
}

// a logger
class alog final {
public:
  class scoped final {
  public:
    scoped() = delete;
    scoped(const scoped&) = default;
    scoped(scoped&&) = default;

    scoped(const std::string& str)
    : scoped(LogLevel::DEBUG, str) {}
    scoped(LogLevel loglevel, const std::string& str)
    : loglevel_(loglevel), oss_(str)
    {
      log_(loglevel_) << "[[[[ " << oss_.str() << std::endl;
    }
    ~scoped()
    {
      log_(loglevel_) << "]]]] " << oss_.str() << std::endl;
    }
    std::string str()
    {
      return oss_.str();
    }
    scoped& clear()
    {
      oss_.str("");
      return *this;
    }
    template<typename T> friend std::ostream& operator<<(scoped& slog, const T& rhs);

  private:
    alog& log_ = alog::get_instance();
    LogLevel loglevel_ = LogLevel::DEBUG;
    std::ostringstream oss_;
  };

  static alog& get_instance()
  {
    return get_instance(nullptr);
  }
  static alog& get_instance(std::ostream& ost)
  {
    return get_instance(&ost);
  }

  bool operator==(const alog& rhs) const
  {
    return &rhs == this || (rhs.level_ == level_ && rhs.ost_ == ost_);
  }
  bool operator!=(const alog& rhs) const
  {
    return (rhs.level_ != level_ || rhs.ost_ != ost_);
  }
  template<typename... Args> static std::string format(const std::string& fmt, Args... args)
  {
    std::string buff;
    int ret = snprintf(&buff[0], buff.capacity(), fmt.c_str(), args...);
    if (ret >= buff.capacity()) {
      buff.reserve(ret+1);
      ret = snprintf(&buff[0], buff.capacity(), fmt.c_str(), args...);
    }
    else if (ret < 0) {
      abort();
    }
    std::string str(buff.c_str());
    return str;

  }

  // for verbose
  std::ostream& v()
  {
    return (*this)(LogLevel::VERBOSE) << "[V]";
  }
  template<typename... Args> std::ostream& v(const std::string& fmt, Args... args)
  {
    return v() << format(fmt, args...) << std::flush;
  }

  // for debug
  std::ostream& d()
  {
    return (*this)(LogLevel::DEBUG) << "[D]";
  }
  template<typename... Args> std::ostream& d(const std::string& fmt, Args... args)
  {
    return d() << format(fmt, args...) << std::flush;
  }

  // for info
  std::ostream& i()
  {
    return (*this)(LogLevel::INFO) << "[I]";
  }
  template<typename... Args> std::ostream& i(const std::string& fmt, Args... args)
  {
    return i() << format(fmt, args...) << std::flush;
  }

  // for warning
  std::ostream& w()
  {
    return (*this)(LogLevel::WARNING) << "[W]";
  }
  template<typename... Args> std::ostream& w(const std::string& fmt, Args... args)
  {
    return w() << format(fmt, args...) << std::flush;
  }

  // for error
  std::ostream& e()
  {
    return (*this)(LogLevel::ERROR) << "[E]";
  }
  template<typename... Args> std::ostream& e(const std::string& fmt, Args... args)
  {
    return e() << format(fmt, args...) << std::flush;
  }

  // ログレベル設定が何であっても強制的に出力する
  std::ostream& force()
  {
    return (*this)();
  }
  template<typename... Args> std::ostream& force(const std::string& fmt, Args... args)
  {
    return force() << format(fmt, args...) << std::flush;
  }

  template<typename T> friend std::ostream& operator<<(alog& log, const T& rhs);

  alog& level(LogLevel lvl)
  {
    assert(LogLevel::UNDER_LVL < lvl && lvl < LogLevel::OVER_LVL);
    if (lvl <= LogLevel::UNDER_LVL || LogLevel::OVER_LVL <= lvl) {
      abort();
    }

    level_ = lvl;
    return *this;
  }
  LogLevel level()
  {
    return level_;
  }

  alog& ostream(std::ostream& ost)
  {
    ost_ = &ost;
    return *this;
  }

private:
  alog() = default;
  alog& operator=(const alog&) = delete;

  std::ostream& output()
  {
    return (*ost_) << now_timestamp() << "[thd:" << std::this_thread::get_id() << "] ";
  }

  std::ostream& operator()()
  {
    return output();
  }
  std::ostream& operator()(LogLevel lvl)
  {
    return lvl >= level_ ? output() : null_ost_;
  }

  static alog& get_instance(std::ostream* ost)
  {
    static alog log;
    if (ost != nullptr) {
      log.ost_ = ost;
    }
    return log;
  }

  std::ostream null_ost_{nullptr}; // /dev/null like ostream
  LogLevel level_ = LogLevel::SILENT;
  std::ostream* ost_ = &null_ost_;
};

template<typename T> std::ostream& operator<<(alog& log, const T& rhs)
{
  return (*log.ost_) << rhs;
}

template<typename T> std::ostream& operator<<(alog::scoped& slog, const T& rhs)
{
  return slog.oss_ << rhs;
}

/// @brief  Error class
///
/// this class contains errno(3), getaddrinfo(3)'s error, regcom(3)'s error or lwsock Errcode.
class Error final {
public:
  Error() = delete;
  Error(const Error&) = default;
  Error(Error&&) = default;
  Error(int errcode, uint32_t line, const std::string& what_arg)
  : errcode_(errcode), line_(line)
  {
    std::ostringstream oss;
    if (line_ > 0) {
      oss << "line:" << line_ << ". " << "errcode=" << errcode_ << ". " << what_arg;
      what_ = oss.str();
    }
    else {
      oss << "errcode=" << errcode_ << ". " << what_arg;
      what_ = oss.str();
    }
    alog& log = alog::get_instance();
    log.e() << what_ << std::endl;
  }
  Error(int errcode, uint32_t line)
  : Error(errcode, line, "")
  { }
  Error(int errcode, const std::string& what_arg)
  : Error(errcode, 0, what_arg)
  { }
  explicit Error(int errcode)
  : Error(errcode, 0, "")
  { }

  ~Error() = default;

  /// @brief get error code
  ///
  /// @retval error code. errno(3), getaddrinfo(3), regcomp(3), lwsock Errcode.
  int code()
  {
    return errcode_;
  }

  void prefix(const std::string& prefix)
  {
    what_ = prefix + what_;
  }

  /// @brief get reason string
  ///
  /// @retval reason string
  const char* what() const
  {
    return what_.c_str();
  }
private:
  int errcode_ = 0;
  uint32_t line_ = 0; // the line when the error occurred
  std::string what_;
};

class CRegexException final: public std::exception {
public:
  CRegexException() = delete;
  CRegexException(const CRegexException&) = default;
  CRegexException(CRegexException&&) = default;
  explicit CRegexException(const Error& error)
  : error_(error)
  { }

  ~CRegexException() = default;

  const char* what() const noexcept override
  {
    error_.prefix("CRegexException: ");
    return error_.what();
  }

  /// @brief get exception code (error code etc.)
  ///
  /// @retavl error code
  virtual int code()
  {
    return error_.code();
  }
private:
  mutable Error error_;
};

class GetaddrinfoException final: public std::exception {
public:
  GetaddrinfoException() = delete;
  GetaddrinfoException(const GetaddrinfoException&) = default;
  GetaddrinfoException(GetaddrinfoException&&) = default;
  explicit GetaddrinfoException(const Error& error)
  : error_(error)
  { }

  ~GetaddrinfoException() = default;

  const char* what() const noexcept override
  {
    error_.prefix("GetaddrinfoException: ");
    return error_.what();
  }

  /// @brief get exception code (error code etc.)
  ///
  /// @retavl error code
  virtual int code()
  {
    return error_.code();
  }
private:
  mutable Error error_;
};

/// @brief libray error exception class
class LwsockException final: public std::exception {
public:
  LwsockException() = delete;
  LwsockException(const LwsockException&) = default;
  LwsockException(LwsockException&&) = default;
  explicit LwsockException(const Error& error)
  : error_(error)
  { }

  ~LwsockException() = default;

  const char* what() const noexcept override
  {
    error_.prefix("LwsockException: ");
    return error_.what();
  }

  /// @brief get exception code (error code etc.)
  ///
  /// @retavl error code
  virtual int code()
  {
    return error_.code();
  }
private:
  mutable Error error_;
};

/// @brief system_error exception class. this is a wrapper class because i want to output logs.
///
class SystemErrorException final: public std::exception {
public:
  SystemErrorException() = delete;
  SystemErrorException(const SystemErrorException&) = default;
  SystemErrorException(SystemErrorException&&) = default;
  explicit SystemErrorException(const Error& error)
  : error_(error)
  { }

  ~SystemErrorException() = default;

  const char* what() const noexcept override
  {
    error_.prefix("SystemErrorException: ");
    return error_.what();
  }

  /// @brief get exception code (error code etc.)
  ///
  /// @retavl error code
  virtual int code()
  {
    return error_.code();
  }
private:
  mutable Error error_;
};

/// @brief regex(3) wrapper
///
/// regex(3) wrapper class. because the std::regex, depending on the version of android it does not work properly.
class CRegex final {
public:
  CRegex() = delete;

  /// @exception CRegexException
  CRegex(const std::string& re, size_t nmatch)
  : nmatch_(nmatch)
  {
    alog& log = alog::get_instance();
    //log(LogLevel::DEBUG) << "CRegex(re=\"" << re << "\", nmatch=" << nmatch << ')' << std::endl;
    log.d() << "CRegex(re=\"" << re << "\", nmatch=" << nmatch << ')' << std::endl;

    int err = regcomp(&regbuff_, re.c_str(), REG_EXTENDED);
    if (err != 0) {
      std::ostringstream oss;
      char errbuf[256] = {0};
      regerror(err, &regbuff_, errbuf, sizeof errbuf);
      oss << "CRegex(re=\"" << re << "\", nmatch=" << nmatch << ") " << errbuf;
      throw CRegexException(Error(err, oss.str()));
    }
  }
  ~CRegex()
  {
    regfree(&regbuff_);
  }

  /// @brief execute regex
  ///
  /// @param [in] src: string for regex
  /// @retval matched string set. if empty then no matched
  std::vector<std::string> exec(const std::string& src)
  {
    std::vector<std::string> matched;
    std::vector<regmatch_t> match(nmatch_, {-1, -1});
    int err = regexec(&regbuff_, src.c_str(), match.size(), &match[0], 0);
    if (err != 0) {
      return matched;
    }
    for (auto& elm : match) {
      int start = elm.rm_so;
      int end = elm.rm_eo;
      if (start == -1 || end == -1) {
        continue;
      }
      std::string str(std::begin(src)+start, std::begin(src)+end);
      matched.push_back(str);
    }
    return matched;
  }
private:
  regex_t regbuff_{};
  size_t nmatch_ = 0;
};

/// @brief base64 encoder
///
/// This referred the https://opensource.apple.com/source/QuickTimeStreamingServer/QuickTimeStreamingServer-452/CommonUtilitiesLib/base64.c
/// @param [in] src_data: array or vector
/// @param [in] src_data_sz: *src_data size. bytes
/// @retval base64 encoded string
inline std::string b64encode(const void* src_data, int src_data_sz)
{
  assert(src_data_sz >= 0);
  std::string dst;
  const uint8_t* src = static_cast<const uint8_t*>(src_data);
  int idx = 0;

  for (; idx < src_data_sz - 2; idx += 3) {
    dst += B64chs[(src[idx] >> 2) & 0x3F];
    dst += B64chs[((src[idx] & 0x3) << 4) | ((src[idx + 1] & 0xF0) >> 4)];
    dst += B64chs[((src[idx + 1] & 0xF) << 2) | ((src[idx + 2] & 0xC0) >> 6)];
    dst += B64chs[src[idx + 2] & 0x3F];
  }
  if (idx < src_data_sz) {
    dst += B64chs[(src[idx] >> 2) & 0x3F];
    if (idx == (src_data_sz - 1)) {
        dst += B64chs[((src[idx] & 0x3) << 4)];
        dst += '=';
    }
    else {
        dst += B64chs[((src[idx] & 0x3) << 4) | ((src[idx + 1] & 0xF0) >> 4)];
        dst += B64chs[((src[idx + 1] & 0xF) << 2)];
    }
    dst += '=';
  }

  return dst;
}

/// @brief base64 decoder
///
/// @param [in] src: base64 encoded string
/// @retval base64 decoded data
/// @exception LwsockException
inline std::vector<uint8_t> b64decode(const std::string& src)
{
  DECLARE_CALLEE(callee, __func__, "(src=\"" << src << "\")");

  if (src.size() % 4 != 0) {
    int err = as_int(LwsockErrc::INVALID_PARAM);
    std::ostringstream oss;
    oss << callee.str() << " src.size()=" << src.size() << " is illegal.";
    throw LwsockException(Error(err, oss.str()));
  }
  constexpr int BLOCK_SZ = 4;
  std::vector<uint8_t> dst;
  for (size_t i = 0; i < src.size(); i += BLOCK_SZ) {
    const char* ptr = &src[i];
    std::array<uint8_t, 3> tmp;
    uint8_t value[BLOCK_SZ] = {0};
    int j = 0;
    for (; j < BLOCK_SZ; ++j) {
      if (std::isupper(ptr[j])) {
        value[j] = ptr[j] - 65;
      }
      else if (std::islower(ptr[j])) {
        value[j] = ptr[j] - 71;
      }
      else if (std::isdigit(ptr[j])) {
        value[j] = ptr[j] + 4;
      }
      else if (ptr[j] == '+') {
        value[j] = ptr[j] + 19;
      }
      else if (ptr[j] == '/') {
        value[j] = ptr[j] + 16;
      }
      else if (ptr[j] == '=') {
        break;
      }
      else {
        int err = as_int(LwsockErrc::INVALID_PARAM);
        std::ostringstream oss;
        char ch = ptr[j];
        oss << callee.str() << " illegal char='" << ch << '\'';
        throw LwsockException(Error(err, oss.str()));
      }
    }
    tmp[0] = value[0] << 2 | value[1] >> 4;
    tmp[1] = value[1] << 4 | value[2] >> 2;
    tmp[2] = value[2] << 6 | value[3];
    std::copy(std::begin(tmp), std::begin(tmp) + j - 1, std::back_inserter(dst));
  }
  return dst;
}

/// @brief sha1 class
///
/// This referred the RFC3174 Section 7.
class Sha1 final {
public:
  static constexpr int SHA1_HASH_SIZE = 20;
  static constexpr int MESSAGE_BLOCK_SIZE = 64; // 512-bit message blocks
  enum {
      shaSuccess = 0,
      shaNull,            /* Null pointer parameter */
      shaInputTooLong,    /* input data too long */
      shaStateError       /* called Input after Result */
  };
  Sha1() = delete;
  Sha1(const Sha1&) = delete;
  Sha1(Sha1&&) = delete;

  // This will hold context information for the SHA-1 hashing operation
  class Context_t final {
  public:
    Context_t()
    {
      Intermediate_Hash[0]   = 0x67452301;
      Intermediate_Hash[1]   = 0xEFCDAB89;
      Intermediate_Hash[2]   = 0x98BADCFE;
      Intermediate_Hash[3]   = 0x10325476;
      Intermediate_Hash[4]   = 0xC3D2E1F0;
    }
    Context_t(const Context_t&) = default;
    Context_t(Context_t&&) = default;

    ~Context_t() = default;

    Context_t& operator=(const Context_t&) = default;
    Context_t& operator=(Context_t&&) = default;

    uint32_t Intermediate_Hash[SHA1_HASH_SIZE / 4]; /* Message Digest  */
    uint32_t Length_Low = 0;  /* Message length in bits */
    uint32_t Length_High = 0; /* Message length in bits */
    int_least16_t Message_Block_Index = 0;
    uint8_t Message_Block[MESSAGE_BLOCK_SIZE]; /* 512-bit message blocks */
  };

  static int32_t Input(Context_t& dst, const void* message_array, int length)
  {
    assert(message_array != nullptr);
    assert(length >= 0);

    const uint8_t* p = static_cast<const uint8_t*>(message_array);

    for (int i = 0; length > 0; --length, ++i) {
      dst.Message_Block[dst.Message_Block_Index++] = (p[i] & 0xFF);
      dst.Length_Low += 8;
      if (dst.Length_Low == 0) {
        dst.Length_High++;
        if (dst.Length_High == 0) {
          /* Message is too long */
          return EMSGSIZE;
        }
      }

      if (dst.Message_Block_Index == MESSAGE_BLOCK_SIZE) {
          dst = SHA1ProcessMessageBlock(dst);
      }
    }

    return 0;;
  }

  static int32_t Result(uint8_t* Message_Digest, size_t sz, const Context_t& context)
  {
    assert(Message_Digest != nullptr);
    assert(sz == SHA1_HASH_SIZE);

    Context_t ctx = SHA1PadMessage(context);
    for (int i = 0; i < MESSAGE_BLOCK_SIZE; ++i) {
      /* message may be sensitive, clear it out */
      ctx.Message_Block[i] = 0;
    }

    // and clear length
    ctx.Length_Low = 0;
    ctx.Length_High = 0;

    for (size_t i = 0; i < sz; ++i) {
      Message_Digest[i] = ctx.Intermediate_Hash[i>>2] >> 8 * ( 3 - ( i & 0x03 ) );
    }

    return 0;
  }

private:
  // SHA1ProcessMessageBlock
  // Description:
  //     This function will process the next 512 bits of the message
  //     stored in the Message_Block array.
  // Comments:
  //     Many of the variable names in this code, especially the
  //     single character names, were used because those were the
  //     names used in the publication.
  static Context_t SHA1ProcessMessageBlock(const Context_t& context)
  {
    Context_t ctx(context);
    constexpr uint32_t K[] = { // Constants defined in SHA-1
      0x5A827999,
      0x6ED9EBA1,
      0x8F1BBCDC,
      0xCA62C1D6
    };
    int      t = 0; // Loop counter
    uint32_t temp = 0; // Temporary word value
    uint32_t W[80] = {0}; // Word sequence
    uint32_t A = 0, B = 0, C = 0, D = 0, E = 0; // Word buffers

    // Initialize the first 16 words in the array W
    for (t = 0; t < 16; ++t) {
        W[t] = ctx.Message_Block[t * 4] << 24;
        W[t] |= ctx.Message_Block[t * 4 + 1] << 16;
        W[t] |= ctx.Message_Block[t * 4 + 2] << 8;
        W[t] |= ctx.Message_Block[t * 4 + 3];
    }
    for (t = 16; t < 80; ++t) {
       W[t] = SHA1CircularShift(1, W[t-3] ^ W[t-8] ^ W[t-14] ^ W[t-16]);
    }
    A = ctx.Intermediate_Hash[0];
    B = ctx.Intermediate_Hash[1];
    C = ctx.Intermediate_Hash[2];
    D = ctx.Intermediate_Hash[3];
    E = ctx.Intermediate_Hash[4];
    for (t = 0; t < 20; ++t) {
        temp =  SHA1CircularShift(5, A) + ((B & C) | ((~B) & D)) + E + W[t] + K[0];
        E = D;
        D = C;
        C = SHA1CircularShift(30,B);
        B = A;
        A = temp;
    }
    for (t = 20; t < 40; ++t) {
        temp = SHA1CircularShift(5,A) + (B ^ C ^ D) + E + W[t] + K[1];
        E = D;
        D = C;
        C = SHA1CircularShift(30,B);
        B = A;
        A = temp;
    }

    for (t = 40; t < 60; ++t) {
        temp = SHA1CircularShift(5,A) + ((B & C) | (B & D) | (C & D)) + E + W[t] + K[2];
        E = D;
        D = C;
        C = SHA1CircularShift(30,B);
        B = A;
        A = temp;
    }

    for (t = 60; t < 80; ++t) {
        temp = SHA1CircularShift(5,A) + (B ^ C ^ D) + E + W[t] + K[3];
        E = D;
        D = C;
        C = SHA1CircularShift(30,B);
        B = A;
        A = temp;
    }

    ctx.Intermediate_Hash[0] += A;
    ctx.Intermediate_Hash[1] += B;
    ctx.Intermediate_Hash[2] += C;
    ctx.Intermediate_Hash[3] += D;
    ctx.Intermediate_Hash[4] += E;
    ctx.Message_Block_Index = 0;

    return ctx;
  }
  /*
   *  SHA1PadMessage
   *
   *  Description:
   *      According to the standard, the message must be padded to an even
   *      512 bits.  The first padding bit must be a '1'.  The last 64
   *      bits represent the length of the original message.  All bits in
   *      between should be 0.  This function will pad the message
   *      according to those rules by filling the Message_Block array
   *      accordingly.  It will also call the ProcessMessageBlock function
   *      provided appropriately.  When it returns, it can be assumed that
   *      the message digest has been computed.
   *
   *  Parameters:
   *      context: [in/out]
   *          The context to pad
   *      ProcessMessageBlock: [in]
   *          The appropriate SHA*ProcessMessageBlock function
   *  Returns:
   *      Nothing.
   *
   */
  static Context_t SHA1PadMessage(const Context_t& context)
  {
    Context_t ctx(context);

    //  Check to see if the current message block is too small to hold
    //  the initial padding bits and length.  If so, we will pad the
    //  block, process it, and then continue padding into a second
    //  block.
    if (ctx.Message_Block_Index > 55) {
        ctx.Message_Block[ctx.Message_Block_Index++] = 0x80;
        while (ctx.Message_Block_Index < MESSAGE_BLOCK_SIZE) {
            ctx.Message_Block[ctx.Message_Block_Index++] = 0;
        }

        ctx = SHA1ProcessMessageBlock(ctx);

        while (ctx.Message_Block_Index < 56) {
            ctx.Message_Block[ctx.Message_Block_Index++] = 0;
        }
    }
    else {
        ctx.Message_Block[ctx.Message_Block_Index++] = 0x80;
        while (ctx.Message_Block_Index < 56) {
            ctx.Message_Block[ctx.Message_Block_Index++] = 0;
        }
    }

    /*
     *  Store the message length as the last 8 octets
     */
    ctx.Message_Block[56] = ctx.Length_High >> 24;
    ctx.Message_Block[57] = ctx.Length_High >> 16;
    ctx.Message_Block[58] = ctx.Length_High >> 8;
    ctx.Message_Block[59] = ctx.Length_High;
    ctx.Message_Block[60] = ctx.Length_Low >> 24;
    ctx.Message_Block[61] = ctx.Length_Low >> 16;
    ctx.Message_Block[62] = ctx.Length_Low >> 8;
    ctx.Message_Block[63] = ctx.Length_Low;

    ctx = SHA1ProcessMessageBlock(ctx);
    return ctx;
  }

  // Define the SHA1 circular left shift macro
  static uint32_t SHA1CircularShift(uint32_t bits, uint32_t word)
  {
    return (((word) << (bits)) | ((word) >> (32-(bits))));
  }
};

/// @brief check it is numerichost
///
/// @param [in] host: host
/// @retval true it is ipaddress (numeric host)
/// @retval false it is hostname (e.g. FQDN)
/// @exception CRegexException
inline bool is_numerichost(const std::string& host)
{
  std::string trimed_host(trim(host, "[]"));
#if 0
//  struct addrinfo hints = {0};
//  struct addrinfo* res0 = nullptr;
//  hints.ai_family = AF_UNSPEC;
//  hints.ai_socktype = SOCK_STREAM;
//  hints.ai_flags = AI_NUMERICHOST;
//  int ret = ::getaddrinfo(trimed_host.c_str(), "80", &hints, &res0);
//  freeaddrinfo(res0);
//  return ret == 0;
#else
  uint8_t tmp[sizeof(struct in6_addr)] = {0};
  int ret = inet_pton(AF_INET, trimed_host.c_str(), tmp);
  if (ret != 1) {
    ret = inet_pton(AF_INET6, trimed_host.c_str(), tmp);
  }
  return ret == 1;
#endif
}

/// @brief split into host_port part and path_query part
///
/// @param [in] uri: uri
/// @retval pair::first: host_port <br>
///         pair::second: path_query
/// @exception CRegexException, LwsockExrepss
inline std::pair<std::string, std::string> split_hostport_pathquery(const std::string& uri)
{
  DECLARE_CALLEE(callee, __func__, "(uri=\"" << uri << "\")");
  alog::scoped slog(LogLevel::DEBUG, callee.str());

  std::string re = R"(^ws://([][0-9A-Za-z\.:\-]+)(/.*)?)";
  size_t nmatch = 4;

  std::pair<std::string, std::string> hostport_pathquery;

  CRegex regex(re, nmatch);
  auto result = regex.exec(uri);
  switch (result.size()) {
  case 3:
    hostport_pathquery.second = result[2];
    //[[fallthrough]];
  case 2:
    hostport_pathquery.first = result[1];
    break;
  default:
    {
      int err = as_int(LwsockErrc::INVALID_PARAM);
      std::ostringstream oss;
      oss << callee.str() << " invalid uri.";
      throw LwsockException(Error(err, __LINE__, oss.str()));
    }
    break;
  }

  alog& log = alog::get_instance();
  log.d() << "    hostport=\"" << hostport_pathquery.first << "\"\n";
  log.d() << "    pathquery=\"" << hostport_pathquery.second << '\"'<< std::endl;

  slog.clear() << __func__ << "(...)";

  return hostport_pathquery;
}

/// @brief split into path part and query part
///
/// @param [in] path_query: path and query string. (e.g. /aaa/bbb/ccc?i=1&j=2)
/// @retval pair::first: path <br>
///         pair::second: query
/// @exception CRegexException, LwsockExrepss
inline std::pair<std::string, std::string> split_path_query(const std::string& path_query_str)
{
  DECLARE_CALLEE(callee, __func__, "(path_query_str=\"" << path_query_str << "\")");
  alog::scoped slog(LogLevel::DEBUG, callee.str());

  std::string re = R"((/?[^? ]*)(\?[^ ]*)?)";
  size_t nmatch = 4;

  std::pair<std::string, std::string> path_query;
  CRegex regex(re, nmatch);
  auto result = regex.exec(path_query_str);
  if (result.size() == 0)
  { return path_query; }
  switch (result.size()) {
  case 3:
    path_query.second = result[2];
    //[[fallthrough]];
  case 2:
    path_query.first = result[1][0] != '/' ? "/" + result[1] : result[1];
    break;
  default:
    { int err = as_int(LwsockErrc::INVALID_PARAM);
      std::ostringstream oss;
      oss << callee.str() << " invalid path_query_str.";
      throw LwsockException(Error(err, __LINE__, oss.str()));
    }
    break;
  }

  alog& log = alog::get_instance();
  log.d() << "    path=\"" << path_query.first << "\"\n";
  log.d() << "    query=\"" << path_query.second << '\"'<< std::endl;

  slog.clear() << __func__ << "(...)";
  return path_query;
}

/// @brief split into host part and port number part.
///
/// @param [in] host_port_str: host and port string. (e.g. aaa.bbb.ccc:12000, 192.168.0.1:12000 etc.)
/// @retval pair::first: host <br>
///         pair::second: port number
/// @exception CRegexException, LwsockExrepss
inline std::pair<std::string, std::string> split_host_port(const std::string& host_port_str)
{
  DECLARE_CALLEE(callee, __func__, "(host_port_str=\"" << host_port_str << "\")");
  alog::scoped slog(LogLevel::DEBUG, callee.str());

  std::pair<std::string, std::string> host_port;

  std::string anyaddr = "[::0]:0.0.0.0";
  if (host_port_str.find(anyaddr) != std::string::npos) {
    host_port.first = ""; // anyaddr node
    if (host_port_str.length() > anyaddr.length()) {
      host_port.second = host_port_str.substr(anyaddr.length()+1);
    }
  }
  else if (host_port_str.find("[") != std::string::npos) { // maybe host part is numeric IPv6
    std::string re = R"((\[.*\])(:[0-9]{1,5})?)";
    size_t nmatch = 4;
    CRegex regex(re, nmatch);
    std::vector<std::string> tmp = regex.exec(host_port_str);
    switch (tmp.size()) {
    case 3:
      host_port.second = tmp[2].at(0) == ':' ? tmp[2].substr(1) : tmp[2];
      //[[fallthrough]];
    case 2:
      host_port.first = trim(tmp[1], "[]");
      break;
    default:
      {
        int err = as_int(LwsockErrc::INVALID_PARAM);
        std::ostringstream oss;
        oss << callee.str() << " invalid host_port_str.";
        throw LwsockException(Error(err, __LINE__, oss.str()));
      }
      break;
    }
  }
  else {
    // There aren't collons.
    //   hostname
    //   IPv4
    // There is one collon.
    //   hostname:port
    //   IPv4:port
    // There are two or more collons.
    //   IPv6
    int cnt = std::count(std::begin(host_port_str), std::end(host_port_str), ':');
    switch (cnt) {
    case 0:
      host_port.first = host_port_str;
      break;
    case 1:
      {
        std::string::size_type pos = host_port_str.find_last_of(':');
        host_port.first = host_port_str.substr(0, pos);
        host_port.second = host_port_str.substr(pos+1);
      }
      break;
    default:
      host_port.first = host_port_str;
      break;
    }
  }

  alog& log = alog::get_instance();
  log.d() << "    host=\"" << host_port.first << "\"\n";
  log.d() << "    port=\"" << host_port.second << '\"'<< std::endl;

  slog.clear() << __func__ << "()";
  return host_port;
}


//  RFC6455 Section 5.2
//  0                   1                   2                   3
//  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
// +-+-+-+-+-------+-+-------------+-------------------------------+
// |F|R|R|R| opcode|M| Payload len |    Extended payload length    |
// |I|S|S|S|  (4)  |A|     (7)     |             (16/64)           |
// |N|V|V|V|       |S|             |   (if payload len==126/127)   |
// | |1|2|3|       |K|             |                               |
// +-+-+-+-+-------+-+-------------+ - - - - - - - - - - - - - - - +
// |     Extended payload length continued, if payload len == 127  |
// + - - - - - - - - - - - - - - - +-------------------------------+
// |                               |Masking-key, if MASK set to 1  |
// +-------------------------------+-------------------------------+
// | Masking-key (continued)       |          Payload Data         |
// +-------------------------------- - - - - - - - - - - - - - - - +
// :                     Payload Data continued ...                :
// + - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
// |                     Payload Data continued ...                |
// +---------------------------------------------------------------+

/// @brief AHead class
///
/// A part of the Header class. fin,rsv1,rsv2,rsv3,opcode,payload_len
/// Little Endian is assumed
class AHead final {
public:
  AHead() = default;
  AHead(const AHead&) = default;
  AHead(AHead&&) = default;
  explicit AHead(uint16_t data) // data is network byte order
  : data_(data) { }

  ~AHead() = default;

  AHead& operator=(const AHead&) = default;
  AHead& operator=(AHead&&) = default;

  /// @brief get raw data
  ///
  /// @retval raw data
  uint16_t data()
  {
    return data_;
  }

  /// @brief get the pointer for data
  ///
  /// @retval the pointer for data
  uint16_t* data_ptr()
  {
    return &data_;
  }

  /// @brief get size for data
  ///
  /// @retval size for data
  size_t size()
  {
    return sizeof data_;
  }

  /// @brief set fin bit
  ///
  /// @param [in] val: fin bit value. 1 or 0
  /// @retval reference of *this 
  /// @note if you set !0 (e.g. 100) then set 1
  AHead& fin(int val)
  {
    int v = val == 0 ? 0 : 1;
    data_ = (data_ & 0xff7f) | (v << 7);
    return *this;
  }

  /// @brief get fin bit
  ///
  /// @retval fin bit value
  int fin()
  {
    return (data_ & 0x0080) >> 7;
  }

  /// @brief set rsv1 bit
  ///
  /// @param [in] val: fin bit value. 1 or 0
  /// @retval reference of *this 
  /// @note if you set !0 (e.g. 100) then set 1
  AHead& rsv1(int val)
  {
    int v = val == 0 ? 0 : 1;
    data_ = (data_ & 0xffbf) | (v << 6);
    return *this;
  }

  /// @brief get rsv1 bit
  ///
  /// @retval rsv1 bit value
  int rsv1()
  {
    return (data_ & 0x0040) >> 6;
  }

  /// @brief set rsv2 bit
  ///
  /// @param [in] val: rsv2 bit value. 1 or 0
  /// @retval reference of *this 
  /// @note if you set !0 (e.g. 100) then set 1
  AHead& rsv2(int val)
  {
    int v = val == 0 ? 0 : 1;
    data_ = (data_ & 0xffdf) | (v << 5);
    return *this;
  }

  /// @brief get rsv2 bit
  ///
  /// @retval rsv2 bit value
  int rsv2()
  {
    return (data_ & 0x0020) >> 5;
  }

  /// @brief set rsv3 bit
  ///
  /// @param [in] val: rsv3 bit value. 1 or 0
  /// @retval reference of *this 
  /// @note if you set !0 (e.g. 100) then set 1
  AHead& rsv3(int val)
  {
    int v = val == 0 ? 0 : 1;
    data_ = (data_ & 0xffef) | (v << 4);
    return *this;
  }

  /// @brief get rsv3 bit
  ///
  /// @retval rsv3 bit value
  int rsv3()
  {
    return (data_ & 0x0010) >> 4;
  }

  /// @brief set opcode
  ///
  /// @param [in] val: opcode value
  /// @retval reference of *this 
  AHead& opcode(Opcode val)
  {
    data_ = (data_ & 0xfff0) | static_cast<uint8_t>(val);
    return *this;
  }

  /// @brief get opcode
  ///
  /// @retval opcode value
  Opcode opcode()
  {
    return static_cast<Opcode>(data_ & 0x000f);
  }

  /// @brief set mask bit
  ///
  /// @param [in] val: mask bit value. 1 or 0
  /// @retval reference of *this 
  /// @note if you set !0 (e.g. 100) then set 1
  AHead& mask(int val)
  {
    int v = val == 0 ? 0 : 1;
    data_ = (data_ & 0x7fff) | (v << 15);
    return *this;
  }

  /// @brief get mask bit
  ///
  /// @retval mask bit value
  int mask()
  {
    return (data_ & 0x8000) >> 15;
  }

  /// @brief set payload len field value
  ///
  /// @param [in] val: payload length. it is less than equal 127
  /// @retval reference of *this 
  AHead& payload_len(int val)
  {
    assert(val <= 127);
    data_ = (data_ & 0x80ff) | (val << 8);
    return *this;
  }

  /// @brief get payload length field value
  ///
  /// @retval payload length field value
  int payload_len()
  {
    return (data_ & 0x7f00) >> 8;
  }

private:
  uint16_t data_ = 0; // network byte order
};

/// @brief Sockaddr class
///
/// sockaddr structure utility class
class Sockaddr final {
public:
  Sockaddr() = default;
  Sockaddr(const Sockaddr&) = default;
  Sockaddr(Sockaddr&&) = default;
  explicit Sockaddr(const struct sockaddr_storage& addr)
  {
    uaddr_.storage = addr;
  }

  /// @brief constructer
  ///
  /// @param [in] saddr: a pointer for struct socaddr instance
  /// @param [in] addrlen: saddr object size. bytes
  /// @exception LwsockException
  Sockaddr(const struct sockaddr* saddr, socklen_t addrlen)
  {
    if (sizeof uaddr_.storage < static_cast<size_t>(addrlen)) {
      int err = as_int(LwsockErrc::INVALID_PARAM);
      std::ostringstream oss;
      oss << "Sockaddr(saddr=" << std::hex << saddr << ", addrlen=" << std::dec << addrlen << ") addrlen is too big. [requier addrlen <= sizeof(struct sockaddr_storage)]";
      throw LwsockException(Error(err, __LINE__, oss.str()));
    }
    ::memcpy(&uaddr_.storage, saddr, addrlen);
  }
  ~Sockaddr() = default;

  Sockaddr& operator=(const Sockaddr&) = default;
  Sockaddr& operator=(Sockaddr&&) = default;

  /// @brief get address family. (e.g. AF_INET, AF_INET6 etc.)
  ///
  /// @retval AF_INET: IPv4
  /// @retval AF_INET6: IPv6
  /// @exception LwsockException
  int af()
  {
    if (ipaddr_.empty()) {
      ip();
    }
    return uaddr_.saddr.sa_family;
  }

  /// @brief get ip address. result of inet_ntop(3)
  ///
  /// @retval ip address string
  /// @exception LwsockException
  std::string ip()
  {
    if (!ipaddr_.empty()) {
      return ipaddr_;
    }
    char tmp[INET6_ADDRSTRLEN] = {0};
    socklen_t len = sizeof tmp;
    switch (uaddr_.saddr.sa_family) {
    case AF_INET:
      inet_ntop(uaddr_.saddr.sa_family, &uaddr_.in.sin_addr, tmp, len);
      port_ = ntohs(uaddr_.in.sin_port);
      break;
    case AF_INET6:
      inet_ntop(uaddr_.saddr.sa_family, &uaddr_.in6.sin6_addr, tmp, len);
      port_ = ntohs(uaddr_.in6.sin6_port);
      break;
    default:
      {
        int err = as_int(LwsockErrc::INVALID_AF);
        std::ostringstream oss;
        oss << "Sockaddr::ip()" << ". sockaddr::sa_family=" << af2str(uaddr_.saddr.sa_family);
        throw LwsockException(Error(err, __LINE__, oss.str()));
      }
      break;
    }
    ipaddr_ = tmp;
    return ipaddr_;
  }

  /// @brief get port number
  ///
  /// @exception LwsockException
  uint16_t port()
  {
    if (ipaddr_.empty()) {
      ip();
    }
    return port_;
  }
private:
  union {
    struct sockaddr_storage storage = {0};
    struct sockaddr saddr;
    struct sockaddr_in in;
    struct sockaddr_in6 in6;
  } uaddr_; // union adder
  std::string ipaddr_;
  uint16_t port_ = 0;
};

/// @brief Timespec data class
class Timespec final {
public:
  enum {
    TIMEOUT_NOSPEC = -1, ///< timeout no specification. it depend on the system
  };

  Timespec() = default;

  /// @brief constructer that the milliseconds specify
  ///
  /// @param [in] msec: millisecond or TIMEOUT_NOSPEC
  /// @exception LwsockException
  explicit Timespec(int32_t msec)
  : msec_(msec)
  {
    if (msec == TIMEOUT_NOSPEC) {
      return;
    }

    if (msec > TIMEOUT_NOSPEC) {
      tm_ = std::make_unique<struct timespec>(timespec{msec / 1000, msec % 1000 * 1000000});
    }
    else {
      throw LwsockException(Error(as_int(LwsockErrc::INVALID_PARAM), __LINE__));
    }
  }
  ~Timespec() = default;

  bool operator==(int32_t msec) const
  {
    return msec_ == msec;
  }
  bool operator!=(int32_t msec) const
  {
    return !(*this == msec);
  }
  bool operator>(int32_t msec) const
  {
    return msec_ > msec;
  }
  bool operator<(int32_t msec) const
  {
    return !(*this > msec);
  }
  bool operator>=(int32_t msec) const
  {
    return msec_ >= msec;
  }
  bool operator<=(int32_t msec) const
  {
    return msec_ <= msec;
  }

  /// @brief get a pointer of struct timespec instance
  ///
  /// @retval a pointer of struct timespec instance
  const struct timespec* ptr() const
  {
    return tm_.get();
  }

  /// @brief transform to string
  ///
  /// @retval "struct timespec" string representation. (e.g. {10, 123} or NOSPEC etc.)
  std::string to_string() const
  {
    if (msec_ == TIMEOUT_NOSPEC) {
      return "NOSPEC";
    }
    else {
      return "{" + std::to_string(tm_->tv_sec) + ", " + std::to_string(tm_->tv_nsec) + "}";
    }
  }
private:
  int32_t msec_ = TIMEOUT_NOSPEC;
  std::unique_ptr<struct timespec> tm_ = nullptr;
};

#define WSMETHOD "WebSocket::" << __func__

/// @brierf WebSocket class
class WebSocket final {
public:
  /// Mode enum
  enum class Mode {
    NONE = -1,
    CLIENT = 0,
    SERVER,
  };

  /// opening handshake headers type
  ///
  /// vector: pair <br>
  ///   pair::first: header name <br>
  ///   pair::second: value (it does not include \r\n)
  using headers_t = std::vector<std::pair<std::string, std::string>>;

  /// opening handshake type
  ///
  ///   first: requset line or status line (it does not include \r\n) <br>
  ///   second: headers_t
  using handshake_t = std::pair<std::string, headers_t>;

  WebSocket() = default;
  WebSocket(const WebSocket&) = delete;
  WebSocket(WebSocket&& ws) noexcept
  {
    mode_ = ws.mode_;
    sfd_ = ws.sfd_;
    ws.sfd_ = -1;
    bind_sfds_ = std::move(ws.bind_sfds_);
    host_ = std::move(ws.host_);
    port_ = ws.port_;

    path_ = std::move(ws.path_);
    query_ = std::move(ws.query_);
    nonce_ = std::move(ws.nonce_);
    recved_rest_buff_ = std::move(ws.recved_rest_buff_);
    remote_ = std::move(ws.remote_);
  }

  // @param [in] mode: Mode::NONE, Mode::CLIENT, Mode::SERVER
  explicit WebSocket(Mode mode)
  : mode_(mode)
  { }

  ~WebSocket()
  {
    if (sfd_ != -1) {
      close(sfd_);
    }
    if (!bind_sfds_.empty()) {
      for (auto& sfd : bind_sfds_) {
        close(sfd);
      }
    }
  }

  WebSocket& operator=(const WebSocket &) = delete;
  WebSocket& operator=(WebSocket&& rhs) noexcept
  {
    mode_ = rhs.mode_;
    sfd_ = rhs.sfd_;
    rhs.sfd_ = -1;
    host_ = std::move(rhs.host_);
    port_ = rhs.port_;
    path_ = std::move(rhs.path_);
    query_ = std::move(rhs.query_);
    nonce_ = std::move(rhs.nonce_);
    recved_rest_buff_ = std::move(rhs.recved_rest_buff_);
    remote_ = std::move(rhs.remote_);

    return *this;
  }

  /// @brief bind address that uri specify. <br>
  ///   this use getaddrinfo(3) for specified uri, then open sockets and bind addresses.
  ///
  /// @param [in] uri: WebSocket URI <br>
  ///     uri ::= "ws://" host (":" port)? path ("?" query)? <br>
  ///     host ::= hostname | IPv4_dot_decimal | IPv6_colon_hex <br>
  /// @retval reference of *this
  /// @exception CRegexException, GetaddrinfoException, LwsockExrepss
  WebSocket& bind(const std::string& uri)
  {
    return bind(uri, AF_UNSPEC);
  }

  /// @brief bind address that uri specify. <br>
  ///   if you use hostname for uri and want to specify IPv4 or IPv6, you should use this method.
  ///
  /// @param [in] uri: WebSocket URI <br>
  ///     uri ::= "ws://" host (":" port)? path ("?" query)? <br>
  ///     host ::= hostname | IPv4_dot_decimal | IPv6_colon_hex <br>
  /// @pram [in] af: AF_INET or AF_INET6 <br>
  ///     if you want to specify that use IPv4 or IPv6 then you set this param.
  /// @retval reference of *this
  /// @exception CRegexException, GetaddrinfoException, LwsockExrepss
  WebSocket& bind(const std::string& uri, int af)
  {
    assert(!uri.empty());
    assert(sfd_ == -1);

    DECLARE_CALLEE(callee, WSMETHOD, "(uri=\"" << uri << "\", af=" << af2str(af) << ")");
    alog::scoped slog(callee.str());

    if (mode_ != Mode::SERVER) {
      int err = as_int(LwsockErrc::INVALID_MODE);
      std::ostringstream oss;
      oss << callee.str() << " invalid mode. expect Mode::SERVER.";
      throw LwsockException(Error(err, __LINE__, oss.str()));
    }

    if (uri.empty()) {
      int err = as_int(LwsockErrc::INVALID_PARAM);
      std::ostringstream oss;
      oss << callee.str() << " invalid uri.";
      throw LwsockException(Error(err, __LINE__, oss.str()));
    }

    if (af != AF_UNSPEC && af != AF_INET && af != AF_INET6) {
      int err = as_int(LwsockErrc::INVALID_PARAM);
      std::ostringstream oss;
      oss << callee.str() << " invalid af=" << af2str(af);
      throw LwsockException(Error(err, __LINE__, oss.str()));
    }

    std::pair<std::string, std::string> host_port;
    std::pair<std::string, std::string> hostport_pathquery;
    try {
      // split into host_port part and path_query part.
      hostport_pathquery = split_hostport_pathquery(uri);

      // split into host part and port number part.
      host_port = split_host_port(hostport_pathquery.first);
    }
    catch (LwsockException& e) {
      int err = as_int(LwsockErrc::INVALID_PARAM);
      std::ostringstream oss;
      oss << callee.str() << " invalid uri.";
      throw LwsockException(Error(err, __LINE__, oss.str()));
    }

    // split into path part and query part.
    std::pair<std::string, std::string> path_query = split_path_query(hostport_pathquery.second);
    path_ = std::move(path_query.first);
    query_ = std::move(path_query.second);

    host_ = host_port.first;
    try {
      port_ = host_port.second.empty() ? 80 : std::stoi(host_port.second);
    }
    catch (std::invalid_argument& e) {
      int err = as_int(LwsockErrc::INVALID_PARAM);
      std::ostringstream oss;
      oss << callee.str() << " invalid port number=" << host_port.second;
      throw LwsockException(Error(err, __LINE__, oss.str()));
    }
    if (port_ > 65535) {
      int err = as_int(LwsockErrc::INVALID_PARAM);
      std::ostringstream oss;
      oss << callee.str() << " invalid port number=" << host_port.second;
      throw LwsockException(Error(err, __LINE__, oss.str()));
    }

    log_.i()
        << "host_=\"" << host_ << '\"' << ", port=" << port_ << ", path_=\"" << path_ << '\"' << ", query=\"" << query_ << '\"'
        << std::endl
        ;

    struct addrinfo hints = {0};
    struct addrinfo* res0 = nullptr;
    struct addrinfo* res = nullptr;
    hints.ai_flags |= AI_PASSIVE;
    hints.ai_flags |= is_numerichost(host_) ? AI_NUMERICHOST : hints.ai_flags;
    hints.ai_family
      = af == AF_INET ? AF_INET
      : af == AF_INET6 ? AF_INET6
      : AF_UNSPEC
      ;
    hints.ai_socktype = SOCK_STREAM;

    int ret = ::getaddrinfo(host_.empty() ? NULL : host_.c_str(), std::to_string(port_).c_str(), &hints, &res0);
    if (ret != 0) {
      int err = ret;
      std::ostringstream oss;
      oss << callee.str() << " getaddrinfo(node=\"" << host_ << "\", port=" << port_ << ")";
      throw GetaddrinfoException(Error(err, __LINE__, oss.str()));
    }

    for (res = res0; res != nullptr; res = res->ai_next) {
      int sfd = ::socket(res->ai_family, res->ai_socktype, res->ai_protocol);
      if (sfd < 0) {
        int err = errno;
        log_.w() << "::socket(" << res->ai_family << ", " << res->ai_socktype << ", " << res->ai_protocol << ") error=" << err << ". " << strerror(err) << ". Try next." << std::endl;
        continue;
      }
      log_.d() << "::socket() sfd=" << sfd << std::endl;

      int on = 1;
      if (res->ai_family == AF_INET6) {
        setsockopt(sfd, IPPROTO_IPV6, IPV6_V6ONLY, &on, sizeof on);
      }
      setsockopt(sfd, IPPROTO_TCP, TCP_NODELAY, &on, sizeof on);
      setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof on);
#if defined(SO_REUSEPORT)
      setsockopt(sfd, SOL_SOCKET, SO_REUSEPORT, &on, sizeof on);
#endif

      Sockaddr saddr(res->ai_addr, res->ai_addrlen);
      ret = ::bind(sfd, res->ai_addr, res->ai_addrlen);
      if (ret < 0) {
        int err = errno;
        log_.w() << "::bind(sfd=" << sfd << ", ip=\"" << saddr.ip() << "\", port=" << saddr.port() << ") error=" << err << ". " << strerror(err) << ". closed socket. Try next." << std::endl;
        close_socket(sfd);
        continue;
      }
      log_.i() << "::bind(sfd=" << sfd << ", ip=\"" << saddr.ip() << "\", port=" << saddr.port() << ")" << std::endl;

      bind_sfds_.push_back(sfd);
    }
    freeaddrinfo(res);

    if (bind_sfds_.empty()) {
      int err = as_int(LwsockErrc::COULD_NOT_OPEN_AVAILABLE_SOCKET);
      std::ostringstream oss;
      oss << callee.str() << " could not bind(2) any sockets.";
      throw LwsockException(Error(err, __LINE__, oss.str()));
    }

    return *this;
  }

  /// @brief listen(2) each binded sockets
  ///
  /// @param [in] backlog: listen(2)'s backlog
  /// @retval reference of *this
  /// @exception SystemErrorException
  WebSocket& listen(int backlog)
  {
    assert(mode_ == Mode::SERVER);
    assert(!bind_sfds_.empty());

    DECLARE_CALLEE(callee, WSMETHOD, "(backlog=" << backlog << ")");
    alog::scoped slog(callee.str());

    std::for_each(std::begin(bind_sfds_), std::end(bind_sfds_), [&](int sfd){
      int ret = ::listen(sfd, backlog);
      if (ret != 0) {
        int err = errno;
        std::ostringstream oss;
        oss << callee.str() << "::listen(sfd=" << sfd << ", backlog=" << backlog << ")";
        throw SystemErrorException(Error(err, __LINE__, oss.str()));
      }
      log_.i() << "::listen(sfd=" << sfd << ", backlog=" << backlog << ")" << std::endl;
    });

    return *this;
  }

  /// @brief accept socket
  ///
  /// @retval a new WebSocket instance
  /// @exception LwsockException, SystemErrorException
  WebSocket accept()
  {
    assert(mode_ == Mode::SERVER);
    assert(!bind_sfds_.empty());

    DECLARE_CALLEE(callee, WSMETHOD, "()");
    alog::scoped slog(callee.str());

    fd_set rfds;
    FD_ZERO(&rfds);
    int maxsfd = -1;

    log_.i() << "::pselect() wait sfds=";
    for (size_t i = 0; i < bind_sfds_.size(); ++i) {
      int sfd = bind_sfds_[i];
      FD_SET(sfd, &rfds);
      maxsfd = std::max(maxsfd, sfd);
      log_ << sfd << (i != bind_sfds_.size()-1 ? "," : "");
    }
    log_ << '\n' << std::flush;

    int nfds = maxsfd + 1;
    int ret = pselect(nfds, &rfds, nullptr, nullptr, nullptr, nullptr);
    if (ret == -1) {
      int err = errno;
      std::ostringstream oss;
      oss << callee.str() << " ::pselect(nfds=" << nfds << ", ...)";
      throw SystemErrorException(Error(err, __LINE__, oss.str()));
    }

    auto ite = std::find_if(std::begin(bind_sfds_), std::end(bind_sfds_), [&rfds](int sfd){
        if (FD_ISSET(sfd, &rfds)) {
          return true;
        }
        else {
          return false;
        }
    });
    int sfd = *ite;

    struct sockaddr_storage remote = {0};
    socklen_t addrlen = sizeof remote;
    //log_(LogLevel::INFO) << "::accept(sfd=" << sfd << ", ...)\n";
    int newsfd = ::accept(sfd, (struct sockaddr*)&remote, &addrlen);
    if (newsfd < 0) {
      int err = errno;
      std::ostringstream oss;
      oss << callee.str() << " ::accept(sfd=" << sfd << ", ...)";
      throw SystemErrorException(Error(err, __LINE__, oss.str()));
    }
    WebSocket ws(Mode::SERVER);
    ws.sfd_ = newsfd;
    ws.host_ = host_;
    ws.port_ = port_;
    ws.path_ = path_;
    ws.query_ = query_;
    remote_ = Sockaddr(remote);

    log_.i() << "::accept(sfd=" << sfd << ", ...) newsfd=" << newsfd << ", remote=" << remote_.ip() << ", port=" << remote_.port() << std::endl;
    return ws;
  }

  /// @brief accept socket
  ///
  /// @param [out] remote: this is set in with the address of the peer socket
  /// @retval a new WebSocket instance
  /// @exception LwsockException, SystemErrorException
  WebSocket accept(Sockaddr& remote)
  {
    WebSocket nws = accept(); // newer Websocket instance
    remote = nws.remote_;
    return nws;
  }

  /// @brief receive a opening handshake request message. blocking receive
  ///
  /// @retval received handshake message parameters
  /// @exception CRegexException, LwsockExrepss, SystemErrorException
  handshake_t recv_req()
  {
    return recv_req(Timespec());
  }

  /// @brief receive opening handshake request message with timeout. <br>
  ///   recv_req internally calls recv(2) multiple times. timeout is effective that times.
  ///
  /// @param [in] timeout: specify timeout. Timespec instance
  /// @retval received handshake message parameters
  /// @exception CRegexException, LwsockExrepss, SystemErrorException
  handshake_t recv_req(const Timespec& timeout)
  {
    assert(sfd_ != -1);
    assert(mode_ == Mode::SERVER);

    DECLARE_CALLEE(callee, WSMETHOD, "(timeout=" << timeout.to_string() << ")");
    alog::scoped slog(callee.str());

    std::string recved_response = recv_until_eoh(sfd_, timeout);
    log_.d() << '\"' << recved_response << '\"'<< std::endl;

    handshake_t handshake_data;
    try {
      handshake_data = parse_handshake_msg(recved_response);
    }
    catch (LwsockException& e) {
      int err = as_int(LwsockErrc::INVALID_HANDSHAKE);
      std::ostringstream oss;
      oss << callee.str() << " INVALID_HANDSHAKE. send 404 and close socket=" << sfd_;
      handshake_t handshake;
      handshake.first = "HTTP/1.1 400 Bad Request";
      send_res_manually(handshake);
      close_socket(sfd_); // 10.7 when the endpoint sees an opening handshake that does not correspond to the values it is expecting, the endpoint MAY drop the TCP connection.
      throw LwsockException(Error(err, __LINE__, oss.str()));
    }

    CRegex regex(R"(^GET +((/[^? ]*)(\?[^ ]*)?)? *HTTP/1\.1)", 20);
    auto tmp = regex.exec(handshake_data.first);
    if (tmp.size() < 1) {
      int err = as_int(LwsockErrc::INVALID_HANDSHAKE);
      std::ostringstream oss;
      oss << callee.str() << " INVALID_HANDSHAKE first_line=\"" << handshake_data.first << "\". send 404 and close socket=" << sfd_;
      handshake_t handshake;
      handshake.first = "HTTP/1.1 400 Bad Request";
      send_res_manually(handshake);
      close_socket(sfd_); // 10.7 when the endpoint sees an opening handshake that does not correspond to the values it is expecting, the endpoint MAY drop the TCP connection.
      throw LwsockException(Error(err, __LINE__, oss.str()));
    }

    // if the request path differ expecting path, then respond 404.
    std::pair<std::string, std::string> path_query = split_path_query(tmp[1]);
    if (path_query.first != path_) {
      int err = as_int(LwsockErrc::INVALID_HANDSHAKE);
      std::ostringstream oss;
      oss << callee.str() << " INVALID_HANDSHAKE path=\"" << path_query.first << "\". require path=" << path_ << ". send 404 and close socket=" << sfd_;
      handshake_t handshake;
      handshake.first = "HTTP/1.1 400 Bad Request";
      send_res_manually(handshake);
      close_socket(sfd_); // 10.7 when the endpoint sees an opening handshake that does not correspond to the values it is expecting, the endpoint MAY drop the TCP connection.
      throw LwsockException(Error(err, __LINE__, oss.str()));
    }

    auto ite4origin = std::find_if(std::begin(handshake_data.second), std::end(handshake_data.second), [](std::pair<std::string, std::string>& headervalue){
      if (str2lower(headervalue.first) == str2lower("Origin")) {
        return true;
      }
      else {
        return false;
      }
    });
    if (ite4origin != std::end(handshake_data.second)) {
      origin_ = ite4origin->second;
    }

    try {
      check_request_headers(handshake_data.second);
    }
    catch (LwsockException& e) {
      std::ostringstream oss;
      oss << callee.str() << e.what() << " received a bad request from the client, then send 400 response and close socekt=" << sfd_;
      handshake_t handshake;
      handshake.first = "HTTP/1.1 400 Bad Request";
      send_res_manually(handshake);
      close_socket(sfd_); // 10.7 when the endpoint sees an opening handshake that does not correspond to the values it is expecting, the endpoint MAY drop the TCP connection.
      throw LwsockException(Error(e.code(), oss.str()));
    }

    return handshake_data;
  }

  /// @brief send an opening handshake response message. <br>
  ///   send default heades. they are Host, Upgrade, Connection, Sec-WebSocket-Key and Sec-WebSocket-Accept.
  ///
  /// @retval sent a message 
  /// @exception SystemErrorException
  std::string send_res()
  {
    return send_res(headers_t{});
  }

  /// @brief send opening handshake response with other headers. <br>
  ///   if you want to send that add other headers to default headers, then use this api.
  ///
  /// @param [in] otherheaders: other headers
  /// @retval sent a message 
  /// @exception SystemErrorException
  std::string send_res(const headers_t& otherheaders)
  {
    assert(sfd_ != -1);
    assert(mode_ == Mode::SERVER);

    DECLARE_CALLEE(callee, WSMETHOD, "() otherheaders cnt=" << otherheaders.size());
    alog::scoped slog(callee.str());

    handshake_t handshake;
    handshake.first = "HTTP/1.1 101 Switching Protocols\r\n";

    headers_t headers;
    headers.push_back({"Upgrade", "websocket"});
    headers.push_back({"Connection", "Upgrade"});

    std::string key = make_key(nonce_, GUID);
    headers.push_back({"Sec-WebSocket-Accept", key});
    if (!otherheaders.empty()) {
      std::copy(std::begin(otherheaders), std::end(otherheaders), std::back_inserter(headers));
    }

    handshake.second = headers;

    return send_ohandshake(handshake);
  }

  /// @brief send an opening handshake response message that is set completely manual.
  ///
  /// @param [in] handshake: handshake message parameters
  /// @retval sent a message 
  /// @exception SystemErrorException
  std::string send_res_manually(const handshake_t& handshake)
  {
    return send_ohandshake(handshake);
  }

  /// @brief connect to the server
  ///
  /// @param [in] uri: WebSocket URI <br>
  ///     uri ::= "ws://" host (":" port)? path ("?" query)? <br>
  ///     host ::= hostname | IPv4_dot_decimal | IPv6_colon_hex <br>
  /// @retval reference of *this
  /// @exception CRegexException, GetaddrinfoException, LwsockExrepss, SystemErrorException
  WebSocket& connect(const std::string& uri)
  {
    return connect(uri, Timespec());
  }

  /// @brief connect to the server with timeout
  ///
  /// @param [in] uri: connect to uri. <br>
  ///     uri ::= "ws://" host (":" port)? path ("?" query)? <br>
  ///     host ::= hostname | IPv4_dot_decimal | IPv6_colon_hex <br>
  /// @param [in] timeout: specify timeout. Timespec instance
  /// @retval reference of *this
  /// @exception CRegexException, GetaddrinfoException, LwsockExrepss, SystemErrorException
  WebSocket& connect(const std::string& uri, const Timespec& timeout)
  {
    return connect(uri, AF_UNSPEC, timeout);
  }

  /// @brief connect to the server with timeout. and if you use hostname for uri and want to specify IPv4 or IPv6, you should use this method.
  ///
  /// @param [in] uri: connect to uri. <br>
  ///     uri ::= "ws://" host (":" port)? path ("?" query)? <br>
  ///     host ::= hostname | IPv4_dot_decimal | IPv6_colon_hex <br>
  /// @pram [in] af: AF_INET or AF_INET6 <br>
  ///     if you want to specify that use IPv4 or IPv6 then you set this param.
  /// @param [in] timeout: specify timeout. Timespec instance
  /// @retval reference of *this
  /// @exception CRegexException, GetaddrinfoException, LwsockExrepss, SystemErrorException
  /// @remarks
  WebSocket& connect(const std::string& uri, int af, const Timespec& timeout)
  {
    assert(mode_ == Mode::CLIENT);
    assert(af == AF_INET || af == AF_INET6 || af == AF_UNSPEC);
    assert(!uri.empty());
    assert(sfd_ == -1);

    DECLARE_CALLEE(callee, WSMETHOD, "(uri=\"" << uri << "\", af=" << af2str(af) << ", timeout=" << timeout.to_string() << ')');
    alog::scoped slog(callee.str());

    // define a function that it set nonblocking/blocking to sfd.
    // If nonblock is true, sfd is sat nonblocking.
    // If nonblock is false, sfd is sat blocking.
    auto sfd_nonblock = [](int sfd, bool nonblock) -> int {
      int val = nonblock;
      int ret = ioctl(sfd, FIONBIO, &val);
      return ret;
    };

    std::pair<std::string, std::string> host_port;
    std::pair<std::string, std::string> hostport_pathquery;
    try {
      // split into host_port part and path_query part.
      hostport_pathquery = split_hostport_pathquery(uri);

      // split into host part and port number part.
      host_port = split_host_port(hostport_pathquery.first);
    }
    catch (LwsockException& e) {
      int err = as_int(LwsockErrc::INVALID_PARAM);
      std::ostringstream oss;
      oss << callee.str() << " invalid uri.";
      throw LwsockException(Error(err, __LINE__, oss.str()));
    }

    // split into path part and query part.
    std::pair<std::string, std::string> path_query = split_path_query(hostport_pathquery.second);
    //path_ = std::move(path_query.first);
    path_ = path_query.first;
    query_ = std::move(path_query.second);

    host_ = host_port.first;
    try {
      port_ = host_port.second.empty() ? 80 : std::stoi(host_port.second);
    }
    catch (std::invalid_argument& e) {
      int err = as_int(LwsockErrc::INVALID_PARAM);
      std::ostringstream oss;
      oss << callee.str() << " invalid port number=" << host_port.second;
      throw LwsockException(Error(err, __LINE__, oss.str()));
    }
    if (port_ > 65535) {
      int err = as_int(LwsockErrc::INVALID_PARAM);
      std::ostringstream oss;
      oss << callee.str() << " invalid port number=" << host_port.second;
      throw LwsockException(Error(err, __LINE__, oss.str()));
    }

    log_.i() << "host=\"" << host_ << "\", port=" << port_ << ", path=\"" << path_ << "\", query=\"" << query_  << '\"' << std::endl;

    int available_sfd = -1;
    struct addrinfo hints = {0};
    struct addrinfo* res0 = nullptr;
    struct addrinfo* res = nullptr;
    hints.ai_flags += is_numerichost(host_) ? AI_NUMERICHOST : hints.ai_flags;
    hints.ai_family
      = af == AF_INET ? AF_INET
      : af == AF_INET6 ? AF_INET6
      : AF_UNSPEC
      ;
    hints.ai_socktype = SOCK_STREAM;

    int ret = ::getaddrinfo(host_.c_str(), std::to_string(port_).c_str(), &hints, &res0);
    if (ret != 0) {
      int err = ret;
      std::ostringstream oss;
      oss << callee.str() << " getaddrinfo(node=\"" << host_ << "\", port=" << port_ << ")";
      throw GetaddrinfoException(Error(err, __LINE__, oss.str()));
    }
    for (res = res0; res != nullptr; res = res->ai_next) {
      int sfd = ::socket(res->ai_family, res->ai_socktype, res->ai_protocol);
      if (sfd < 0) {
        int err = errno;
        log_.w() << "socket(" << res->ai_family << ", " << res->ai_socktype << ", " << res->ai_protocol << ") error=" << err << ". " << strerror(err) << ". Try next." << std::endl;
        continue;
      }
      log_.d() << "socket() opened sfd=" << sfd << std::endl;

      int on = 1;
      if (res->ai_family == AF_INET6) {
        setsockopt(sfd, IPPROTO_IPV6, IPV6_V6ONLY, &on, sizeof on);
      }
      setsockopt(sfd, IPPROTO_TCP, TCP_NODELAY, &on, sizeof on);
      sfd_nonblock(sfd, true); // set nonblocking mode

      ret = ::connect(sfd, res->ai_addr, res->ai_addrlen);
      if (ret == 0) {
        sfd_nonblock(sfd, false); // reset blocking mode
        Sockaddr saddr(res->ai_addr, res->ai_addrlen);
        log_.d() << "::connect(sfd=" << sfd << ", ip=\"" << saddr.ip() << "\", port=" << saddr.port() << ") success" << std::endl;
        available_sfd = sfd;
        break;
      }
      else {
        int err = errno;
        if (err == EINPROGRESS) {
          Sockaddr saddr(res->ai_addr, res->ai_addrlen);

          fd_set rfd;
          FD_ZERO(&rfd);
          FD_SET(sfd, &rfd);
          fd_set wfd;
          FD_ZERO(&wfd);
          FD_SET(sfd, &wfd);
          ret = 0;
          log_.d() << "::connect(sfd=" << sfd << ", ip=\"" << saddr.ip() << "\", port=" << saddr.port() << ", timeout=" << timeout.to_string() << ')' << std::endl;
          int nfds = sfd + 1;
          ret = pselect(nfds, &rfd, &wfd, nullptr, timeout.ptr(), nullptr);
          if (ret == -1) {
            int err = errno;
            std::ostringstream oss;
            oss << callee.str() << " ::pselect(nfds=" << nfds << ", ...)";
            close_socket(sfd);
            throw SystemErrorException(Error(err, __LINE__, oss.str()));
          }
          else if (ret == 0) {
            log_.w() << "::connect() is timeouted, try next." << std::endl;
            close_socket(sfd);
            break; // try a next connection
          }
          else {
            if (FD_ISSET(sfd, &rfd)) {
              char tmp[8]; // '8' has no particular meaning.
              int err = 0;
              ret = recv(sfd, tmp, sizeof tmp, 0);
              if (ret < 0) {
                err = errno;
              }
              Sockaddr saddr(res->ai_addr, res->ai_addrlen);
              if (ret == 0) {
                log_.w() << "::connect(sfd=" << sfd << ", ip=\"" << saddr.ip() << "\", port=" << saddr.port() << ") is closed from the server. Try next" << std::endl;
              }
              else {
                log_.w() << "::connect(sfd=" << sfd << ", ip=\"" << saddr.ip() << "\", port=" << saddr.port() << ") error=" << err << ". " << strerror(err) << ". Try next" << std::endl;
              }
              close_socket(sfd);
              continue;
            }
            if (FD_ISSET(sfd, &wfd)) {
              // connect successed
              sfd_nonblock(sfd, false); // set blocking mode
              available_sfd = sfd;
              remote_ = Sockaddr(res->ai_addr, res->ai_addrlen);
              break;
            }

            throw SystemErrorException(Error(EBADE, __LINE__, "FD_ISSET() result is an unexpected."));
          }
        }
        else {
          close_socket(sfd);
          Sockaddr saddr(res->ai_addr, res->ai_addrlen);
          log_.w() << "::connect(sfd=" << sfd << ", ip=\"" << saddr.ip() << "\", port=" << saddr.port() << ") error=" << err << ". " << strerror(err) << ". closed socket. Try next." << std::endl;

        }
      }
    }
    freeaddrinfo(res);

    if (available_sfd == -1) {
      int err = as_int(LwsockErrc::COULD_NOT_OPEN_AVAILABLE_SOCKET);
      std::ostringstream oss;
      oss << callee.str() << " COULD_NOT_OPEN_AVAILABLE_SOCKET.";
      throw LwsockException(Error(err, __LINE__, oss.str()));
    }

    sfd_ = available_sfd;
    log_.i() << WSMETHOD << "(sfd=" << sfd_ << ") connect success." << std::endl;

    return *this;
  }

  /// @brief send an opening handshake request message
  ///
  /// @retval sent a message 
  std::string send_req()
  {
    return send_req(headers_t{});
  }

  /// @brief send an opening handshake request message with other headers
  ///
  /// @param [in] otherheaders: other headers
  /// @retval sent a message 
  /// @exception SystemErrorException
  std::string send_req(const headers_t& otherheaders)
  {
    assert(sfd_ != -1);
    assert(mode_ == Mode::CLIENT);

    DECLARE_CALLEE(callee, WSMETHOD, "() otherheaders cnt=" << otherheaders.size());
    alog::scoped slog(callee.str());

    std::ostringstream first_line;
    first_line << "GET " << path_ << query_ << " HTTP/1.1" << EOL;

    handshake_t handshake;
    handshake.first = first_line.str();

    headers_t headers;
    headers.push_back({"Host", (port_ == 80) ? host_ : (host_ + ":" + std::to_string(port_))});
    headers.push_back({"Upgrade", "websocket"});
    headers.push_back({"Connection", "Upgrade"});
    nonce_ = make_nonce();
    headers.push_back({"Sec-WebSocket-Key", nonce_});
    headers.push_back({"Sec-WebSocket-Version", "13"});
    if (!otherheaders.empty()) {
      std::copy(std::begin(otherheaders), std::end(otherheaders), std::back_inserter(headers));
    }

    handshake.second = headers;

    return send_ohandshake(handshake);
  }

  /// @brief send an opening handshake request message that is set completely manual.
  ///
  /// @param [in] handshake: handshake message parameters
  /// @retval sent a message 
  /// @exception SystemErrorException
  std::string send_req_manually(const handshake_t& handshake)
  {
    return send_ohandshake(handshake);
  }

  /// @brief receive an opening handshake response message
  ///
  /// @retval pair::first: received handshake message parameters <br>
  ///         pair::second: status code of the 1st line <br>
  /// @exception CRegexException, LwsockExrepss, SystemErrorException
  std::pair<handshake_t, int32_t> recv_res()
  {
    return recv_res(Timespec());
  }

  /// @brief receive an opening handshake response message with timeout
  ///
  /// @param [in] timeout: specify timeout. Timespec instance
  /// @retval pair::first: received handshake params <br>
  ///         pair::second: status code <br>
  /// @exception CRegexException, LwsockExrepss, SystemErrorException
  std::pair<handshake_t, int32_t> recv_res(const Timespec& timeout)
  {
    assert(sfd_ != -1);

    DECLARE_CALLEE(callee, WSMETHOD, "(timeout=" << timeout.to_string() << ")");
    alog::scoped slog(callee.str());

    std::string recved_response = recv_until_eoh(sfd_, timeout);

    handshake_t handshake_data = parse_handshake_msg(recved_response);
    CRegex regex(R"(^HTTP/1.1 ([0-9]+)(.*)?)", 20);
    std::vector<std::string> tmp = regex.exec(handshake_data.first);

    if (tmp.size() < 3) {
      int err = as_int(LwsockErrc::INVALID_HANDSHAKE);
      std::ostringstream oss;
      oss << callee.str() << " INVALID_HANDSHAKE first_line=\"" << handshake_data.first << "\", then send CLOSE frame and close socket=" << sfd_;
      send_close(1002);
      close_socket(sfd_); // 10.7 when the endpoint sees an opening handshake that does not correspond to the values it is expecting, the endpoint MAY drop the TCP connection.
      throw LwsockException(Error(err, __LINE__, oss.str()));
    }
    int32_t status_code = std::stoi(tmp[1]);
    if (status_code != 101) {
      return std::pair<handshake_t, int32_t> {{}, status_code};
    }

    try {
      check_response_headers(handshake_data.second);
    }
    catch (LwsockException& e) {
      std::ostringstream oss;
      oss << callee.str() << e.what() << ", then send CLOSE frame and close socket=" << sfd_;
      send_close(1002);
      close_socket(sfd_); // 10.7 when the endpoint sees an opening handshake that does not correspond to the values it is expecting, the endpoint MAY drop the TCP connection.
      throw LwsockException(Error(e.code(), __LINE__, oss.str()));
    }

    log_.d() << handshake_data.first << std::endl;
    for (auto& elm : handshake_data.second) {
      log_.d() << elm.first << ':' << elm.second << '\n';
    }
    log_.d() << std::endl;

    return std::pair<handshake_t, int32_t>(handshake_data, status_code);
  }

  /// @brief send a websocket text message to the remote
  ///
  /// @param [in] payload_data: WebSocket payload data
  /// @retval sent data size. bytes
  /// @exception SystemErrorException
  ssize_t send_msg_txt(const std::string& payload_data)
  {
    return send_msg(Opcode::TEXT, payload_data.data(), payload_data.size());
  }

  /// @brief send a websocket binary message to the remote
  ///
  /// @param [in] payload_data: WebSocket payload data
  /// @retval sent data size. bytes
  /// @exception SystemErrorException
  ssize_t send_msg_bin(const std::vector<uint8_t>& payload_data)
  {
    return send_msg(Opcode::BINARY, payload_data.data(), payload_data.size());
  }

  /// @brief send a websocket binary message to the remote
  ///
  /// @param [in] payload_data: websocket payload data
  /// @retval sent data size. bytes
  /// @exception SystemErrorException
  template<size_t N> ssize_t send_msg_bin(const std::array<uint8_t, N>& payload_data)
  {
    return send_msg(Opcode::BINARY, payload_data.data(), payload_data.size());
  }

  /// @brief receive a websocket text message from the remote
  ///
  /// @retval pair::first: a received string message <br>
  ///         pair::second: status code when recieved a CLOSE frame <br>
  /// @exception CRegexException, LwsockExrepss, SystemErrorException
  std::pair<std::string, int32_t> recv_msg_txt()
  {
    return recv_msg_txt(Timespec());
  }

  /// @brief receive a websocket text message from the remote with timeout
  ///
  /// @param [in] timeout: specify timeout. Timespec instance
  /// @retval pair::first: a received string message <br>
  ///         pair::second: status code when recieved a CLOSE frame <br>
  /// @exception LwsockException, SystemErrorException
  std::pair<std::string, int32_t> recv_msg_txt(const Timespec& timeout)
  {
    assert(sfd_ != -1);

    DECLARE_CALLEE(callee, WSMETHOD, "(timeout=" << timeout.to_string() << ")");
    alog::scoped slog(callee.str());

    std::pair<std::string, int32_t> result = recv_msg<std::string>(timeout);

    return result;
  }

  /// @brief receive a websocket binary message from the remote
  ///
  /// @retval pair::first: a received binary message <br>
  ///         pair::second: status code when recieved a CLOSE frame <br>
  /// @exception LwsockException, SystemErrorException
  std::pair<std::vector<uint8_t>, int32_t> recv_msg_bin()
  {
    return recv_msg_bin(Timespec());
  }

  /// @brief receive a websocket binary message from the remote with timeout
  ///
  /// @param [in] timeout: specify timeout. Timespec instance
  /// @retval pair::first: a received binary message <br>
  ///         pair::second: status code when recieved a CLOSE <br>
  /// @exception LwsockException, SystemErrorException
  std::pair<std::vector<uint8_t>, int32_t> recv_msg_bin(const Timespec& timeout)
  {
    assert(sfd_ != -1);

    DECLARE_CALLEE(callee, WSMETHOD, "(timeout=" << timeout.to_string() << ")");
    alog::scoped slog(callee.str());

    std::pair<std::vector<uint8_t>, int32_t> result = recv_msg<std::vector<uint8_t>>(timeout);
    return result;
  }

  /// @brief send a PING frame
  ///
  /// @retval sent data size. bytes
  /// @exception SystemErrorException
  ssize_t send_ping()
  {
    return send_msg(Opcode::PING, "", 0);
  }

  /// @brief send a PING frame with a text app data
  ///
  /// @param [in] app_data: app data
  /// @retval sent data size. bytes
  /// @exception SystemErrorException
  ssize_t send_ping(const std::string& app_data)
  {
    return send_msg(Opcode::PING, app_data.data(), app_data.size());
  }

  /// @brief send PING frame with a binary app data
  ///
  /// @param [in] app_data: app data
  /// @retval sent size. bytes
  /// @exception SystemErrorException
  ssize_t send_ping(const std::vector<uint8_t>& app_data)
  {
    return send_msg(Opcode::PING, app_data.data(), app_data.size());
  }

  /// @brief send PING frame with a binary app data
  ///
  /// @param [in] app_data: app data
  /// @retval sent size. bytes
  /// @exception SystemErrorException
  template<size_t N> ssize_t send_ping(const std::array<uint8_t, N>& app_data)
  {
    return send_msg(Opcode::PING, app_data.data(), app_data.size());
  }

  /// @brief send a PONG frame
  ///
  /// @retval sent size. bytes
  /// @exception SystemErrorException
  ssize_t send_pong()
  {
    return send_msg(Opcode::PONG, nullptr, 0);
  }

  /// @brief send a PONG frame with a text app data
  ///
  /// @param [in] app_data: app data
  /// @retval sent size. bytes
  /// @exception SystemErrorException
  ssize_t send_pong(const std::string& app_data)
  {
    DECLARE_CALLEE(callee, WSMETHOD, "(app_data=0x" << std::hex << app_data << ")");
    alog::scoped slog(callee.str());

    return send_msg(Opcode::PONG, app_data.data(), app_data.size());
  }

  /// @brief send a PONG frame with a binary app data
  ///
  /// @param [in] app_data: app data
  /// @retval sent size. bytes
  /// @exception SystemErrorException
  ssize_t send_pong(const std::vector<uint8_t>& app_data)
  {
    DECLARE_CALLEE(callee, WSMETHOD, "(app_data=0x" << std::hex << &app_data << ")");
    alog::scoped slog(callee.str());

    return send_msg(Opcode::PONG, app_data.data(), app_data.size());
  }

  /// @brief send a PONG frame with a binary app data
  ///
  /// @param [in] app_data: app data
  /// @retval sent size. bytes
  /// @exception SystemErrorException
  template<size_t N> ssize_t send_pong(const std::array<uint8_t, N>& app_data)
  {
    DECLARE_CALLEE(callee, WSMETHOD, "(app_data=0x" << std::hex << &app_data << ")");
    alog::scoped slog(callee.str());

    return send_msg(Opcode::PONG, app_data.data(), app_data.size());
  }

  /// @brief send CLOSE frame. send CLOSE frame, then wait a response (maybe CLOSE frame) or wait closing socket from the remote.
  ///
  /// @param [in] status_code: status code
  /// @exception LwsockException, SystemErrorException
  void send_close(const uint16_t status_code)
  {
    send_close(status_code, "", Timespec());
  }

  /// @brief send CLOSE frame. send CLOSE frame, then wait a response (maybe CLOSE frame) or wait closing socket from the remote.
  ///
  /// @param [in] status_code: status code
  /// @param [in] reason: reason string
  /// @exception LwsockException, SystemErrorException
  void send_close(const uint16_t status_code, const std::string& reason)
  {
    send_close(status_code, reason, Timespec());
  }

  /// @brief send CLOSE frame with timeout. send CLOSE frame, then wait a response (maybe CLOSE frame) or wait closing socket from the remote.
  ///
  /// @param [in] status_code: status code
  /// @param [in] reason: reason
  /// @param [in] timeout: specify timeout. Timespec instance
  /// @exception LwsockException, SystemErrorException
  void send_close(const uint16_t status_code, const std::string& reason, const Timespec& timeout)
  {
    DECLARE_CALLEE(callee, WSMETHOD, "(status_code=" << status_code << ", reason=\"" << reason << "\", timeout=" << timeout.to_string() << ")");
    alog::scoped slog(callee.str());

    std::vector<uint8_t> appdata(sizeof status_code + reason.size());
    {
      uint16_t be_scode = htons(status_code); // big endian status code
      uint8_t* p = &appdata[0];
      ::memcpy(p, &be_scode, sizeof be_scode);
      p += sizeof be_scode;
      ::memcpy(p, reason.data(), reason.size());
    }
    try {
      send_msg(Opcode::CLOSE, appdata.data(), appdata.size());
    }
    catch (SystemErrorException& see) {
      if (see.code() == EBADF || see.code() == EPIPE) {
        ; // nop
      }
      else {
        throw see;
      }
    }
    close_websocket(sfd_, timeout);
  }

  /// @brief get Sockaddr about the remote
  ///
  /// @retval Sockaddr
  Sockaddr remote()
  {
    return remote_;
  }

  /// @brief get the request path
  ///
  /// @retval path (e.g. "/path/a/b/c")
  std::string path()
  {
    return path_;
  }

  /// @brief get the request query parameters.
  ///
  /// @retval query (e.g. "?aa=123&bb=xyz")
  std::string query()
  {
    return query_;
  }

  /// @brief get the Origin header's value in the request headers, if client is a web browser.
  ///
  /// @retval a value of Origin header
  std::string origin()
  {
    return origin_;
  }


  /// @brief get the raw sockfd that connected or accepted. you must not close it.
  ///
  /// @retval raw sockfd
  /// @note: you must not close the socket
  int sfd_ref()
  {
    return sfd_;
  }

  /// @brief get the raw sockfd that connected or accepted.
  ///
  /// @retval raw sockfd
  /// @note: you must close the socket yourself when sockfd was no necessary
  int sfd_mv()
  {
    int sfd = sfd_;
    init();
    return sfd;
  }

  /// @brief get reference of binded sockfds
  ///
  /// @retval sockfds
  /// @note: you must not close the socket
  const std::vector<int>& bind_sfds()
  {
    return bind_sfds_;
  }

  /// @brief set ostream for log. output log to the ostream
  ///
  /// @param [in] ost: ostream
  /// @retval reference of *this
  WebSocket& ostream4log(std::ostream& ost)
  {
    log_.ostream(ost);
    return *this;
  }

  /// @briaf set log level
  ///
  /// @param [in] lvl: log level
  /// @retval reference of *this
  WebSocket& loglevel(LogLevel lvl)
  {
    log_.level(lvl);
    return *this;
  }

  /// @brief get now log level

  /// @retval now log level
  LogLevel loglevel()
  {
    return log_.level();
  }

private:
  void init()
  {
    //mode_ = Mode::NONE;
    sfd_ = -1;
    host_.clear();
    port_ = 0;
    path_.clear();
    query_.clear();
    nonce_.clear();
    recved_rest_buff_ = std::vector<uint8_t>();
    remote_ = Sockaddr();
  }

  /// @brief close sockfd
  ///
  /// @param [in out] sfd: sockfd
  void close_socket(int& sfd)
  {
    DECLARE_CALLEE(callee, WSMETHOD, "(sfd=" << sfd << ")");
    if (sfd != -1) {
      alog::scoped slog(LogLevel::DEBUG, callee.str());

      log_.i() << "::close(sfd=" << sfd << ')' << std::endl;

      int ret = ::close(sfd);
      if (ret == -1) {
        int err = errno;
        std::ostringstream oss;
        oss << "::close(sfd=" << sfd << ')' << std::endl;
        Error(err, oss.str());
      }

      sfd = -1;

      if (!recved_rest_buff_.empty()) {
        recved_rest_buff_.clear();
      }
    }
  }

  /// @brief close websocket with timeout. refered RFC6455 7.1.1.
  ///
  /// @param [in out] sfd: sockfd
  /// @param [in] timeout: specify timeout. Timespec instance
  /// @exception SystemErrorException
  void close_websocket(int& sfd, const Timespec& timeout)
  {
    DECLARE_CALLEE(callee, WSMETHOD, "(sfd=" << sfd << ", timeout=" << timeout.to_string() << ")");
    alog::scoped slog(LogLevel::DEBUG, callee.str());

    if (sfd == -1) {
      return;
    }

    if (::shutdown(sfd, SHUT_WR) == 0) {
      uint8_t buff[16] = {0};
      try {
        recv_with_timeout(sfd, buff, sizeof buff, timeout);
      }
      catch (SystemErrorException& see) {
        if (see.code() == EBADF || see.code() == ECONNREFUSED) {
          ; // nop
        }
        else {
          throw see;
        }
      }
    }
    close_socket(sfd);
  }

  /// @brief send opening handshake
  ///
  /// @param [in] handshake_data: handshake data
  /// @retval string transformed handshake data 
  /// @exception SystemErrorException
  std::string send_ohandshake(const handshake_t& handshake_data)
  {
    DECLARE_CALLEE(callee, WSMETHOD, "(handshake_data=" << std::hex << &handshake_data << ")");
    alog::scoped slog(LogLevel::DEBUG, callee.str());

    std::string first_line = handshake_data.first;
    headers_t headers = handshake_data.second;

    std::ostringstream oss;
    oss << first_line;
    for (auto& header : headers) {
      oss << header.first << ": " << header.second << EOL;
    }
    oss << EOL;
    //log_(LogLevel::DEBUG) << "\"" << oss.str() << "\" size=" << std::dec << oss.str().size() << std::endl;
    log_.d() << '\"' << oss.str() << '\"' << std::endl;

    size_t ret = send_fill(sfd_, oss.str().c_str(), oss.str().size());
    assert(ret == oss.str().size());
    log_.d() << "sent size=" << oss.str().size() << std::endl;
    return oss.str();
  }

  /// @brief receive a message with timeout
  ///
  /// @param [in] timeout: specify timeout. Timespec instance
  /// @retval pair::first: received a message. if websocket was closed from the remote, then size()==0 <br>
  ///         pair::second: staus code when websocket is closed from the remote <br>
  /// @exception LwsockException, SystemErrorException
  template<typename T> std::pair<T, int32_t> recv_msg(const Timespec& timeout)
  {
    static_assert(std::is_base_of<T, std::string>::value || std::is_base_of<T, std::vector<uint8_t>>::value, "Require T is std::string or std::vector<uint8_t>");

    assert(sfd_ != -1);
    assert(timeout >= -1);

    DECLARE_CALLEE(callee, WSMETHOD, "(timeout=" << timeout.to_string() << ")");
    alog::scoped slog(LogLevel::DEBUG, callee.str());

    std::pair<T, int32_t> result{{}, 0};
    AHead ahead;
    bool txtflg = false;
    do {
      log_.d() << "  [[ receive a part of header ..." << std::endl;
      ssize_t ret = recv_fill(sfd_, ahead.data_ptr(), ahead.size(), timeout);
      if (ret == 0) { // socket is closed.
        result.first.clear();
        result.second = 1006; // RFC6455 7.1.5. "The WebSocket Connection Close Code is considered to be 1006."
        close_socket(sfd_);
        return result;
      }
      log_.d() << "  ]] receive a part of header...result="
        << " raw=0x" << std::hex << std::setw(4) << std::setfill('0') << ahead.data() << std::dec
        << ", fin=" << ahead.fin() << ", rsv1=" << ahead.rsv1() << ", rsv2=" << ahead.rsv2() << ", rsv3=" << ahead.rsv3()
        << ", opcode=0x" << std::hex << std::setw(2) << std::setfill('0') << as_int(ahead.opcode()) << std::setw(0) << std::dec
        << ", mask=" << ahead.mask()
        << ", payload_len=" << ahead.payload_len() << std::endl
        ;

      if (ahead.opcode() == Opcode::TEXT) {
        txtflg = true;
      }
      if (ahead.rsv1() != 0 || ahead.rsv2() != 0 || ahead.rsv3() != 0) {
        int err = as_int(LwsockErrc::FRAME_ERROR);
        std::ostringstream oss;
        oss << callee.str() << " rsv1=" << ahead.rsv1() << ", rsv2=" << ahead.rsv2() << ", rsv3=" << ahead.rsv3();
        log_.w() << oss.str() << std::endl;
        close_websocket(sfd_, timeout);
        throw LwsockException(Error(err, __LINE__, oss.str()));
      }

      uint64_t payload_len = 0;
      switch (ahead.payload_len()) {
      case 126:
        {
          uint16_t tmp = 0;
          ret = recv_fill(sfd_, &tmp, sizeof tmp, timeout);
          // TODO ret <= 0 case
          payload_len = ntohs(tmp);
        }
        break;
      case 127:
        {
          uint64_t tmp = 0;
          // for environment without be64ton() or betoh64()
          auto be2h64 = [](uint64_t be) -> uint64_t {
            uint64_t ret = 0;
            uint32_t* p32_1 = reinterpret_cast<uint32_t*>(&be);
            uint32_t* p32_2 = p32_1 + 1;
            uint32_t le_1 = ntohl(*p32_1);
            uint32_t le_2 = ntohl(*p32_2);
            uint32_t* r1 = reinterpret_cast<uint32_t*>(&ret);
            uint32_t* r2 = r1 + 1;
            ::memcpy(r1, &le_2, sizeof le_2);
            ::memcpy(r2, &le_1, sizeof le_1);
            return ret;
          };
          ret = recv_fill(sfd_, &tmp, sizeof tmp, timeout);
          // TODO ret <= 0 case
          payload_len = be2h64(tmp);
        }
        break;
      default:
        payload_len = ahead.payload_len();
        break;
      }
      log_.d() << "  eventually payload len=" << payload_len << std::endl;

      if ((mode_ == Mode::SERVER && ahead.mask() == 0) || (mode_ == Mode::CLIENT && ahead.mask() == 1)) {
        int err = as_int(LwsockErrc::BAD_MESSAGE);
        std::ostringstream oss;
        oss << callee.str() << "received invalid maskbit=" << ahead.mask() << ", then send CLOSE 1002 frame, then close socket=" << sfd_;
        send_close(1002);
        close_socket(sfd_);
        throw LwsockException(Error(err, __LINE__, oss.str()));
      }

      uint32_t masking_key = 0;
      if (ahead.mask()) {
        log_.d() << "  [[ receive masking key..." << std::endl;
        ret = recv_fill(sfd_, &masking_key, sizeof masking_key, timeout);
        // TODO ret == 0 case
        log_.d() << "  ]] receive masking key...raw=0x" << std::hex << std::setw(8) << std::setfill('0') << masking_key << std::endl;
      }

      // receive payload data
      std::vector<uint8_t> tmp_recved(payload_len);
      ret = recv_fill(sfd_, &tmp_recved[0], tmp_recved.size(), timeout);
      // TODO ret == 0 case

      std::vector<uint8_t> payload_data;
      if (ahead.mask()) {
        payload_data = mask_data(tmp_recved.data(), tmp_recved.size(), masking_key);
      }
      else {
        payload_data = std::move(tmp_recved);
      }

      switch (ahead.opcode()) {
      case Opcode::CONTINUE:
      case Opcode::TEXT: 
      case Opcode::BINARY: 
        break;

      case Opcode::PING:
        {
          log_.i() << "received Ping frame. app_data_sz=" << payload_data.size() << ", then send PONG" << std::endl;
          send_pong(payload_data);
        }
        continue;
      case Opcode::PONG:
        log_.i() << "received Pong frame. app_data_sz=" << payload_data.size() << std::endl;
        continue;
      case Opcode::CLOSE:
        {
          uint16_t scode = 0; // status code;
          std::string reason;
          if (payload_data.size() > 0) {
            uint16_t be_scode = 0; // big endian status code
            ::memcpy(&be_scode, payload_data.data(), sizeof be_scode);
            scode = ntohs(be_scode); // status code
            log_.i() << "received CLOSE frame from the remote, status_code=" << std::dec << scode << ", then send CLOSE" << std::endl;
            result.first.clear();
            result.second = scode;
          }
          else {
            log_.i() << "received CLOSE frame from the remote, status_code is none," << ", then send CLOSE" << std::endl;
            result.first.clear();
            result.second = 1005;
            reason = "RFC6455 7.1.5. \"If this Close control frame contains no status code, The WebSocket Connection Close Code is considered to be 1005.\"";
          }
          try {
            if (scode != 0) {
              send_close(scode, reason, timeout);
            }
            else {
              send_close(timeout);
            }
          }
          catch (SystemErrorException& e) {
            if (e.code() == EBADF || e.code() == EPIPE) {
                ; // nop. socket is closed already
            }
            else {
              throw e;
            }
          }
          close_socket(sfd_);
        }
        continue;

      default: // faild
        // TODO
        close_socket(sfd_);
        continue;
      }
      // append received data
      std::copy(std::begin(payload_data), std::end(payload_data), std::back_inserter(result.first));
    } while (ahead.fin() == 0 && ahead.opcode() != Opcode::CLOSE);

    if (txtflg) {
      // TODO
      // Check if result is UTF-8 data.
    }

    return result;
  }

  /// @brief send a message
  ///
  /// @pwaram [in] opcode: opcode
  /// @param [in] payload_data_org : extension data + app data
  /// @param [in] payload_data_sz: payload_data_org object size. bytes
  /// @retval sent size
  /// @exception SystemErrorException
  ssize_t send_msg(Opcode opcode, const void* payload_data_org, const size_t payload_data_sz)
  {
    assert(mode_ == Mode::CLIENT || mode_ == Mode::SERVER);
    assert(sfd_ != -1);
    assert(opcode == Opcode::TEXT || opcode == Opcode::BINARY || opcode == Opcode::CLOSE || opcode == Opcode::PING);

    DECLARE_CALLEE(callee, WSMETHOD, "(opcode=0x" << std::hex << std::setw(2) << std::setfill('0') << as_int(opcode) << std::setw(0) << ", payload_data_org=" << payload_data_org << ", payload_data_sz=" << std::dec << payload_data_sz << ")");
    alog::scoped slog(LogLevel::DEBUG, callee.str());

    AHead ahead;
    ahead.fin(1);
    ahead.opcode(opcode);
    ahead.mask(mode_ == Mode::CLIENT ? 1 : 0);

    union {
      uint16_t bit16;
      uint64_t bit64;
    } ext_payload_len = {0};
    if (payload_data_sz <= 125) {
      ahead.payload_len(payload_data_sz);
    }
    else if (payload_data_sz <= 0xffff) {
      ahead.payload_len(126);
      ext_payload_len.bit16 = htons(payload_data_sz);
    }
    else {
      ahead.payload_len(127);
      ext_payload_len.bit64 = htobe64(payload_data_sz);
    }
    uint64_t frame_sz
      = ahead.size()
      + (ahead.payload_len() <= 125 ? 0 : ahead.payload_len() == 126 ? sizeof ext_payload_len.bit16 : sizeof ext_payload_len.bit64) // Extended payload length
      + (mode_ == Mode::CLIENT ? 4 : 0) // for masking key
      + payload_data_sz
      ;

    std::vector<uint8_t> frame(frame_sz);
    uint8_t* p = &frame[0];

    ::memcpy(p, ahead.data_ptr(), ahead.size());
    p += ahead.size();

    if (ahead.payload_len() == 126) {
      ::memcpy(p, &ext_payload_len.bit16, sizeof ext_payload_len.bit16);
      p += sizeof ext_payload_len.bit16;
    }
    else if (ahead.payload_len() == 127) {
      ::memcpy(p, &ext_payload_len.bit64, sizeof ext_payload_len.bit64);
      p += sizeof ext_payload_len.bit64;
    }

    if (mode_ == Mode::CLIENT) {
      std::mt19937 rd;
      std::uniform_int_distribution<uint32_t> dist(0, 0xffffffff);
      uint32_t masking_key = dist(rd);

      ::memcpy(p, &masking_key, sizeof masking_key);
      p += sizeof masking_key;

      std::vector<uint8_t> payload_data = mask_data(payload_data_org, payload_data_sz, masking_key);
      ::memcpy(p, payload_data.data(), payload_data_sz);
    }
    else {
      ::memcpy(p, payload_data_org, payload_data_sz);
    }

    ssize_t sentsz = send_fill(sfd_, frame.data(), frame.size());

    slog.clear() << "WebSocket::send_msg(opcode=0x" << std::hex << std::setw(2) << std::setfill('0') << as_int(opcode) << std::dec << ", ...) total sent size=" << sentsz;

    return sentsz;
  }

  /// @brief send data untill specified size
  ///
  /// @param [in] sfd: sockfd
  /// @param [in] buff: data pointer
  /// @param [in] buffsz: buff object size
  /// @retval sent size
  /// @exception SystemErrorException
  ssize_t send_fill(int sfd, const void* buff, const size_t buffsz)
  {
    DECLARE_CALLEE(callee, WSMETHOD, "(sfd=" << sfd << ", buff=" << std::hex << buff << ", buffsz=" << std::dec << buffsz << ")");
    alog::scoped slog(LogLevel::DEBUG, callee.str());

    const uint8_t* ptr = static_cast<const uint8_t*>(buff);
    size_t sent_sz = 0;

    while (sent_sz < buffsz) {
      int ret = ::send(sfd, ptr, buffsz - sent_sz, MSG_NOSIGNAL);
      if (ret < 0) {
        int err = errno;
        std::ostringstream oss;
        oss << callee.str() << " ::send(sfd=" << sfd << ", ...)";
        throw SystemErrorException(Error(err, __LINE__, oss.str()));
      }

      ptr += ret;
      sent_sz += ret;

      // Urge context switching.
      struct timespec ts{0, 1};
      nanosleep(&ts, nullptr);
    }

    slog.clear() << WSMETHOD << "(sfd=" << sfd << ", ...) result=" << sent_sz;
    return sent_sz;
  }

  /// @brief recv(2) with timeout.
  ///
  /// @param [in] sfd: sockfd
  /// @param [out] buff: buffer pointer
  /// @param [in] buffsz: buffer size
  /// @param [in] timeout: specify timeout. Timespec instance
  /// @reval > 0 received size
  /// @reval ==0 socket was closed
  /// @exception SystemErrorException
  ssize_t recv_with_timeout(int sfd, void* buff, size_t buffsz, const Timespec& timeout)
  {
    DECLARE_CALLEE(callee, WSMETHOD, "(sfd=" << sfd << ", buff=" << std::hex << buff << ", buffsz=" << std::dec << buffsz << ", timeout=" << timeout.to_string() << ")");
    alog::scoped slog(LogLevel::DEBUG, callee.str());

    fd_set rfd;
    FD_ZERO(&rfd);
    FD_SET(sfd, &rfd);
    int nfds = sfd + 1;
    int ret = pselect(nfds, &rfd, nullptr, nullptr, timeout.ptr(), nullptr);
    if (ret == 0) {
      int err = as_int(LwsockErrc::TIMED_OUT);
      std::ostringstream oss;
      oss << callee.str() << " ::peslect(nfds=" << nfds << ", ...) TIMED OUT.";
      throw SystemErrorException(Error(err, __LINE__, oss.str()));
    }
    else if (ret == -1) {
      int err = errno;
      std::ostringstream oss;
      oss << callee.str() << " ::pselect(nfds=" << nfds << ", ...) errno=" << err;
      throw SystemErrorException(Error(err, __LINE__, oss.str()));
    }

    ssize_t result = recv(sfd, buff, buffsz, 0);
    if (result == -1) {
      int err = errno;
      std::ostringstream oss;
      oss << callee.str() << " ::recv(sfd=" << sfd << ", ...) error.";
      throw SystemErrorException(Error(err, __LINE__, oss.str()));
    }

    slog.clear() << WSMETHOD << "(sfd=" << sfd << ", ...) result=" << result;
    return result;
  }

  /// @brief receive untill specified size with timeout
  ///
  /// @param [in] sfd: sockfd
  /// @param [out] buff: buffer's pointer
  /// @param [in] expect_sz: expect size
  /// @param [in] timeout: specify timeout. Timespec instance
  /// @reval > 0 received size
  /// @reval ==0 socket was closed
  /// @exception LwsockException, SystemErrorException
  ssize_t recv_fill(int sfd, void* buff, const size_t expect_sz, const Timespec& timeout)
  {
    assert(sfd != -1);

    DECLARE_CALLEE(callee, WSMETHOD, "(sfd=" << sfd << ", buff=" << std::hex << buff << ", expect_sz=" << std::dec << expect_sz << ", timeout=" << timeout.to_string() << ")");
    alog::scoped slog(LogLevel::DEBUG, callee.str());

    uint8_t* ptr = static_cast<uint8_t*>(buff);
    size_t recved_sz = 0;

    // if received data when opening handshake is rest, then copy it
    log_.d() << "    recved_rest_buff.size()=" << recved_rest_buff_.size() << std::endl;
    if (!recved_rest_buff_.empty()) {
      size_t sz = recved_rest_buff_.size();
      if (sz > expect_sz) {
        memcpy(ptr, &recved_rest_buff_[0], expect_sz);
        std::vector<uint8_t> rest(std::begin(recved_rest_buff_)+expect_sz, std::end(recved_rest_buff_));
        recved_rest_buff_ = std::move(rest);
        return expect_sz;
      }
      memcpy(ptr, &recved_rest_buff_[0], sz);
      recved_rest_buff_.clear();
      ptr += sz;
      recved_sz += sz;
    }

    ssize_t ret = 0;
    while (recved_sz < expect_sz && (ret = recv_with_timeout(sfd, ptr, expect_sz - recved_sz, timeout)) > 0) {
      ptr += ret;
      recved_sz += ret;

      // Urge context switching.
      struct timespec ts{0, 1};
      nanosleep(&ts, nullptr);
    }

    ret = recved_sz == expect_sz ? recved_sz : ret;

    slog.clear() << "WebSocket::recv_fill(sfd=" << sfd << ", ...) result=" << ret;
    return ret;
  }

  /// @brief receive untill CRLFCRLF. if there is data after CRLFCRLF, save it
  ///
  /// @param [in] sfd: sockfd
  /// @param [in] timeout: specify timeout. Timespec instance
  /// @retval received data
  /// @exception LwsockException, SystemErrorException
  std::string recv_until_eoh(int sfd, const Timespec& timeout)
  {
    assert(recved_rest_buff_.empty());

    DECLARE_CALLEE(callee, WSMETHOD, "(sfd=" << sfd << ", timeout=" << timeout.to_string() << ")");
    alog::scoped slog(LogLevel::DEBUG, callee.str());

    constexpr std::string::size_type NPOS = std::string::npos;
    std::string recved_msg;

    constexpr char EOH[] = "\r\n\r\n"; // end of header
    std::string::size_type pos = NPOS;
    while ((pos = recved_msg.find(EOH)) == NPOS) {
      char tmp[512] = {0};
      ssize_t ret = recv_with_timeout(sfd, tmp, (sizeof tmp) -1, timeout); // -1 is that tmp[] has at least '\0'.
      if (ret == 0) {
        int err = as_int(LwsockErrc::SOCKET_CLOSED);
        std::ostringstream oss;
        oss << callee.str() << " socket was closed from the remote.";
        throw LwsockException(Error(err, __LINE__, oss.str()));
      }
      recved_msg += tmp;

      // Urge context switching.
      struct timespec ts{0, 1};
      nanosleep(&ts, nullptr);
    }

    constexpr int eohsz = sizeof EOH -1; // end of header size
    std::string result = recved_msg.substr(0, pos + eohsz); // result data include CRLFCRLF

    // if there is data after crlfcrl, save that data to recved_rest_buff_
    if (pos + eohsz < recved_msg.size()) {
      std::copy(std::begin(recved_msg) + pos + eohsz, std::end(recved_msg), std::back_inserter(recved_rest_buff_));
    }

    log_.d() << result << std::endl;

    return result;
  }

  /// @brief send empty body CLOSE frame.
  ///
  /// @param [in] timeout: specify timeout. Timespec instance
  /// this function is when receiving empty body CLOSE frame, then called.
  /// @exception LwsockException, SystemErrorException
  void send_close(const Timespec& timeout)
  {
    DECLARE_CALLEE(callee, WSMETHOD, "(timeout=" << timeout.to_string() << ")");
    alog::scoped slog(LogLevel::DEBUG, callee.str());
    send_msg(Opcode::CLOSE, nullptr, 0);
    close_websocket(sfd_, timeout);
  }

  /// @brief split headers
  ///
  /// @param [in] lines_msg: headers string
  /// @retval splited headers
  std::vector<std::pair<std::string, std::string>> split_headers(const std::string& lines_msg)
  {
    DECLARE_CALLEE(callee, WSMETHOD, "(...)");
    alog::scoped slog(LogLevel::DEBUG, callee.str());

    using size_type = std::string::size_type;
    constexpr size_type NPOS(std::string::npos);

    std::vector<std::pair<std::string, std::string>> headers;
    for (size_type p0 = 0, pos = NPOS; (pos = lines_msg.find(EOL, p0)) != NPOS; p0 = pos + 2) {
      std::string line = lines_msg.substr(p0, pos - p0);
      if (line.empty()) {
        break;
      }
      size_type p = line.find_first_of(':');
      std::string header_name = line.substr(0, p);
      std::string value = p == NPOS ? "" : trim(line.substr(p+1));
      log_.d() << "  header_name=\"" << header_name << "\", value=\"" << value << '\"' << std::endl;
      headers.push_back(std::make_pair(std::move(header_name), std::move(value)));
    }

    return headers;
  }

  /// @brief check response headers
  ///
  /// @param [in] hlines: splited headers
  /// @exception LwsockException
  void check_response_headers(const std::vector<std::pair<std::string, std::string>>& hv_lines)
  {
    DECLARE_CALLEE(callee, WSMETHOD, "(\n");
    for (auto& e : hv_lines) {
      callee << "    " << e.first << ": " << e.second << '\n';
    }
    callee << ")";
    alog::scoped slog(LogLevel::DEBUG, callee.str());

    // check "Upgrade" header
    {
      std::string header_name = "Upgrade";
      auto ite = std::find_if(std::begin(hv_lines), std::end(hv_lines), [&header_name](auto& hv){
        if (str2lower(hv.first) == str2lower(header_name))
        { return true; }
        else
        { return false; }
      });
      if (ite == std::end(hv_lines)) {
        std::ostringstream oss;
        oss << " \"" << header_name << "\" header is not found.";
        throw LwsockException(Error(as_int(LwsockErrc::INVALID_HANDSHAKE), __LINE__, oss.str()));
      }
      if (str2lower(ite->second) != "websocket") {
        std::ostringstream oss;
        oss << " \"" << header_name << ": " << ite->second << "\" dose not include \"websocket\".";
        throw LwsockException(Error(as_int(LwsockErrc::INVALID_HANDSHAKE), __LINE__, oss.str()));
      }
    }

    // check "Connection" header
    {
      std::string header_name = "Connection";
      auto ite = std::find_if(std::begin(hv_lines), std::end(hv_lines), [&header_name](auto& hv){
        if (str2lower(hv.first) == str2lower(header_name))
        { return true; }
        else
        { return false; }
      });
      if (ite == std::end(hv_lines)) {
        std::ostringstream oss;
        oss << " \"" << header_name << "\" header is not found.";
        throw LwsockException(Error(as_int(LwsockErrc::INVALID_HANDSHAKE), __LINE__, oss.str()));
      }
      std::string str = str2lower(ite->second);
      std::string token = str2lower("Upgrade");
      if (str.find(token) == std::string::npos) {
        std::ostringstream oss;
        oss << " \"" << header_name << ": " << ite->second << "\" dose not include \"Upgrade\".";
        throw LwsockException(Error(as_int(LwsockErrc::INVALID_HANDSHAKE), __LINE__, oss.str()));
      }
    }

    // check "Sec-WebSocket-Accept"
    {
      std::string header_name = "Sec-WebSocket-Accept";
      auto ite = std::find_if(std::begin(hv_lines), std::end(hv_lines), [&header_name](auto& hv){
        if (str2lower(hv.first) == str2lower(header_name))
        { return true; }
        else
        { return false; }
      });
      if (ite == std::end(hv_lines)) {
        std::ostringstream oss;
        oss << " \"" << header_name << "\" header is not found.";
        throw LwsockException(Error(as_int(LwsockErrc::INVALID_HANDSHAKE), __LINE__, oss.str()));
      }
      std::string key = make_key(nonce_, GUID);
      if (ite->second != key) {
        std::ostringstream oss;
        oss << " invalid \"Sec-WebSocket-Accept: " << ite->second << '\"';
        throw LwsockException(Error(as_int(LwsockErrc::INVALID_HANDSHAKE), __LINE__, oss.str()));
      }
    }

    slog.clear() << "WebSocket::check_response_headers() ok";
  }

  /// @brief check request headers
  ///
  /// @param [in] hlines: splited headers
  /// @exception LwsockException
  void check_request_headers(const std::vector<std::pair<std::string, std::string>>& hv_lines)
  {
    DECLARE_CALLEE(callee, WSMETHOD, "(\n");
    for (auto& e : hv_lines) {
      callee << "    \"" << e.first << "\": \"" << e.second << "\"\n";
    }
    callee << ")";
    alog::scoped slog(LogLevel::DEBUG, callee.str());
    slog.clear() << WSMETHOD << "(...)";

    // check "Host" header existing
    { auto ite = std::find_if(std::begin(hv_lines), std::end(hv_lines), [](auto& hv){
        if (str2lower(hv.first) == str2lower("Host")) {
          return true;
        }
        else {
          return false;
        }
      });
      if (ite == std::end(hv_lines)) {
        std::ostringstream oss;
        oss << " \"Host\" header is not found.";
        throw LwsockException(Error(as_int(LwsockErrc::INVALID_HANDSHAKE), oss.str()));
      }
    }

    // extract values of "Upgrade" header. it header is possible multiple.
    {
      std::vector<std::string> values; // "Upgrade" Header's values
      std::for_each(std::begin(hv_lines), std::end(hv_lines), [&values](auto& hvs){
        if (str2lower(hvs.first) == str2lower("Upgrade")) {
          values.push_back(hvs.second);
        }
      });
      if (values.empty()) {
        std::ostringstream oss;
        oss << " \"Upgrade\" header is not found.";
        throw LwsockException(Error(as_int(LwsockErrc::INVALID_HANDSHAKE), oss.str()));
      }
      auto ite = std::find_if(std::begin(values), std::end(values), [](const std::string& value){
        auto str = str2lower(value);
        if (str.find("websocket") != std::string::npos) {
          return true;
        }
        else {
          return false;
        }
      });
      if (ite == std::end(values)) {
        std::ostringstream oss;
        oss << " \"Upgrade\" header does not have the value of \"websocket\".";
        throw LwsockException(Error(as_int(LwsockErrc::INVALID_HANDSHAKE), oss.str()));
      }
    }

    // extract values of "Connection" header. it header is possible multiple.
    {
      std::vector<std::string> values; // "Connection" Header's values
      std::for_each(std::begin(hv_lines), std::end(hv_lines), [&values](auto& hv){
        if (str2lower(hv.first) == str2lower("Connection")) {
          values.push_back(hv.second);
        }
      });
      if (values.empty()) {
        std::ostringstream oss;
        oss << " \"Connection\" header is not found.";
        throw LwsockException(Error(as_int(LwsockErrc::INVALID_HANDSHAKE), oss.str()));
      }
      auto ite = std::find_if(std::begin(values), std::end(values), [](const std::string& value){
        std::string str = str2lower(value);
        std::string token = str2lower("Upgrade");
        if (str.find(token) != std::string::npos) {
          return true;
        }
        else {
          return false;
        }
      });
      if (ite == std::end(values)) {
        std::ostringstream oss;
        oss << " \"Connection\" header does not include the value of \"Upgrade\".";
        throw LwsockException(Error(as_int(LwsockErrc::INVALID_HANDSHAKE), oss.str()));
      }
    }

    // search "Sec-WebSocket-Key"
    {
      auto sec_websocket_key_line = std::find_if(std::begin(hv_lines), std::end(hv_lines), [](auto& hv){
        if (str2lower(hv.first) == str2lower("Sec-WebSocket-Key")) {
          return true;
        }
        else {
          return false;
        }
      });
      if (sec_websocket_key_line == std::end(hv_lines)) {
        std::ostringstream oss;
        oss << " \"Sec-WebSocket-Key\" header is not found.";
        throw LwsockException(Error(as_int(LwsockErrc::INVALID_HANDSHAKE), oss.str()));
      }
      std::vector<uint8_t> value = b64decode(sec_websocket_key_line->second);
      if (value.size() != 16) {
        std::ostringstream oss;
        oss << " \"Sec-WebSocket-Key\" header is invalid size: " << std::to_string(value.size());
        throw LwsockException(Error(as_int(LwsockErrc::INVALID_HANDSHAKE), oss.str()));
      }
      nonce_ = sec_websocket_key_line->second;
    }

    // extract values of "Sec-WebSocket-Version" header. it header is possible multiple.
    {
      std::vector<std::string> values; // "Sec-WebSocket-Version" Header's values
      std::for_each(std::begin(hv_lines), std::end(hv_lines), [&values](auto& hvs){
        if (str2lower(hvs.first) == str2lower("Sec-WebSocket-Version")) {
          values.push_back(hvs.second);
        }
      });
      if (values.empty()) {
        std::ostringstream oss;
        oss << " \"Sec-WebSocket-Version\" header is not found.";
        throw LwsockException(Error(as_int(LwsockErrc::INVALID_HANDSHAKE), oss.str()));
      }
      auto ite = std::find_if(std::begin(values), std::end(values), [](const std::string& value){
        std::string str = str2lower(value);
        std::string token = "13";
        if (str.find(token) != std::string::npos) {
          return true;
        }
        else {
          return false;
        }
      });
      if (ite == std::end(values)) {
        std::ostringstream oss;
        oss << " \"Sec-WebSocket-Version\" header does not include \"13\".";
        throw LwsockException(Error(as_int(LwsockErrc::INVALID_HANDSHAKE), oss.str()));
      }
    }
  }

  /// @biref make a nonce for a opening handshake
  ///
  /// @param nonce
  std::string make_nonce()
  {
    std::mt19937 rd;
    std::uniform_int_distribution<uint64_t> dist(0, 0xffffffffffffffff);
    uint64_t tmp = dist(rd);
    uint8_t x1[16] = {0};
    ::memcpy(&x1[0], &tmp, sizeof tmp);
    ::memcpy(&x1[8], &Magic[0], (sizeof x1)-8);
    std::string nonce = b64encode(x1, sizeof x1);
    return nonce;
  }

  /// @brief make a key for a opening handshake
  ///
  /// @param key
  std::string make_key(const std::string& nonce, const std::string& guid)
  {
    std::string key = nonce + guid;
    Sha1::Context_t ctx;
    Sha1::Input(ctx, key.data(), key.size());
    uint8_t sha1val[Sha1::SHA1_HASH_SIZE] = {0};
    Sha1::Result(sha1val, sizeof sha1val, ctx);
    std::string b64 = b64encode(sha1val, sizeof sha1val);
    return b64;
  }

  /// @brief parse a opening handshake message
  ///
  /// @param [in] received handshake message
  /// @retval parsed handshake message
  /// @exception LwsockException
  handshake_t parse_handshake_msg(const std::string& handshake_msg)
  {
    using size_type = std::string::size_type;
    size_type pos = handshake_msg.find(EOL);
    if (pos == std::string::npos) {
      int err = as_int(LwsockErrc::INVALID_HANDSHAKE);
      std::ostringstream oss;
      oss << "invliad handshake=\"" << handshake_msg << '\"';
      throw LwsockException(Error(err, oss.str()));
    }
    std::string first_line = handshake_msg.substr(0, pos);
    size_type headers_start_pos = pos + (sizeof EOL -1); // -1 is '\0'
    headers_t headers = split_headers(handshake_msg.substr(headers_start_pos));

    return handshake_t{first_line, headers};
  }

  /// @brief mask data
  ///
  /// @param [in] src: data pointer
  /// @param [in] src_sz: data size. bytes
  /// @param [in] masking_key: masking key
  /// @retval masked data
  std::vector<uint8_t> mask_data(const void* src, size_t src_sz, uint32_t masking_key)
  {
    std::vector<uint8_t> masked_data(src_sz);
    const uint8_t* p0 = static_cast<const uint8_t*>(src);
    const uint8_t* p1 = reinterpret_cast<const uint8_t*>(&masking_key);
    for (size_t i = 0; i < src_sz; ++i) {
      uint8_t j = i % 4;
      uint8_t ti = p0[i] ^ p1[j];
      masked_data[i] = ti;
    }
    return masked_data;
  }

  alog& log_ = alog::get_instance();
  Mode mode_ = Mode::NONE;
  int sfd_ = -1;
  std::vector<int> bind_sfds_;
  std::string host_;
  uint32_t port_ = 0;
  std::string path_;
  std::string query_;
  std::string nonce_;
  std::string origin_;
  std::vector<uint8_t> recved_rest_buff_;
  Sockaddr remote_;
};

} // namespace lwsock
