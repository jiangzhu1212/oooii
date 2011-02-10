/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2011 Antony Arciuolo & Kevin Myers                       *
 *                                                                        *
 * Permission is hereby granted, free of charge, to any person obtaining  *
 * a copy of this software and associated documentation files (the        *
 * "Software"), to deal in the Software without restriction, including    *
 * without limitation the rights to use, copy, modify, merge, publish,    *
 * distribute, sublicense, and/or sell copies of the Software, and to     *
 * permit persons to whom the Software is furnished to do so, subject to  *
 * the following conditions:                                              *
 *                                                                        *
 * The above copyright notice and this permission notice shall be         *
 * included in all copies or substantial portions of the Software.        *
 *                                                                        *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        *
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     *
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND                  *
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE *
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION *
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION  *
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.        *
 **************************************************************************/
#include "pch.h"
#include <oooii/oTest.h>
#include <oImageProcessor/oImageProcessingFragments.h>

struct TESTImageProcessing : public oTest
{
	static const unsigned int WIDTH = 512;
	static const unsigned int HEIGHT = 512;
	float Temp[WIDTH*HEIGHT];

	RESULT Run(char* _StrStatus, size_t _SizeofStrStatus) override
	{
		oRef<oImageProcessingDevice> IPDevice;
		oTESTB( oImageProcessingDevice::Create( oImageProcessingDevice::DEVICE_OPENCL_CPU, &IPDevice ), "Failed to create OpenCL device" );

		oStreamingImage::DESC ImageDesc( oSurface::R32_FLOAT, 512, 512, 1,  oStreamingImage::DESC::HOST_WRITE_ACCESS);

		oRef<oStreamingImage> InputImage;
		oTESTB( IPDevice->CreateStreamingImage( ImageDesc, &InputImage ), "Failed to create input image" );
		
		oRef<oStreamingImage> OutputImage;
		ImageDesc.access = oStreamingImage::DESC::HOST_READ_ACCESS;
		oTESTB( IPDevice->CreateStreamingImage( ImageDesc, &OutputImage ), "Failed to create output image" );

		oImageProcessingKernel::IMAGE_BINDING Bindings;
		Bindings.inputs[0] = InputImage;
		Bindings.outputImage = OutputImage;

		memset( Temp, 0, oCOUNTOF(Temp) *sizeof(float) );

		oStreamingImage::LOAD Upload;
		Upload.userData = (unsigned char*)Temp;
		Upload.userDataSize = oCOUNTOF(Temp) * sizeof(float);
		Upload.userDataStride = WIDTH * sizeof(float);
		
		oRef<oImageProcessingKernel> TestKernel;
		oTESTB( IPDevice->CreateImageProcessingKernel( Bindings, &TestKernel ), "Failed to create Test Kernel" );
		
		
		AppendBoxFilterFragment( 5, 5, TestKernel );

		for( int i = 0; i < 4; ++i )
		{
			InputImage->UploadImage( Upload );
			IPDevice->ExecuteKernel( TestKernel );
			IPDevice->Flush();
		}

		return SUCCESS;
	}
};

TESTImageProcessing TESTImageProcessing;