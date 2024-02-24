#include "Vietnamese.h"

#include "tstring.h"

#include <unordered_map>
#include <cassert>

namespace viet {

namespace internal {

const std::array<std::string, TABLE_LENGTH> utf8Table = {
    "\xc3\x80",     "\xc3\x81",     "\xc3\x82",     "\xc3\x83",     "\xc3\x88",     "\xc3\x89",     "\xc3\x8a",
    "\xc3\x8c",     "\xc3\x8d",     "\xc3\x92",     "\xc3\x93",     "\xc3\x94",     "\xc3\x95",     "\xc3\x99",
    "\xc3\x9a",     "\xc3\x9d",     "\xc3\xa0",     "\xc3\xa1",     "\xc3\xa2",     "\xc3\xa3",     "\xc3\xa8",
    "\xc3\xa9",     "\xc3\xaa",     "\xc3\xac",     "\xc3\xad",     "\xc3\xb2",     "\xc3\xb3",     "\xc3\xb4",
    "\xc3\xb5",     "\xc3\xb9",     "\xc3\xba",     "\xc3\xbd",     "\xc4\x82",     "\xc4\x83",     "\xc4\x90",
    "\xc4\x91",     "\xc4\xa8",     "\xc4\xa9",     "\xc5\xa8",     "\xc5\xa9",     "\xc6\xa0",     "\xc6\xa1",
    "\xc6\xaf",     "\xc6\xb0",     "\xe1\xba\xa0", "\xe1\xba\xa1", "\xe1\xba\xa2", "\xe1\xba\xa3", "\xe1\xba\xa4",
    "\xe1\xba\xa5", "\xe1\xba\xa6", "\xe1\xba\xa7", "\xe1\xba\xa8", "\xe1\xba\xa9", "\xe1\xba\xaa", "\xe1\xba\xab",
    "\xe1\xba\xac", "\xe1\xba\xad", "\xe1\xba\xae", "\xe1\xba\xaf", "\xe1\xba\xb0", "\xe1\xba\xb1", "\xe1\xba\xb2",
    "\xe1\xba\xb3", "\xe1\xba\xb4", "\xe1\xba\xb5", "\xe1\xba\xb6", "\xe1\xba\xb7", "\xe1\xba\xb8", "\xe1\xba\xb9",
    "\xe1\xba\xba", "\xe1\xba\xbb", "\xe1\xba\xbc", "\xe1\xba\xbd", "\xe1\xba\xbe", "\xe1\xba\xbf", "\xe1\xbb\x80",
    "\xe1\xbb\x81", "\xe1\xbb\x82", "\xe1\xbb\x83", "\xe1\xbb\x84", "\xe1\xbb\x85", "\xe1\xbb\x86", "\xe1\xbb\x87",
    "\xe1\xbb\x88", "\xe1\xbb\x89", "\xe1\xbb\x8a", "\xe1\xbb\x8b", "\xe1\xbb\x8c", "\xe1\xbb\x8d", "\xe1\xbb\x8e",
    "\xe1\xbb\x8f", "\xe1\xbb\x90", "\xe1\xbb\x91", "\xe1\xbb\x92", "\xe1\xbb\x93", "\xe1\xbb\x94", "\xe1\xbb\x95",
    "\xe1\xbb\x96", "\xe1\xbb\x97", "\xe1\xbb\x98", "\xe1\xbb\x99", "\xe1\xbb\x9a", "\xe1\xbb\x9b", "\xe1\xbb\x9c",
    "\xe1\xbb\x9d", "\xe1\xbb\x9e", "\xe1\xbb\x9f", "\xe1\xbb\xa0", "\xe1\xbb\xa1", "\xe1\xbb\xa2", "\xe1\xbb\xa3",
    "\xe1\xbb\xa4", "\xe1\xbb\xa5", "\xe1\xbb\xa6", "\xe1\xbb\xa7", "\xe1\xbb\xa8", "\xe1\xbb\xa9", "\xe1\xbb\xaa",
    "\xe1\xbb\xab", "\xe1\xbb\xac", "\xe1\xbb\xad", "\xe1\xbb\xae", "\xe1\xbb\xaf", "\xe1\xbb\xb0", "\xe1\xbb\xb1",
    "\xe1\xbb\xb2", "\xe1\xbb\xb3", "\xe1\xbb\xb4", "\xe1\xbb\xb5", "\xe1\xbb\xb6", "\xe1\xbb\xb7", "\xe1\xbb\xb8",
    "\xe1\xbb\xb9",
};

const std::array<uint16_t, TABLE_LENGTH> utf16LETable = {
    L'\u00C0', L'\u00C1', L'\u00C2', L'\u00C3', L'\u00C8', L'\u00C9', L'\u00CA', L'\u00CC', L'\u00CD', L'\u00D2',
    L'\u00D3', L'\u00D4', L'\u00D5', L'\u00D9', L'\u00DA', L'\u00DD', L'\u00E0', L'\u00E1', L'\u00E2', L'\u00E3',
    L'\u00E8', L'\u00E9', L'\u00EA', L'\u00EC', L'\u00ED', L'\u00F2', L'\u00F3', L'\u00F4', L'\u00F5', L'\u00F9',
    L'\u00FA', L'\u00FD', L'\u0102', L'\u0103', L'\u0110', L'\u0111', L'\u0128', L'\u0129', L'\u0168', L'\u0169',
    L'\u01A0', L'\u01A1', L'\u01AF', L'\u01B0', L'\u1EA0', L'\u1EA1', L'\u1EA2', L'\u1EA3', L'\u1EA4', L'\u1EA5',
    L'\u1EA6', L'\u1EA7', L'\u1EA8', L'\u1EA9', L'\u1EAA', L'\u1EAB', L'\u1EAC', L'\u1EAD', L'\u1EAE', L'\u1EAF',
    L'\u1EB0', L'\u1EB1', L'\u1EB2', L'\u1EB3', L'\u1EB4', L'\u1EB5', L'\u1EB6', L'\u1EB7', L'\u1EB8', L'\u1EB9',
    L'\u1EBA', L'\u1EBB', L'\u1EBC', L'\u1EBD', L'\u1EBE', L'\u1EBF', L'\u1EC0', L'\u1EC1', L'\u1EC2', L'\u1EC3',
    L'\u1EC4', L'\u1EC5', L'\u1EC6', L'\u1EC7', L'\u1EC8', L'\u1EC9', L'\u1ECA', L'\u1ECB', L'\u1ECC', L'\u1ECD',
    L'\u1ECE', L'\u1ECF', L'\u1ED0', L'\u1ED1', L'\u1ED2', L'\u1ED3', L'\u1ED4', L'\u1ED5', L'\u1ED6', L'\u1ED7',
    L'\u1ED8', L'\u1ED9', L'\u1EDA', L'\u1EDB', L'\u1EDC', L'\u1EDD', L'\u1EDE', L'\u1EDF', L'\u1EE0', L'\u1EE1',
    L'\u1EE2', L'\u1EE3', L'\u1EE4', L'\u1EE5', L'\u1EE6', L'\u1EE7', L'\u1EE8', L'\u1EE9', L'\u1EEA', L'\u1EEB',
    L'\u1EEC', L'\u1EED', L'\u1EEE', L'\u1EEF', L'\u1EF0', L'\u1EF1', L'\u1EF2', L'\u1EF3', L'\u1EF4', L'\u1EF5',
    L'\u1EF6', L'\u1EF7', L'\u1EF8', L'\u1EF9',
};

// overlapped with ASCII
const std::array<std::string, internal::TABLE_LENGTH> vniTable = {
    "\x41\xD8", "\x41\xD9", "\x41\xC2", "\x41\xD5", "\x45\xD8", "\x45\xD9", "\x45\xC2", "\xCC",     "\xCD",
    "\x4F\xD8", "\x4F\xD9", "\x4F\xC2", "\x4F\xD5", "\x55\xD8", "\x55\xD9", "\x59\xD9", "\x61\xF8", "\x61\xF9",
    "\x61\xE2", "\x61\xF5", "\x65\xF8", "\x65\xF9", "\x65\xE2", "\xEC",     "\xED",     "\x6F\xF8", "\x6F\xF9",
    "\x6F\xE2", "\x6F\xF5", "\x75\xF8", "\x75\xF9", "\x79\xF9", "\x41\xCA", "\x61\xEA", "\xD1",     "\xF1",
    "\xD3",     "\xF3",     "\x55\xD5", "\x75\xF5", "\xD4",     "\xF4",     "\xD6",     "\xF6",     "\x41\xCF",
    "\x61\xEF", "\x41\xDB", "\x61\xFB", "\x41\xC1", "\x61\xE1", "\x41\xC0", "\x61\xE0", "\x41\xC5", "\x61\xE5",
    "\x41\xC3", "\x61\xE3", "\x41\xC4", "\x61\xE4", "\x41\xC9", "\x61\xE9", "\x41\xC8", "\x61\xE8", "\x41\xDA",
    "\x61\xFA", "\x41\xDC", "\x61\xFC", "\x41\xCB", "\x61\xEB", "\x45\xCF", "\x65\xEF", "\x45\xDB", "\x65\xFB",
    "\x45\xD5", "\x65\xF5", "\x45\xC1", "\x65\xE1", "\x45\xC0", "\x65\xE0", "\x45\xC5", "\x65\xE5", "\x45\xC3",
    "\x65\xE3", "\x45\xC4", "\x65\xE4", "\xC6",     "\xE6",     "\xD2",     "\xF2",     "\x4F\xCF", "\x6F\xEF",
    "\x4F\xDB", "\x6F\xFB", "\x4F\xC1", "\x6F\xE1", "\x4F\xC0", "\x6F\xE0", "\x4F\xC5", "\x6F\xE5", "\x4F\xC3",
    "\x6F\xE3", "\x4F\xC4", "\x6F\xE4", "\xD4\xD9", "\xF4\xF9", "\xD4\xD8", "\xF4\xF8", "\xD4\xDB", "\xF4\xFB",
    "\xD4\xD5", "\xF4\xF5", "\xD4\xCF", "\xF4\xEF", "\x55\xCF", "\x75\xEF", "\x55\xDB", "\x75\xFB", "\xD6\xD9",
    "\xF6\xF9", "\xD6\xD8", "\xF6\xF8", "\xD6\xDB", "\xF6\xFB", "\xD6\xD5", "\xF6\xF5", "\xD6\xCF", "\xF6\xEF",
    "\x59\xD8", "\x79\xF8", "\xCE",     "\xEE",     "\x59\xDB", "\x79\xFB", "\x59\xD5", "\x79\xF5",
};

// overlapped with ASCII
const std::array<char, internal::TABLE_LENGTH> vpsTable = {
    '\x80', '\xC1', '\xC2', '\x82', '\xD7', '\xC9', '\xCA', '\xB5', '\xB4', '\xBC', '\xB9', '\xD4', '\xBE', '\xA8',
    '\xDA', '\xDD', '\xE0', '\xE1', '\xE2', '\xE3', '\xE8', '\xE9', '\xEA', '\xEC', '\xED', '\xF2', '\xF3', '\xF4',
    '\xF5', '\xF9', '\xFA', '\x9A', '\x88', '\xE6', '\xF1', '\xC7', '\xB8', '\xEF', '\xAC', '\xDB', '\xF7', '\xD6',
    '\xD0', '\xDC', '\x2',  '\xE5', '\x81', '\xE4', '\x83', '\xC3', '\x84', '\xC0', '\x85', '\xC4', '\x1C', '\xC5',
    '\x3',  '\xC6', '\x8D', '\xA1', '\x8E', '\xA2', '\x8F', '\xA3', '\xF0', '\xA4', '\x4',  '\xA5', '\x5',  '\xCB',
    '\xDE', '\xC8', '\xFE', '\xEB', '\x90', '\x89', '\x93', '\x8A', '\x94', '\x8B', '\x95', '\xCD', '\x6',  '\x8C',
    '\xB7', '\xCC', '\x10', '\xCE', '\x11', '\x86', '\xBD', '\xD5', '\x96', '\xD3', '\x97', '\xD2', '\x98', '\xB0',
    '\x99', '\x87', '\x12', '\xB6', '\x9D', '\xA7', '\x9E', '\xA9', '\x9F', '\xAA', '\xA6', '\xAB', '\x13', '\xAE',
    '\x14', '\xF8', '\xD1', '\xFB', '\xAD', '\xD9', '\xAF', '\xD8', '\xB1', '\xBA', '\x1D', '\xBB', '\x15', '\xBF',
    '\xB2', '\xFF', '\x19', '\x9C', '\xFD', '\x9B', '\xB3', '\xCF',
};

// overlapped with ASCII
const std::array<char, internal::TABLE_LENGTH> visciiTable = {
    '\xC0', '\xC1', '\xC2', '\xC3', '\xC8', '\xC9', '\xCA', '\xCC', '\xCD', '\xD2', '\xD3', '\xD4', '\xA0', '\xD9',
    '\xDA', '\xDD', '\xE0', '\xE1', '\xE2', '\xE3', '\xE8', '\xE9', '\xEA', '\xEC', '\xED', '\xF2', '\xF3', '\xF4',
    '\xF5', '\xF9', '\xFA', '\xFD', '\xC5', '\xE5', '\xD0', '\xF0', '\xCE', '\xEE', '\x9D', '\xFB', '\xB4', '\xBD',
    '\xBF', '\xDF', '\x80', '\xD5', '\xC4', '\xE4', '\x84', '\xA4', '\x85', '\xA5', '\x86', '\xA6', '\x6',  '\xE7',
    '\x87', '\xA7', '\x81', '\xA1', '\x82', '\xA2', '\x2',  '\xC6', '\x5',  '\xC7', '\x83', '\xA3', '\x89', '\xA9',
    '\xCB', '\xEB', '\x88', '\xA8', '\x8A', '\xAA', '\x8B', '\xAB', '\x8C', '\xAC', '\x8D', '\xAD', '\x8E', '\xAE',
    '\x9B', '\xEF', '\x98', '\xB8', '\x9A', '\xF7', '\x99', '\xF6', '\x8F', '\xAF', '\x90', '\xB0', '\x91', '\xB1',
    '\x92', '\xB2', '\x93', '\xB5', '\x95', '\xBE', '\x96', '\xB6', '\x97', '\xB7', '\xB3', '\xDE', '\x94', '\xFE',
    '\x9E', '\xF8', '\x9C', '\xFC', '\xBA', '\xD1', '\xBB', '\xD7', '\xBC', '\xD8', '\xFF', '\xE6', '\xB9', '\xF1',
    '\x9F', '\xCF', '\x1E', '\xDC', '\x14', '\xD6', '\x19', '\xDB',
};

// NOTICE: TCVN3 is not double-byte, but due to the nature of its encoding, capital letters (vowels) are mapped to a
// separate, capital font that is similar to the normal, lowercase one. As such, the hex code and character
// representation of TCVN3 capital letters in the table is not correct—it is shown here for the purpose of illustration.
// Users must take this peculiarity into account when converting files in TCVN3 format to Unicode; some post-conversion
// editing is necessary.
const std::array<std::string, TABLE_LENGTH> tcvn3Table = {
    "\x41\xB5", "\x41\xB8", "\xA2",     "\x41\xB7", "\x45\xCC", "\x45\xD0", "\xA3",     "\x49\xD7", "\x49\xDD",
    "\x4F\xDF", "\x4F\xE3", "\xA4",     "\x4F\xE2", "\x55\xEF", "\x55\xF3", "\x59\xFD", "\xB5",     "\xB8",
    "\xA9",     "\xB7",     "\xCC",     "\xD0",     "\xAA",     "\xD7",     "\xDD",     "\xDF",     "\xE3",
    "\xAB",     "\xE2",     "\xEF",     "\xF3",     "\xFD",     "\xA1",     "\xA8",     "\xA7",     "\xAE",
    "\x49\xDC", "\xDC",     "\x55\xF2", "\xF2",     "\xA5",     "\xAC",     "\xA6",     "\xAD",     "\x41\xB9",
    "\xB9",     "\x41\xB6", "\xB6",     "\xA2\xCA", "\xCA",     "\xA2\xC7", "\xC7",     "\xA2\xC8", "\xC8",
    "\xA2\xC9", "\xC9",     "\xA2\xCB", "\xCB",     "\xA1\xBE", "\xBE",     "\xA1\xBB", "\xBB",     "\xA1\xBC",
    "\xBC",     "\xA1\xBD", "\xBD",     "\xA1\xC6", "\xC6",     "\x45\xD1", "\xD1",     "\x45\xCE", "\xCE",
    "\x45\xCF", "\xCF",     "\xA3\xD5", "\xD5",     "\xA3\xD2", "\xD2",     "\xA3\xD3", "\xD3",     "\xA3\xD4",
    "\xD4",     "\xA3\xD6", "\xD6",     "\x49\xD8", "\xD8",     "\x49\xDE", "\xDE",     "\x4F\xE4", "\xE4",
    "\x4F\xE1", "\xE1",     "\xA4\xE8", "\xE8",     "\xA4\xE5", "\xE5",     "\xA4\xE6", "\xE6",     "\xA4\xE7",
    "\xE7",     "\xA4\xE9", "\xE9",     "\xA5\xED", "\xED",     "\xA5\xEA", "\xEA",     "\xA5\xEB", "\xEB",
    "\xA5\xEC", "\xEC",     "\xA5\xEE", "\xEE",     "\x55\xF4", "\xF4",     "\x55\xF1", "\xF1",     "\xA6\xF8",
    "\xF8",     "\xA6\xF5", "\xF5",     "\xA6\xF6", "\xF6",     "\xA6\xF7", "\xF7",     "\xA6\xF9", "\xF9",
    "\x59\xFA", "\xFA",     "\x59\xFE", "\xFE",     "\x59\xFB", "\xFB",     "\x59\xFC", "\xFC",
};

const std::array<std::string, internal::TABLE_LENGTH> descriptionTable = {
    "LATIN CAPITAL LETTER A WITH GRAVE",
    "LATIN CAPITAL LETTER A WITH ACUTE",
    "LATIN CAPITAL LETTER A WITH CIRCUMFLEX",
    "LATIN CAPITAL LETTER A WITH TILDE",
    "LATIN CAPITAL LETTER E WITH GRAVE",
    "LATIN CAPITAL LETTER E WITH ACUTE",
    "LATIN CAPITAL LETTER E WITH CIRCUMFLEX",
    "LATIN CAPITAL LETTER I WITH GRAVE",
    "LATIN CAPITAL LETTER I WITH ACUTE",
    "LATIN CAPITAL LETTER O WITH GRAVE",
    "LATIN CAPITAL LETTER O WITH ACUTE",
    "LATIN CAPITAL LETTER O WITH CIRCUMFLEX",
    "LATIN CAPITAL LETTER O WITH TILDE",
    "LATIN CAPITAL LETTER U WITH GRAVE",
    "LATIN CAPITAL LETTER U WITH ACUTE",
    "LATIN CAPITAL LETTER Y WITH ACUTE",
    "LATIN SMALL LETTER A WITH GRAVE",
    "LATIN SMALL LETTER A WITH ACUTE",
    "LATIN SMALL LETTER A WITH CIRCUMFLEX",
    "LATIN SMALL LETTER A WITH TILDE",
    "LATIN SMALL LETTER E WITH GRAVE",
    "LATIN SMALL LETTER E WITH ACUTE",
    "LATIN SMALL LETTER E WITH CIRCUMFLEX",
    "LATIN SMALL LETTER I WITH GRAVE",
    "LATIN SMALL LETTER I WITH ACUTE",
    "LATIN SMALL LETTER O WITH GRAVE",
    "LATIN SMALL LETTER O WITH ACUTE",
    "LATIN SMALL LETTER O WITH CIRCUMFLEX",
    "LATIN SMALL LETTER O WITH TILDE",
    "LATIN SMALL LETTER U WITH GRAVE",
    "LATIN SMALL LETTER U WITH ACUTE",
    "LATIN SMALL LETTER Y WITH ACUTE",
    "LATIN CAPITAL LETTER A WITH BREVE",
    "LATIN SMALL LETTER A WITH BREVE",
    "LATIN CAPITAL LETTER D WITH STROKE",
    "LATIN SMALL LETTER D WITH STROKE",
    "LATIN CAPITAL LETTER I WITH TILDE",
    "LATIN SMALL LETTER I WITH TILDE",
    "LATIN CAPITAL LETTER U WITH TILDE",
    "LATIN SMALL LETTER U WITH TILDE",
    "LATIN CAPITAL LETTER O WITH HORN",
    "LATIN SMALL LETTER O WITH HORN",
    "LATIN CAPITAL LETTER U WITH HORN",
    "LATIN SMALL LETTER U WITH HORN",
    "LATIN CAPITAL LETTER A WITH DOT BELOW",
    "LATIN SMALL LETTER A WITH DOT BELOW",
    "LATIN CAPITAL LETTER A WITH HOOK ABOVE",
    "LATIN SMALL LETTER A WITH HOOK ABOVE",
    "LATIN CAPITAL LETTER A WITH CIRCUMFLEX AND ACUTE",
    "LATIN SMALL LETTER A WITH CIRCUMFLEX AND ACUTE",
    "LATIN CAPITAL LETTER A WITH CIRCUMFLEX AND GRAVE",
    "LATIN SMALL LETTER A WITH CIRCUMFLEX AND GRAVE",
    "LATIN CAPITAL LETTER A WITH CIRCUMFLEX AND HOOK ABOVE",
    "LATIN SMALL LETTER A WITH CIRCUMFLEX AND HOOK ABOVE",
    "LATIN CAPITAL LETTER A WITH CIRCUMFLEX AND TILDE",
    "LATIN SMALL LETTER A WITH CIRCUMFLEX AND TILDE",
    "LATIN CAPITAL LETTER A WITH CIRCUMFLEX AND DOT BELOW",
    "LATIN SMALL LETTER A WITH CIRCUMFLEX AND DOT BELOW",
    "LATIN CAPITAL LETTER A WITH BREVE AND ACUTE",
    "LATIN SMALL LETTER A WITH BREVE AND ACUTE",
    "LATIN CAPITAL LETTER A WITH BREVE AND GRAVE",
    "LATIN SMALL LETTER A WITH BREVE AND GRAVE",
    "LATIN CAPITAL LETTER A WITH BREVE AND HOOK ABOVE",
    "LATIN SMALL LETTER A WITH BREVE AND HOOK ABOVE",
    "LATIN CAPITAL LETTER A WITH BREVE AND TILDE",
    "LATIN SMALL LETTER A WITH BREVE AND TILDE",
    "LATIN CAPITAL LETTER A WITH BREVE AND DOT BELOW",
    "LATIN SMALL LETTER A WITH BREVE AND DOT BELOW",
    "LATIN CAPITAL LETTER E WITH DOT BELOW",
    "LATIN SMALL LETTER E WITH DOT BELOW",
    "LATIN CAPITAL LETTER E WITH HOOK ABOVE",
    "LATIN SMALL LETTER E WITH HOOK ABOVE",
    "LATIN CAPITAL LETTER E WITH TILDE",
    "LATIN SMALL LETTER E WITH TILDE",
    "LATIN CAPITAL LETTER E WITH CIRCUMFLEX AND ACUTE",
    "LATIN SMALL LETTER E WITH CIRCUMFLEX AND ACUTE",
    "LATIN CAPITAL LETTER E WITH CIRCUMFLEX AND GRAVE",
    "LATIN SMALL LETTER E WITH CIRCUMFLEX AND GRAVE",
    "LATIN CAPITAL LETTER E WITH CIRCUMFLEX AND HOOK ABOVE",
    "LATIN SMALL LETTER E WITH CIRCUMFLEX AND HOOK ABOVE",
    "LATIN CAPITAL LETTER E WITH CIRCUMFLEX AND TILDE",
    "LATIN SMALL LETTER E WITH CIRCUMFLEX AND TILDE",
    "LATIN CAPITAL LETTER E WITH CIRCUMFLEX AND DOT BELOW",
    "LATIN SMALL LETTER E WITH CIRCUMFLEX AND DOT BELOW",
    "LATIN CAPITAL LETTER I WITH HOOK ABOVE",
    "LATIN SMALL LETTER I WITH HOOK ABOVE",
    "LATIN CAPITAL LETTER I WITH DOT BELOW",
    "LATIN SMALL LETTER I WITH DOT BELOW",
    "LATIN CAPITAL LETTER O WITH DOT BELOW",
    "LATIN SMALL LETTER O WITH DOT BELOW",
    "LATIN CAPITAL LETTER O WITH HOOK ABOVE",
    "LATIN SMALL LETTER O WITH HOOK ABOVE",
    "LATIN CAPITAL LETTER O WITH CIRCUMFLEX AND ACUTE",
    "LATIN SMALL LETTER O WITH CIRCUMFLEX AND ACUTE",
    "LATIN CAPITAL LETTER O WITH CIRCUMFLEX AND GRAVE",
    "LATIN SMALL LETTER O WITH CIRCUMFLEX AND GRAVE",
    "LATIN CAPITAL LETTER O WITH CIRCUMFLEX AND HOOK ABOVE",
    "LATIN SMALL LETTER O WITH CIRCUMFLEX AND HOOK ABOVE",
    "LATIN CAPITAL LETTER O WITH CIRCUMFLEX AND TILDE",
    "LATIN SMALL LETTER O WITH CIRCUMFLEX AND TILDE",
    "LATIN CAPITAL LETTER O WITH CIRCUMFLEX AND DOT BELOW",
    "LATIN SMALL LETTER O WITH CIRCUMFLEX AND DOT BELOW",
    "LATIN CAPITAL LETTER O WITH HORN AND ACUTE",
    "LATIN SMALL LETTER O WITH HORN AND ACUTE",
    "LATIN CAPITAL LETTER O WITH HORN AND GRAVE",
    "LATIN SMALL LETTER O WITH HORN AND GRAVE",
    "LATIN CAPITAL LETTER O WITH HORN AND HOOK ABOVE",
    "LATIN SMALL LETTER O WITH HORN AND HOOK ABOVE",
    "LATIN CAPITAL LETTER O WITH HORN AND TILDE",
    "LATIN SMALL LETTER O WITH HORN AND TILDE",
    "LATIN CAPITAL LETTER O WITH HORN AND DOT BELOW",
    "LATIN SMALL LETTER O WITH HORN AND DOT BELOW",
    "LATIN CAPITAL LETTER U WITH DOT BELOW",
    "LATIN SMALL LETTER U WITH DOT BELOW",
    "LATIN CAPITAL LETTER U WITH HOOK ABOVE",
    "LATIN SMALL LETTER U WITH HOOK ABOVE",
    "LATIN CAPITAL LETTER U WITH HORN AND ACUTE",
    "LATIN SMALL LETTER U WITH HORN AND ACUTE",
    "LATIN CAPITAL LETTER U WITH HORN AND GRAVE",
    "LATIN SMALL LETTER U WITH HORN AND GRAVE",
    "LATIN CAPITAL LETTER U WITH HORN AND HOOK ABOVE",
    "LATIN SMALL LETTER U WITH HORN AND HOOK ABOVE",
    "LATIN CAPITAL LETTER U WITH HORN AND TILDE",
    "LATIN SMALL LETTER U WITH HORN AND TILDE",
    "LATIN CAPITAL LETTER U WITH HORN AND DOT BELOW",
    "LATIN SMALL LETTER U WITH HORN AND DOT BELOW",
    "LATIN CAPITAL LETTER Y WITH GRAVE",
    "LATIN SMALL LETTER Y WITH GRAVE",
    "LATIN CAPITAL LETTER Y WITH DOT BELOW",
    "LATIN SMALL LETTER Y WITH DOT BELOW",
    "LATIN CAPITAL LETTER Y WITH HOOK ABOVE",
    "LATIN SMALL LETTER Y WITH HOOK ABOVE",
    "LATIN CAPITAL LETTER Y WITH TILDE",
    "LATIN SMALL LETTER Y WITH TILDE",
};

} // namespace internal

std::unordered_map<std::string_view, std::string_view> vniToUtf8;
std::unordered_map<char, std::string_view> vpsToUtf8;
std::unordered_map<char, std::string_view> viscii3ToUtf8;
std::unordered_map<std::string_view, std::string_view> tcvn3ToUtf8;

std::unordered_map<std::string_view, uint16_t> vniToUtf16LE;
std::unordered_map<char, uint16_t> vpsToUtf16LE;
std::unordered_map<char, uint16_t> viscii3ToUtf16LE;
std::unordered_map<std::string_view, uint16_t> tcvn3ToUtf16LE;

struct Rune {
    const std::string_view utf8;
    const uint16_t utf16LE;
    const std::string_view vni;
    char vps;
    char viscii;
    const std::string_view tcvn3;
    const std::string_view description;

    void AddToString(std::string &out, Encoding targetEncoding) const noexcept {
        switch (targetEncoding) {
        case Encoding::VNI:
            out += vni;
            break;
        case Encoding::VPS:
            out += vps;
            break;
        case Encoding::VISCII:
            out += viscii;
            break;
        case Encoding::TCVN3:
            out += tcvn3;
            break;
        default:
            assert(0 && "unsupported encoding");
        }
    }
};

std::unordered_map<std::string_view, Rune> utf8ToOthers;

ConvertError::ConvertError(std::string content, int position, Encoding srcEncoding, Encoding destEncoding) noexcept
    : std::runtime_error("parse error"), content(content), position(position), srcEncoding(srcEncoding),
      destEncoding(destEncoding) {
    errMsg = std::string("[") + to_string(srcEncoding).data() + "->" + to_string(destEncoding).data() +
             "] convert error at position " + std::to_string(position);
    errMsg += "\n";
    errMsg += "with content:\n";
    for (auto c : content) {
        errMsg += "\\x";
        errMsg += to_hex(c);
    }
}

bool &Initialized() noexcept {
    static bool initialized = false;
    return initialized;
}

void Init() noexcept {
    if (Initialized())
        return;

    for (int i = 0; i < internal::TABLE_LENGTH; ++i) {
        vniToUtf8[internal::vniTable[i]] = internal::utf8Table[i];
        vpsToUtf8[internal::vpsTable[i]] = internal::utf8Table[i];
        viscii3ToUtf8[internal::visciiTable[i]] = internal::utf8Table[i];
        tcvn3ToUtf8[internal::tcvn3Table[i]] = internal::utf8Table[i];

        vniToUtf16LE[internal::vniTable[i]] = internal::utf16LETable[i];
        vpsToUtf16LE[internal::vpsTable[i]] = internal::utf16LETable[i];
        viscii3ToUtf16LE[internal::visciiTable[i]] = internal::utf16LETable[i];
        tcvn3ToUtf16LE[internal::tcvn3Table[i]] = internal::utf16LETable[i];

        std::string_view sv = internal::utf8Table[i];

        utf8ToOthers.emplace(internal::utf8Table[i],
                             Rune{internal::utf8Table[i], internal::utf16LETable[i], internal::vniTable[i],
                                  internal::vpsTable[i], internal::visciiTable[i], internal::tcvn3Table[i],
                                  internal::descriptionTable[i]});
    }
    assert(vniToUtf8.size() == internal::TABLE_LENGTH);
    assert(vpsToUtf8.size() == internal::TABLE_LENGTH);
    assert(viscii3ToUtf8.size() == internal::TABLE_LENGTH);
    assert(tcvn3ToUtf8.size() == internal::TABLE_LENGTH);
    assert(vniToUtf16LE.size() == internal::TABLE_LENGTH);
    assert(vpsToUtf16LE.size() == internal::TABLE_LENGTH);
    assert(viscii3ToUtf16LE.size() == internal::TABLE_LENGTH);
    assert(tcvn3ToUtf16LE.size() == internal::TABLE_LENGTH);
    assert(utf8ToOthers.size() == internal::TABLE_LENGTH);

    Initialized() = true;
}

void CheckInit() noexcept {
    assert(Initialized() && "viet module is not initialized");
}

bool CheckEncoding(const char *str, int len, Encoding encoding) noexcept {
    CheckInit();
    if (encoding == Encoding::VPS || encoding == Encoding::VISCII) {
        const std::unordered_map<char, std::string_view> *dict = nullptr;
        switch (encoding) {
        case Encoding::VPS:
            dict = &vpsToUtf8;
            break;
        case Encoding::VISCII:
            dict = &viscii3ToUtf8;
            break;
        }

        for (int i = 0; i < len; ++i) {
            auto c = str[i];
            if (isascii(c)) {
                continue;
            }
            if (dict->find(c) == dict->end()) {
                return false;
            }
        }
        return true;
    }

    const std::unordered_map<std::string_view, std::string_view> *dict = nullptr;
    switch (encoding) {
    case Encoding::VNI:
        dict = &vniToUtf8;
        break;
    case Encoding::TCVN3:
        dict = &tcvn3ToUtf8;
        break;
    default:
        assert(0 && "unsupported encoding");
        break;
    }

    for (std::size_t i = 0; i < len; ++i) {
        char c = str[i];
        if (isascii(c)) {
            continue;
        }

        std::string word(1, c);

        if (dict->find(word) != dict->end()) {
            continue;
        }

        i++;
        if (i == len)
            break;

        word += str[i];
        if (dict->find(word) != dict->end()) {
            continue;
        }

        return false;
    }

    return true;
}

bool CheckEncoding(const std::string &str, Encoding encoding) noexcept {
    return CheckEncoding(str.c_str(), str.size(), encoding);
}

template <typename T>
std::string ConvertTCVN3ToUtfX(std::string_view src) {
    CheckInit();

    std::string ret;
    const std::unordered_map<T, std::string_view> *dict = nullptr;

    if constexpr (std::is_same_v<T, std::string_view>) {
        dict = &tcvn3ToUtf8;
    } else if constexpr (std::is_same_v<T, uint16_t>) {
        dict = &tcvn3ToUtf16LE;
    } else {
        static_assert(0);
    }

    // 虽然TCVN3存在部分2Byte映射的第1个char落在ASCII码表范围内的情况，但是根据
    // https://vietunicode.sourceforge.net/charset/ 的描述，忽略2B的情况并且报错
    for (std::size_t i = 0; i < src.size(); ++i) {
        char c = src[i];
        if (isascii(c)) {
            ret += c;
            continue;
        }

        std::string word(1, c);

        auto iter = dict->find(word);
        if (iter != dict->end()) {
            ret += iter->second;
            continue;
        }

        // 不匹配，报错
        throw ConvertError(word, i, Encoding::TCVN3, Encoding::UTF8);
    }
    return ret;
}

template <typename T>
std::string ConvertVNIToUtfX(std::string_view src) {
    CheckInit();

    std::string ret;
    const std::unordered_map<T, std::string_view> *dict = nullptr;

    if constexpr (std::is_same_v<T, std::string_view>) {
        dict = &vniToUtf8;
    } else if constexpr (std::is_same_v<T, uint16_t>) {
        dict = &vniToUtf16LE;
    } else {
        static_assert(0);
    }

    for (std::size_t i = 0; i < src.size();) {
        // 由于VNI存在第1个char落在ASCII码表范围内的问题，所以先判断2字节
        if (i + 1 < src.size()) {
            std::string_view word = src.substr(i, 2);
            auto iter = dict->find(word);
            if (iter != dict->end()) {
                ret += iter->second;
                i += 2;
                continue;
            }

            // fallthrough
        }

        // 由于VNI存在单个char和ASCII码表重叠的问题，所以先判断
        std::string_view word = src.substr(i, 1);

        auto iter = dict->find(word);
        if (iter != dict->end()) {
            ret += iter->second;
            i++;
            continue;
        }

        char c = src[i];
        if (isascii(c)) {
            ret += c;
            i++;
            continue;
        }

        throw ConvertError(std::string(word), i, Encoding::VNI, Encoding::UTF8);
    }
    return ret;
}

template <typename T, typename ReturnType>
auto ConvertToUtfX(T src, Encoding srcEncoding) -> ReturnType {
    CheckInit();
    if (srcEncoding == Encoding::UTF8) {
        return ReturnType(src);
    }
    if (srcEncoding == Encoding::UTF16LE) {
        return ReturnType(src);
    }

    ReturnType ret;

    if (srcEncoding == Encoding::VPS || srcEncoding == Encoding::VISCII) {
        const std::unordered_map<char, T> *dict = nullptr;
        switch (srcEncoding) {
        case Encoding::VPS:
            if constexpr (std::is_same_v<T, std::string_view>) {
                dict = &vpsToUtf8;
            } else if constexpr (std::is_same_v<T, uint16_t>) {
                dict = &vpsToUtf16LE;
            } else {
                static_assert(0);
            }
            break;
        case Encoding::VISCII:
            if constexpr (std::is_same_v<T, std::string_view>) {
                dict = &viscii3ToUtf8;
            } else if constexpr (std::is_same_v<T, uint16_t>) {
                dict = &visciiToUtf16LE;
            } else {
                static_assert(0);
            }
            break;
        }

        for (int i = 0; i < src.size(); ++i) {
            auto c = src[i];
            // VPS和VISCII覆盖了ASCII的部分码点，应该先查VPS/VISCII表
            auto iter = dict->find(c);
            if (iter != dict->end()) {
                ret += iter->second;
                continue;
            }

            if (isascii(c)) {
                ret += c;
                continue;
            }

            throw ConvertError(std::string(1, c), i, srcEncoding, Encoding::UTF8);
        }
        return ret;
    }

    const std::unordered_map<T, std::string_view> *dict = nullptr;

    if (srcEncoding == Encoding::VNI) {
        return ConvertVNIToUtfX<T>(src);
    }

    if (srcEncoding == Encoding::TCVN3) {
        return ConvertTCVN3ToUtfX<T>(src);
    }

    assert(0 && "unsupported encoding");
    return ret;
}

std::string ConvertToUtf8(std::string_view src, Encoding srcEncoding) {
    return ConvertToUtfX<std::string_view, std::string>(src, srcEncoding);
}

std::string ConvertFromUtf8(std::string_view utf8Str, Encoding destEncoding) {
    CheckInit();
    std::string ret;

    auto srcSize = utf8Str.size();
    for (std::size_t i = 0; i < srcSize;) {
        char c = utf8Str[i];
        if (isascii(c)) {
            ret += c;
            i++;
            continue;
        }

        i++;
        if (i == srcSize)
            break;

        // 2B utf8
        std::string word{utf8Str.substr(i - 1, 2)};
        auto iter = utf8ToOthers.find(word);
        if (iter != utf8ToOthers.end()) {
            iter->second.AddToString(ret, destEncoding);
            i++;
            continue;
        }

        i++;
        if (i == srcSize)
            break;

        // 3B utf8
        word += utf8Str[i];
        iter = utf8ToOthers.find(word);
        if (iter != utf8ToOthers.end()) {
            iter->second.AddToString(ret, destEncoding);
            i++;
            continue;
        }

        throw ConvertError(word, i, Encoding::UTF8, destEncoding);
    }
    return ret;
}

std::string Convert(std::string_view src, Encoding srcEncoding, Encoding destEncoding) {
    if (srcEncoding == destEncoding) {
        return std::string(src);
    }

    // utf8 -> other
    if (srcEncoding == Encoding::UTF8) {
        return ConvertFromUtf8(src, destEncoding);
    }

    // other -> utf8
    if (destEncoding == Encoding::UTF8) {
        return ConvertToUtf8(src, srcEncoding);
    }

    // other -> other
    auto temp = ConvertToUtf8(src, srcEncoding);
    return ConvertFromUtf8(temp, destEncoding);
}

} // namespace viet