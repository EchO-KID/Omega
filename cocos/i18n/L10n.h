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

#ifndef L10N_H
#define L10N_H

#include <string>
#include <vector>

namespace tinygettext
{
	class Dictionary;
}
/**
 * %Singleton for internationalization and localization.
 *
 * @sa http://trac.wildfiregames.com/wiki/Internationalization_and_Localization
 */
class L10n
{

public:
	static L10n& getInstance();
	
	L10n();
	~L10n();
	void init(const std::string& localeString);
	/**
	 * Returns the code of the current locale.
	 *
	 * A locale code is a string such as "de" or "pt_BR".
	 *
	 * @sa GetCurrentLocale()
	 * @sa GetSupportedLocaleBaseNames()
	 * @sa GetAllLocales()
	 * @sa ReevaluateCurrentLocaleAndReload()
	 */
	std::string getCurrentLocaleString() const;

	std::string translate(const std::string& sourceString) const;

	/**
	 * Returns the translation of the specified string to the
	 * @link L10n::GetCurrentLocale() current locale@endlink in the specified
	 * context.
	 *
	 * @param context Context where the string is used. See
	 *        http://www.gnu.org/software/gettext/manual/html_node/Contexts.html
	 * @param sourceString String to translate to the current locale.
	 * @return Translation of @p sourceString to the current locale in the
	 *         specified @p context, or @p sourceString if there is no
	 *         translation available.
	 */
	std::string translateWithContext(const std::string& context, const std::string& sourceString) const;

	/**
	 * Returns the translation of the specified string to the
	 * @link L10n::GetCurrentLocale() current locale@endlink based on the
	 * specified number.
	 *
	 * @param singularSourceString String to translate to the current locale,
	 *        in English’ singular form.
	 * @param pluralSourceString String to translate to the current locale, in
	 *        English’ plural form.
	 * @param number Number that determines the required form of the translation
	 *        (or the English string if no translation is available).
	 * @return Translation of the source string to the current locale for the
	 *         specified @p number, or either @p singularSourceString (if
	 *         @p number is 1) or @p pluralSourceString (if @p number is not 1)
	 *         if there is no translation available.
	 */
	std::string translatePlural(const std::string& singularSourceString, const std::string& pluralSourceString, int number) const;

	/**
	 * Returns the translation of the specified string to the
	 * @link L10n::GetCurrentLocale() current locale@endlink in the specified
	 * context, based on the specified number.
	 *
	 * @param context Context where the string is used. See
	 *        http://www.gnu.org/software/gettext/manual/html_node/Contexts.html
	 * @param singularSourceString String to translate to the current locale,
	 *        in English’ singular form.
	 * @param pluralSourceString String to translate to the current locale, in
	 *        English’ plural form.
	 * @param number Number that determines the required form of the translation
	 *        (or the English string if no translation is available).	 *
	 * @return Translation of the source string to the current locale in the
	 *         specified @p context and for the specified @p number, or either
	 *         @p singularSourceString (if @p number is 1) or
	 *         @p pluralSourceString (if @p number is not 1) if there is no
	 *         translation available.
	 */
	std::string translatePluralWithContext(const std::string& context, const std::string& singularSourceString, const std::string& pluralSourceString, int number) const;

	/**
	 * Translates a text line by line to the
	 * @link L10n::GetCurrentLocale() current locale@endlink.
	 *
	 * TranslateLines() is helpful when you need to translate a plain text file
	 * after you load it.
	 *
	 * @param sourceString Text to translate to the current locale.
	 * @return Line by line translation of @p sourceString to the current
	 *         locale. Some of the lines in the returned text may be in English
	 *         because there was not translation available for them.
	 */
	// std::string translateLines(const std::string& sourceString) const;

private:
	tinygettext::Dictionary* dictionary;
	std::string localeString;
};

#endif // L10N_H
