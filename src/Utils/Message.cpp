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

#include "CepGen/Utils/Message.h"
#include "CepGen/Utils/String.h"

using namespace cepgen;

std::string Message::now() { return utils::timeAs("%H:%M:%S"); }

LoggedMessage::LoggedMessage(const std::string& module,
                             const std::string& from,
                             MessageType type,
                             const std::string& file,
                             short lineno) noexcept
    : from_(from), file_(file), line_num_(lineno), type_(type), module_(module) {}

LoggedMessage::LoggedMessage(const LoggedMessage& rhs) noexcept
    : message_(rhs.message_.str()),  // only reason to customise the copy constructor
      from_(rhs.from_),
      file_(rhs.file_),
      line_num_(rhs.line_num_),
      type_(rhs.type_),
      module_(rhs.module_) {}

LoggedMessage::~LoggedMessage() noexcept {
  if (type_ != MessageType::undefined)
    dump();
}

void LoggedMessage::dump(std::ostream* os) const noexcept {
  if (!os)
    os = utils::Logger::get().output().get();
  if (!os)
    return;

  if (type_ == MessageType::verbatim || type_ == MessageType::undefined) {
    (*os) << message_.str() << "\n";
    return;
  }
  (*os) << type_;
  if (type_ == MessageType::info) {
    if (utils::Logger::get().extended())
      (*os) << colourise("{" + from_ + "}\n\t", utils::Colour::none, utils::Modifier::dimmed | utils::Modifier::italic);
    else
      (*os) << ":\t";
    (*os) << message_.str() << "\n";
    return;
  }
  (*os) << " " << colourise(from_, utils::Colour::none, utils::Modifier::underline | utils::Modifier::dimmed);
  if (utils::Logger::get().extended())
    (*os) << " "
          << colourise(
                 file_, utils::Colour::none, utils::Modifier::bold | utils::Modifier::italic | utils::Modifier::dimmed)
          << ":"
          << colourise(
                 std::to_string(line_num_), utils::Colour::none, utils::Modifier::italic | utils::Modifier::dimmed)
          << "\n";
  else
    (*os) << ": ";
  if (type_ == MessageType::debug) {
    (*os) << colourise(message_.str(), utils::Colour::none, utils::Modifier::dimmed) << "\n";
    return;
  }
  if (type_ == MessageType::warning)
    (*os) << message_.str() << "\n";
}

namespace cepgen {
  const LoggedMessage& operator<<(const LoggedMessage& exc, const bool& var) noexcept {
    LoggedMessage& nc_except = const_cast<LoggedMessage&>(exc);
    nc_except.message_ << (var ? colourise("true", utils::Colour::green) : colourise("false", utils::Colour::red));
    return exc;
  }

  const LoggedMessage& operator<<(const LoggedMessage& exc, const std::wstring& var) noexcept {
    LoggedMessage& nc_except = const_cast<LoggedMessage&>(exc);
    nc_except.message_ << utils::toString(var);
    return exc;
  }

  std::ostream& operator<<(std::ostream& os, const LoggedMessage::MessageType& type) {
    switch (type) {
      case LoggedMessage::MessageType::info:
        return os << colourise("Info", utils::Colour::green, utils::Modifier::bold);
      case LoggedMessage::MessageType::debug:
        return os << colourise("Debug", utils::Colour::yellow, utils::Modifier::bold);
      case LoggedMessage::MessageType::warning:
        return os << colourise("Warning", utils::Colour::blue, utils::Modifier::bold);
      case LoggedMessage::MessageType::verbatim:
        return os << colourise("Verbatim", utils::Colour::none, utils::Modifier::bold);
      case LoggedMessage::MessageType::undefined:
        return os << colourise("Undefined exception", utils::Colour::none, utils::Modifier::reverse);
    }
    return os;
  }
}  // namespace cepgen
