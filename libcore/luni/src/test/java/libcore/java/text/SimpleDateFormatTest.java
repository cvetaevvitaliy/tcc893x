/*
 * Copyright (C) 2010 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package libcore.java.text;

import java.text.DateFormat;
import java.text.ParsePosition;
import java.text.SimpleDateFormat;
import java.util.Calendar;
import java.util.Date;
import java.util.GregorianCalendar;
import java.util.Locale;
import java.util.TimeZone;

public class SimpleDateFormatTest extends junit.framework.TestCase {

    private static final TimeZone AMERICA_LOS_ANGELES = TimeZone.getTimeZone("America/Los_Angeles");
    private static final TimeZone AUSTRALIA_LORD_HOWE = TimeZone.getTimeZone("Australia/Lord_Howe");

    // The RI fails this test.
    public void test2DigitYearStartIsCloned() throws Exception {
        // Test that get2DigitYearStart returns a clone.
        SimpleDateFormat sdf = new SimpleDateFormat();
        Date originalDate = sdf.get2DigitYearStart();
        assertNotSame(sdf.get2DigitYearStart(), originalDate);
        assertEquals(sdf.get2DigitYearStart(), originalDate);
        originalDate.setTime(0);
        assertFalse(sdf.get2DigitYearStart().equals(originalDate));
        // Test that set2DigitYearStart takes a clone.
        Date newDate = new Date();
        sdf.set2DigitYearStart(newDate);
        assertNotSame(sdf.get2DigitYearStart(), newDate);
        assertEquals(sdf.get2DigitYearStart(), newDate);
        newDate.setTime(0);
        assertFalse(sdf.get2DigitYearStart().equals(newDate));
    }

    // The RI fails this test because this is an ICU-compatible Android extension.
    // Necessary for correct localization in various languages (http://b/2633414).
    public void testStandAloneNames() throws Exception {
        TimeZone.setDefault(TimeZone.getTimeZone("GMT"));
        Locale en = Locale.ENGLISH;
        Locale pl = new Locale("pl");
        Locale ru = new Locale("ru");

        assertEquals("January", formatDate(en, "MMMM"));
        assertEquals("January", formatDate(en, "LLLL"));
        assertEquals("stycznia", formatDate(pl, "MMMM"));
        assertEquals("stycze\u0144", formatDate(pl, "LLLL"));

        assertEquals("Thursday", formatDate(en, "EEEE"));
        assertEquals("Thursday", formatDate(en, "cccc"));
        assertEquals("\u0447\u0435\u0442\u0432\u0435\u0440\u0433", formatDate(ru, "EEEE"));
        assertEquals("\u0427\u0435\u0442\u0432\u0435\u0440\u0433", formatDate(ru, "cccc"));

        assertEquals(Calendar.JUNE, parseDate(en, "yyyy-MMMM-dd", "1980-June-12").get(Calendar.MONTH));
        assertEquals(Calendar.JUNE, parseDate(en, "yyyy-LLLL-dd", "1980-June-12").get(Calendar.MONTH));
        assertEquals(Calendar.JUNE, parseDate(pl, "yyyy-MMMM-dd", "1980-czerwca-12").get(Calendar.MONTH));
        assertEquals(Calendar.JUNE, parseDate(pl, "yyyy-LLLL-dd", "1980-czerwiec-12").get(Calendar.MONTH));

        assertEquals(Calendar.TUESDAY, parseDate(en, "EEEE", "Tuesday").get(Calendar.DAY_OF_WEEK));
        assertEquals(Calendar.TUESDAY, parseDate(en, "cccc", "Tuesday").get(Calendar.DAY_OF_WEEK));
        assertEquals(Calendar.TUESDAY, parseDate(ru, "EEEE", "\u0432\u0442\u043e\u0440\u043d\u0438\u043a").get(Calendar.DAY_OF_WEEK));
        assertEquals(Calendar.TUESDAY, parseDate(ru, "cccc", "\u0412\u0442\u043e\u0440\u043d\u0438\u043a").get(Calendar.DAY_OF_WEEK));
    }

    // The RI fails this test because it doesn't fully support UTS #35.
    // https://code.google.com/p/android/issues/detail?id=39616
    public void testFiveCount_parsing() throws Exception {
      // It's pretty silly to try to parse the shortest names, because they're almost always ambiguous.
      try {
        parseDate(Locale.ENGLISH, "MMMMM", "J");
        fail();
      } catch (junit.framework.AssertionFailedError expected) {
      }
      try {
        parseDate(Locale.ENGLISH, "LLLLL", "J");
        fail();
      } catch (junit.framework.AssertionFailedError expected) {
      }
      try {
        parseDate(Locale.ENGLISH, "EEEEE", "T");
        fail();
      } catch (junit.framework.AssertionFailedError expected) {
      }
      try {
        parseDate(Locale.ENGLISH, "ccccc", "T");
        fail();
      } catch (junit.framework.AssertionFailedError expected) {
      }
    }

    // The RI fails this test because it doesn't fully support UTS #35.
    // https://code.google.com/p/android/issues/detail?id=39616
    public void testFiveCount_M() throws Exception {
      TimeZone.setDefault(TimeZone.getTimeZone("GMT"));
      assertEquals("1", formatDate(Locale.ENGLISH, "M"));
      assertEquals("01", formatDate(Locale.ENGLISH, "MM"));
      assertEquals("Jan", formatDate(Locale.ENGLISH, "MMM"));
      assertEquals("January", formatDate(Locale.ENGLISH, "MMMM"));
      assertEquals("J", formatDate(Locale.ENGLISH, "MMMMM"));
    }

    // The RI fails this test because it doesn't fully support UTS #35.
    // https://code.google.com/p/android/issues/detail?id=39616
    public void testFiveCount_L() throws Exception {
      TimeZone.setDefault(TimeZone.getTimeZone("GMT"));
      assertEquals("1", formatDate(Locale.ENGLISH, "L"));
      assertEquals("01", formatDate(Locale.ENGLISH, "LL"));
      assertEquals("Jan", formatDate(Locale.ENGLISH, "LLL"));
      assertEquals("January", formatDate(Locale.ENGLISH, "LLLL"));
      assertEquals("J", formatDate(Locale.ENGLISH, "LLLLL"));
    }

    // The RI fails this test because it doesn't fully support UTS #35.
    // https://code.google.com/p/android/issues/detail?id=39616
    public void testFiveCount_E() throws Exception {
      TimeZone.setDefault(TimeZone.getTimeZone("GMT"));
      assertEquals("Thu", formatDate(Locale.ENGLISH, "E"));
      assertEquals("Thu", formatDate(Locale.ENGLISH, "EE"));
      assertEquals("Thu", formatDate(Locale.ENGLISH, "EEE"));
      assertEquals("Thursday", formatDate(Locale.ENGLISH, "EEEE"));
      assertEquals("T", formatDate(Locale.ENGLISH, "EEEEE"));
      // assertEquals("Th", formatDate(Locale.ENGLISH, "EEEEEE")); // icu4c doesn't support 6.
    }

    // The RI fails this test because it doesn't fully support UTS #35.
    // https://code.google.com/p/android/issues/detail?id=39616
    public void testFiveCount_c() throws Exception {
      TimeZone.setDefault(TimeZone.getTimeZone("GMT"));
      assertEquals("Thu", formatDate(Locale.ENGLISH, "c"));
      assertEquals("Thu", formatDate(Locale.ENGLISH, "cc"));
      assertEquals("Thu", formatDate(Locale.ENGLISH, "ccc"));
      assertEquals("Thursday", formatDate(Locale.ENGLISH, "cccc"));
      assertEquals("T", formatDate(Locale.ENGLISH, "ccccc"));
      // assertEquals("Th", formatDate(Locale.ENGLISH, "cccccc")); // icu4c doesn't support 6.
    }

    // The RI fails this test because it doesn't fully support UTS #35.
    // https://code.google.com/p/android/issues/detail?id=39616
    public void testFiveCount_Z() throws Exception {
      TimeZone.setDefault(TimeZone.getTimeZone("GMT"));
      assertEquals("+0000", formatDate(Locale.ENGLISH, "Z"));
      assertEquals("+0000", formatDate(Locale.ENGLISH, "ZZ"));
      assertEquals("+0000", formatDate(Locale.ENGLISH, "ZZZ"));
      assertEquals("GMT+00:00", formatDate(Locale.ENGLISH, "ZZZZ"));
      assertEquals("+00:00", formatDate(Locale.ENGLISH, "ZZZZZ"));
    }

    // The RI fails this test because it doesn't fully support UTS #35.
    // https://code.google.com/p/android/issues/detail?id=39616
    public void test_parsing_Z() throws Exception {
      TimeZone.setDefault(TimeZone.getTimeZone("GMT"));
      assertEquals(1325421240000L, parseTime("yyyy-MM-dd' 'Z", "2012-01-01 -1234"));
      assertEquals(1325421240000L, parseTime("yyyy-MM-dd' 'ZZ", "2012-01-01 -1234"));
      assertEquals(1325421240000L, parseTime("yyyy-MM-dd' 'ZZZ", "2012-01-01 -1234"));
      assertEquals(1325421240000L, parseTime("yyyy-MM-dd' 'ZZZZ", "2012-01-01 GMT-12:34"));
      assertEquals(1325421240000L, parseTime("yyyy-MM-dd' 'ZZZZZ", "2012-01-01 -12:34"));
    }

    private static long parseTime(String fmt, String value) {
      return parseDate(Locale.ENGLISH, fmt, value).getTime().getTime();
    }

    public void test2038() {
        SimpleDateFormat format = new SimpleDateFormat("EEE MMM dd HH:mm:ss yyyy", Locale.US);
        format.setTimeZone(TimeZone.getTimeZone("UTC"));

        assertEquals("Sun Nov 24 17:31:44 1833",
                format.format(new Date(((long) Integer.MIN_VALUE + Integer.MIN_VALUE) * 1000L)));
        assertEquals("Fri Dec 13 20:45:52 1901",
                format.format(new Date(Integer.MIN_VALUE * 1000L)));
        assertEquals("Thu Jan 01 00:00:00 1970",
                format.format(new Date(0L)));
        assertEquals("Tue Jan 19 03:14:07 2038",
                format.format(new Date(Integer.MAX_VALUE * 1000L)));
        assertEquals("Sun Feb 07 06:28:16 2106",
                format.format(new Date((2L + Integer.MAX_VALUE + Integer.MAX_VALUE) * 1000L)));
    }

    private String formatDate(Locale l, String fmt) {
        DateFormat dateFormat = new SimpleDateFormat(fmt, l);
        dateFormat.setTimeZone(TimeZone.getTimeZone("UTC"));
        return dateFormat.format(new Date(0));
    }

    private static Calendar parseDate(Locale l, String fmt, String value) {
        SimpleDateFormat sdf = new SimpleDateFormat(fmt, l);
        ParsePosition pp = new ParsePosition(0);
        Date d = sdf.parse(value, pp);
        if (d == null) {
            fail(pp.toString());
        }
        Calendar c = Calendar.getInstance(TimeZone.getTimeZone("UTC"));
        c.setTime(d);
        return c;
    }

    // http://code.google.com/p/android/issues/detail?id=13420
    public void testParsingUncommonTimeZoneAbbreviations() {
        String fmt = "yyyy-MM-dd HH:mm:ss.SSS z";
        String date = "2010-12-23 12:44:57.0 CET";
        // ICU considers "CET" (Central European Time) to be common in Britain...
        assertEquals(1293104697000L, parseDate(Locale.UK, fmt, date).getTimeInMillis());
        // ...but not in the US. Check we can parse such a date anyway.
        assertEquals(1293104697000L, parseDate(Locale.US, fmt, date).getTimeInMillis());
    }

    public void testFormattingUncommonTimeZoneAbbreviations() {
        // In Honeycomb, only one Olson id was associated with CET (or any
        // other "uncommon" abbreviation).
        String fmt = "yyyy-MM-dd HH:mm:ss.SSS z";
        String date = "1970-01-01 01:00:00.000 CET";
        SimpleDateFormat sdf = new SimpleDateFormat(fmt, Locale.US);
        sdf.setTimeZone(TimeZone.getTimeZone("Europe/Berlin"));
        assertEquals(date, sdf.format(new Date(0)));
        sdf = new SimpleDateFormat(fmt, Locale.US);
        sdf.setTimeZone(TimeZone.getTimeZone("Europe/Zurich"));
        assertEquals(date, sdf.format(new Date(0)));
    }

    // http://code.google.com/p/android/issues/detail?id=8258
    public void testTimeZoneFormatting() throws Exception {
        Date epoch = new Date(0);

        // Create a SimpleDateFormat that defaults to America/Chicago...
        TimeZone.setDefault(TimeZone.getTimeZone("America/Chicago"));
        SimpleDateFormat sdf = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss Z");
        // We should see something appropriate to America/Chicago...
        assertEquals("1969-12-31 18:00:00 -0600", sdf.format(epoch));
        // We can set any TimeZone we want:
        sdf.setTimeZone(AMERICA_LOS_ANGELES);
        assertEquals("1969-12-31 16:00:00 -0800", sdf.format(epoch));
        sdf.setTimeZone(TimeZone.getTimeZone("UTC"));
        assertEquals("1970-01-01 00:00:00 +0000", sdf.format(epoch));

        // A new SimpleDateFormat will default to America/Chicago...
        sdf = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss Z");
        // ...and parsing an America/Los_Angeles time will *not* change that...
        sdf.parse("2010-12-03 00:00:00 -0800");
        // ...so our time zone here is "America/Chicago":
        assertEquals("1969-12-31 18:00:00 -0600", sdf.format(epoch));
        // We can set any TimeZone we want:
        sdf.setTimeZone(AMERICA_LOS_ANGELES);
        assertEquals("1969-12-31 16:00:00 -0800", sdf.format(epoch));
        sdf.setTimeZone(TimeZone.getTimeZone("UTC"));
        assertEquals("1970-01-01 00:00:00 +0000", sdf.format(epoch));

        sdf = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss");
        sdf.setTimeZone(TimeZone.getTimeZone("UTC"));
        Date date = sdf.parse("2010-07-08 02:44:48");
        assertEquals(1278557088000L, date.getTime());
        sdf = new SimpleDateFormat("yyyy-MM-dd'T'HH:mm:ssZ");
        sdf.setTimeZone(AMERICA_LOS_ANGELES);
        assertEquals("2010-07-07T19:44:48-0700", sdf.format(date));
        sdf.setTimeZone(TimeZone.getTimeZone("UTC"));
        assertEquals("2010-07-08T02:44:48+0000", sdf.format(date));
    }

    /**
     * Africa/Cairo standard time is EET and daylight time is EEST. They no
     * longer use their DST zone but we should continue to parse it properly.
     */
    public void testObsoleteDstZoneName() throws Exception {
        SimpleDateFormat format = new SimpleDateFormat("yyyy-MM-dd'T'HH:mm zzzz", Locale.US);
        Date normal = format.parse("1970-01-01T00:00 EET");
        Date dst = format.parse("1970-01-01T00:00 EEST");
        assertEquals(60 * 60 * 1000, normal.getTime() - dst.getTime());
    }

    public void testDstZoneNameWithNonDstTimestamp() throws Exception {
        SimpleDateFormat format = new SimpleDateFormat("yyyy-MM-dd'T'HH:mm zzzz", Locale.US);
        Calendar calendar = new GregorianCalendar(AMERICA_LOS_ANGELES);
        calendar.setTime(format.parse("2011-06-21T10:00 Pacific Standard Time")); // 18:00 GMT-8
        assertEquals(11, calendar.get(Calendar.HOUR_OF_DAY)); // 18:00 GMT-7
        assertEquals(0, calendar.get(Calendar.MINUTE));
    }

    public void testNonDstZoneNameWithDstTimestamp() throws Exception {
        SimpleDateFormat format = new SimpleDateFormat("yyyy-MM-dd'T'HH:mm zzzz", Locale.US);
        Calendar calendar = new GregorianCalendar(AMERICA_LOS_ANGELES);
        calendar.setTime(format.parse("2010-12-21T10:00 Pacific Daylight Time")); // 17:00 GMT-7
        assertEquals(9, calendar.get(Calendar.HOUR_OF_DAY)); // 17:00 GMT-8
        assertEquals(0, calendar.get(Calendar.MINUTE));
    }

    // http://b/4723412
    public void testDstZoneWithNonDstTimestampForNonHourDstZone() throws Exception {
        SimpleDateFormat format = new SimpleDateFormat("yyyy-MM-dd'T'HH:mm zzzz", Locale.US);
        Calendar calendar = new GregorianCalendar(AUSTRALIA_LORD_HOWE);
        calendar.setTime(format.parse("2011-06-21T20:00 Lord Howe Daylight Time")); // 9:00 GMT+11
        assertEquals(19, calendar.get(Calendar.HOUR_OF_DAY)); // 9:00 GMT+10:30
        assertEquals(30, calendar.get(Calendar.MINUTE));
    }

    public void testNonDstZoneWithDstTimestampForNonHourDstZone() throws Exception {
        SimpleDateFormat format = new SimpleDateFormat("yyyy-MM-dd'T'HH:mm zzzz", Locale.US);
        Calendar calendar = new GregorianCalendar(AUSTRALIA_LORD_HOWE);
        calendar.setTime(format.parse("2010-12-21T19:30 Lord Howe Standard Time")); //9:00 GMT+10:30
        assertEquals(20, calendar.get(Calendar.HOUR_OF_DAY)); // 9:00 GMT+11:00
        assertEquals(0, calendar.get(Calendar.MINUTE));
    }

    public void testLocales() throws Exception {
        // Just run through them all. Handy as a poor man's benchmark, and a sanity check.
        for (Locale l : Locale.getAvailableLocales()) {
            SimpleDateFormat sdf = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss zzzz", l);
            sdf.format(new Date(0));
        }
    }

    // http://code.google.com/p/android/issues/detail?id=14963
    public void testParseTimezoneOnly() throws Exception {
        new SimpleDateFormat("z", Locale.FRANCE).parse("UTC");
        new SimpleDateFormat("z", Locale.US).parse("UTC");
    }

    // http://code.google.com/p/android/issues/detail?id=36689
    public void testParseArabic() throws Exception {
        SimpleDateFormat sdf = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss", new Locale("ar", "EG"));
        sdf.setTimeZone(TimeZone.getTimeZone("America/Los_Angeles"));

        // Can we parse an ASCII-formatted date in an Arabic locale?
        Date d = sdf.parse("2012-08-29 10:02:45");
        assertEquals(1346259765000L, d.getTime());

        // Can we format a date correctly in an Arabic locale?
        String formatted = sdf.format(d);
        assertEquals("????????-????-???? ????:????:????", formatted);

        // Can we parse the Arabic-formatted date in an Arabic locale, and get the same date
        // we started with?
        Date d2 = sdf.parse(formatted);
        assertEquals(d, d2);
    }

    public void test_59383() throws Exception {
        SimpleDateFormat sdf = new SimpleDateFormat("d. MMM yyyy H:mm", Locale.GERMAN);
        sdf.setTimeZone(TimeZone.getTimeZone("America/Los_Angeles"));
        assertEquals(1376927400000L, sdf.parse("19. Aug 2013 8:50").getTime());
        assertEquals(1376927400000L, sdf.parse("19. Aug. 2013 8:50").getTime());
    }
}
