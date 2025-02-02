#include "xml_string_writer.h"

namespace util {

cXMLStringWriter::cXMLStringWriter() :
  writer(nullptr),
  buf(nullptr)
{
}

cXMLStringWriter::~cXMLStringWriter()
{
  Close();

  xmlBufferFree(buf);
  buf = nullptr;
}

bool cXMLStringWriter::Open()
{
  Close();

  /* Create a new XML buffer, to which the XML document will be
    * written */
  buf = xmlBufferCreate();
  if (buf == nullptr) {
    return false;
  }

  /* Create a new XmlWriter for memory, with no compression.
    * Remark: there is no compression for this kind of xmlTextWriter */
  writer = xmlNewTextWriterMemory(buf, 0);
  return (writer != nullptr);
}

void cXMLStringWriter::Close()
{
  if (writer != nullptr) {
    xmlFreeTextWriter(writer);
    writer = nullptr;
  }
}

bool cXMLStringWriter::BeginDocument()
{
  if (writer == nullptr) {
    return false;
  }

  // Set the formatting options for the XML document.
  xmlTextWriterSetIndent(writer, 1);
  xmlTextWriterSetIndentString(writer, BAD_CAST "  ");

  // Start the XML document with the XML declaration.
  const int result = xmlTextWriterStartDocument(writer, nullptr, "UTF-8", nullptr);
  return (result >= 0);
}

bool cXMLStringWriter::EndDocument()
{
  const int result = xmlTextWriterEndDocument(writer);
  if (result < 0) {
    return false;
  }

  Close();
  return true;
}

bool cXMLStringWriter::BeginElement(const std::string& name)
{
  const int result = xmlTextWriterStartElement(writer, BAD_CAST name.c_str());
  return (result >= 0);
}

bool cXMLStringWriter::EndElement()
{
  const int result = xmlTextWriterEndElement(writer);
  return (result >= 0);
}

bool cXMLStringWriter::WriteElementNamespace(const std::string& name, const std::string& value)
{
  const int result = xmlTextWriterWriteAttributeNS(writer, nullptr, BAD_CAST name.c_str(), nullptr, BAD_CAST value.c_str());
  return (result >= 0);
}

bool cXMLStringWriter::WriteElementAttribute(const std::string& name, const std::string& value)
{
  const int result = xmlTextWriterWriteAttribute(writer, BAD_CAST name.c_str(), BAD_CAST value.c_str());
  return (result >= 0);
}

bool cXMLStringWriter::WriteElementWithContent(const std::string& name, const std::string& content)
{
  const int result = xmlTextWriterWriteElement(writer, BAD_CAST name.c_str(), BAD_CAST content.c_str());
  return (result >= 0);
}

}
