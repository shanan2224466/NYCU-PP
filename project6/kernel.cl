__kernel void convolution(const int imageWidth, const int imageHeight,
						const int filterWidth, const __global float *inputImage,
						__constant float *filter, __global float4 *outputImage)
{
	
	float4 sum = 0.0;

	int globalId = get_global_id(0) * 4;
	int row = globalId / imageWidth;
	int column = globalId % imageWidth;
	int halffilterSize = filterWidth / 2;

	int filterIndex = 0, ix, iy, inputIndex;

	for (int k = -halffilterSize; k <= halffilterSize; k++)
	{
		iy = row + k;
		for (int l = -halffilterSize; l <= halffilterSize; l++)
		{
			ix = column + l;
			if(filter[filterIndex] != 0)
			{
				inputIndex = ix + iy * imageWidth;

				float4 value = (float4)(inputImage[inputIndex], inputImage[inputIndex + 1],
									inputImage[inputIndex + 2], inputImage[inputIndex + 3]);
				sum += value * filter[filterIndex];
			}
			filterIndex++;
		}
	}
	outputImage[globalId / 4] = sum;

	return;
}
