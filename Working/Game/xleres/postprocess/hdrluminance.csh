// Copyright 2015 XLGAMES Inc.
//
// Distributed under the MIT License (See
// accompanying file "LICENSE" or the website
// http://www.opensource.org/licenses/mit-license.php)

#include "../TextureAlgorithm.h"
#include "tonemap.h"

Texture2D_MaybeMS<float4>	InputTexture;
RWTexture2D<float>			OutputLuminance : register(u0);
RWTexture2D<float4>			OutputBrightPass : register(u1);

struct LuminanceBufferStruct
{
	float	_currentLuminance;
	float	_prevLuminance;
};

RWStructuredBuffer<LuminanceBufferStruct>	OutputLuminanceBuffer : register(u2);

	//		The "geometric mean" is slightly more expensive,
	//		but might give a slightly nicer result
#define USE_GEOMETRIC_MEAN 1

cbuffer LuminanceConstants
{
	int		FrameIndex;
	int		TotalSamplesCount;
	float	ElapsedTime;
}

float CalculateLuminance(float3 colour)
{
		//
		//	Typical weights for calculating luminance (assuming
		//	linear space input)
		//
		//		See also here, for a description of lightness:
		//			http://www.poynton.com/notes/colour_and_gamma/GammaFAQ.html
		//			--	humans have a non linear response to comparing
		//				luminance values. So perhaps we should use
		//				a curve like this in this calculation.
		//
	const bool usePerceivedBrightness = true;
	if (usePerceivedBrightness) {
			//
			//		See some interesting results from this "perceived brightness"
			//		algorithm:
			//			http://alienryderflex.com/hsp.html
			//			http://stackoverflow.com/questions/596216/formula-to-determine-brightness-of-rgb-color
			//		scroll down on the stackoverflow page to see how this
			//		method distributes brightness evenly and randomly through
			//		a colour cube -- curious property...?
			//
		const float3 componentWeights = float3(0.299f, .587f, .114f);
		return sqrt(dot(componentWeights, colour*colour));
	} else {	
		const float3 componentWeights = float3(0.2126f, 0.7152f, 0.0722f);
		return dot(colour, componentWeights);
	}
}

float3 BrightPassFilter(float3 colour)
{
	const float scale	= SceneKey / OutputLuminanceBuffer[0]._currentLuminance;
	float3 l			= scale * colour.rgb;
	
		//	We could use the whitepoint to calculate
		//	this threshold value... Pixels very close (or over)
		//	the whitepoint should get the bloom.
	const float threshold		= BloomThreshold;
	const float rampingFactor	= BloomRampingFactor;
	l   = saturate(l-float(threshold).rrr);
	l	= l / (rampingFactor+l);
	l = lerp(l, CalculateLuminance(l).xxx, BloomDesaturationFactor);	// desaturate a bit
	return saturate(l);
}

[numthreads(16, 16, 1)]
	void SampleInitialLuminance(uint3 dispatchThreadId : SV_DispatchThreadID)
{
		//
		//		We're sampling from an arbitrarily sized input texture
		//		and writing to a power of 2 texture. The output texture is at
		//		least quarter resolution.
		//
		//		It makes sense to do 4 samples of the input texture -- though
		//		we may loose some information.
		//
		//		Actually, I wonder if it really matters that much. Each individual
		//		pixel will only add a tiny amount to the final average. Small
		//		variations are ignorable. Also, the values are averaged over 
		//		a few frames. 
		//
		//		So perhaps we could get by with sampling fewer input texels, and
		//		varying the sample pattern per frame. Let's try doing a single
		//		sample at this step, but varying that sample per frame.
		//

	uint2 inputDims, outputDims;
	#if MSAA_SAMPLERS != 0
		int ignore;
		InputTexture.GetDimensions(inputDims.x, inputDims.y, ignore);
	#else
		InputTexture.GetDimensions(inputDims.x, inputDims.y);
	#endif
	OutputLuminance.GetDimensions(outputDims.x, outputDims.y);
	
	uint ditherArray[16] = 
	{
		 4, 12,  0,  8,
		10,  2, 14,  6,
		15,  7, 11,  3,
		 1,  9,  5, 13
	};

	int2 ditherAddress	= dispatchThreadId.xy + int2(FrameIndex, FrameIndex);
	uint randomValue	= ditherArray[(ditherAddress.x%4)+(ditherAddress.y%4)*4];

	int2 readOffset		= int2(randomValue%4, randomValue/4);
	float2 sizeRatio	= float2(inputDims) / float2(outputDims);
	float2 readPosition = (float2(dispatchThreadId.xy) + float2(readOffset)/4.f) * sizeRatio;

		// single tap, no bilinear filtering
	#if MSAA_SAMPLERS != 0
		float4 inputColour	= InputTexture.Load(int3(readPosition, 0), 0);
	#else
		float4 inputColour	= InputTexture.Load(int3(readPosition, 0));
	#endif
	float l = CalculateLuminance(inputColour.rgb);

	#if USE_GEOMETRIC_MEAN==1
			//	Using log-average method to calculate geometric mean
		const float tinyValue = 0.000001f;
		float logl = log(tinyValue + l);
		OutputLuminance[dispatchThreadId.xy] = logl;
	#else
		OutputLuminance[dispatchThreadId.xy] = l;
	#endif

		//
		//		When writing the bright pass value, we actually need to read
		//		all of the input pixels. Otherwise, we get a lot of flickering
		//		as bright pixels move in and out of sampling.
		//
		//		Take the average of the pixels in our area, then run that 
		//		through a single bright pass filter. (ie, this isn't the
		//		average of post-filtered values).
		//
		//		Unfortunately these loops aren't fixed length -- maybe there's
		//		a better way to do this?
		//
	int2 inputMins = int2(float2(dispatchThreadId.xy) * sizeRatio);
	int2 inputMaxs = int2(float2(dispatchThreadId.xy+uint2(1,1)) * sizeRatio);
	float3 acculumatedColour = 0.0.xxx;
	for (int y=inputMins.y; y<inputMaxs.y; ++y) {
		for (int x=inputMins.x; x<inputMaxs.x; ++x) {
			#if MSAA_SAMPLERS != 0
				acculumatedColour += InputTexture.Load(int3(x,y,0), 0).rgb;
			#else
				acculumatedColour += InputTexture.Load(int3(x,y,0)).rgb;
			#endif
		}
	}
	int pixelSampleCount = (inputMaxs.x-inputMins.x)*(inputMaxs.y-inputMins.y);
	OutputBrightPass[dispatchThreadId.xy] = float4(BrightPassFilter(acculumatedColour/float(pixelSampleCount)), 1);
}

[numthreads(16, 16, 1)]
	void LuminanceStepDown(uint3 dispatchThreadId : SV_DispatchThreadID)
{
	uint2 outputDims;
	OutputLuminance.GetDimensions(outputDims.x, outputDims.y);
	if (dispatchThreadId.x < outputDims.x && dispatchThreadId.y < outputDims.y) {
			//	Each step-down is a quartering step. So each time we add the 
			//	contribution of 4 pixels.
		int2 baseTX = dispatchThreadId.xy*2;
		float l0 = InputTexture[baseTX + int2(0,0)].r;
		float l1 = InputTexture[baseTX + int2(1,0)].r;
		float l2 = InputTexture[baseTX + int2(0,1)].r;
		float l3 = InputTexture[baseTX + int2(1,1)].r;
		OutputLuminance[dispatchThreadId.xy] = l0 + l1 + l2 + l3;
	}
}

[numthreads(16, 16, 1)]
	void BrightPassStepDown(uint3 dispatchThreadId : SV_DispatchThreadID)
{
	uint2 outputDims;
	OutputBrightPass.GetDimensions(outputDims.x, outputDims.y);
	if (dispatchThreadId.x < outputDims.x && dispatchThreadId.y < outputDims.y) {
			//	Each step-down is a quartering step. So each time we add the 
			//	contribution of 4 pixels.
			//	In this case, we need to take the average of the 4 samples
		int2 baseTX = dispatchThreadId.xy*2;
		float3 l0 = InputTexture[baseTX + int2(0,0)].rgb;
		float3 l1 = InputTexture[baseTX + int2(1,0)].rgb;
		float3 l2 = InputTexture[baseTX + int2(0,1)].rgb;
		float3 l3 = InputTexture[baseTX + int2(1,1)].rgb;
		OutputBrightPass[dispatchThreadId.xy] = float4((l0 + l1 + l2 + l3) * 0.25f, 1.f);
	}
}

[numthreads(1,1,1)]
	void UpdateOverallLuminance()
{
		//	Single thread shader -- just updates the lumiance value for this frame.
		//	Input should be a 2x2 texture... so we just read 4 taps here.
	uint2 inputDims;
	#if MSAA_SAMPLERS != 0
		int ignore;
		InputTexture.GetDimensions(inputDims.x, inputDims.y, ignore);
	#else
		InputTexture.GetDimensions(inputDims.x, inputDims.y);
	#endif
	float finalLuminanceSum = 0.f;
	for (uint y=0; y<inputDims.y; ++y) {
		for (uint x=0; x<inputDims.x; ++x) {
			finalLuminanceSum += InputTexture[uint2(x,y)].r;
		}
	}

		//	I hope we have enough precision left to do this
		//	accurately...
	float A;
	if (TotalSamplesCount > 0) {
		A = 1.0f / float(TotalSamplesCount);
	} else {
		A = 1.f;
	}
	#if USE_GEOMETRIC_MEAN==1
		float result = exp(A * finalLuminanceSum);
	#else
		float result = A * finalLuminanceSum;
	#endif

		//	Clamp min/max lumiance values. This prevents unusual situations,
		//	(like brightening a very dark room).
		//	Note that some engines clamp the "luminance/sceneKey" value. But it seems
		//	that it would be better to clamp here -- because otherwise when
		//	lumiance falls below the clamping threshold, it could take some time
		//	to recover and return back to the valid range. That seems like it would
		//	be wrong.
	result = clamp(result, LuminanceMin, LuminanceMax);

	if (isnan(result) || isinf(result)) {
		result = 1.f;
	}

	float prevLum = OutputLuminanceBuffer[0]._currentLuminance;
	if (isnan(prevLum) || isinf(prevLum)) {
		prevLum = result;
	}
	prevLum = clamp(prevLum, LuminanceMin, LuminanceMax);		// clamp to prevent wierd values (eg, on first frame)
	OutputLuminanceBuffer[0]._prevLuminance = prevLum;
	OutputLuminanceBuffer[0]._currentLuminance = lerp(prevLum, result, 0.2f);
}

