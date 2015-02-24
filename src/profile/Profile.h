#if !defined(INC_PROFILE_H)
#define INC_PROFILE_H

#include <string>
#include <map>
#include <sstream>
#include <stdexcept>

/**
 * Parses contents like
 *    globalName1 = globalValue1
 *    globalName2 = globalValue2
 *    [SectionName1]
 *    // comment
 *    name1 = value1 // comment
 *    ; comment
 *    name2 = value2
 *    [SectionName2]
 *    name1 = value1 ; comment
 *    name2          ; name2 is equal to empty string
 * then returns value of requested type by given name.
 * Also reports about used, unused or missed (default values were used) parameters.
 */
class Profile
{
public:
  class Error : public std::runtime_error {
  public:
    Error(const std::string & i_s) : std::runtime_error(i_s) { }
  };

  // Thrown when profile is unable to read from the source stream
  class BadSource : public Error {
  public:
    BadSource(const std::string & i_src)
      : Error(std::string("Bad profile source") + (i_src.empty() ? "" : ": ") + i_src) { }
  };

  // Thrown when the name of the requested parameter is not found
  class NotFound : public Error {
  public:
    NotFound(const std::string & i_name)
      : Error(std::string("<") + i_name + "> not found") { }
  };

  // Thrown when parameter value does not match requested type
  class ParseError : public Error {
  public:
    ParseError(const std::string & i_name, const std::string & i_value)
      : Error(std::string("Cannot parse value of <") + i_name + ">: <" + i_value + ">") { }
  };

public:
  // Creates empty profile
  Profile();
  // Creates profile from a stream, throws BadSource
  explicit Profile(std::istream & i_in);
  // Creates profile from a file with given name, throws BadSource
  // If i_fileName doesn't contain path specification then appliction
  // EXE location will be used
  explicit Profile(const std::string & i_fileName);

public:
  // Reads profile from a stream and appends its content to the profile, throws BadSource
  void append(std::istream & i_in);
  // Reads profile from a file, throws BadSource
  // If i_fileName doesn't contain path specification then appliction
  // EXE location will be used
  void appendf(const std::string & i_fileName);
  // Reads profile from a string, throws BadSource
  void appends(const std::string & i_s);
  // Appends another profile (existing values are overridden)
  void append(const Profile & i_anotherProfile);
  void clear();
  void swap(Profile & i_anotherProfile);

public:
  std::string getActiveSection() const
    { return d_activeSection; }
  void setActiveSection(const std::string & i_section)
    { d_activeSection = i_section; }

public:
  // If error occurs, an exception is thrown (ParseError or NotFound)
  template <typename T>
  T demand(const std::string & i_section, const std::string & i_name) const;
  // The same but active section is used
  template <typename T>
  T demand(const std::string & i_name) const
    { return demand<T>(d_activeSection, i_name); }

  // If error occurs, default value is returned
  template <typename T>
  T request(const std::string & i_section, const std::string & i_name, const T & i_defValue) const;
  // The same but active section is used
  template <typename T>
  T request(const std::string & i_name, const T & i_defValue) const
    { return request(d_activeSection, i_name, i_defValue); }

  // Returns true if given parameter exists
  bool query(const std::string & i_section, const std::string & i_name) const;
  // The same but active section is used
  bool query(const std::string & i_name) const
    { return query(d_activeSection, i_name); }

  // Returns true if given parameter exists;
  // if parse error occurs, an exception (ParseError) is thrown
  template <typename T>
  bool query(const std::string & i_section, const std::string & i_name, T & o_value) const;
  // The same but active section is used
  template <typename T>
  bool query(const std::string & i_name, T & o_value) const
    { return query(d_activeSection, i_name, o_value); }

  // Returns true if given parameter exists;
  // if parse error occurs, default value is returned
  template <typename T>
  bool query(const std::string & i_section, const std::string & i_name, T & o_value, const T & i_defValue) const;
  // The same but active section is used
  template <typename T>
  bool query(const std::string & i_name, T & o_value, const T & i_defValue) const
    { return query(d_activeSection, i_name, o_value, i_defValue); }

public:
  // Returns true if there are used parameters
  bool used() const
    { return suchParametersExist(USAGE_USED); }
  // Returns true if there are unused parameters
  bool unused() const
    { return suchParametersExist(USAGE_UNUSED); }
  // Returns true if there are missed parameters (default values were used)
  bool usedDefault() const
    { return suchParametersExist(USAGE_USED_DEFAUT); }

public:
  // Allows to set up operator << behavior: outputState all parameters,
  // used parameters only or unused parameters only
  enum UsageState { USAGE_USED, USAGE_USED_DEFAUT, USAGE_UNUSED, USAGE_ANY };
  UsageState outputState() const
    { return d_outputState; }
  void outputState(UsageState i_outputState)
    { d_outputState = i_outputState; }
  friend std::ostream & operator << (std::ostream & o_stream, const Profile & i_profile);

private:
  // Contains additional usage tag
  struct Value {
    mutable UsageState used;
    std::string        str;
  public:
    Value()
      : used(USAGE_UNUSED) { }
    Value(const std::string & i_str, UsageState i_used = USAGE_UNUSED)
      : used(i_used), str(i_str) { }
  };  
  typedef std::map<std::string, Value>     ValuesMap;
  typedef std::map<std::string, ValuesMap> SectionsMap;

private:
  mutable SectionsMap d_items;
  UsageState          d_outputState;
  std::string         d_activeSection;

private:
  // Trims all white characters from both ends
  void trimWhiteSpaces(std::string & io_s) const;
  // Trims all comments
  void trimComments(std::string & io_s) const;
  // Appends contents of the stream to the map;
  // i_srcDesc is a string that describes the stream (e.g. file name)
  void append(std::istream & i_in, const std::string & i_srcDesc) /* throw(BadSource) */;
  void parse(const std::string & i_s);
  // Returns true if there are parameters with specified usage state
  bool suchParametersExist(UsageState i_usageState) const;
};

#include "src/Profile.inl"

#endif // PROFILE_H
