/*
 * Copyright (C) 2013 The Android Open Source Project
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

package com.android.inputmethod.latin.utils;

import android.test.AndroidTestCase;
import android.test.suitebuilder.annotation.SmallTest;

import java.util.Locale;

@SmallTest
public class RecapitalizeStatusTests extends AndroidTestCase {
    public void testTrim() {
        final RecapitalizeStatus status = new RecapitalizeStatus();
        status.initialize(30, 40, "abcdefghij", Locale.ENGLISH, " ");
        status.trim();
        assertEquals("abcdefghij", status.getRecapitalizedString());
        assertEquals(30, status.getNewCursorStart());
        assertEquals(40, status.getNewCursorEnd());

        status.initialize(30, 44, "    abcdefghij", Locale.ENGLISH, " ");
        status.trim();
        assertEquals("abcdefghij", status.getRecapitalizedString());
        assertEquals(34, status.getNewCursorStart());
        assertEquals(44, status.getNewCursorEnd());

        status.initialize(30, 40, "abcdefgh  ", Locale.ENGLISH, " ");
        status.trim();
        assertEquals("abcdefgh", status.getRecapitalizedString());
        assertEquals(30, status.getNewCursorStart());
        assertEquals(38, status.getNewCursorEnd());

        status.initialize(30, 45, "   abcdefghij  ", Locale.ENGLISH, " ");
        status.trim();
        assertEquals("abcdefghij", status.getRecapitalizedString());
        assertEquals(33, status.getNewCursorStart());
        assertEquals(43, status.getNewCursorEnd());
    }

    public void testRotate() {
        final RecapitalizeStatus status = new RecapitalizeStatus();
        status.initialize(29, 40, "abcd efghij", Locale.ENGLISH, " ");
        status.rotate();
        assertEquals("Abcd Efghij", status.getRecapitalizedString());
        assertEquals(29, status.getNewCursorStart());
        assertEquals(40, status.getNewCursorEnd());
        status.rotate();
        assertEquals("ABCD EFGHIJ", status.getRecapitalizedString());
        status.rotate();
        assertEquals("abcd efghij", status.getRecapitalizedString());
        status.rotate();
        assertEquals("Abcd Efghij", status.getRecapitalizedString());

        status.initialize(29, 40, "Abcd Efghij", Locale.ENGLISH, " ");
        status.rotate();
        assertEquals("ABCD EFGHIJ", status.getRecapitalizedString());
        assertEquals(29, status.getNewCursorStart());
        assertEquals(40, status.getNewCursorEnd());
        status.rotate();
        assertEquals("abcd efghij", status.getRecapitalizedString());
        status.rotate();
        assertEquals("Abcd Efghij", status.getRecapitalizedString());
        status.rotate();
        assertEquals("ABCD EFGHIJ", status.getRecapitalizedString());

        status.initialize(29, 40, "ABCD EFGHIJ", Locale.ENGLISH, " ");
        status.rotate();
        assertEquals("abcd efghij", status.getRecapitalizedString());
        assertEquals(29, status.getNewCursorStart());
        assertEquals(40, status.getNewCursorEnd());
        status.rotate();
        assertEquals("Abcd Efghij", status.getRecapitalizedString());
        status.rotate();
        assertEquals("ABCD EFGHIJ", status.getRecapitalizedString());
        status.rotate();
        assertEquals("abcd efghij", status.getRecapitalizedString());

        status.initialize(29, 39, "AbCDefghij", Locale.ENGLISH, " ");
        status.rotate();
        assertEquals("abcdefghij", status.getRecapitalizedString());
        assertEquals(29, status.getNewCursorStart());
        assertEquals(39, status.getNewCursorEnd());
        status.rotate();
        assertEquals("Abcdefghij", status.getRecapitalizedString());
        status.rotate();
        assertEquals("ABCDEFGHIJ", status.getRecapitalizedString());
        status.rotate();
        assertEquals("AbCDefghij", status.getRecapitalizedString());
        status.rotate();
        assertEquals("abcdefghij", status.getRecapitalizedString());

        status.initialize(29, 40, "Abcd efghij", Locale.ENGLISH, " ");
        status.rotate();
        assertEquals("abcd efghij", status.getRecapitalizedString());
        assertEquals(29, status.getNewCursorStart());
        assertEquals(40, status.getNewCursorEnd());
        status.rotate();
        assertEquals("Abcd Efghij", status.getRecapitalizedString());
        status.rotate();
        assertEquals("ABCD EFGHIJ", status.getRecapitalizedString());
        status.rotate();
        assertEquals("Abcd efghij", status.getRecapitalizedString());
        status.rotate();
        assertEquals("abcd efghij", status.getRecapitalizedString());

        status.initialize(30, 34, "gr????", Locale.GERMAN, " "); status.rotate();
        assertEquals("Gr????", status.getRecapitalizedString());
        assertEquals(30, status.getNewCursorStart());
        assertEquals(34, status.getNewCursorEnd());
        status.rotate();
        assertEquals("GR??SS", status.getRecapitalizedString());
        assertEquals(30, status.getNewCursorStart());
        assertEquals(35, status.getNewCursorEnd());
        status.rotate();
        assertEquals("gr????", status.getRecapitalizedString());
        assertEquals(30, status.getNewCursorStart());
        assertEquals(34, status.getNewCursorEnd());
        status.rotate();
        assertEquals("Gr????", status.getRecapitalizedString());
        assertEquals(30, status.getNewCursorStart());
        assertEquals(34, status.getNewCursorEnd());

        status.initialize(30, 33, "??uf", Locale.FRENCH, " "); status.rotate();
        assertEquals("??uf", status.getRecapitalizedString());
        assertEquals(30, status.getNewCursorStart());
        assertEquals(33, status.getNewCursorEnd());
        status.rotate();
        assertEquals("??UF", status.getRecapitalizedString());
        assertEquals(30, status.getNewCursorStart());
        assertEquals(33, status.getNewCursorEnd());
        status.rotate();
        assertEquals("??uf", status.getRecapitalizedString());
        assertEquals(30, status.getNewCursorStart());
        assertEquals(33, status.getNewCursorEnd());
        status.rotate();
        assertEquals("??uf", status.getRecapitalizedString());
        assertEquals(30, status.getNewCursorStart());
        assertEquals(33, status.getNewCursorEnd());

        status.initialize(30, 33, "??Uf", Locale.FRENCH, " "); status.rotate();
        assertEquals("??uf", status.getRecapitalizedString());
        assertEquals(30, status.getNewCursorStart());
        assertEquals(33, status.getNewCursorEnd());
        status.rotate();
        assertEquals("??uf", status.getRecapitalizedString());
        assertEquals(30, status.getNewCursorStart());
        assertEquals(33, status.getNewCursorEnd());
        status.rotate();
        assertEquals("??UF", status.getRecapitalizedString());
        assertEquals(30, status.getNewCursorStart());
        assertEquals(33, status.getNewCursorEnd());
        status.rotate();
        assertEquals("??Uf", status.getRecapitalizedString());
        assertEquals(30, status.getNewCursorStart());
        assertEquals(33, status.getNewCursorEnd());
        status.rotate();
        assertEquals("??uf", status.getRecapitalizedString());
        assertEquals(30, status.getNewCursorStart());
        assertEquals(33, status.getNewCursorEnd());

        status.initialize(30, 35, "??cole", Locale.FRENCH, " "); status.rotate();
        assertEquals("??cole", status.getRecapitalizedString());
        assertEquals(30, status.getNewCursorStart());
        assertEquals(35, status.getNewCursorEnd());
        status.rotate();
        assertEquals("??COLE", status.getRecapitalizedString());
        assertEquals(30, status.getNewCursorStart());
        assertEquals(35, status.getNewCursorEnd());
        status.rotate();
        assertEquals("??cole", status.getRecapitalizedString());
        assertEquals(30, status.getNewCursorStart());
        assertEquals(35, status.getNewCursorEnd());
        status.rotate();
        assertEquals("??cole", status.getRecapitalizedString());
        assertEquals(30, status.getNewCursorStart());
        assertEquals(35, status.getNewCursorEnd());
    }
}
