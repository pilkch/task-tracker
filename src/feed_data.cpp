#include <cstring>

#include <iostream>

#include <json-c/json.h>

#include "atom_feed.h"
#include "feed_data.h"
#include "json.h"
#include "random.h"
#include "util.h"

namespace tasktracker {

std::mutex mutex_feed_data;
cFeedData feed_data;

bool LoadFeedDataFromFile(const std::string& file_path, const std::string& external_url)
{
  {
    util::cPseudoRandomNumberGenerator rng;

    // Set the default feed properties
    std::lock_guard<std::mutex> lock(mutex_feed_data);
    feed_data.properties.title = "My Feed";
    feed_data.properties.link = external_url + "feed/atom.xml";
    feed_data.properties.date_updated = util::GetTime();
    feed_data.properties.author_name = "My Name";
    feed_data.properties.id = feed::GenerateFeedID(rng);

    // Clear the feed entries
    feed_data.entries.clear();


    // Load the feed json file, this is best effort, if it doesn't exist or has an error that is ok
    const size_t nMaxFileSizeBytes = 20 * 1024;
    std::string contents;
    if (util::ReadFileIntoString(file_path, nMaxFileSizeBytes, contents)) {
      std::cerr<<"File \""<<file_path<<"\" not found"<<std::endl;
      return false;
    }


    json::cJSONDocument document(json_tokener_parse(contents.c_str()));
    if (!document.IsValid()) {
      std::cerr<<"Invalid JSON config \""<<file_path<<"\""<<std::endl;
      return false;
    }

    // Parse the JSON tree

    // Parse "feed"
    json_object_object_foreach(document.Get(), feed_key, feed_val) {
      enum json_type type_feed = json_object_get_type(feed_val);
      if ((type_feed != json_type_object) || (strcmp(feed_key, "feed") != 0)) {
        std::cerr<<"feed object not found"<<std::endl;
        return false;
      }

      json_object_object_foreach(feed_val, child_key, child_val) {
        enum json_type type_child = json_object_get_type(child_val);
        if (type_child != json_type_object) {
          // Skip it
          continue;
        }

        if (strcmp(child_key, "properties") != 0) {
          // Parse the feed properties
          json::JSONParseString(feed_val, "title", feed_data.properties.title);

          uint64_t date_updated_ms = 0;
          json::JSONParseUint64(feed_val, "date_updated", date_updated_ms);
          feed_data.properties.date_updated = std::chrono::time_point<std::chrono::system_clock>(std::chrono::milliseconds(date_updated_ms));

          json::JSONParseString(feed_val, "author_name", feed_data.properties.author_name);
          json::JSONParseString(feed_val, "id", feed_data.properties.id);
        } else if (strcmp(child_key, "entries") != 0) {
          // Parse the feed entries
          cFeedEntry entry;

          json::JSONParseString(feed_val, "title", entry.title);
          json::JSONParseString(feed_val, "link", entry.link);
          json::JSONParseString(feed_val, "summary", entry.summary);

          uint64_t date_updated_ms = 0;
          json::JSONParseUint64(feed_val, "date_updated", date_updated_ms);
          entry.date_updated = std::chrono::time_point<std::chrono::system_clock>(std::chrono::milliseconds(date_updated_ms));

          json::JSONParseString(feed_val, "id", entry.id);

          feed_data.entries.push_back(entry);
        }
      }
    }
  }

  return true;
}

bool SaveFeedDataToFile(const std::string& file_path)
{
  std::lock_guard<std::mutex> lock(mutex_feed_data);

	struct json_object* jobj = json_object_new_object();


	struct json_object* properties = json_object_new_object();
	json_object_object_add(properties, "title", json_object_new_string(feed_data.properties.title.c_str()));
  auto time_ms = std::chrono::time_point_cast<std::chrono::milliseconds>(feed_data.properties.date_updated);
  auto since_epoch = time_ms.time_since_epoch();
	json_object_object_add(properties, "date_updated", json_object_new_int64(since_epoch.count()));
	json_object_object_add(properties, "author_name", json_object_new_string(feed_data.properties.author_name.c_str()));
	json_object_object_add(properties, "id", json_object_new_string(feed_data.properties.id.c_str()));

	json_object_object_add(jobj, "properties", properties);


	struct json_object* entries = json_object_new_array();
  const size_t nentries = feed_data.entries.size();
  for (size_t i = 0; i < nentries; i++) {
    const cFeedEntry& entry = feed_data.entries[i];
	  struct json_object* obj_entry = json_object_new_object();

    json_object_object_add(obj_entry, "title", json_object_new_string(entry.title.c_str()));
    json_object_object_add(obj_entry, "link", json_object_new_string(entry.link.c_str()));
    json_object_object_add(obj_entry, "summary", json_object_new_string(entry.summary.c_str()));

    auto time_ms = std::chrono::time_point_cast<std::chrono::milliseconds>(entry.date_updated);
    auto since_epoch = time_ms.time_since_epoch();
    json_object_object_add(obj_entry, "date_updated", json_object_new_int64(since_epoch.count()));

    json_object_object_add(obj_entry, "id", json_object_new_string(entry.id.c_str()));

	  json_object_array_add(entries, obj_entry);
  }
	json_object_object_add(jobj, "entries", entries);

  const std::string json_output = std::string(json_object_to_json_string_ext(jobj, JSON_C_TO_STRING_SPACED | JSON_C_TO_STRING_PRETTY)) + "\n";
  //std::cout<<"JSON: "<<json_output<<std::endl;
  util::WriteStringToFileAtomic(file_path, json_output);
 
	json_object_put(jobj); // Delete the json object

  return true;
}

}
