#pragma once

#include <json-c/json.h>

namespace json {

// Helpers
bool JSONParseString(const struct json_object* json, const std::string& name, std::string& out_value);
bool JSONParseBool(const struct json_object* json, const std::string& name, bool& out_value);
bool JSONParseUint16(const struct json_object* json, const std::string& name, uint16_t& out_value);
bool JSONParseUint64(const struct json_object* json, const std::string& name, uint64_t& out_value);


class cJSONDocument {
public:
  explicit cJSONDocument(json_object* _pJSONObject) : pJSONObject(_pJSONObject) {}
  ~cJSONDocument() { if (pJSONObject != nullptr) json_object_put(pJSONObject); }

  constexpr bool IsValid() const { return (pJSONObject != nullptr); }

  constexpr const json_object* Get() const { return pJSONObject; }
  json_object* Get() { return pJSONObject; }

private:
  json_object* pJSONObject;
};

}
