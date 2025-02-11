#pragma once

#include <json-c/json.h>

namespace json {

// Helpers
bool JSONParseString(struct json_object* json, const std::string& name, std::string& out_value);
bool JSONParseBool(struct json_object* json, const std::string& name, bool& out_value);
bool JSONParseUint16(struct json_object* json, const std::string& name, uint16_t& out_value);


class cJSONDocument {
public:
  explicit cJSONDocument(json_object* _pJSONObject) : pJSONObject(_pJSONObject) {}
  ~cJSONDocument() { if (pJSONObject != nullptr) json_object_put(pJSONObject); }

  bool IsValid() const { return (pJSONObject != nullptr); }

  const json_object* Get() const { return pJSONObject; }
  json_object* Get() { return pJSONObject; }

private:
  json_object* pJSONObject;
};

}
