/*
 *  CepGen: a central exclusive processes event generator
 *  Copyright (C) 2013-2025  Laurent Forthomme
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <unistd.h>

#include <codecvt>
#include <cstdint>
#include <locale>
#include <random>
#include <unordered_set>

#include "CepGen/Core/Exception.h"
#include "CepGen/Core/ParametersList.h"
#include "CepGen/Utils/String.h"

#ifndef __APPLE__
#include <cstring>
#endif

#ifdef __GNUG__
#include <cxxabi.h>

#include <memory>
#endif

using namespace std::string_literals;

namespace cepgen::utils {
  std::regex kFloatRegex("[+-]?[0-9]*\\.?[0-9]+([eEdD][+-]?[0-9]+)?", std::regex_constants::extended);

  std::string yesno(bool test) { return test ? colourise("true", Colour::green) : colourise("false", Colour::red); }

  template <typename T>
  std::string boldify(T) {
    throw CG_ERROR("utils:boldify") << "Boldify not yet implemented for this variable type.";
  }

  /// String implementation of the boldification procedure
  template <>
  std::string boldify(std::string str) {
    return colourise(std::move(str), Colour::none, Modifier::bold);
  }

  /// C-style character array implementation of the boldification procedure
  template <>
  std::string boldify(const char* str) {
    return boldify(std::string(str));
  }

  /// Unsigned long integer implementation of the boldification procedure
  template <>
  std::string boldify(unsigned long ui) {
    return boldify(std::to_string(ui));
  }

  Modifier operator|(const Modifier& lhs, const Modifier& rhs) {
    std::bitset<7> mod1(static_cast<int>(lhs)), mod2(static_cast<int>(rhs));
    return static_cast<Modifier>((mod1 | mod2).to_ulong());
  }

  std::string colourise(const std::string& str, const Colour& col, const Modifier& mod) {
    if (!Logger::get().isTTY())
      return str;
    std::string out;
    auto get_mod_str = [](const Colour& col, const Modifier& mod) -> std::string {
      std::string mod_str("\033[");
      if (col != Colour::none)
        mod_str += std::to_string(static_cast<int>(col));
      if (mod > Modifier::reset)
        for (size_t i = 0; i < 7; ++i)
          if ((static_cast<uint16_t>(mod) >> i) & 0x1)
            mod_str += ";" + std::to_string(i + 1);
      return mod_str + "m";
    };
    return get_mod_str(col, mod) + str + get_mod_str(Colour::reset, Modifier::reset);
  }

  std::string parseSpecialChars(const std::string& str) {
    return replaceAll(
        str, {{"Α", "\\Alpha"},      {"Β", "\\Beta"},      {"Χ", "\\Chi"},     {"Δ", "\\Delta"},   {"Ε", "\\Epsilon"},
              {"Φ", "\\Phi"},        {"Γ", "\\Gamma"},     {"Η", "\\Eta"},     {"Ι", "\\Iota"},    {"Κ", "\\Kappa"},
              {"Λ", "\\Lambda"},     {"Μ", "\\Mu"},        {"Ν", "\\Nu"},      {"Ο", "\\Omicron"}, {"Π", "\\Pi"},
              {"Θ", "\\Theta"},      {"Ρ", "\\Rho"},       {"Σ", "\\Sigma"},   {"Τ", "\\Tau"},     {"Υ", "\\Upsilon"},
              {"Ω", "\\Omega"},      {"Ξ", "\\Xi"},        {"Ψ", "\\Psi"},     {"Ζ", "\\Zeta"},    {"α", "\\alpha"},
              {"β", "\\beta"},       {"χ", "\\Chi"},       {"δ", "\\delta"},   {"ε", "\\epsilon"}, {"ɸ", "\\phi"},
              {"γ", "\\gamma"},      {"η", "\\eta"},       {"ι", "\\iota"},    {"κ", "\\kappa"},   {"λ", "\\lambda"},
              {"μ", "\\mu"},         {"ν", "\\nu"},        {"ο", "\\omicron"}, {"π", "\\pi"},      {"θ", "\\theta"},
              {"ρ", "\\rho"},        {"σ", "\\sigma"},     {"τ", "\\tau"},     {"υ", "\\upsilon"}, {"ω", "\\omega"},
              {"ξ", "\\xi"},         {"ψ", "\\psi"},       {"ζ", "\\zeta"},    {"⁺", "^{+}"},      {"¯", "^{-}"},
              {"→", "\\rightarrow"}, {"←", "\\leftarrow"}, {"↝ ", "\\leadsto"}});
  }

  std::string sanitise(const std::string& str) {
    return toLower(replaceAll(str, {{")", ""}, {"(", "_"}, {"{", "_"}, {".", ""}, {",", "_"}, {":", "_"}, {"-", ""}}));
  }

  std::string timeAs(const std::string& fmt) {
    auto now = std::time(nullptr);
    auto tm = *std::localtime(&now);
    char out_str[50];
    strftime(out_str, 50, fmt.c_str(), &tm);
    return {out_str};
  }

  size_t replaceAll(std::string& str, const std::string& from, const std::string& to) {
    size_t count = 0, pos = 0;
    while ((pos = str.find(from, pos)) != std::string::npos) {
      str.replace(pos, from.length(), to);
      pos += to.length();
      ++count;
    }
    return count;
  }

  std::string replaceAll(const std::string& str, const std::string& from, const std::string& to) {
    auto out{str};
    if (replaceAll(out, from, to) == 0)
      CG_DEBUG_LOOP("replaceAll") << "No occurrence of {"
                                  << replaceAll(from, {{"\n", "\\n"}, {"\t", "\\t"}, {"\r", "\\r"}})
                                  << "} found in input string.";
    return out;
  }

  std::string replaceAll(const std::string& str, const std::vector<std::pair<std::string, std::string> >& keys) {
    auto out{str};
    for (const auto& key : keys)
      replaceAll(out, key.first, key.second);
    CG_DEBUG_LOOP("replaceAll").log([&keys, &out](auto& log) {
      log << "Values to be replaced: ";
      for (const auto& key : keys)
        log << "\n\t{\"" << key.first << "\" -> \"" << key.second << "\"}";
      log << "\n-> output: \"" << out << "\".";
    });
    return out;
  }

  template <>
  std::string toString(const std::wstring& str) {
    typedef std::codecvt_utf8_utf16<wchar_t> convert_type;
    std::wstring_convert<convert_type> converter;
    return converter.to_bytes(str);
  }

  template <>
  std::string toString(const ParametersList& params) {
    std::ostringstream os;
    os << params;
    return os.str();
  }

  template <>
  std::string toString(const Limits& lim) {
    std::ostringstream os;
    os << lim;
    return os.str();
  }

  std::wstring toWstring(const std::string& str) {
    typedef std::codecvt_utf8_utf16<wchar_t> convert_type;
    std::wstring_convert<convert_type> converter;
    return converter.from_bytes(str);
  }

  template <>
  std::string toString(const double& val) {
    if (val == std::numeric_limits<double>::infinity())  // check for infinity
      return "inf"s;
    auto out = format("%.16g", val);
    if (out.find('.') == std::string::npos && out.find('e') == std::string::npos && out.find('E') == std::string::npos)
      out += ".0";
    return out;
  }

  std::string toCamelCase(const std::string& in, bool lower) {
    auto out = in;
    if (in.size() < 2 ||
        (in.find('_') == std::string::npos && in.find('-') == std::string::npos && in.find(' ') == std::string::npos)) {
      if (!std::isupper(in[0]))
        return in;
      for (size_t i = 1; i < out.size(); ++i) {  // convert "weird" CAMELCase string
        if (std::islower(out[i])) {
          out[i - 1] = std::toupper(out[i - 1]);
          break;
        }
        out[i] = std::tolower(out[i]);
      }
    } else {
      bool tail = true;  // start from lowercase "tail-like" characters
      size_t n = 0;
      for (const auto& c : out) {
        if (c == '-' || c == '_' || c == ' ') {
          tail = false;
          continue;  // skip this character to concentrate on the following ones (head+tail)
        }
        if (tail)
          out[n++] = std::tolower(c);
        else {  // head (uppercase letter)
          out[n++] = std::toupper(c);
          tail = true;
        }
      }
      out.resize(n);
    }
    if (lower)
      out[0] = std::tolower(out[0]);
    return out;
  }

  std::string randomString(size_t size) {
    static constexpr auto charset =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";
    auto rng = std::mt19937{std::random_device{}()};
    auto dist = std::uniform_int_distribution{{}, std::strlen(charset) - 1};
    auto output = std::string(size, '\0');
    std::generate_n(output.begin(), size, [=, &dist, &rng]() { return charset[dist(rng)]; });
    return output;
  }

  std::string s(const std::string& word, float num, bool show_number) {
    return show_number ? (num == 0 ? "no" : format("%g", num)) + format(" %s%s", word.c_str(), num > 1. ? "s" : "")
                       : format("%s%s", word.c_str(), num > 1. ? "s" : "");
  }

  std::vector<std::string> split(const std::string& str, char delim, bool trim_parts) {
    if (str.empty())
      return {};
    std::string token;
    std::istringstream iss(str);
    std::vector<std::string> out;
    while (std::getline(iss, token, delim))
      if (const auto tok = trim_parts ? trim(token) : token; !trim_parts || !tok.empty())
        out.emplace_back(tok);
    return out;
  }

  template <typename T>
  std::string merge(const std::vector<T>& vec, const std::string& delim) {
    if (vec.empty())
      return {};
    std::ostringstream oss;
    std::for_each(vec.begin(), vec.end(), [&oss, &delim, sep = ""s](const auto& val) mutable {
      oss << sep << val;
      sep = delim;
    });
    return oss.str();
  }

  template std::string merge<std::string>(const std::vector<std::string>&, const std::string&);
  template std::string merge<Limits>(const std::vector<Limits>&, const std::string&);
  template std::string merge<unsigned short>(const std::vector<unsigned short>&, const std::string&);
  template std::string merge<int>(const std::vector<int>&, const std::string&);
  template std::string merge<unsigned long long>(const std::vector<unsigned long long>&, const std::string&);
  template std::string merge<double>(const std::vector<double>&, const std::string&);
  template std::string merge<ParametersList>(const std::vector<ParametersList>&, const std::string&);

  template <typename T>
  std::string merge(const std::vector<std::vector<T> >& vec, const std::string& delim) {
    if (vec.empty())
      return {};
    std::ostringstream oss;
    std::for_each(vec.begin(), vec.end(), [&oss, &delim, sep = ""s](const auto& val) mutable {
      const auto mrg = merge(val, delim);
      oss << sep << mrg;
      sep = delim;
    });
    return oss.str();
  }

  template std::string merge<double>(const std::vector<std::vector<double> >&, const std::string&);

  std::string merge(const ParametersList& plist, const std::string&) { return plist.print(); }

  std::string merge(const Limits& lim, const std::string&) {
    std::ostringstream os;
    os << lim;
    return os.str();
  }

  bool isInt(const std::string& str) {
    return !str.empty() &&
           std::find_if(str.begin(), str.end(), [](unsigned char c) { return !std::isdigit(c); }) == str.end();
  }

  bool isFloat(const std::string& str) { return std::regex_match(str, kFloatRegex); }

  std::string toUpper(const std::string& str) {
    std::string out;
    out.resize(str.size());
    std::transform(str.begin(), str.end(), out.begin(), ::toupper);
    return out;
  }

  std::string toLower(const std::string& str) {
    std::string out;
    out.resize(str.size());
    std::transform(str.begin(), str.end(), out.begin(), ::tolower);
    return out;
  }

  template <typename T>
  void normalise(std::vector<T>& coll) {
    std::unordered_set<T> set;
    for (const auto& it : coll)
      set.insert(it);
    coll.assign(set.begin(), set.end());
    std::sort(coll.begin(), coll.end());
  }
  template void normalise(std::vector<std::string>&);

  std::string ltrim(const std::string& str) {
    auto out{str};
    out.erase(out.begin(), std::find_if(out.begin(), out.end(), [](unsigned char ch) { return !std::isspace(ch); }));
    return out;
  }

  std::string rtrim(const std::string& str) {
    auto out{str};
    out.erase(std::find_if(out.rbegin(), out.rend(), [](unsigned char ch) { return !std::isspace(ch); }).base(),
              out.end());
    return out;
  }

  std::string strip(const std::string& str) {
    auto out{str};
    out.resize(std::remove_if(out.begin(), out.end(), [](char x) { return !std::isalnum(x) && !std::isspace(x); }) -
               out.begin());
    return out;
  }

  std::string demangle(const char* name) {
#ifdef __GNUG__
    int status = 0;  // some arbitrary value to eliminate the compiler warning
    std::unique_ptr<char, void (*)(void*)> res{abi::__cxa_demangle(name, nullptr, nullptr, &status), std::free};
    return status == 0 ? res.get() : name;
#else
    return name;
#endif
  }

  std::vector<std::string> between(const std::string& str, const std::string& beg, const std::string& end) {
    size_t ptr = 0;
    std::vector<std::string> out;
    while (ptr < str.size()) {
      const auto beg_delim_pos = str.find_first_of(beg, ptr);
      if (beg_delim_pos == std::string::npos)
        break;
      const auto beg_pos = beg_delim_pos + beg.size(), end_delim_pos = str.find_first_of(end, beg_pos);
      out.emplace_back(str.substr(beg_pos, end_delim_pos - beg_pos));
      ptr = end_delim_pos;
    }
    return out;
  }

  bool startsWith(const std::string& str, const std::string& beg) { return ltrim(str).rfind(beg, 0) == 0; }

  bool endsWith(const std::string& str, const std::string& end) {
    if (end.size() > str.size())
      return false;
    return std::equal(end.rbegin(), end.rend(), str.rbegin());
  }

  std::string describeError(int error_number) {
#ifdef __APPLE__
    return std::to_string(errnum);
#else
    char* error = strerror(error_number);
    std::string error_description;
    if (const auto error_description_length = std::strlen(error); error_description_length > 0)
      error_description = " (" + std::string(error, error_description_length) + ")";
    return std::to_string(error_number) + error_description;
#endif
  }

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#define DEF_COLOUR(col) \
  case Colour::col:     \
    return os << colourise(TOSTRING(col), Colour::col);
#define DEF_MODIFIER(mod) \
  case Modifier::mod:     \
    return os << colourise(TOSTRING(mod), Colour::none, Modifier::mod);

  std::ostream& operator<<(std::ostream& os, const Colour& col) {
    switch (col) {
      DEF_COLOUR(reset)
      DEF_COLOUR(black)
      DEF_COLOUR(red)
      DEF_COLOUR(green)
      DEF_COLOUR(yellow)
      DEF_COLOUR(blue)
      DEF_COLOUR(magenta)
      DEF_COLOUR(cyan)
      DEF_COLOUR(white)
      default:
        DEF_COLOUR(none)
    }
  }

  std::ostream& operator<<(std::ostream& os, const Modifier& mod) {
    switch (mod) {
      DEF_MODIFIER(reset)
      DEF_MODIFIER(bold)
      DEF_MODIFIER(dimmed)
      DEF_MODIFIER(italic)
      DEF_MODIFIER(underline)
      DEF_MODIFIER(blink)
      DEF_MODIFIER(reverse)
      default:
        DEF_MODIFIER(none)
    }
  }

#undef DEF_COLOUR
#undef DEF_MODIFIER
}  // namespace cepgen::utils
