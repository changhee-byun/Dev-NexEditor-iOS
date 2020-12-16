package com.nexstreaming.kminternal.kinemaster.codeccolorformat;

import java.nio.ByteBuffer;

public interface WrapMediaListener {
	boolean frameReceived(int iType, long usTime);
	boolean frameReceived(ByteBuffer buffer, int iType, long usTime);
	boolean frameReceived(byte[] outData, int iType, int iLength, long usTime);
	boolean configReceived(byte[] sps, byte[] pps);
	boolean flagReceived(int iFlag);
}
