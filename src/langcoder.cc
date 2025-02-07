/* This file is (c) 2008-2013 Konstantin Isakov <ikm@goldendict.org>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include "langcoder.hh"
#include "folding.hh"
#include "language.hh"
#include "utf8.hh"

#ifdef _MSC_VER
#include <stub_msvc.h>
#endif
#if (QT_VERSION >= QT_VERSION_CHECK(6,0,0))
#include <QtCore5Compat/QRegExp>
#endif
#include <QLocale>

LangCoder langCoder;

// Language codes

static GDLangCode LangCodes[] = {

    { "aa", "aar", -1, "Afar" },
    { "ab", "abk", -1, "Abkhazian" },
    { "ae", "ave", -1, "Avestan" },
    { "af", "afr", -1, "Afrikaans" },
    { "ak", "aka", -1, "Akan" },
    { "am", "amh", -1, "Amharic" },
    { "an", "arg", -1, "Aragonese" },
    { "ar", "ara", 1, "Arabic" },
    { "as", "asm", -1, "Assamese" },
    { "av", "ava", -1, "Avaric" },
    { "ay", "aym", -1, "Aymara" },
    { "az", "aze", 0, "Azerbaijani" },
    { "ba", "bak", 0, "Bashkir" },
    { "be", "bel", 0, "Belarusian" },
    { "bg", "bul", 0, "Bulgarian" },
    { "bh", "bih", -1, "Bihari" },
    { "bi", "bis", -1, "Bislama" },
    { "bm", "bam", -1, "Bambara" },
    { "bn", "ben", -1, "Bengali" },
    { "bo", "tib", -1, "Tibetan" },
    { "br", "bre", -1, "Breton" },
    { "bs", "bos", 0, "Bosnian" },
    { "ca", "cat", -1, "Catalan" },
    { "ce", "che", -1, "Chechen" },
    { "ch", "cha", -1, "Chamorro" },
    { "co", "cos", -1, "Corsican" },
    { "cr", "cre", -1, "Cree" },
    { "cs", "cze", 0, "Czech" },
    { "cu", "chu", 0, "Church Slavic" },
    { "cv", "chv", 0, "Chuvash" },
    { "cy", "wel", 0, "Welsh" },
    { "da", "dan", 0, "Danish" },
    { "de", "ger", 0, "German" },
    { "dv", "div", -1, "Divehi" },
    { "dz", "dzo", -1, "Dzongkha" },
    { "ee", "ewe", -1, "Ewe" },
    { "el", "gre", 0, "Greek" },
    { "en", "eng", 0, "English" },
    { "eo", "epo", 0, "Esperanto" },
    { "es", "spa", 0, "Spanish" },
    { "et", "est", 0, "Estonian" },
    { "eu", "baq", 0, "Basque" },
    { "fa", "per", -1, "Persian" },
    { "ff", "ful", -1, "Fulah" },
    { "fi", "fin", 0, "Finnish" },
    { "fj", "fij", -1, "Fijian" },
    { "fo", "fao", -1, "Faroese" },
    { "fr", "fre", 0, "French" },
    { "fy", "fry", -1, "Western Frisian" },
    { "ga", "gle", 0, "Irish" },
    { "gd", "gla", 0, "Scottish Gaelic" },
    { "gl", "glg", -1, "Galician" },
    { "gn", "grn", -1, "Guarani" },
    { "gu", "guj", -1, "Gujarati" },
    { "gv", "glv", -1, "Manx" },
    { "ha", "hau", -1, "Hausa" },
    { "he", "heb", 1, "Hebrew" },
    { "hi", "hin", -1, "Hindi" },
    { "ho", "hmo", -1, "Hiri Motu" },
    { "hr", "hrv", 0, "Croatian" },
    { "ht", "hat", -1, "Haitian" },
    { "hu", "hun", 0, "Hungarian" },
    { "hy", "arm", 0, "Armenian" },
    { "hz", "her", -1, "Herero" },
    { "ia", "ina", -1, "Interlingua" },
    { "id", "ind", -1, "Indonesian" },
    { "ie", "ile", -1, "Interlingue" },
    { "ig", "ibo", -1, "Igbo" },
    { "ii", "iii", -1, "Sichuan Yi" },
    { "ik", "ipk", -1, "Inupiaq" },
    { "io", "ido", -1, "Ido" },
    { "is", "ice", -1, "Icelandic" },
    { "it", "ita", 0, "Italian" },
    { "iu", "iku", -1, "Inuktitut" },
    { "ja", "jpn", 0, "Japanese" },
    { "jv", "jav", -1, "Javanese" },
    { "ka", "geo", 0, "Georgian" },
    { "kg", "kon", -1, "Kongo" },
    { "ki", "kik", -1, "Kikuyu" },
    { "kj", "kua", -1, "Kwanyama" },
    { "kk", "kaz", 0, "Kazakh" },
    { "kl", "kal", -1, "Kalaallisut" },
    { "km", "khm", -1, "Khmer" },
    { "kn", "kan", -1, "Kannada" },
    { "ko", "kor", 0, "Korean" },
    { "kr", "kau", -1, "Kanuri" },
    { "ks", "kas", -1, "Kashmiri" },
    { "ku", "kur", -1, "Kurdish" },
    { "kv", "kom", 0, "Komi" },
    { "kw", "cor", -1, "Cornish" },
    { "ky", "kir", -1, "Kirghiz" },
    { "la", "lat", 0, "Latin" },
    { "lb", "ltz", 0, "Luxembourgish" },
    { "lg", "lug", -1, "Ganda" },
    { "li", "lim", -1, "Limburgish" },
    { "ln", "lin", -1, "Lingala" },
    { "lo", "lao", -1, "Lao" },
    { "lt", "lit", 0, "Lithuanian" },
    { "lu", "lub", -1, "Luba-Katanga" },
    { "lv", "lav", 0, "Latvian" },
    { "mg", "mlg", -1, "Malagasy" },
    { "mh", "mah", -1, "Marshallese" },
    { "mi", "mao", -1, "Maori" },
    { "mk", "mac", 0, "Macedonian" },
    { "ml", "mal", -1, "Malayalam" },
    { "mn", "mon", -1, "Mongolian" },
    { "mr", "mar", -1, "Marathi" },
    { "ms", "may", -1, "Malay" },
    { "mt", "mlt", -1, "Maltese" },
    { "my", "bur", -1, "Burmese" },
    { "na", "nau", -1, "Nauru" },
    { "nb", "nob", 0, "Norwegian Bokmal" },
    { "nd", "nde", -1, "North Ndebele" },
    { "ne", "nep", -1, "Nepali" },
    { "ng", "ndo", -1, "Ndonga" },
    { "nl", "dut", -1, "Dutch" },
    { "nn", "nno", -1, "Norwegian Nynorsk" },
    { "no", "nor", 0, "Norwegian" },
    { "nr", "nbl", -1, "South Ndebele" },
    { "nv", "nav", -1, "Navajo" },
    { "ny", "nya", -1, "Chichewa" },
    { "oc", "oci", -1, "Occitan" },
    { "oj", "oji", -1, "Ojibwa" },
    { "om", "orm", -1, "Oromo" },
    { "or", "ori", -1, "Oriya" },
    { "os", "oss", -1, "Ossetian" },
    { "pa", "pan", -1, "Panjabi" },
    { "pi", "pli", -1, "Pali" },
    { "pl", "pol", 0, "Polish" },
    { "ps", "pus", -1, "Pashto" },
    { "pt", "por", 0, "Portuguese" },
    { "qu", "que", -1, "Quechua" },
    { "rm", "roh", -1, "Raeto-Romance" },
    { "rn", "run", -1, "Kirundi" },
    { "ro", "rum", 0, "Romanian" },
    { "ru", "rus", 0, "Russian" },
    { "rw", "kin", -1, "Kinyarwanda" },
    { "sa", "san", -1, "Sanskrit" },
    { "sc", "srd", -1, "Sardinian" },
    { "sd", "snd", -1, "Sindhi" },
    { "se", "sme", -1, "Northern Sami" },
    { "sg", "sag", -1, "Sango" },
    { "sh", "shr", 0, "Serbo-Croatian" },
    { "si", "sin", -1, "Sinhala" },
    { "sk", "slo", 0, "Slovak" },
    { "sl", "slv", 0, "Slovenian" },
    { "sm", "smo", -1, "Samoan" },
    { "sn", "sna", -1, "Shona" },
    { "so", "som", -1, "Somali" },
    { "sq", "alb", 0, "Albanian" },
    { "sr", "srp", 0, "Serbian" },
    { "ss", "ssw", -1, "Swati" },
    { "st", "sot", -1, "Southern Sotho" },
    { "su", "sun", -1, "Sundanese" },
    { "sv", "swe", 0, "Swedish" },
    { "sw", "swa", -1, "Swahili" },
    { "ta", "tam", -1, "Tamil" },
    { "te", "tel", -1, "Telugu" },
    { "tg", "tgk", 0, "Tajik" },
    { "th", "tha", -1, "Thai" },
    { "ti", "tir", -1, "Tigrinya" },
    { "tk", "tuk", 0, "Turkmen" },
    { "tl", "tgl", -1, "Tagalog" },
    { "tn", "tsn", -1, "Tswana" },
    { "to", "ton", -1, "Tonga" },
    { "tr", "tur", 0, "Turkish" },
    { "ts", "tso", -1, "Tsonga" },
    { "tt", "tat", -1, "Tatar" },
    { "tw", "twi", -1, "Twi" },
    { "ty", "tah", -1, "Tahitian" },
    { "ug", "uig", -1, "Uighur" },
    { "uk", "ukr", -1, "Ukrainian" },
    { "ur", "urd", -1, "Urdu" },
    { "uz", "uzb", 0, "Uzbek" },
    { "ve", "ven", -1, "Venda" },
    { "vi", "vie", -1, "Vietnamese" },
    { "vo", "vol", 0, "Volapuk" },
    { "wa", "wln", -1, "Walloon" },
    { "wo", "wol", -1, "Wolof" },
    { "xh", "xho", -1, "Xhosa" },
    { "yi", "yid", -1, "Yiddish" },
    { "yo", "yor", -1, "Yoruba" },
    { "za", "zha", -1, "Zhuang" },
    { "zh", "chi", 0, "Chinese" },
    { "zu", "zul", -1, "Zulu" },
    { "jb", "jbo", 0, "Lojban" },
};

LangCoder::LangCoder()
{
  for ( const auto & lc : LangCodes ) {
    codeMap[ lc.code ] = lc;
  }
}

QString LangCoder::decode(quint32 code)
{
  auto code2 = intToCode2( code );
  if ( langCoder.codeMap.contains( code2 ) )
    return QString::fromStdString( langCoder.codeMap[ code2 ].lang );

  return {};
}

QIcon LangCoder::icon( quint32 _code )
{
  auto code = intToCode2( _code );

  if (langCoder.codeMap.contains(code))
  {
    const GDLangCode & lc = langCoder.codeMap[ code ];
    return QIcon( ":/flags/" + QString(lc.code) + ".png" );
  }

  return {};
}

QString LangCoder::intToCode2( quint32 val )
{
  if ( !val || val == 0xFFffFFff )
    return {};

  char code[ 2 ];

  code[ 0 ] = val & 0xFF;
  code[ 1 ] = ( val >> 8 ) & 0xFF;

  return QString::fromLatin1( code, 2 );
}

quint32 LangCoder::findIdForLanguage( gd::wstring const & lang )
{
  const auto langFolded = Utf8::encode( lang );

  for ( auto const & lc : LangCodes ) {
    if ( langFolded == lc.lang ) {
      return code2toInt( lc.code.toStdString().c_str() );
    }
  }

  return Language::findBlgLangIDByEnglishName( lang );
}

quint32 LangCoder::findIdForLanguageCode3( std::string const & code )
{
  for ( auto const & lc : LangCodes ) {
    if ( code == lc.code3 ) {
      return code2toInt( lc.code );
    }
  }

  return 0;
}

quint32 LangCoder::guessId( const QString & lang )
{
  QString lstr = lang.simplified().toLower();

  // too small to guess
  if (lstr.size() < 2)
    return 0;

  // check if it could be the whole language name
  if (lstr.size() >= 3)
  {
    for ( auto const & lc : LangCodes ) {
      if ( lstr == ( lstr.size() == 3 ? QString::fromStdString( lc.code3 ) : QString::fromStdString( lc.lang ) ) ) {
        return code2toInt( lc.code );
      }
    }
  }

  // still not found - try to match by 2-symbol code
  return code2toInt( lstr.left(2).toLatin1().data() );
}

QPair<quint32,quint32> LangCoder::findIdsForName( QString const & name )
{
  QString nameFolded = "|" + name.toCaseFolded() + "|";
  QRegExp reg( "[^a-z]([a-z]{2,3})-([a-z]{2,3})[^a-z]" ); reg.setMinimal(true);
  int off = 0;

  while ( reg.indexIn( nameFolded, off ) >= 0 )
  {
    quint32 from = guessId( reg.cap(1) );
    quint32 to = guessId( reg.cap(2) );
    if (from && to)
      return QPair<quint32,quint32>(from, to);

    off += reg.matchedLength();
  }

  return QPair<quint32,quint32>(0, 0);
}

QPair<quint32,quint32> LangCoder::findIdsForFilename( QString const & name )
{
  return findIdsForName( QFileInfo( name ).fileName() );
}

bool LangCoder::isLanguageRTL( quint32 _code )
{
  auto code = intToCode2( _code );
  if ( langCoder.codeMap.contains( code ) )
  {
    GDLangCode & lc = langCoder.codeMap[ code ];
    if( lc.isRTL < 0 )
    {
      lc.isRTL = static_cast< int >( QLocale( lc.code ).textDirection() == Qt::RightToLeft );
    }
    return lc.isRTL != 0;
  }

  return false;
}
