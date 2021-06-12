#pragma once

#include <sstream>
#include <string>
#include <regex>

// Turns a number into a string with a customizable degree of precision,
// While also truncating out any trailing zeroes at the end.
template <typename numType>
std::string to_string_with_precision(const numType number, const int precision = 6)
{
    // Final string will be stored here
    std::stringstream result;

    // Create output stream and set precision, then assign the number
    std::ostringstream out;
    out.precision(precision);
    out << std::fixed << number;

    // Get resulting number string
    std::string precisionStr = out.str();

    // Set regex to catch trailing zeroes at the end
    std::regex expression("\\.?0+$");
    
    // Remove trailing zeroes
    std::regex_replace(std::ostream_iterator<char>(result), precisionStr.begin(), precisionStr.end(), expression, "");
    
    return result.str();
}