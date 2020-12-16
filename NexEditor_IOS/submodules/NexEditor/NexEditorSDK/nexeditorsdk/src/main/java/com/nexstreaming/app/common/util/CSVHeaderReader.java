package com.nexstreaming.app.common.util;

import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import android.util.Log;

import com.nexstreaming.kminternal.kinemaster.config.LL;

public class CSVHeaderReader {
	
	private static final String LOG_TAG = "CSVHeaderReader";

	private CSVReader mReader;
	private List<String> mFirstRecord;
	private List<String> mCurrentRecord;
	private Map<String,Integer> mColMap;
	private String[] mFilterValues;
	private int mRecordNumber;
	
	public CSVHeaderReader(String path) throws IOException {
		mReader = new CSVReader(path);
		init();
	}
	
	public CSVHeaderReader(File path) throws IOException {
		mReader = new CSVReader(path);
		init();
	}
	
	public CSVHeaderReader(InputStream inputStream) throws IOException {
		mReader = new CSVReader(inputStream);
		init();
	}

	public CSVHeaderReader(CSVReader reader) throws IOException {
		mReader = reader;
		init();
	}
	
	public void setFilterValues( String[] filterValues ) {
		mFilterValues = filterValues;
	}
	
	private void init() throws IOException {
		mFirstRecord = mReader.readRecord();
		if( mFirstRecord==null ) {
			throw new IOException("CSV header row missing");
		}
		mColMap = new HashMap<String,Integer>();
		for( int i=0; i<mFirstRecord.size(); i++) {
			mColMap.put(mFirstRecord.get(i), i);
			if( LL.V ) Log.v(LOG_TAG,"colmap " + mFirstRecord.get(i) + ":" + i);
		}
		mRecordNumber = 0;
	}
	
	public boolean next() throws IOException {
		mCurrentRecord = mReader.readRecord();
		if( mCurrentRecord!=null ) {
			mRecordNumber++;
			if( LL.V ) Log.v(LOG_TAG,"got record : " + mCurrentRecord.toString());
		} else {
			if( LL.V ) Log.v(LOG_TAG,"got EOF");
		}
		return mCurrentRecord!=null;
	}
	
	public int getRecordNumber() {
		return mRecordNumber;
	}
	
	public List<String> getCurrentRecord() {
		return mCurrentRecord;
	}
	public void setCurrentRecord( List<String> currentRecord ) {
		mCurrentRecord = currentRecord;
	}
	
	public String getString( String key ) {
		Integer col = mColMap.get(key);
		if( col==null ) {
			throw new RuntimeException("Key not found: " + key);
		}
		if( col >= mCurrentRecord.size() ) {
			throw new RuntimeException("Column missing: " + key);
		}
		String s = mCurrentRecord.get(col).trim();
		if( mFilterValues!=null ) {
			for( String filterValue: mFilterValues ) {
				if( s.equals(filterValue) ) {
					return "";
				}
			}
		}
		return s;
	}
	
	public String getString( String key, String defValue ) {
		Integer col = mColMap.get(key);
		if( col==null ) {
			return defValue;
		}
		if( col >= mCurrentRecord.size() ) {
			return defValue;
		}
		String s = mCurrentRecord.get(col).trim();
		if( mFilterValues!=null ) {
			for( String filterValue: mFilterValues ) {
				if( s.equals(filterValue) ) {
					return defValue;
				}
			}
		}
		return s;
	}
	
	public int getInt( String key ) {
		Integer col = mColMap.get(key);
		if( col==null ) {
			throw new RuntimeException("Key not found: " + key);
		}
		if( col >= mCurrentRecord.size() ) {
			throw new RuntimeException("Column missing: " + key);
		}
		String s = mCurrentRecord.get(col).trim();
		if( mFilterValues!=null ) {
			for( String filterValue: mFilterValues ) {
				if( s.equals(filterValue) ) {
					return 0;
				}
			}
		}
		return Integer.parseInt(s);
	}

	public int getInt( String key, int defValue ) {
		Integer col = mColMap.get(key);
		if( col==null ) {
			return defValue;
		}
		if( col >= mCurrentRecord.size() ) {
			return defValue;
		}
		String s = mCurrentRecord.get(col).trim();
		if( mFilterValues!=null ) {
			for( String filterValue: mFilterValues ) {
				if( s.equals(filterValue) ) {
					return defValue;
				}
			}
		}
		try {
			return Integer.parseInt(s);
		} catch (NumberFormatException e) {
			return defValue;
		}
	}
	
	public void close() throws IOException {
		mReader.close();
	}

}
