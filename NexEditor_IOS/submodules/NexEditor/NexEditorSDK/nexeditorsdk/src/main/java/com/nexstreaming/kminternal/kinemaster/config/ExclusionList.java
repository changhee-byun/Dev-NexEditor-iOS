package com.nexstreaming.kminternal.kinemaster.config;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.io.OptionalDataException;
import java.io.Serializable;
import java.io.StreamCorruptedException;
import java.util.HashMap;
import java.util.Map;

public class ExclusionList implements Serializable {
	
	private static final long serialVersionUID = 1L;

	private transient File m_backingFile = null; 
	private Map<String,Integer> m_refs = null;
	
	private ExclusionList(File f) {
		m_backingFile = f;
		m_refs = new HashMap<String,Integer>();
	}

	public static ExclusionList exclusionListBackedBy( File f ) {
		ExclusionList eList = null;
		if( f.exists() ) {
			try {
				ObjectInputStream in = new ObjectInputStream( new FileInputStream(f));
				eList = (ExclusionList)in.readObject();
				in.close();
			} catch (OptionalDataException e) {
			} catch (StreamCorruptedException e) {
			} catch (FileNotFoundException e) {
			} catch (ClassNotFoundException e) {
			} catch (IOException e) {
			}
		}
		if( eList==null ) {
			eList = new ExclusionList(f);
		} else {
			eList.m_backingFile = f;
		}
		return eList;
	}

	public synchronized void add( String item ) {
		Integer num_refs = m_refs.get(item);
		if( num_refs==null ) {
			m_refs.put(item, 1);
		} else {
			m_refs.put(item, num_refs+1);
		}
		try {
			ObjectOutputStream out = new ObjectOutputStream(new FileOutputStream(m_backingFile));
			out.writeObject(this);
			out.close();
		} catch (FileNotFoundException e) {
		} catch (IOException e) {
		}
	}
	
	public synchronized void remove( String item ) {
		Integer num_refs = m_refs.get(item);
		if( num_refs==null ) {
			return;
		} else {
			if( num_refs==1 ) {
				m_refs.remove(item);
			} else {
				m_refs.put(item, num_refs-1);
			}
		}
		try {
			ObjectOutputStream out = new ObjectOutputStream(new FileOutputStream(m_backingFile));
			out.writeObject(this);
			out.close();
		} catch (FileNotFoundException e) {
		} catch (IOException e) {
		}
	}
	
	public synchronized boolean isExcluded( String item ) {
		Integer num_refs = m_refs.get(item);
		if( num_refs!=null && num_refs > 0 )
			return true;
		else
			return false;
	}
	
}
