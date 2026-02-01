#ifndef EMOJI_H
#define EMOJI_H

#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include "simdjson.h"

struct EmojiEntry {
    std::string trigger;
    std::string output;

    bool        operator<(const EmojiEntry& other) const {
        return trigger < other.trigger;
    }
};

class EmojiLoader {
  private:
    std::vector<EmojiEntry> emojiList;

    std::string             codepointToUTF8(uint32_t cp) {
        std::string out;
        if (cp <= 0x7F) {
            out += static_cast<char>(cp);
        } else if (cp <= 0x7FF) {
            out += static_cast<char>(0xC0 | ((cp >> 6) & 0x1F));
            out += static_cast<char>(0x80 | (cp & 0x3F));
        } else if (cp <= 0xFFFF) {
            out += static_cast<char>(0xE0 | ((cp >> 12) & 0x0F));
            out += static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
            out += static_cast<char>(0x80 | (cp & 0x3F));
        } else if (cp <= 0x10FFFF) {
            out += static_cast<char>(0xF0 | ((cp >> 18) & 0x07));
            out += static_cast<char>(0x80 | ((cp >> 12) & 0x3F));
            out += static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
            out += static_cast<char>(0x80 | (cp & 0x3F));
        }
        return out;
    }

    std::string parseHexToEmoji(std::string_view hexStr) {
        std::string       result;
        std::stringstream ss;
        std::string       segment;

        std::string       raw(hexStr);
        std::stringstream rawStream(raw);

        while (std::getline(rawStream, segment, '-')) {
            uint32_t          cp;
            std::stringstream converter;
            converter << std::hex << segment;
            converter >> cp;
            result += codepointToUTF8(cp);
        }
        return result;
    }

    void sortData() {
        std::sort(emojiList.begin(), emojiList.end());
    }

  public:
    EmojiLoader() = default;

    bool load(const std::string& filename) {
        simdjson::dom::parser  parser;
        simdjson::dom::element root;

        try {
            root = parser.load(filename);
        } catch (simdjson::simdjson_error& e) {
            std::cerr << "JSON Load Error: " << e.what() << std::endl;
            return false;
        }

        if (!root.is_object())
            return false;

        simdjson::dom::object rootObj = root;
        for (auto [key, value] : rootObj) {
            if (!value.is_object())
                continue;
            simdjson::dom::object emojiData = value;

            std::string_view      hexCode;
            if (emojiData["code_points"]["output"].get(hexCode) != simdjson::SUCCESS) {
                continue;
            }
            std::string      emojiChar = parseHexToEmoji(hexCode);

            std::string_view shortname;
            if (emojiData["shortname"].get(shortname) == simdjson::SUCCESS) {
                emojiList.push_back({std::string(shortname), emojiChar});
            }

            simdjson::dom::array alternates;
            if (emojiData["shortname_alternates"].get(alternates) == simdjson::SUCCESS) {
                for (auto alt : alternates) {
                    std::string_view altStr;
                    if (alt.get(altStr) == simdjson::SUCCESS) {
                        emojiList.push_back({std::string(altStr), emojiChar});
                    }
                }
            }

            simdjson::dom::array asciiArr;
            if (emojiData["ascii"].get(asciiArr) == simdjson::SUCCESS) {
                for (auto asc : asciiArr) {
                    std::string_view ascStr;
                    if (asc.get(ascStr) == simdjson::SUCCESS) {
                        emojiList.push_back({std::string(ascStr), emojiChar});
                    }
                }
            }
        }

        sortData();
        return true;
    }

    std::vector<EmojiEntry> search(const std::string& prefix, size_t numResult) {
        std::vector<EmojiEntry> results;

        if (emojiList.empty())
            return results;

        EmojiEntry target;
        target.trigger = prefix;

        auto it = std::lower_bound(emojiList.begin(), emojiList.end(), target);

        for (; it != emojiList.end(); ++it) {
            if (results.size() >= numResult)
                break;

            if (it->trigger.compare(0, prefix.size(), prefix) != 0) {
                break;
            }

            results.push_back(*it);
        }

        return results;
    }

    size_t size() const {
        return emojiList.size();
    }
};

#endif // EMOJI_H