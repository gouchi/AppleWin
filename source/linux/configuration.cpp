#include "linux/configuration.h"
#include "linux/wincompat.h"

#include "Log.h"

#include <boost/property_tree/ini_parser.hpp>
#include <boost/property_tree/ptree.hpp>

class Configuration
{
 public:
  Configuration(const std::string & filename);
  ~Configuration();

  static std::shared_ptr<Configuration> instance;

  template<typename T>
  T getValue(const std::string & section, const std::string & key) const;

  template<typename T>
  void putValue(const std::string & section, const std::string & key, const T & value);

 private:
  const std::string myFilename;

  boost::property_tree::ptree myINI;
};

std::shared_ptr<Configuration> Configuration::instance;

Configuration::Configuration(const std::string & filename) : myFilename(filename)
{
  boost::property_tree::ini_parser::read_ini(myFilename, myINI);
}

Configuration::~Configuration()
{
  boost::property_tree::ini_parser::write_ini(myFilename, myINI);
}

template <typename T>
T Configuration::getValue(const std::string & section, const std::string & key) const
{
  const std::string path = section + "." + key;
  const T value = myINI.get<T>(path);
  return value;
}

template <typename T>
void Configuration::putValue(const std::string & section, const std::string & key, const T & value)
{
  const std::string path = section + "." + key;
  myINI.put(path, value);
}

void InitializeRegistry(const std::string & filename)
{
  Configuration::instance.reset(new Configuration(filename));
}

BOOL RegLoadString (LPCTSTR section, LPCTSTR key, BOOL peruser,
                    LPTSTR buffer, DWORD chars)
{
  BOOL result;
  try
  {
    const std::string s = Configuration::instance->getValue<std::string>(section, key);
    strncpy(buffer, s.c_str(), chars);
    buffer[chars - 1] = 0;
    result = TRUE;
  }
  catch (const std::exception & e)
  {
    buffer[0] = 0;
    result = FALSE;
  }

  LogFileOutput("RegLoadString: %s - %s = %s\n", section, key, buffer);
  return result;
}

BOOL RegLoadValue (LPCTSTR section, LPCTSTR key, BOOL peruser, DWORD *value)
{
  BOOL result;
  try
  {
    *value = Configuration::instance->getValue<DWORD>(section, key);
    result = TRUE;
  }
  catch (const std::exception & e)
  {
    *value = 0;
    result = FALSE;
  }

  LogFileOutput("RegLoadValue: %s - %s = %d\n", section, key, *value);
  return result;
}

void RegSaveString (LPCTSTR section, LPCTSTR key, BOOL peruser, LPCTSTR buffer)
{
  Configuration::instance->putValue(section, key, buffer);
  LogFileOutput("RegSaveString: %s - %s = %s\n", section, key, buffer);
}

void RegSaveValue (LPCTSTR section, LPCTSTR key, BOOL peruser, DWORD value)
{
  Configuration::instance->putValue(section, key, value);
  LogFileOutput("RegSaveValue: %s - %s = %d\n", section, key, value);
}
