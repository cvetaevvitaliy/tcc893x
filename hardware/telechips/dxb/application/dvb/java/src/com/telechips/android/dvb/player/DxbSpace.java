/*
 * Copyright (C) 2013 Telechips, Inc.
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

package com.telechips.android.dvb.player;

import java.io.IOException;

import android.content.res.XmlResourceParser;
import org.xmlpull.v1.XmlPullParser;
import org.xmlpull.v1.XmlPullParserException;

import com.telechips.android.dvb.player.dvb.TuneSpace;

public class DxbSpace
{
	static public TuneSpace[] get(XmlResourceParser xpp)
	{
		TuneSpace tuneSpace[] = null;
		try
		{
			int eventType = xpp.getEventType();
			int i = -1, j = 0;
			while (eventType != XmlPullParser.END_DOCUMENT) {
				if(eventType == XmlPullParser.START_DOCUMENT) {
					//DxbLog(D, "Start document");
				} else if(eventType == XmlPullParser.START_TAG) {
					//DxbLog(D, "Start Tag : " + xpp.getName());
					if ("CountryList".equals(xpp.getName()))
					{
						tuneSpace = new TuneSpace[Integer.parseInt(xpp.getAttributeValue(null, "numbers"))];
						i = 0;
					}
					else if (0 <= i && i < tuneSpace.length)
					{
						if ("country".equals(xpp.getName()))
						{
							j = 0;
							tuneSpace[i]					= new TuneSpace(Integer.parseInt(xpp.getAttributeValue(null, "numbers")));
							tuneSpace[i].name				= xpp.getAttributeValue(null, "name");
							tuneSpace[i].countryCode		= Integer.parseInt(xpp.getAttributeValue(null, "code"));
							tuneSpace[i].space				= Integer.parseInt(xpp.getAttributeValue(null, "space"));
							tuneSpace[i].typicalBandwidth	= Integer.parseInt(xpp.getAttributeValue(null, "bandwidth"));
							tuneSpace[i].minChannel			= Integer.parseInt(xpp.getAttributeValue(null, "minChannel"));
							tuneSpace[i].maxChannel			= Integer.parseInt(xpp.getAttributeValue(null, "maxChannel"));
							tuneSpace[i].finetunes[0]		= Integer.parseInt(xpp.getAttributeValue(null, "finetunes1"));
							tuneSpace[i].finetunes[1]		= Integer.parseInt(xpp.getAttributeValue(null, "finetunes2"));
							tuneSpace[i].finetunes[2]		= Integer.parseInt(xpp.getAttributeValue(null, "finetunes3"));
							tuneSpace[i].finetunes[3]		= Integer.parseInt(xpp.getAttributeValue(null, "finetunes4"));
						}
						else if (0 <= j && j < tuneSpace[i].channels.length)
						{
							if ("channel".equals(xpp.getName()))
							{
								tuneSpace[i].channels[j].name			= xpp.getAttributeValue(null, "name");
								tuneSpace[i].channels[j].frequency1		= Integer.parseInt(xpp.getAttributeValue(null, "frequency1"));
								tuneSpace[i].channels[j].frequency2		= Integer.parseInt(xpp.getAttributeValue(null, "frequency2"));
								tuneSpace[i].channels[j].finetune		= Integer.parseInt(xpp.getAttributeValue(null, "finetune"));
								tuneSpace[i].channels[j].bandwidth1		= Integer.parseInt(xpp.getAttributeValue(null, "bandwidth1"));
								tuneSpace[i].channels[j].bandwidth2		= Integer.parseInt(xpp.getAttributeValue(null, "bandwidth2"));
								tuneSpace[i].channels[j].alt_selected	= Integer.parseInt(xpp.getAttributeValue(null, "alt_selected"));
							}
						}
					}
				} else if(eventType == XmlPullParser.END_TAG) {
					//DxbLog(D, "End Tag : " + xpp.getName());
					if ("CountryList".equals(xpp.getName()))
					{
						i = -1;
					}
					else if ("country".equals(xpp.getName()))
					{
						j = -1;
						i++;
					}
					else if ("channel".equals(xpp.getName()))
					{
						j++;
					}
				} else if(eventType == XmlPullParser.TEXT) {
					//DxbLog(D, "Text : " + xpp.getText());
				}
				eventType = xpp.next();
			}
			//DxbLog(D, "End document");
		} catch (XmlPullParserException e) {
			e.printStackTrace();
		} catch (IOException e) {
			e.printStackTrace();
		}

		return tuneSpace;
	}
}
