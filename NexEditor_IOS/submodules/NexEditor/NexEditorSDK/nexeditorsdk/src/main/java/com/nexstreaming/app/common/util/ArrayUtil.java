package com.nexstreaming.app.common.util;

import java.util.Arrays;

public class ArrayUtil {

	private ArrayUtil() {}
	
	public static <T> T[] concat(T[] first, T[]... rest) {
		// From http://stackoverflow.com/questions/80476/how-to-concatenate-two-arrays-in-java
		int totalLength = first.length;
		for (T[] array : rest) {
			totalLength += array.length;
		}
		T[] result = Arrays.copyOf(first, totalLength);
		int offset = first.length;
		for (T[] array : rest) {
			System.arraycopy(array, 0, result, offset, array.length);
			offset += array.length;
		}
		return result;
	}

}
