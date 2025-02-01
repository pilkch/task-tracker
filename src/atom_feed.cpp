#include <iostream>
#include <sstream>
#include <string>

#include <libxml/xmlwriter.h>

#include "atom_feed.h"
#include "util.h"

namespace {

const char hex_lookup[] = {
  '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'
};

char GetRandomHexCharacter()
{
  // Return a pseudorandom alphanumeric character
  return hex_lookup[rand() % 16];
}

std::string GetRandomHexCharacters(size_t length)
{
  std::ostringstream o;
  for (size_t i = 0; i < length; i++) {
    o<<GetRandomHexCharacter();
  }
  return o.str();
}

}

namespace feed {

std::string GenerateFeedID()
{
  // Return something like "urn:uuid:60a76c80-d399-11d9-b93C-0003939e0af6"
  return "urn:uuid:" + GetRandomHexCharacters(8) + "-" + GetRandomHexCharacters(4) + "-" + GetRandomHexCharacters(4) + "-" + GetRandomHexCharacters(4) + "-" + GetRandomHexCharacters(12);
}

class cXMLFileWriter {
public:
  cXMLFileWriter();
  ~cXMLFileWriter();

  bool Open(const std::string& file_path);
  void Close();

  bool BeginDocument();
  bool EndDocument();

  bool BeginElement(const std::string& name);
  bool EndElement();

  bool WriteElementNamespace(const std::string& name, const std::string& value);
  bool WriteElementAttribute(const std::string& name, const std::string& value);
  bool WriteElementWithContent(const std::string& name, const std::string& content);

private:
  xmlTextWriterPtr writer;
};

cXMLFileWriter::cXMLFileWriter() :
  writer(nullptr)
{
}

cXMLFileWriter::~cXMLFileWriter()
{
  if (writer != nullptr) {
    xmlFreeTextWriter(writer);
    writer = nullptr;
  }
}

bool cXMLFileWriter::Open(const std::string& file_path)
{
  Close();

  writer = xmlNewTextWriterFilename("./output.xml", 0);
  return (writer != nullptr);
}

void cXMLFileWriter::Close()
{
  if (writer != nullptr) {
    xmlFreeTextWriter(writer);
    writer = nullptr;
  }
}

bool cXMLFileWriter::BeginDocument()
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

bool cXMLFileWriter::EndDocument()
{
  const int result = xmlTextWriterEndDocument(writer);
  return (result >= 0);
}

bool cXMLFileWriter::BeginElement(const std::string& name)
{
  const int result = xmlTextWriterStartElement(writer, BAD_CAST name.c_str());
  return (result >= 0);
}

bool cXMLFileWriter::EndElement()
{
  const int result = xmlTextWriterEndElement(writer);
  return (result >= 0);
}

bool cXMLFileWriter::WriteElementNamespace(const std::string& name, const std::string& value)
{
  const int result = xmlTextWriterWriteAttributeNS(writer, nullptr, BAD_CAST name.c_str(), nullptr, BAD_CAST value.c_str());
  return (result >= 0);
}

bool cXMLFileWriter::WriteElementAttribute(const std::string& name, const std::string& value)
{
  const int result = xmlTextWriterWriteAttribute(writer, BAD_CAST name.c_str(), BAD_CAST value.c_str());
  return (result >= 0);
}

bool cXMLFileWriter::WriteElementWithContent(const std::string& name, const std::string& content)
{
  const int result = xmlTextWriterWriteElement(writer, BAD_CAST name.c_str(), BAD_CAST content.c_str());
  return (result >= 0);
}

/*
  <entry>
    <title>Atom-Powered Robots Run Amok</title>
    <link href="http://example.org/2003/12/13/atom03"/>
    <id>urn:uuid:1225c695-cfb8-4ebb-aaaa-80da344efa6a</id>
    <updated>2003-12-13T18:30:02Z</updated>
    <summary>Some text.</summary>
  </entry>
*/
bool AddEntry(cXMLFileWriter& writer)
{
  // Start the entry element
  if (!writer.BeginElement("entry")) {
    std::cerr<<"Failed to start entry element"<<std::endl;
    return false;
  }

  // Write the title element
  if (!writer.WriteElementWithContent("title", "Atom-Powered Robots Run Amok")) {
    std::cerr<<"Failed to write title element"<<std::endl;
    return false;
  }

  // Write the link element
  if (!writer.BeginElement("link")) {
    std::cerr<<"Failed to begin link element"<<std::endl;
    return false;
  }
  if (!writer.WriteElementAttribute("href", "http://example.org/2003/12/13/atom03")) {
    std::cerr<<"Failed to add link href attribute"<<std::endl;
    return false;
  }
  if (!writer.EndElement()) {
    std::cerr<<"Failed to end link element"<<std::endl;
    return false;
  }

  // Write the id element
  if (!writer.WriteElementWithContent("id", GenerateFeedID())) {
    std::cerr<<"Failed to write id element"<<std::endl;
    return false;
  }

  if (!writer.WriteElementWithContent("updated", "2003-12-13T18:30:02Z")) {
    std::cerr<<"Failed to write child XML element"<<std::endl;
    return false;
  }

  // Write the summary element
  if (!writer.WriteElementWithContent("summary", "Some text.")) {
    std::cerr<<"Failed to write summary element"<<std::endl;
    return false;
  }

  // End the entry element
  if (!writer.EndElement()) {
    std::cerr<<"Failed to end entry element"<<std::endl;
    return false;
  }

  return true;
}

/*
<?xml version="1.0" encoding="utf-8"?>
<feed xmlns="http://www.w3.org/2005/Atom">

  <title>Example Feed</title>
  <link href="http://example.org/"/>
  <updated>2003-12-13T18:30:02Z</updated>
  <author>
    <name>John Doe</name>
  </author>
  <id>urn:uuid:60a76c80-d399-11d9-b93C-0003939e0af6</id>

  <entry>
    <title>Atom-Powered Robots Run Amok</title>
    <link href="http://example.org/2003/12/13/atom03"/>
    <id>urn:uuid:1225c695-cfb8-4ebb-aaaa-80da344efa6a</id>
    <updated>2003-12-13T18:30:02Z</updated>
    <summary>Some text.</summary>
  </entry>

</feed>
*/
bool WriteFeedXML(std::ostringstream& output)
{
  output.clear();

  cXMLFileWriter writer;

  // Create a new XML writer context
  if (!writer.Open("./output.xml")) {
    std::cerr<<"Failed to create XML writer context"<<std::endl;
    return false;
  }

  if (!writer.BeginDocument()) {
    std::cerr<<"Failed to write XML declaration"<<std::endl;
    return false;
  }


  // Start the feed element
  if (!writer.BeginElement("feed")) {
    std::cerr<<"Failed to start feed element"<<std::endl;
    return false;
  }
  if (!writer.WriteElementNamespace("xmlns", "http://www.w3.org/2005/Atom")) {
    std::cerr<<"Failed to add feed namespace element"<<std::endl;
    return false;
  }

  // Write the title element
  if (!writer.WriteElementWithContent("title", "Example Feed")) {
    std::cerr<<"Failed to write child XML element"<<std::endl;
    return false;
  }

  // Write the link element
  if (!writer.BeginElement("link")) {
    std::cerr<<"Failed to begin link element"<<std::endl;
    return false;
  }
  if (!writer.WriteElementAttribute("href", "http://example.org/")) {
    std::cerr<<"Failed to add link href attribute"<<std::endl;
    return false;
  }
  if (!writer.EndElement()) {
    std::cerr<<"Failed to end link element"<<std::endl;
    return false;
  }

  if (!writer.WriteElementWithContent("updated", "2003-12-13T18:30:02Z")) {
    std::cerr<<"Failed to write child XML element"<<std::endl;
    return false;
  }

  // Author
  if (!writer.BeginElement("author")) {
    std::cerr<<"Failed to start author element"<<std::endl;
    return false;
  }

  if (!writer.WriteElementWithContent("name", "John Doe")) {
    std::cerr<<"Failed to write name element"<<std::endl;
    return false;
  }

  if (!writer.EndElement()) {
    std::cerr<<"Failed to end author element"<<std::endl;
    return false;
  }

  if (!writer.WriteElementWithContent("id", GenerateFeedID())) {
    std::cerr<<"Failed to write id element"<<std::endl;
    return false;
  }

  AddEntry(writer);
  AddEntry(writer);

  // End the feed element
  if (!writer.EndElement()) {
    std::cerr<<"Failed to end feed element"<<std::endl;
    return false;
  }

  // End the XML document
  if (!writer.EndDocument()) {
    std::cerr<<"Failed to end document"<<std::endl;
    return false;
  }

  return true;
}

}
