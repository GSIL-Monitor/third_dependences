#ifndef _MYUTIL_URI_H_
#define _MYUTIL_URI_H_

#include <string>
#include <sstream>
#include <vector>
#include <regex>

namespace artanis {


class Uri {
 public:

  Uri(const std::string& str) : has_authority_(false), port_(0) {
    static const std::regex uri_regex(
        "([a-zA-Z][a-zA-Z0-9+.-]*):"  // scheme:
        "([^?#]*)"                    // authority and path
        "(?:\\?([^#]*))?"             // ?query
        "(?:#(.*))?");                // #fragment
    static const std::regex authority_and_path_regex("//([^/]*)(/.*)?");

    std::smatch match;
    if (!std::regex_match(str, match, uri_regex)) {
      throw std::invalid_argument("invalid URI " + str);
    }

    scheme_ = submatch(match, 1);
    toLower(scheme_);

    std::string authority_and_path(match[2].first, match[2].second);
    std::smatch authority_and_path_match;
    if (!std::regex_match(authority_and_path,
          authority_and_path_match,
          authority_and_path_regex)) {
      // Does not start with //, doesn't have authority
      has_authority_ = false;
      path_ = authority_and_path;
    } else {
      static const std::regex authority_regex(
          "(?:([^@:]*)(?::([^@]*))?@)?"  // username, password
          "(\\[[^\\]]*\\]|[^\\[:]*)"     // host (IP-literal (e.g. '['+IPv6+']',
                   // dotted-IPv4, or named host)
                   "(?::(\\d*))?");               // port

          auto authority = authority_and_path_match[1];
          std::smatch authority_match;
          if (!std::regex_match(authority.first,
              authority.second,
              authority_match,
              authority_regex)) {
          throw std::invalid_argument(
            "invalid URI authority " + 
            std::string(authority.first, authority.second));
          }

          std::string port(authority_match[4].first, authority_match[4].second);
          port = decode(port);
          if (!port.empty()) {
            char* endptr;
            unsigned long tmp_port = strtoul(port.c_str(), &endptr, 10);
            if (endptr != port.c_str() + port.size()) {
              throw std::invalid_argument("unsupported cast to uint16_t, port:" + port);
            } else if (errno==ERANGE) {
              throw std::invalid_argument("unsupported cast to uint16_t, port:" + port);
            } else if (tmp_port > std::numeric_limits<uint16_t>::max() ||
                tmp_port < std::numeric_limits<uint16_t>::min()) {
              throw std::invalid_argument("unsupported cast to uint16_t, port:" + port);
            }
            port_ = tmp_port;
          }

          has_authority_ = true;
          username_ = decode(submatch(authority_match, 1));
          password_ = decode(submatch(authority_match, 2));
          host_ = decode(submatch(authority_match, 3));
          path_ = decode(submatch(authority_and_path_match, 2));
    }

    query_ = decode(submatch(match, 3));
    fragment_ = decode(submatch(match, 4));
  }

  std::string authority() const {
    std::string result;

    result.reserve(host().size() + username().size() + password().size() + 8);

    if (!username().empty() || !password().empty()) {
      result.append(username());

      if (!password().empty()) {
        result.push_back(':');
        result.append(password());
      }

      result.push_back('@');
    }

    result.append(host());

    if (port() != 0) {
      result.push_back(':');
      result.append(std::to_string(port()));
    }

    return result;
  }

  std::string hostname() const {
    if (host_.size() > 0 && host_[0] == '[') {
      return host_.substr(1, host_.size() - 2);
    }
    return host_;
  }

  const std::vector<std::pair<std::string, std::string>>& getQueryParams() {
    if (!query_.empty() && query_params_.empty()) {
      // Parse query string
      static const std::regex query_param_regex(
          "(^|&)" /*start of query or start of parameter "&"*/
          "([^=&]*)=?" /*parameter name and "=" if value is expected*/
          "([^=&]*)" /*parameter value*/
          "(?=(&|$))" /*forward reference, next should be end of query or
                        start of next parameter*/);
        std::cregex_iterator param_begin_itr(
            query_.data(), query_.data() + query_.size(), query_param_regex);
      std::cregex_iterator param_end_itr;
      for (auto itr = param_begin_itr; itr != param_end_itr; itr++) {
        if (itr->length(2) == 0) {
          // key is empty, ignore it
          continue;
        }
        query_params_.emplace_back(
            std::string((*itr)[2].first, (*itr)[2].second), // parameter name
            std::string((*itr)[3].first, (*itr)[3].second) // parameter value
            );
      }
    }
    return query_params_;
  }

  const std::string& scheme() const { return scheme_; }
  const std::string& username() const { return username_; }
  const std::string& password() const { return password_; }
  const std::string& host() const { return host_; }
  uint16_t port() const { return port_; }
  const std::string& path() const { return path_; }
  const std::string& query() const { return query_; }
  const std::string& fragment() const { return fragment_; }

 private:
  static inline std::string submatch(const std::smatch& m, int idx) {
    auto& sub = m[idx];
    return std::string(sub.first, sub.second);
  }

  static inline void toLower(std::string& s) {
    for (auto& c : s) {
      c = char(tolower(c));
    }
  }

  static std::string decode(const std::string& s) {
    std::ostringstream oss;
    for (int i = 0; i < s.size(); ++i) {
      char c = s[i];
      if (c == '%') {
        int d;
        std::istringstream iss(s.substr(i+1, 2));
        iss >> std::hex >> d;
        oss << static_cast<char>(d);
        i += 2;
      } else {
        oss << c;
      }
    }
    return oss.str();
  }
 private:
  std::string scheme_;
  std::string username_;
  std::string password_;
  std::string host_;
  bool has_authority_;
  uint16_t port_;
  std::string path_;
  std::string query_;
  std::string fragment_;
  std::vector<std::pair<std::string, std::string>> query_params_;
};

} // namespace artanis

#endif //_MYUTIL_URI_H_
