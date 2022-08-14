/*
 * Copyright (C) 2014 Telechips, Inc.
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

package com.telechips.android.dvb.player.diseqc;

public class Motor
{
	public final static int LOCATION_MAX		= 30;
	public final static int LOCATION_DEFAULT	= 24;	// 24 - Seoul
	public final static int LONGITUDE_DIRECTION_DEFAULT	= 0;
	public final static int LONGITUDE_ANGLE_DEFAULT		= 0;
	public final static int LATITUDE_DIRECTION_DEFAULT	= 0;
	public final static int LATITUDE_ANGLE_DEFAULT		= 0;

	private int    id;
	private int    location;			//	0~29 : index, 30 : menual
	private int    longitude_direction;
	private int    longitude_angle;		//	0~29 : index, 30 : menual-angle
	private int    latitude_direction;
	private int    latitude_angle;		//	0~29 : index, 30 : menual-angle

	// longitude of Antenna
	private static int mLongitude = 0;

	// latitude of Antenna
	private static int mLatitude = 0;

	public Motor()
	{
		location	= LOCATION_DEFAULT;
		longitude_direction	= 0;
		longitude_angle		= 0;
		latitude_direction	= 0;
		latitude_angle		= 0;
	}

	public void setId(int id) {
		this.id = id;
	}

	public int getId() {
		return this.id;
	}
	
	public void setLocation(int index) {
		this.location = index;	
	}

	public int getLocation() {
		return this.location;
	}

	public void setLongitudeDirection(int index) {
		this.longitude_direction = index;	
	}

	public int getLongitudeDirection() {
		return this.longitude_direction;
	}

	public void setLongitudeAngle(int value) {
		this.longitude_angle = value;	
	}

	public int getLongitudeAngle() {
		return this.longitude_angle;
	}

	public void setLatitudeDirection(int index) {
		this.latitude_direction = index;	
	}

	public int getLatitudeDirection() {
		return this.latitude_direction;
	}

	public void setLatitudeAngle(int value) {
		this.latitude_angle = value;	
	}

	public int getLatitudeAngle() {
		return this.latitude_angle;
	}
	
	public void setAngle(int Longitude, int Latitude)
	{
		mLongitude	= Longitude;
		mLatitude	= Latitude;
	}
	
	public static int getAngle_Longitude()
	{
		return mLongitude;
	}
	
	public static int getAngle_Latitude()
	{
		return mLatitude;
	}
}