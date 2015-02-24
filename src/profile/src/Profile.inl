template <typename T>
T Profile::demand(const std::string & i_section, const std::string & i_name) const
{
  SectionsMap::const_iterator i = d_items.find(i_section);
  if (i == d_items.end())
    throw NotFound(i_name);
  ValuesMap::const_iterator j = i->second.find(i_name);
  if (j == i->second.end() || j->second.used == USAGE_USED_DEFAUT)
    throw NotFound(i_name);
  j->second.used = USAGE_USED;
  std::istringstream in(j->second.str);
  in.clear();
  in.exceptions(std::ios::goodbit);
  T value;
  in >> value;
  if (in.fail() || in.bad() || !in.eof())
    throw ParseError(i_name, j->second.str);
  return value;
}

template <>
std::string Profile::demand<std::string>(const std::string & i_section, const std::string & i_name) const;

template <>
std::string Profile::demand<std::string>(const std::string & i_name) const;

template <typename T>
T Profile::request(const std::string & i_section, const std::string & i_name, const T & i_defValue) const
{
  try
  {
    return demand<T>(i_section, i_name);
  }
  catch(Error &)
  {
    std::ostringstream out;
    out << i_defValue;
    d_items[i_section][i_name] = Value(out.str(), USAGE_USED_DEFAUT);
    return i_defValue;
  }
}

template <typename T>
bool Profile::query(const std::string & i_section, const std::string & i_name, T & o_value) const
{  
  try
  {
    o_value = demand<T>(i_section, i_name);
    return true;
  }
  catch(NotFound &)
  {
    return false;
  }
}

template <typename T>
bool Profile::query(const std::string & i_section, const std::string & i_name, T & o_value, const T & i_defValue) const
{
  try
  {
    return query(i_section, i_name, o_value);
  }
  catch(ParseError &)
  {
    o_value = i_defValue;
    std::ostringstream out;
    out << i_defValue;
    d_items[i_section][i_name] = Profile::Value(out.str(), USAGE_USED_DEFAUT);
    return true;
  }
}
