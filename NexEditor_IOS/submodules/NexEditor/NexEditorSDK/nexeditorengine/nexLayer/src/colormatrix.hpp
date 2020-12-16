#pragma once

struct ColorMatrix{

	float matrix_[20];

	ColorMatrix(){

	}

	ColorMatrix(float* matrix){

		if (matrix)
			memcpy(matrix_, matrix, sizeof(matrix_));
	}

	void set(ColorMatrix& color_matrix){

		memcpy(matrix_, color_matrix.matrix_, sizeof(matrix_));
	}

	void set(float* matrix){

		if (matrix)
			memcpy(matrix_, matrix, sizeof(matrix_));
	}

	void reset() {

		memset(matrix_, 0, sizeof(matrix_));
		matrix_[0] = matrix_[6] = matrix_[12] = matrix_[18] = 1;
	}

	void setConcat(ColorMatrix* matA, ColorMatrix* matB) {

		float tmp_matrix[20];
		float *tmp;

		if (matA == this || matB == this) {

			tmp = tmp_matrix;
		}
		else {

			tmp = matrix_;
		}

		const float* a = matA->matrix_;
		const float* b = matB->matrix_;
		int index = 0;
		for (int j = 0; j < 20; j += 5) {
			for (int i = 0; i < 4; i++) {
				tmp[index++] = a[j + 0] * b[i + 0] + a[j + 1] * b[i + 5] +
					a[j + 2] * b[i + 10] + a[j + 3] * b[i + 15];
			}
			tmp[index++] = a[j + 0] * b[4] + a[j + 1] * b[9] +
				a[j + 2] * b[14] + a[j + 3] * b[19] +
				a[j + 4];
		}

		if (tmp != matrix_) {
			
			memcpy(matrix_, tmp, sizeof(matrix_));
		}
	}

	void preConcat(ColorMatrix* prematrix) {

		setConcat(this, prematrix);
	}

	void applyTo44(float* pmatrix){

		pmatrix[0] = matrix_[0];
		pmatrix[1] = matrix_[1];
		pmatrix[2] = matrix_[2];
		pmatrix[3] = matrix_[4];

		pmatrix[4] = matrix_[5];
		pmatrix[5] = matrix_[6];
		pmatrix[6] = matrix_[7];
		pmatrix[7] = matrix_[9];

		pmatrix[8] = matrix_[10];
		pmatrix[9] = matrix_[11];
		pmatrix[10] = matrix_[12];
		pmatrix[11] = matrix_[14];

		pmatrix[12] = matrix_[15];
		pmatrix[13] = matrix_[16];
		pmatrix[14] = matrix_[17];
		pmatrix[15] = 1.0f;
	}

	void setScale(float rScale, float gScale, float bScale, float aScale) {

		for (int i = 0; i < 20; i++) {
			matrix_[i] = 0;
		}

		matrix_[0] = rScale;
		matrix_[6] = gScale;
		matrix_[12] = bScale;
		matrix_[18] = aScale;
	}
};