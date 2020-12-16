#pragma once

class Math {

public:
	static double E;
	static double PI;
};

double Math::E = 2.718281828459045;
double Math::PI = 3.141592653589793;

class Matrix{

public:
	static void perspectiveM(float* m, int offset,
		float fovy, float aspect, float zNear, float zFar) {
		float f = 1.0f / (float)tan(fovy * (Math::PI / 360.0));
		float rangeReciprocal = 1.0f / (zNear - zFar);

		m[offset + 0] = f / aspect;
		m[offset + 1] = 0.0f;
		m[offset + 2] = 0.0f;
		m[offset + 3] = 0.0f;

		m[offset + 4] = 0.0f;
		m[offset + 5] = f;
		m[offset + 6] = 0.0f;
		m[offset + 7] = 0.0f;

		m[offset + 8] = 0.0f;
		m[offset + 9] = 0.0f;
		m[offset + 10] = (zFar + zNear) * rangeReciprocal;
		m[offset + 11] = -1.0f;

		m[offset + 12] = 0.0f;
		m[offset + 13] = 0.0f;
		m[offset + 14] = 2.0f * zFar * zNear * rangeReciprocal;
		m[offset + 15] = 0.0f;
	}

	static void scaleM(float* m, int mOffset,
		float x, float y, float z) {
		for (int i = 0; i<4; i++) {
			int mi = mOffset + i;
			m[mi] *= x;
			m[4 + mi] *= y;
			m[8 + mi] *= z;
		}
	}

	static void translateM(
		float* m, int mOffset,
		float x, float y, float z) {
		for (int i = 0; i<4; i++) {
			int mi = mOffset + i;
			m[12 + mi] += m[mi] * x + m[4 + mi] * y + m[8 + mi] * z;
		}
	}

	static void setIdentityM(float* sm, int smOffset) {
		for (int i = 0; i<16; i++) {
			sm[smOffset + i] = 0;
		}
		for (int i = 0; i < 16; i += 5) {
			sm[smOffset + i] = 1.0f;
		}
	}

	static float length(float x, float y, float z) {

		return (float)sqrt(x * x + y * y + z * z);
	}

	static void setRotateM(float* rm, int rmOffset,
		float a, float x, float y, float z) {
		rm[rmOffset + 3] = 0;
		rm[rmOffset + 7] = 0;
		rm[rmOffset + 11] = 0;
		rm[rmOffset + 12] = 0;
		rm[rmOffset + 13] = 0;
		rm[rmOffset + 14] = 0;
		rm[rmOffset + 15] = 1;
		a *= (float)(Math::PI / 180.0f);
		float s = (float)sin(a);
		float c = (float)cos(a);
		if (1.0f == x && 0.0f == y && 0.0f == z) {
			rm[rmOffset + 5] = c;   rm[rmOffset + 10] = c;
			rm[rmOffset + 6] = s;   rm[rmOffset + 9] = -s;
			rm[rmOffset + 1] = 0;   rm[rmOffset + 2] = 0;
			rm[rmOffset + 4] = 0;   rm[rmOffset + 8] = 0;
			rm[rmOffset + 0] = 1;
		}
		else if (0.0f == x && 1.0f == y && 0.0f == z) {
			rm[rmOffset + 0] = c;   rm[rmOffset + 10] = c;
			rm[rmOffset + 8] = s;   rm[rmOffset + 2] = -s;
			rm[rmOffset + 1] = 0;   rm[rmOffset + 4] = 0;
			rm[rmOffset + 6] = 0;   rm[rmOffset + 9] = 0;
			rm[rmOffset + 5] = 1;
		}
		else if (0.0f == x && 0.0f == y && 1.0f == z) {
			rm[rmOffset + 0] = c;   rm[rmOffset + 5] = c;
			rm[rmOffset + 1] = s;   rm[rmOffset + 4] = -s;
			rm[rmOffset + 2] = 0;   rm[rmOffset + 6] = 0;
			rm[rmOffset + 8] = 0;   rm[rmOffset + 9] = 0;
			rm[rmOffset + 10] = 1;
		}
		else {
			float len = length(x, y, z);
			if (1.0f != len) {
				float recipLen = 1.0f / len;
				x *= recipLen;
				y *= recipLen;
				z *= recipLen;
			}
			float nc = 1.0f - c;
			float xy = x * y;
			float yz = y * z;
			float zx = z * x;
			float xs = x * s;
			float ys = y * s;
			float zs = z * s;
			rm[rmOffset + 0] = x*x*nc + c;
			rm[rmOffset + 4] = xy*nc - zs;
			rm[rmOffset + 8] = zx*nc + ys;
			rm[rmOffset + 1] = xy*nc + zs;
			rm[rmOffset + 5] = y*y*nc + c;
			rm[rmOffset + 9] = yz*nc - xs;
			rm[rmOffset + 2] = zx*nc - ys;
			rm[rmOffset + 6] = yz*nc + xs;
			rm[rmOffset + 10] = z*z*nc + c;
		}
	}

	static void multiplyMV(float* result, const float *mat, int rows, const float* vec, int cols)
	{
		int i,j;
		float res;
		for (i = 0; i < rows; i++)
		{			
			res = 0.0f;
			for (j = 0; j < cols; j++)
			{
				res += mat[(i * rows) + j] * vec[j];
			}
			result[i] = res;
		}
	}

	static void multiplyMM(float* result, int resultOffset,
		float* lhs, int lhsOffset, float* rhs, int rhsOffset){

		MultiplyMatrices4by4OpenGL_FLOAT(&result[resultOffset], &lhs[lhsOffset], &rhs[rhsOffset]);
	}

	static void MultiplyMatrices4by4OpenGL_FLOAT(float *result, const float *matrix1, const float *matrix2)
	{
		result[0] = matrix1[0] * matrix2[0] +
			matrix1[4] * matrix2[1] +
			matrix1[8] * matrix2[2] +
			matrix1[12] * matrix2[3];
		result[4] = matrix1[0] * matrix2[4] +
			matrix1[4] * matrix2[5] +
			matrix1[8] * matrix2[6] +
			matrix1[12] * matrix2[7];
		result[8] = matrix1[0] * matrix2[8] +
			matrix1[4] * matrix2[9] +
			matrix1[8] * matrix2[10] +
			matrix1[12] * matrix2[11];
		result[12] = matrix1[0] * matrix2[12] +
			matrix1[4] * matrix2[13] +
			matrix1[8] * matrix2[14] +
			matrix1[12] * matrix2[15];
		result[1] = matrix1[1] * matrix2[0] +
			matrix1[5] * matrix2[1] +
			matrix1[9] * matrix2[2] +
			matrix1[13] * matrix2[3];
		result[5] = matrix1[1] * matrix2[4] +
			matrix1[5] * matrix2[5] +
			matrix1[9] * matrix2[6] +
			matrix1[13] * matrix2[7];
		result[9] = matrix1[1] * matrix2[8] +
			matrix1[5] * matrix2[9] +
			matrix1[9] * matrix2[10] +
			matrix1[13] * matrix2[11];
		result[13] = matrix1[1] * matrix2[12] +
			matrix1[5] * matrix2[13] +
			matrix1[9] * matrix2[14] +
			matrix1[13] * matrix2[15];
		result[2] = matrix1[2] * matrix2[0] +
			matrix1[6] * matrix2[1] +
			matrix1[10] * matrix2[2] +
			matrix1[14] * matrix2[3];
		result[6] = matrix1[2] * matrix2[4] +
			matrix1[6] * matrix2[5] +
			matrix1[10] * matrix2[6] +
			matrix1[14] * matrix2[7];
		result[10] = matrix1[2] * matrix2[8] +
			matrix1[6] * matrix2[9] +
			matrix1[10] * matrix2[10] +
			matrix1[14] * matrix2[11];
		result[14] = matrix1[2] * matrix2[12] +
			matrix1[6] * matrix2[13] +
			matrix1[10] * matrix2[14] +
			matrix1[14] * matrix2[15];
		result[3] = matrix1[3] * matrix2[0] +
			matrix1[7] * matrix2[1] +
			matrix1[11] * matrix2[2] +
			matrix1[15] * matrix2[3];
		result[7] = matrix1[3] * matrix2[4] +
			matrix1[7] * matrix2[5] +
			matrix1[11] * matrix2[6] +
			matrix1[15] * matrix2[7];
		result[11] = matrix1[3] * matrix2[8] +
			matrix1[7] * matrix2[9] +
			matrix1[11] * matrix2[10] +
			matrix1[15] * matrix2[11];
		result[15] = matrix1[3] * matrix2[12] +
			matrix1[7] * matrix2[13] +
			matrix1[11] * matrix2[14] +
			matrix1[15] * matrix2[15];
	}

	static void transposeM(float* mTrans, int mTransOffset, float* m,
		int mOffset) {
		for (int i = 0; i < 4; i++) {
			int mBase = i * 4 + mOffset;
			mTrans[i + mTransOffset] = m[mBase];
			mTrans[i + 4 + mTransOffset] = m[mBase + 1];
			mTrans[i + 8 + mTransOffset] = m[mBase + 2];
			mTrans[i + 12 + mTransOffset] = m[mBase + 3];
		}
	}

	static void rotateM(float* m, int mOffset,
		float a, float x, float y, float z) {

		float tmp[32];
		setRotateM(tmp, 0, a, x, y, z);
		multiplyMM(tmp, 16, m, mOffset, tmp, 0);
		memcpy(m, tmp + 16, sizeof(float) * 16);
	}
};
