#pragma once

#include <string>

#include <libxml/xmlwriter.h>

namespace util {

class cXMLStringWriter {
public:
  cXMLStringWriter();
  ~cXMLStringWriter();

  bool Open();
  void Close();

  bool BeginDocument();
  bool EndDocument();

  bool BeginElement(const std::string& name);
  bool EndElement();

  bool WriteElementNamespace(const std::string& name, const std::string& value);
  bool WriteElementAttribute(const std::string& name, const std::string& value);
  bool WriteElementWithContent(const std::string& name, const std::string& content);

  const char* GetOutput() const { return (((buf != nullptr) && (buf->content != nullptr)) ? (const char*)(buf->content) : ""); }

private:
  xmlTextWriterPtr writer;
  xmlBufferPtr buf;
};

}
