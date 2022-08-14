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
package com.telechips.android.isdbt.player;

import com.telechips.android.isdbt.player.isdbt.TeletextData;

import android.content.Context;
import android.database.Cursor;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.Path;
import android.graphics.PorterDuff;
import android.graphics.Rect;
import android.graphics.Typeface;
import android.util.Log;
import android.view.View;
import android.widget.ImageView;

class cFontInfo
{		
	public int mHeight, mWidth, mWeight;
	public boolean mItalic, mUnderline;
	public String mFontName;
};

class cSize
{
	public int cx, cy;
	public cSize(int w, int h)
	{
		cx = w; cy = h;
	}
};

public class DxbTeletext extends ImageView
{
	private String TAG = "[[DxbTeletext]]";
	
	public static final int TELETEXT_PAGE_DISPLAY_COLUMN = (40);
	public static final int TELETEXT_PAGE_DISPLAY_ROW = (25+1);
	public static final int TELETEXT_PAGE_MAXCOUNT_ROW = (32);
	
	public static final int SPACING_ALPHA_COLOR_BLACK = (0x00);
	public static final int SPACING_ALPHA_COLOR_RED = (0x01);
	public static final int SPACING_ALPHA_COLOR_GREEN = (0x02);
	public static final int SPACING_ALPHA_COLOR_YELLOW = (0x03);
	public static final int SPACING_ALPHA_COLOR_BLUE = (0x04);
	public static final int SPACING_ALPHA_COLOR_MAGENTA = (0x05);
	public static final int SPACING_ALPHA_COLOR_CYAN = (0x06);
	public static final int SPACING_ALPHA_COLOR_WHITE = (0x07);
	public static final int SPACING_FLASH = (0x08);
	public static final int SPACING_STEADY = (0x09);
	public static final int SPACING_END_BOX = (0x0A);
	public static final int SPACING_START_BOX = (0x0B);
	public static final int SPACING_NORMAL_SIZE = (0x0C);
	public static final int SPACING_DOUBLE_HEIGHT = (0x0D);
	public static final int SPACING_DOUBLE_WIDTH = (0x0E);
	public static final int SPACING_DOUBLE_SIZE = (0x0F);
	public static final int SPACING_MOSAIC_COLOR_BLACK = (0x10);
	public static final int SPACING_MOSAIC_COLOR_RED = (0x11);
	public static final int SPACING_MOSAIC_COLOR_GREEN = (0x12);
	public static final int SPACING_MOSAIC_COLOR_YELLOW = (0x13);
	public static final int SPACING_MOSAIC_COLOR_BLUE = (0x14);
	public static final int SPACING_MOSAIC_COLOR_MAGENTA = (0x15);
	public static final int SPACING_MOSAIC_COLOR_CYAN = (0x16);
	public static final int SPACING_MOSAIC_COLOR_WHITE = (0x17);
	public static final int SPACING_CONCEAL = (0x18);
	public static final int SPACING_CONTIGUOUS_MOSAIC_GRAPHICS = (0x19);
	public static final int SPACING_SEPARATED_MOSAIC_GRAPHICS = (0x1A);
	public static final int SPACING_ESC_OR_SWITCH = (0x1B);
	public static final int SPACING_BLACK_BACKGROUND = (0x1C);
	public static final int SPACING_NEW_BACKGROUND = (0x1D);
	public static final int SPACING_HOLD_MOSAICS = (0x1E);
	public static final int SPACING_RELEASE_MOSAICS = (0x1F);
	public static final int SPACING_UNUSED_CHARACTER = (0x20);
	
	class LATIN_NATIONAL_SUB_SET
	{
		byte bFontType; // 0 : microsoft san serif font, 1 : symbol, 2 : latin subset
	    char wFontSubSet;	  
	};	
	private LATIN_NATIONAL_SUB_SET[][] gUnicodeSubSet;	
	private final int[][][] gUnicodeSubSetData = new int[][][]
	{ // 2212
	    //       2/3          2/4          4/0          5/B          5/C          5/D          5/E          5/F          6/0          7/B          7/C          7/D          7/E
	    {{0, 0x00A3}, {0, 0x0024}, {0, 0x0040}, {1, 0xF0AC}, {0, 0x00BD}, {1, 0xF0AE}, {1, 0xF0AD}, {0, 0x0023}, {0, 0x002D}, {0, 0x00BC}, {0, 0x00B6}, {0, 0x00BE}, {0, 0x00F7}}, // english
	    {{0, 0x0023}, {0, 0x0024}, {0, 0x00A7}, {0, 0x00C4}, {0, 0x00D6}, {0, 0x00DC}, {0, 0x005E}, {0, 0x005F}, {0, 0x00B0}, {0, 0x00E4}, {0, 0x00F6}, {0, 0x00FC}, {0, 0x00DF}}, // german		    
	    {{0, 0x0023}, {0, 0x00A4}, {0, 0x00C9}, {0, 0x00C4}, {0, 0x00D6}, {0, 0x00C5}, {0, 0x00DC}, {0, 0x005F}, {0, 0x00E9}, {0, 0x00E4}, {0, 0x00F6}, {0, 0x00E5}, {0, 0x00FC}}, // swedish/finnish/hungarian
	    {{0, 0x00A3}, {0, 0x0024}, {0, 0x00E9}, {0, 0x00B0}, {0, 0x00E7}, {1, 0xF0AE}, {1, 0xF0AD}, {0, 0x0023}, {0, 0x00F9}, {0, 0x00E0}, {0, 0x00F2}, {0, 0x00E8}, {0, 0x00EC}}, // italian
	    {{0, 0x00E9}, {0, 0x00EF}, {0, 0x00E0}, {0, 0x00EB}, {0, 0x00EA}, {0, 0x00F9}, {0, 0x00EE}, {0, 0x0023}, {0, 0x00E8}, {0, 0x00E2}, {0, 0x00F4}, {0, 0x00FB}, {0, 0x00E7}}, // french
	    {{0, 0x00E7}, {0, 0x0024}, {0, 0x0069}, {0, 0x00E1}, {0, 0x00E9}, {0, 0x00ED}, {0, 0x00F3}, {0, 0x00FA}, {0, 0x00BF}, {0, 0x00FC}, {0, 0x00F1}, {0, 0x00E8}, {0, 0x00E0}}, // portuguese/spanish
	    {{0, 0x0023}, {0, 0x016F}, {0, 0x010D}, {2, 0x003B}, {2, 0x004D}, {0, 0x00FD}, {0, 0x00ED}, {0, 0x0159}, {0, 0x00E9}, {0, 0x00E1}, {0, 0x011B}, {0, 0x00FA}, {0, 0x0161}}, // czech/slovak
	    {{0, 0x0023}, {0, 0x0144}, {0, 0x0105}, {2, 0x0042}, {0, 0x015A}, {0, 0x0141}, {0, 0x0107}, {0, 0x00F3}, {0, 0x0119}, {0, 0x017C}, {0, 0x015B}, {0, 0x0142}, {0, 0x017A}}, // polish
	    {{2, 0x0027}, {0, 0x011F}, {0, 0x0130}, {0, 0x015E}, {0, 0x00D6}, {0, 0x00C7}, {0, 0x00DC}, {0, 0x011E}, {0, 0x0131}, {0, 0x015F}, {0, 0x00F6}, {0, 0x00E7}, {0, 0x00FC}}, // turkish
	    {{0, 0x0023}, {0, 0x00CB}, {0, 0x010C}, {0, 0x0106}, {0, 0x017D}, {0, 0x00D0}, {0, 0x0160}, {0, 0x00EB}, {0, 0x010D}, {0, 0x0107}, {0, 0x017E}, {0, 0x010F}, {0, 0x0161}}, // serbian
	    {{0, 0x0023}, {0, 0x00A4}, {0, 0x0162}, {0, 0x00C2}, {0, 0x015E}, {0, 0x0102}, {0, 0x00CE}, {0, 0x0131}, {0, 0x0163}, {0, 0x00E2}, {0, 0x015F}, {0, 0x0103}, {0, 0x00EE}}, // rumanian
	    {{0, 0x0023}, {0, 0x00F5}, {0, 0x0160}, {0, 0x00C4}, {0, 0x00D6}, {0, 0x017D}, {0, 0x00DC}, {0, 0x00D5}, {0, 0x0161}, {0, 0x00E4}, {0, 0x00F6}, {0, 0x017E}, {0, 0x00FC}}, // estonian
	    {{0, 0x0023}, {0, 0x0024}, {0, 0x0160}, {0, 0x0117}, {0, 0x0119}, {0, 0x017D}, {0, 0x010D}, {0, 0x016B}, {0, 0x0161}, {0, 0x0105}, {0, 0x0173}, {0, 0x017E}, {0, 0x012F}} // lettish/lithuanian	    
	};
		
	private int[][] gSubset=
	{
	    {0xA3 ,0x24 ,0x40 ,49   ,0xBD ,51   ,52   ,0x23 ,54   ,0xBC ,0xB6 ,0xBE ,0xF7}, // 0:English
	    {0x23 ,0x24 ,0xA7 ,0xC4 ,0xD6 ,0xDC ,0x5E ,0x5F ,0xB0 ,0xE4 ,0xF6 ,0xFC ,0xDF}, // 1:German
	    {0x23 ,0xA4 ,0xC9 ,0xC4 ,0xD6 ,0xC5 ,0xDC ,0x5F ,0xE9 ,0xE4 ,0xF6 ,0xE5 ,0xFC}, // 2:Swedish/Finnish/Hungarian
	    {0xA3 ,0x24 ,0xE9 ,0xB0 ,0xE7 ,51   ,52   ,0x23 ,0xF9 ,0xE0 ,0xF2 ,0xE8 ,0xEC}, // 3:Italian
	    {0xE9 ,0xEF ,0xE0 ,0xEB ,0xEA ,0xF9 ,0xEE ,0x23 ,0xE8 ,0xE2 ,0xF4 ,0xFB ,0xE7}, // 4:French
	    {0xE7 ,0x24 ,0x69 ,0xE1 ,0xE9 ,0xED ,0xF3 ,0xFA ,0xBF ,0xFC ,0xF1 ,0xE8 ,0xE0}, // 5:Portuguese/Spanish
	    {0x23 ,129  ,35   ,36   ,37   ,0xFD ,0xED ,40   ,0xE9 ,0xE1 ,43   ,0xFA ,45  }, // 6:Czech/Slovak
	    {0x23 ,93   ,90   ,94   ,95   ,96   ,97   ,0xF3 ,99   ,100  ,61   ,101  ,102 }, // 7:Polish
	    {123  ,124  ,125  ,108  ,0xD6 ,0xC7 ,0xDC ,127  ,111  ,113  ,0xF6 ,0xE7 ,0xFC}, // 8:Turkish
	    {0x23 ,0xCB ,116  ,117  ,64   ,0xD0 ,89   ,0xEB ,35   ,97   ,37   ,119  ,45  }, // 9:Serbian/Croatian/Slovenian
	    {0x23 ,0xA4 ,106  ,0xC2 ,108  ,109  ,0xCE ,111  ,112  ,0xE2 ,113  ,114  ,0xEE}, //10:Rumanian
	    {0x23 ,0xF5 ,89   ,0xC4 ,0xD6 ,64   ,0xDC ,0xD5 ,45   ,0xE4 ,0xF6 ,37   ,0xFC}, //11:Estonian
	    {0x23 ,0x24 ,89   ,86   ,87   ,64   ,35   ,88   ,45   ,90   ,91   ,37   ,92  } //12:Lettish/Lithuanian
	};
	private int mCLUTAlpha = 255;
	private int mCLUT[]=
	{
	    Color.argb(mCLUTAlpha, 0, 0, 0), // Black	    
	    Color.argb(mCLUTAlpha, 255, 0, 0), // Red
	    Color.argb(mCLUTAlpha, 0, 255, 0), // Green Fixed at Levels 1, 1.5 and 2.5
	    Color.argb(mCLUTAlpha, 255, 255, 0), // Yellow
	    Color.argb(mCLUTAlpha, 0, 0, 255), // Blue Re-definable using X/28/4 or
	    Color.argb(mCLUTAlpha, 255, 0, 255), // Magenta M/29/4 at Level 3.5
	    Color.argb(mCLUTAlpha, 0, 255, 255), // Cyan
	    Color.argb(mCLUTAlpha, 255, 255, 255), // White

	    Color.argb(mCLUTAlpha, 16, 16, 16),  // Transparent Valid at Levels 2.5 and 3.5 (fixed)
	    Color.argb(mCLUTAlpha, 127, 0, 0), // Half red
	    Color.argb(mCLUTAlpha, 0, 127, 0), // Half green Valid at Levels 2.5 and 3.5
	    Color.argb(mCLUTAlpha, 127, 127, 0), // Half yellow
	    Color.argb(mCLUTAlpha, 0, 0, 127), // Half blue
	    Color.argb(mCLUTAlpha, 127, 0, 127), // Half magenta
	    Color.argb(mCLUTAlpha, 0, 127, 127), // Half cyan Re-definable using X/28/4 or
	    Color.argb(mCLUTAlpha, 127, 127, 127), // Grey M/29/4 at Level 3.5

	    Color.argb(mCLUTAlpha, 255, 0, 5*16+15), //
	    Color.argb(mCLUTAlpha, 255, 127, 0), //
	    Color.argb(mCLUTAlpha, 0, 255, 127), //
	    Color.argb(mCLUTAlpha, 255, 255, 11*16+15), //
	    Color.argb(mCLUTAlpha, 0, 12*16+15, 10*16+15), //
	    Color.argb(mCLUTAlpha, 5*16+15, 0, 0), //
	    Color.argb(mCLUTAlpha, 6*16+15, 5*16+15, 2*16+15), //
	    Color.argb(mCLUTAlpha, 12*16+15, 127, 127), //

	    Color.argb(mCLUTAlpha, 3*16+15, 3*16+15, 3*16+15), //
	    Color.argb(mCLUTAlpha, 255, 127, 127), //
	    Color.argb(mCLUTAlpha, 127, 255, 127), //
	    Color.argb(mCLUTAlpha, 255, 255, 127), //
	    Color.argb(mCLUTAlpha, 127, 127, 255), //
	    Color.argb(mCLUTAlpha, 255, 127, 255), //
	    Color.argb(mCLUTAlpha, 127, 255, 255), //
	    Color.argb(mCLUTAlpha, 13*16+15, 13*16+15, 13*16+15) //	    
	};
	
	private String[] gUnicodeSubSetFont={"System", "Symbol", "TX Latin Subset"};
	private String gG1BlockMosaic="TX G1 Block Mosaic";
	private String gLatinSubset="TX Latin Subset";

	private cSize msiCell;
	private cSize msiPage;
	private final int FONT_HEGIHT_OFFSET = 2;
	//Draw Teletext	
	private Bitmap	mBitmap;
	private Canvas	mCanvas;
	private Path	mPath;
	private Paint	mPaintBG, mPaintFont;
	
	private cFontInfo	mFont;
	private boolean misTTXSubtDraw = false, misStartTeletext=true;	
	private DxbPlayer mPlayer;

	// Teletext Information
	public class Page_Descriptor
	{
		public String	sLanguageCode;
		public int		iPage;
	}

	public class TTX
	{
		public boolean VISIBLE	= true;

		public Page_Descriptor Init = new Page_Descriptor();
		public Page_Descriptor[] Subtitle;

		public int iCount_Subtitle;
		public boolean isDisplay_Subtitle;

		public int Subset;
		public int NationalOption;
	}
	private TTX cTTX = new TTX();
	
	public DxbTeletext(Context context, DxbPlayer player)
	{
		super(context);

		//Initialize table
		gUnicodeSubSet = new LATIN_NATIONAL_SUB_SET[13][13];
		for(int i=0;i<13;i++)
		{			
			for(int j=0;j<13;j++)
			{
				gUnicodeSubSet[i][j] = new LATIN_NATIONAL_SUB_SET();
				gUnicodeSubSet[i][j].bFontType = (byte)gUnicodeSubSetData[i][j][0];
				gUnicodeSubSet[i][j].wFontSubSet = (char)gUnicodeSubSetData[i][j][1];
			}
		}

		mPlayer = player;

		mFont = new cFontInfo();
		
		int width, height;
		msiCell = new cSize(16, 14);		
		width = msiCell.cx*TELETEXT_PAGE_DISPLAY_COLUMN;
		height = msiCell.cy*TELETEXT_PAGE_DISPLAY_ROW;
					
		msiPage = new cSize(width, height);				
				
		mBitmap	= Bitmap.createBitmap(msiPage.cx, msiPage.cy, Bitmap.Config.ARGB_8888);
		mCanvas	= new Canvas(mBitmap);
		mPath	= new Path();
		mPaintBG = new Paint(Paint.ANTI_ALIAS_FLAG);
		mPaintFont = new Paint(Paint.ANTI_ALIAS_FLAG);
		initBitmap();		
	}
	
	private void initBitmap()
	{
		//Rect rect = new Rect();
		//rect.set(0, 0, msiPage.cx, msiPage.cy);				
		//mCanvas.clipRect(rect);
		//mCanvas.translate(4, 4);		
		mCanvas.drawColor(Color.BLACK);		
		//mCanvas.drawColor(Color.argb(0, 0, 0, 0));
		
		mPaintBG.setColor(Color.BLUE);
		mPaintBG.setStrokeWidth(4);				//	Line width
		mPaintBG.setStyle(Paint.Style.STROKE);	//	No fill.
		
		mPath.addRect(0, 0, msiPage.cx, msiPage.cy, Path.Direction.CW);		
		mCanvas.drawPath(mPath, mPaintBG);		//	draw Canvas
				
		mPaintFont.setColor(Color.WHITE);
		mPaintFont.setTypeface(Typeface.create(Typeface.MONOSPACE, Typeface.NORMAL));	//	Text type		
		//mPaintFont.setTypeface(Typeface.MONOSPACE);
		
		mPaintFont.setTextSize(msiCell.cy-FONT_HEGIHT_OFFSET);		
		//mCanvas.drawText("Hello World", 0, msiCell.cy, mPaintFont);
		//ttxView.setScaleType(ImageView.ScaleType.CENTER);
		mPath.reset();
		mPaintBG.setStyle(Paint.Style.FILL);
		//mCanvas.translate(0, msiCell.cy);		
	}
	
	private void Space(int x, int y, int w, int h, boolean bDoubleWidth, boolean bDoubleHeight)
	{
	    if(misTTXSubtDraw)
	    {
	        return;
        }

	    Rect rt = new Rect(x, y, x+w, y+h);

	    if( bDoubleWidth )
	        rt.right+=w;

	    if( bDoubleHeight )
	        rt.bottom+=h;

	    //::FillRect(hDC, &rt, hBrush);	    
        mPath.reset();
        mPath.addRect(rt.left, rt.top, rt.right, rt.bottom, Path.Direction.CW);		
        mCanvas.drawPath(mPath, mPaintBG);		//	draw Canvas
    }
	
	private void eraseSpace(int x, int y, int w, int h, boolean bDoubleWidth, boolean bDoubleHeight)
	{
		if(misTTXSubtDraw == false)
		{
			return;
		}
		
	    Rect rt = new Rect(x, y, x+w, y+h);

	    if( bDoubleWidth )
	        rt.right+=w;

	    if( bDoubleHeight )
	        rt.bottom+=h;

	    mPath.reset();
	    mPath.addRect(rt.left, rt.top, rt.right, rt.bottom, Path.Direction.CW);		
		mCanvas.drawPath(mPath, mPaintBG);		//	draw Canvas	    
	}
	
	private boolean renderingRow(TeletextData pPage, int Row, boolean bDoubleHeight)
	{
		int Column;
		boolean bDoubleWidth=false;
	    for(Column=0; Column<TELETEXT_PAGE_DISPLAY_COLUMN; ++Column)
	    {
	        if(		1<Row && (0x0d==pPage.mData[(Row-1)*TELETEXT_PAGE_DISPLAY_COLUMN+Column]
	        	||	0x0f==pPage.mData[(Row-1)*TELETEXT_PAGE_DISPLAY_COLUMN+Column])
	        )
	        {
	            return false;
	        }

	        if( 0x0d==pPage.mData[Row*TELETEXT_PAGE_DISPLAY_COLUMN+Column] || 0x0f==pPage.mData[Row*TELETEXT_PAGE_DISPLAY_COLUMN+Column] )
	        {
	            bDoubleHeight=true;
	            if( 0x0f==pPage.mData[Row*TELETEXT_PAGE_DISPLAY_COLUMN+Column] )
	            {
	                bDoubleWidth=true;
	            }
	            break;
	        }
	    }
	    
	    // Start-of-row default conditions
	    boolean bAlpha=true;	    
	    mPaintFont.setColor(Color.WHITE);; //white
	    mPaintBG.setColor(mCLUT[0]);
	    int h=msiCell.cx;
	    int w=msiCell.cy;
	    boolean bConceal=false;
	    boolean bContiguousMosaicGraphics=false;
	    boolean bEscOrSwitch=false;
	    String st = new String();
	    boolean eraseLine = true;
	    for(Column=0; Column<TELETEXT_PAGE_DISPLAY_COLUMN; ++Column)
	    {
	        char c=(char)pPage.mData[Row*TELETEXT_PAGE_DISPLAY_COLUMN+Column];
	        switch( c )
	        {
	        case SPACING_ALPHA_COLOR_BLACK:
	        case SPACING_ALPHA_COLOR_RED:
	        case SPACING_ALPHA_COLOR_GREEN:
	        case SPACING_ALPHA_COLOR_YELLOW:
	        case SPACING_ALPHA_COLOR_BLUE:
	        case SPACING_ALPHA_COLOR_MAGENTA:
	        case SPACING_ALPHA_COLOR_CYAN:
	        case SPACING_ALPHA_COLOR_WHITE:
	            Space(Column*msiCell.cx, Row*msiCell.cy, msiCell.cx, msiCell.cy, bDoubleWidth, bDoubleHeight);

	            bAlpha=true;
	            bConceal=false;
	            //::SetTextColor(hDC, mCLUT[0][c]);
	            mPaintFont.setColor(mCLUT[c]);
	            break;

	        case SPACING_FLASH:
	        case SPACING_STEADY:
	            Space(Column*msiCell.cx, Row*msiCell.cy, msiCell.cx, msiCell.cy, bDoubleWidth, bDoubleHeight);
	            break;

	        case SPACING_END_BOX:
	        case SPACING_START_BOX:     	
	            Space(Column*msiCell.cx, Row*msiCell.cy, msiCell.cx, msiCell.cy, bDoubleWidth, bDoubleHeight);
	            break;

	        case SPACING_NORMAL_SIZE:
	            w=msiCell.cx;
	            h=msiCell.cy;
	            Space(Column*msiCell.cx, Row*msiCell.cy, msiCell.cx, msiCell.cy, bDoubleWidth, bDoubleHeight);
	            break;

	        case SPACING_DOUBLE_HEIGHT:
	            Space(Column*msiCell.cx, Row*msiCell.cy, msiCell.cx, msiCell.cy, bDoubleWidth, bDoubleHeight);
	            h=msiCell.cy*2;
	            break;

	        case SPACING_DOUBLE_WIDTH:
	            Space(Column*msiCell.cx, Row*msiCell.cy, msiCell.cx, msiCell.cy, bDoubleWidth, bDoubleHeight);
	            w=msiCell.cx*2;
	            break;

	        case SPACING_DOUBLE_SIZE:
	            Space(Column*msiCell.cx, Row*msiCell.cy, msiCell.cx, msiCell.cy, bDoubleWidth, bDoubleHeight);
	            w=msiCell.cx*2;
	            h=msiCell.cy*2;
	            break;

	        case SPACING_MOSAIC_COLOR_BLACK:
	        case SPACING_MOSAIC_COLOR_RED:
	        case SPACING_MOSAIC_COLOR_GREEN:
	        case SPACING_MOSAIC_COLOR_YELLOW:
	        case SPACING_MOSAIC_COLOR_BLUE:
	        case SPACING_MOSAIC_COLOR_MAGENTA:
	        case SPACING_MOSAIC_COLOR_CYAN:
	        case SPACING_MOSAIC_COLOR_WHITE:
	            Space(Column*msiCell.cx, Row*msiCell.cy, msiCell.cx, msiCell.cy, bDoubleWidth, bDoubleHeight);

	            bAlpha=false;
	            bConceal=false;
	            //::SetTextColor(hDC, mCLUT[0][c-0x10]);
	            mPaintFont.setColor(mCLUT[c-0x10]);	            
	            break;

	        case SPACING_CONCEAL:
	            bConceal=true;
	            Space(Column*msiCell.cx, Row*msiCell.cy, msiCell.cx, msiCell.cy, bDoubleWidth, bDoubleHeight);
	            break;

	        case SPACING_CONTIGUOUS_MOSAIC_GRAPHICS:
	        case SPACING_SEPARATED_MOSAIC_GRAPHICS:
	            //bContiguousMosaicGraphics=(BOOL)SPACING_CONTIGUOUS_MOSAIC_GRAPHICS==c;
	            Space(Column*msiCell.cx, Row*msiCell.cy, msiCell.cx, msiCell.cy, bDoubleWidth, bDoubleHeight);
	            break;

	        case SPACING_ESC_OR_SWITCH:
	            if( bAlpha )
	            {
	                bEscOrSwitch=bEscOrSwitch?false:true;
	            }
	            else
	            {
	                bAlpha=true;
	                bEscOrSwitch=false;
	            }
	            Space(Column*msiCell.cx, Row*msiCell.cy, msiCell.cx, msiCell.cy, bDoubleWidth, bDoubleHeight);
	            break;

	        case SPACING_BLACK_BACKGROUND:
	            //::SetBkColor(hDC, mCLUT[0][0]);

	            //::DeleteObject(hBrushBackground);
	            //hBrushBackground=::CreateSolidBrush(RGB(0, 0, 0));
	        	//mPath.reset();
	        	mPaintBG.setColor(mCLUT[0]);
	            Space(Column*msiCell.cx, Row*msiCell.cy, msiCell.cx, msiCell.cy, bDoubleWidth, bDoubleHeight);
	            break;

	        case SPACING_NEW_BACKGROUND:
	            //::SetBkColor(hDC, ::GetTextColor(hDC));

	            //::DeleteObject(hBrushBackground);
	            //hBrushBackground=::CreateSolidBrush(::GetTextColor(hDC));
	        	//mPath.reset();
	        	mPaintBG.setColor(mPaintFont.getColor());
	            Space(Column*msiCell.cx, Row*msiCell.cy, msiCell.cx, msiCell.cy, bDoubleWidth, bDoubleHeight);
	            break;

	        case SPACING_HOLD_MOSAICS:
	        case SPACING_RELEASE_MOSAICS:
	            bAlpha=true;
	        case SPACING_UNUSED_CHARACTER:
	            Space(Column*msiCell.cx, Row*msiCell.cy, msiCell.cx, msiCell.cy, bDoubleWidth, bDoubleHeight);
	            break;

	        default:	 	        	
	            Space(Column*msiCell.cx, Row*msiCell.cy, msiCell.cx, msiCell.cy, bDoubleWidth, bDoubleHeight);	            
	            if( false==bConceal )
	            {           	
	                int FontWeight=0;
	                boolean FontItalic=false;
	                boolean FontUnderline=(0x20==(0x20&pPage.mAttribute[Row*TELETEXT_PAGE_DISPLAY_COLUMN+Column]));

	                int FontWidth=0;
	                int FontHeight=h;

	                char t=c;
	                String pszFontName="system";
	                if( bAlpha )
	                {
	                    if( c==0x23 || c==0x24 ) // '#' || '$'
	                    {
	                        t=gUnicodeSubSet[pPage.mNationalOptionCharacterSubset][c-0x23].wFontSubSet;
	                        pszFontName=gUnicodeSubSetFont[gUnicodeSubSet[pPage.mNationalOptionCharacterSubset][c-0x23].bFontType];
	                    }
	                    else if( c==0x40 ) // '@'
	                    {	
	                        t=gUnicodeSubSet[pPage.mNationalOptionCharacterSubset][2].wFontSubSet;
	                        pszFontName=gUnicodeSubSetFont[gUnicodeSubSet[pPage.mNationalOptionCharacterSubset][2].bFontType];
	                    }
	                    else if( c>=0x5b && c<=0x60 )
	                    {
	                        t=gUnicodeSubSet[pPage.mNationalOptionCharacterSubset][c-0x5b+3].wFontSubSet;
	                        pszFontName=gUnicodeSubSetFont[gUnicodeSubSet[pPage.mNationalOptionCharacterSubset][c-0x5b+3].bFontType];
	                    }
	                    else if( c>=0x7b && c<=0x7e )
	                    {
	                        t=gUnicodeSubSet[pPage.mNationalOptionCharacterSubset][c-0x7b+9].wFontSubSet;
	                        pszFontName=gUnicodeSubSetFont[gUnicodeSubSet[pPage.mNationalOptionCharacterSubset][c-0x7b+9].bFontType];
	                    }
	                    else
	                    {
	                        pszFontName=bEscOrSwitch?"TX G2 Latin Supplementary":"system";
	                    }
	                }
	                else
	                {
	                    if( (c>=(char)'A') && (c<=(char)'Z') )
	                    {
	                        pszFontName="system";
	                    }
	                    else
	                    {
	                        if( 0x40==c )
	                        {
	                            pszFontName=gLatinSubset;
	                            c=(char)(gSubset[0][2]-2);
	                        }
	                        else if( c>=0x5b && c<=0x5f )
	                        {
	                            pszFontName=gLatinSubset;
	                            c=(char)(gSubset[0][c-0x5b+3]-2);
	                        }
	                        else
	                        {
	                            pszFontName=gG1BlockMosaic;
	                        }
	                    }
	                    if( bContiguousMosaicGraphics )
	                    {
	                        FontWidth=w;
	                    }
	                    else
	                    {
	                        FontWidth=w/2;
	                        FontHeight=h/2;
	                    }
	                }
	                //HFONT hFont=(HFONT)::SelectObject(hDC, Font(FontHeight, FontWidth, FontWeight, FontItalic, FontUnderline, pszFontName));
	                //::ExtTextOut(hDC, Column*msiCell.cx, Row*msiCell.cy, 0, NULL, (LPCTSTR)&t, 1, NULL);
	                //::SelectObject(hDC, hFont);
	                
	                if(eraseLine && misTTXSubtDraw){
	                    eraseSpace(0,Row*msiCell.cy,msiCell.cx*TELETEXT_PAGE_DISPLAY_COLUMN, msiCell.cy+2, bDoubleWidth, bDoubleHeight);	
                        eraseLine = false;
                    }

	                setFont(FontHeight, FontWidth, FontWeight, FontItalic, FontUnderline, pszFontName);
	                st += t;
	                int y_offset = msiCell.cy-FONT_HEGIHT_OFFSET;
	                if(bDoubleHeight) {
	                	y_offset *= 2;
	                }	                
	                mCanvas.drawText(st, Column*msiCell.cx, Row*msiCell.cy+y_offset, mPaintFont);
	                st ="";	                 
	                break;
	            }
	        }
	    }
		return true;
	}
	
	private void setFont(int height, int width, int weight, boolean italic, boolean underline, String fontname )
	{						
		if(		mFont.mHeight == height
			&&	mFont.mWidth == width
			&&	mFont.mWeight == weight
			&&	mFont.mItalic == italic
			&&	mFont.mUnderline == underline
			&&	mFont.mFontName == fontname
		)
		{				
			return;
		}
		
		mFont.mHeight = height;
		mFont.mWidth = width;
		mFont.mWeight = weight;
		mFont.mItalic = italic;
		mFont.mUnderline = underline;
		mFont.mFontName = fontname;
		
		Typeface fontTypeface = null;
		float scaleX = 1;
		if(fontname == "system") {
			//Log.d(TAG, "Set TTX font = " + fontname);
			fontTypeface = Typeface.create(Typeface.MONOSPACE, Typeface.NORMAL);			
		} else if(fontname == "Symbol") {
			//Log.d(TAG, "Set TTX font = " + fontname);
			fontTypeface = Typeface.create(Typeface.MONOSPACE, Typeface.NORMAL);
		} else if(fontname == "TX Latin Subset") {
			//Log.d(TAG, "Set TTX font = " + fontname);
			fontTypeface = Typeface.createFromFile("/system/fonts/subset.ttf");
		} else if(fontname == "TX G1 Block Mosaic") {
			//Log.d(TAG, "Set TTX font = " + fontname);
			fontTypeface = Typeface.createFromFile("/system/fonts/g1_block_mosaic.ttf");			
		}
		else {
			fontTypeface = Typeface.create(Typeface.MONOSPACE, Typeface.NORMAL);
		}
		if(width > height/2) {
			scaleX = (float)width/((float)height/(float)2) + 1;
		}
		mPaintFont.setTextScaleX(scaleX);
		mPaintFont.setUnderlineText(underline);
		mPaintFont.setTextSize((float)height); 
		mPaintFont.setTypeface(fontTypeface);		
        //Log.d(TAG, fontname + "-" + scaleX + "-" + underline + "-" + height);
	}

	public void Rendering(TeletextData pPage, boolean bErase, boolean bforceDisplay)
	{
		//Log.i(TAG, "Rendering()");
		
		int y=0;
		boolean updateScreen = false;
        if(misStartTeletext == false)
            return;

		if (misTTXSubtDraw)
		{			
			y = 1;
		}		
		
		if (pPage != null)
		{
			if (pPage.mErase == 1)
			{
				if (misTTXSubtDraw)
				{
					mCanvas.drawColor(0, PorterDuff.Mode.CLEAR);
					updateScreen = true;				
				}				
			}
			else
			{
				for (; y<TELETEXT_PAGE_DISPLAY_ROW; ++y)
				{
		            if( bforceDisplay || pPage.mbUpdated[y] == 1 )
		            {	            	
		                boolean bDoubleHeight=false;
		                if( renderingRow(pPage, y, bDoubleHeight) )
		                {
		                	updateScreen = true;
		                }
		            }
		        }
			}
		}
		
		if(updateScreen)
		{
			if(misTTXSubtDraw)
			{				
				setImageBitmap(mBitmap);
			}
			else
			{
				mPlayer.updateTeletextData(mBitmap);
			}
		}
	}

	public void startTeletext(boolean isSubtitle) {
		misTTXSubtDraw = isSubtitle;
		misStartTeletext = true;
		mCanvas.drawColor(0, PorterDuff.Mode.CLEAR);
		if(misTTXSubtDraw == false) {
			initBitmap();
			setVisibility(View.INVISIBLE);
		} else {
			setVisibility(View.VISIBLE);
		}
	}

	public void stopTeletext(){
	    misStartTeletext = false;
   		mFont.mHeight = 0;
		mFont.mWidth = 0;
		mFont.mWeight = 0;

        mCanvas.drawColor(0, PorterDuff.Mode.CLEAR);
		if(misTTXSubtDraw){				
			setImageBitmap(mBitmap);
		}
		setVisibility(View.INVISIBLE);
		cTTX.isDisplay_Subtitle	= false;
    }

	public void startSubtitle(int i) {
		startTeletext(true);
		mPlayer.playTeletext(2);	// 0-off, 1-on, 2-subtitle
		mPlayer.setTeletext_UpdatePage(cTTX.Subtitle[i].iPage);
	}
	
	public int getTTX_initPage() {
		return cTTX.Init.iPage;
	}

	public String getTTX_LanguageCode(int i) {
		return cTTX.Subtitle[i].sLanguageCode.substring(0, 3);
	}

	public int getSubtitleCount() {
		return cTTX.iCount_Subtitle;
	}

	public void setTeletextInformation(Cursor init_cursor, Cursor subtitle_cursor)
	{
		cTTX.Init.sLanguageCode	= "eng";
		cTTX.Init.iPage			= 100;
		cTTX.iCount_Subtitle	= 0;

		if (init_cursor != null)
		{
			int count	= init_cursor.getCount();
			if(count > 0)
			{
				init_cursor.moveToFirst();
				for (int i=0 ; i<count ; i++)
				{
					cTTX.Init.sLanguageCode	= init_cursor.getString(1);
					cTTX.Init.iPage			= init_cursor.getInt(2)*100 + init_cursor.getInt(3);
					init_cursor.moveToNext();
				}
			}
			
			init_cursor.close();
		}
		
		if(subtitle_cursor != null)
		{
			cTTX.iCount_Subtitle	= subtitle_cursor.getCount();
			if(cTTX.iCount_Subtitle > 0)
			{
				cTTX.Subtitle	= new Page_Descriptor[cTTX.iCount_Subtitle];
				subtitle_cursor.moveToFirst();
				for(int i=0 ; i<cTTX.iCount_Subtitle ; i++)
				{
					cTTX.Subtitle[i]				= new Page_Descriptor();
					cTTX.Subtitle[i].sLanguageCode	= subtitle_cursor.getString(1);
					cTTX.Subtitle[i].iPage			= subtitle_cursor.getInt(2)*100 + subtitle_cursor.getInt(3);
					subtitle_cursor.moveToNext();
				}
			}
			
			subtitle_cursor.close();
		}
	}
}
