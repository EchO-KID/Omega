/* Copyright (c) 2015 Wildfire Games
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */


#include "i18n/L10n.h"
#include <string>
#include "tinygettext\tinygettext.hpp"
#include "LeakDump.h"

L10n& L10n::getInstance()
{
	static L10n s_instance;
	return s_instance;
}

L10n::L10n()
	:dictionary(New tinygettext::Dictionary())
	, localeString("")
{
	// Determine whether or not to print tinygettext messages to the standard
	// error output, which it tinygettext’s default behavior, but not ours.
	//bool tinygettext_debug = false;
	//if (!tinygettext_debug)
	//{
	//	tinygettext::Log::log_info_callback = 0;
	//	tinygettext::Log::log_warning_callback = 0;
	//	tinygettext::Log::log_error_callback = 0;
	//}
}

void L10n::init(const std::string& localeString)
{
	this->localeString = localeString;


}

L10n::~L10n()
{

	Delete dictionary;
}


std::string L10n::getCurrentLocaleString() const
{
	return this->localeString;
}

std::string L10n::translate(const std::string& sourceString) const
{
	return dictionary->translate(sourceString);
}

std::string L10n::translateWithContext(const std::string& context, const std::string& sourceString) const
{
	return dictionary->translate_ctxt(context, sourceString);
}

std::string L10n::translatePlural(const std::string& singularSourceString, const std::string& pluralSourceString, int number) const
{
	return dictionary->translate_plural(singularSourceString, pluralSourceString, number);
}

std::string L10n::translatePluralWithContext(const std::string& context, const std::string& singularSourceString, const std::string& pluralSourceString, int number) const
{
	return dictionary->translate_ctxt_plural(context, singularSourceString, pluralSourceString, number);
}

//std::string L10n::translateLines(const std::string& sourceString) const
//{
//	std::string targetString;
//	std::stringstream stringOfLines(sourceString);
//	std::string line;
//
//	while (std::getline(stringOfLines, line))
//	{
//		if (!line.empty())
//			targetString.append(translate(line));
//		targetString.append("\n");
//	}
//
//	return targetString;
//}

