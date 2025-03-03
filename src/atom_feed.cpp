#include <iostream>
#include <sstream>
#include <string>

#include "atom_feed.h"
#include "util.h"
#include "xml_string_writer.h"

namespace {

const char hex_lookup[] = {
  '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'
};

char GetRandomHexCharacter(util::cPseudoRandomNumberGenerator& rng)
{
  // Return a pseudorandom alphanumeric character
  return hex_lookup[rng.random(16)];
}

std::string GetRandomHexCharacters(util::cPseudoRandomNumberGenerator& rng, size_t length)
{
  std::ostringstream o;
  for (size_t i = 0; i < length; i++) {
    o<<GetRandomHexCharacter(rng);
  }
  return o.str();
}

}

namespace feed {

std::string GenerateFeedID(util::cPseudoRandomNumberGenerator& rng)
{
  // Return something like "urn:uuid:60a76c80-d399-11d9-b93C-0003939e0af6"
  return "urn:uuid:" + GetRandomHexCharacters(rng, 8) + "-" + GetRandomHexCharacters(rng, 4) + "-" + GetRandomHexCharacters(rng, 4) + "-" + GetRandomHexCharacters(rng, 4) + "-" + GetRandomHexCharacters(rng, 12);
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
bool WriteFeedXMLEntry(util::cXMLStringWriter& writer, const tasktracker::cFeedEntry& entry)
{
  // Start the entry element
  if (!writer.BeginElement("entry")) {
    std::cerr<<"Failed to start entry element"<<std::endl;
    return false;
  }

  // Write the title element
  if (!writer.WriteElementWithContent("title", entry.title)) {
    std::cerr<<"Failed to write title element"<<std::endl;
    return false;
  }

  // Write the link element
  if (!writer.BeginElement("link")) {
    std::cerr<<"Failed to begin link element"<<std::endl;
    return false;
  }
  if (!writer.WriteElementAttribute("href", entry.link)) {
    std::cerr<<"Failed to add link href attribute"<<std::endl;
    return false;
  }
  if (!writer.EndElement()) {
    std::cerr<<"Failed to end link element"<<std::endl;
    return false;
  }

  // Write the id element
  if (!writer.WriteElementWithContent("id", entry.id)) {
    std::cerr<<"Failed to write id element"<<std::endl;
    return false;
  }

  // TODO: Convert entry.time to the date time
  if (!writer.WriteElementWithContent("updated", util::GetDateTimeUTCISO8601(entry.date_updated))) {
    std::cerr<<"Failed to write child XML element"<<std::endl;
    return false;
  }

  // Write the summary element
  if (!writer.WriteElementWithContent("summary", entry.summary)) {
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
bool WriteFeedXML(const tasktracker::cFeedData& feed_data, std::ostringstream& output)
{
  output.clear();

  util::cXMLStringWriter writer;

  // Create a new XML writer context
  if (!writer.Open()) {
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
  if (!writer.WriteElementWithContent("title", feed_data.properties.title)) {
    std::cerr<<"Failed to write child XML element"<<std::endl;
    return false;
  }

  // Write the link element
  if (!writer.BeginElement("link")) {
    std::cerr<<"Failed to begin link element"<<std::endl;
    return false;
  }
  if (!writer.WriteElementAttribute("href", feed_data.properties.link)) {
    std::cerr<<"Failed to add link href attribute"<<std::endl;
    return false;
  }
  if (!writer.EndElement()) {
    std::cerr<<"Failed to end link element"<<std::endl;
    return false;
  }

  if (!writer.WriteElementWithContent("updated", util::GetDateTimeUTCISO8601(feed_data.properties.date_updated))) {
    std::cerr<<"Failed to write child XML element"<<std::endl;
    return false;
  }

  // Author
  if (!writer.BeginElement("author")) {
    std::cerr<<"Failed to start author element"<<std::endl;
    return false;
  }

  if (!writer.WriteElementWithContent("name", feed_data.properties.author_name)) {
    std::cerr<<"Failed to write name element"<<std::endl;
    return false;
  }

  if (!writer.EndElement()) {
    std::cerr<<"Failed to end author element"<<std::endl;
    return false;
  }

  if (!writer.WriteElementWithContent("id", feed_data.properties.id)) {
    std::cerr<<"Failed to write id element"<<std::endl;
    return false;
  }

  // NOTE: We actually want to output the feed data in reverse order, new events are at the top of the feed, older items drop off the end
  const size_t nentries = feed_data.entries.size();
  for (size_t i = 0; i < nentries; i++) {
    WriteFeedXMLEntry(writer, feed_data.entries[(nentries - i) - 1]);
  }
 
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

  output<<writer.GetOutput();

  return true;
}

}
