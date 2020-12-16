package com.nexstreaming.app.common.util;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.Reader;
import java.util.ArrayList;
import java.util.List;

public class CSVReader {

	private Reader mReader;
	private int mPeakSize = 16;
	private boolean mEOF = false;
	private int mPrevChar = 0; 
	private int mStartQuote = 0;
	
	public CSVReader(String path) throws FileNotFoundException {
		mReader = new BufferedReader(new FileReader(path));
	}
	
	public CSVReader(File path) throws FileNotFoundException {
		mReader = new BufferedReader(new FileReader(path));
	}
	
	public CSVReader(InputStream inputStream) {
		mReader = new BufferedReader(new InputStreamReader(inputStream));
	}

	public List<String> readRecord() throws IOException {
		
		if( mEOF ) {
			return null;
		}
		
		List<String> result = new ArrayList<String>(mPeakSize);
		
		int ch;
		boolean first = true;
		StringBuilder sb = new StringBuilder();
		for(;;) {
			ch = mReader.read();
			if( ch<0 ) {
				mEOF = true;
				if( first ) {
					return null;
				}
				break;
			}
			if( ch==0x0a && mPrevChar==0x0d ) {
				mPrevChar = ch;
				continue;
			}
			if( ch==0x0d || ch==0x0a ) {
				mPrevChar = ch;
				if( mStartQuote!=0 ) {
					sb.append('\n');
					continue;
				} else {
					break;
				}
			}
			first = false;
			if( mStartQuote!=0 ) {
				if( ch==mStartQuote ) {
					mStartQuote=0;
				} else {
					sb.append((char)ch);
				}
			} else {
				if( ch=='"') {
					mStartQuote = ch;
					if( mPrevChar==mStartQuote ) {
						sb.append((char)ch);
					}
				} else if( ch==',') {
					result.add(sb.toString());
					sb.setLength(0);
				} else {
					sb.append((char)ch);
				}
			}
			mPrevChar = ch;
		}
		result.add(sb.toString());
		
		if( result.size() > mPeakSize ) {
			mPeakSize = result.size();
		}
		return result;
	}
	
	public void close() throws IOException {
		mReader.close();
	}
}
