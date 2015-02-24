#include "../Profile.h"

using std::string;

#include <iostream>
#include <fstream>
#include <sstream>

void Profile::trimWhiteSpaces(string & io_s) const
{
  if (io_s.empty())
    return;

  // remove blanks from the left side
  string blanks(" \t\n\r");
  string::size_type leftBlanksStart = io_s.find_first_not_of(blanks);
  if (leftBlanksStart != string::npos)
    io_s.erase(0, leftBlanksStart);

  // remove blanks from the right side
  string::size_type rightBlanksStart = io_s.find_last_not_of(blanks);
  if (rightBlanksStart != string::npos)
    io_s.erase(rightBlanksStart + 1);
}

void Profile::trimComments(string & io_s) const
{
  string::size_type commentsStart = io_s.find("//");
  if (commentsStart != string::npos)
    io_s.erase(commentsStart);

  commentsStart = io_s.find(';');
  if (commentsStart != string::npos)
    io_s.erase(commentsStart);
}

void Profile::append(std::istream & i_in, const string & i_srcDesc)
{
  if (i_in.bad() || i_in.fail())
  {
    throw BadSource(i_srcDesc);
  }
  string savedActiveSection = d_activeSection;
  d_activeSection = "";
  while (!i_in.eof())
  {
    string line;
    std::getline(i_in, line);
    parse(line);
  }
  d_activeSection = savedActiveSection;
}

void Profile::parse(const string & i_s)
{
  string s(i_s);
  trimComments(s);
  trimWhiteSpaces(s);
  if ( s.empty() )
    return;

  if (*s.begin() == '[' && *s.rbegin() == ']')
  {
    // it is a section declaration
    d_activeSection = s.substr(1, s.length() - 2);
    return;
  }

  string::size_type pos = s.find('=');
  string name = s.substr(0, pos);
  trimWhiteSpaces(name);
  if (name.empty())
    return;
  string value = (pos != string::npos) ? s.substr(pos + 1) : "";
  trimWhiteSpaces(value);
  d_items[d_activeSection][name].str = value;
}

Profile::Profile()
{
  d_outputState = USAGE_ANY;
}

Profile::Profile(std::istream & i_in)
{
  d_outputState = USAGE_ANY;
  append(i_in);
}

Profile::Profile(const string & i_fileName)
{
  d_outputState = USAGE_ANY;
  appendf(i_fileName);
}

void Profile::append(std::istream & i_in)
{
  append(i_in, "<external stream>");
}

void Profile::appendf(const string & i_fileName)
{
  std::ifstream fin(i_fileName.c_str());
  append(fin, string("file <") + i_fileName + ">");
}

void Profile::appends(const string & i_s)
{
  std::istringstream sin(i_s);
  append(sin, string("string <") + i_s + ">");
}

void Profile::append(const Profile & i_anotherProfile)
{
  for (SectionsMap::const_iterator i = i_anotherProfile.d_items.begin(); i != i_anotherProfile.d_items.end(); ++i)
  {
    for (ValuesMap::const_iterator j = i->second.begin(); j != i->second.end(); j++)
      d_items[i->first][j->first].str = j->second.str;
  }
}

void Profile::clear()
{
  d_items.clear();
}

void Profile::swap(Profile & i_anotherProfile)
{
  d_items.swap(i_anotherProfile.d_items);
}

std::ostream & operator << (std::ostream & o_stream, const Profile & i_profile)
{
  for (Profile::SectionsMap::const_iterator i = i_profile.d_items.begin(); i != i_profile.d_items.end(); ++i)
  {
    for (Profile::ValuesMap::const_iterator j = i->second.begin(); j != i->second.end(); j++)
    {
      if (i_profile.d_outputState == Profile::USAGE_ANY || i_profile.d_outputState == j->second.used)
      {
        if ( !i->first.empty() )
          o_stream << "[" << i->first << "] ";
        o_stream << j->first << " = " << j->second.str << std::endl;
      }
    }
  }
  return o_stream;
}

bool Profile::suchParametersExist(UsageState i_usageState) const
{
  for (SectionsMap::const_iterator i = d_items.begin(); i != d_items.end(); ++i)
  {
    for (ValuesMap::const_iterator j = i->second.begin(); j != i->second.end(); j++)
    {
      if (j->second.used == i_usageState)
        return true;
    }
  }
  return false;
}

template <>
string Profile::demand<string>(const string & i_section, const string & i_name) const
{
  SectionsMap::const_iterator i = d_items.find(i_section);
  if (i == d_items.end())
    throw NotFound(i_name);
  ValuesMap::const_iterator j = i->second.find(i_name);
  if (j == i->second.end() || j->second.used == USAGE_USED_DEFAUT)
    throw NotFound(i_name);
  j->second.used = USAGE_USED;
  return j->second.str;
}

template <>
string Profile::demand<string>(const string & i_name) const
{
  return demand<string>(d_activeSection, i_name);
}

bool Profile::query(const string & i_section, const string & i_name) const
{
  SectionsMap::const_iterator i = d_items.find(i_section);
  if (i == d_items.end())
    return false;
  ValuesMap::const_iterator j = i->second.find(i_name);
  if (j == i->second.end() || j->second.used == USAGE_USED_DEFAUT)
    return false;
  j->second.used = USAGE_USED;
  return true;
}
