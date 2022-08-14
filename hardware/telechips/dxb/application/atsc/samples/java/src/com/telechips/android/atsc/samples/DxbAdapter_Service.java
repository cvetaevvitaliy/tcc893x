package com.telechips.android.atsc.samples;

import android.content.AsyncQueryHandler;
import android.content.ContentResolver;
import android.content.Context;
import android.database.Cursor;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageView;
import android.widget.SimpleCursorAdapter;
import android.widget.TextView;
import android.util.Log;

public class DxbAdapter_Service extends SimpleCursorAdapter
{
	private static final String TAG = "[[[DxbAdapter_Service]]] ";
	
	private AsyncQueryHandler mQueryHandler;
	private String mConstraint = null;
	private boolean mConstraintIsValid = false;

	private Cursor mCursor;

	class ViewHolder
	{
		TextView	icon_text;
		TextView	num;
		TextView	name;
		ImageView	indicator;
	}

	class QueryHandler extends AsyncQueryHandler
	{
		QueryHandler(ContentResolver res)
		{
			super(res);
			//Log.d(TAG, "QueryHandler()");
		}

		@Override
		protected void onQueryComplete(int token, Object cookie, Cursor cursor)
		{
			//Log.d(TAG, "onQueryComplete()");
		}
	}

	public DxbAdapter_Service(Context context, int layout, Cursor c, String[] from, int[] to)
	{
		super(context, layout, c, from, to);
		//Log.d(TAG, "DxbAdapter_Service()");

		mCursor = c;
		mQueryHandler = new QueryHandler(context.getContentResolver());
		
		DxbPlayer_Control.setService_ColumnIndex(c);
	}

	public AsyncQueryHandler getQueryHandler()
	{
		//Log.d(TAG, "AsyncQueryHandler()");
		return mQueryHandler;
	}

	@Override
	public View newView(Context context, Cursor cursor, ViewGroup parent)
	{
		//Log.d(TAG, "newView()");
		View v = super.newView(context, cursor, parent);
		ViewHolder vh = new ViewHolder();
		
		vh.icon_text	= (TextView)	v.findViewById(R.id.list_row_icon_text);
		vh.num		= (TextView) 	v.findViewById(R.id.list_row_num_text);
		vh.name			= (TextView)	v.findViewById(R.id.list_row_cname_text);
		vh.indicator	= (ImageView)	v.findViewById(R.id.list_row_indicator);

		v.setTag(vh);
		
		return v;
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see android.widget.SimpleCursorAdapter#bindView(android.view.View,
	 * android.content.Context, android.database.Cursor)
	 */
	@Override
	public void bindView(View view, Context context, Cursor cursor)
	{
		//Log.d(TAG, "bindView()");
		ViewHolder vh = (ViewHolder) view.getTag();

		DxbPlayer_Control.bindView(cursor, vh);
	}

	@Override
	public void changeCursor(Cursor cursor)
	{
		super.changeCursor(cursor);
		//Log.d(TAG, "changeCursor()");
	}

	@Override
	public Cursor runQueryOnBackgroundThread(CharSequence constraint)
	{
		//Log.d(TAG, "runQueryOnBackgroundThread()");
		
		String s = constraint.toString();
		if(		mConstraintIsValid
			&&	(		(s == null && mConstraint == null)
					||	(s != null && s.equals(mConstraint))
				)
		)
		{
			return getCursor();
		}
		
		Cursor c = mCursor;
		mConstraint = s;
		mConstraintIsValid = true;
		return c;
	}
}
