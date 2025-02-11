#include <climits>

#include <string>
#include <iostream>

#include "json.h"

namespace json {

bool JSONParseString(struct json_object* json, const std::string& name, std::string& out_value)
{
  out_value.clear();

  struct json_object* obj = json_object_object_get(json, name.c_str());
  if (obj == nullptr) {
    std::cerr<<name<<" not found"<<std::endl;
    return false;
  }

  enum json_type type = json_object_get_type(obj);
  if (type != json_type_string) {
    std::cerr<<name<<" is not a string"<<std::endl;
    return false;
  }

  const char* value = json_object_get_string(obj);
  if (value == nullptr) {
    std::cerr<<name<<" is not valid"<<std::endl;
    return false;
  }

  out_value = value;

  return true;
}

bool JSONParseBool(struct json_object* json, const std::string& name, bool& out_value)
{
  out_value = false;

  struct json_object* obj = json_object_object_get(json, name.c_str());
  if (obj == nullptr) {
    std::cerr<<name<<" not found"<<std::endl;
    return false;
  }

  enum json_type type = json_object_get_type(obj);
  if (type != json_type_boolean) {
    std::cerr<<name<<" is not a bool"<<std::endl;
    return false;
  }

  out_value = json_object_get_boolean(obj);

  return true;
}

bool JSONParseUint16(struct json_object* json, const std::string& name, uint16_t& out_value)
{
  out_value = 0;

  struct json_object* obj = json_object_object_get(json, name.c_str());
  if (obj == nullptr) {
    std::cerr<<name<<" not found"<<std::endl;
    return false;
  }

  enum json_type type = json_object_get_type(obj);
  if (type != json_type_int) {
    std::cerr<<name<<" is not an int"<<std::endl;
    return false;
  }

  const int value = json_object_get_int(obj);
  if ((value <= 0) || (value > USHRT_MAX)) {
    std::cerr<<name<<" is not valid"<<std::endl;
    return false;
  }

  out_value = value;
  return true;
}

}
