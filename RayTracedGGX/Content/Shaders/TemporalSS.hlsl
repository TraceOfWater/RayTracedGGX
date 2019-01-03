//--------------------------------------------------------------------------------------
// By Stars XU Tianchen
//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
// Definitions
//--------------------------------------------------------------------------------------
#define	_VARIANCE_AABB_		1

#define	NUM_NEIGHBORS		8
#define	NUM_SAMPLES			(NUM_NEIGHBORS + 1)
#define	NUM_NEIGHBORS_H		4

#define GET_LUMA(v)			dot(v, g_lumBase)

//--------------------------------------------------------------------------------------
// Constants
//--------------------------------------------------------------------------------------
static const min16float3 g_lumBase = { 0.25, 0.5, 0.25 };
static int2 g_texOffsets[] =
{
	int2(-1, 0), int2(1, 0), int2(0, -1), int2(0, 1),
	int2(-1, -1), int2(1, -1), int2(1, 1), int2(-1, 1)
};

//--------------------------------------------------------------------------------------
// Texture and buffers
//--------------------------------------------------------------------------------------
RWTexture2D<min16float4>	RenderTarget;
Texture2DArray<min16float4>	g_currentImage;
Texture2D<min16float4>		g_historyImage;
Texture2D<min16float2>		g_velocity;

//--------------------------------------------------------------------------------------
// Samplers
//--------------------------------------------------------------------------------------
SamplerState g_sampler;

//--------------------------------------------------------------------------------------
// Maxinum velocity of 3x3
//--------------------------------------------------------------------------------------
min16float4 VelocityMax(int2 tex)
{
	min16float4 velocity = g_velocity[tex].xyxy;
	min16float speedSq = dot(velocity.xy, velocity.xy);

	min16float2 velocities[NUM_NEIGHBORS_H];
	[unroll]
	for (uint i = 0; i < NUM_NEIGHBORS_H; ++i)
		velocities[i] = g_velocity[tex + g_texOffsets[i + NUM_NEIGHBORS_H]];

	//[unroll]
	for (i = 0; i < NUM_NEIGHBORS_H; ++i)
	{
		const min16float speedSqN = dot(velocities[i], velocities[i]);
		if (speedSqN > speedSq)
		{
			velocity.xy = velocities[i];
			speedSq = speedSqN;
		}
	}

	return velocity;
}

//--------------------------------------------------------------------------------------
// Minimum and maxinum of the neighbor samples, returning Gaussian blurred color
//--------------------------------------------------------------------------------------
min16float3 NeighborMinMax(out min16float4 neighborMin, out min16float4 neighborMax,
	min16float3 mu, min16float3 current, int2 tex, min16float gamma = 1.0)
{
	static min16float weights[] =
	{
		0.5, 0.5, 0.5, 0.5,
		0.25, 0.25, 0.25, 0.25
	};

	min16float3 neighbors[NUM_NEIGHBORS];
	[unroll]
	for (uint i = 0; i < NUM_NEIGHBORS; ++i)
		neighbors[i] = g_currentImage[uint3(tex + g_texOffsets[i], 1)].xyz;

	min16float3 gaussian = mu;

#if	_VARIANCE_AABB_
#define	m1	mu
	min16float3 m2 = m1 * m1 + current * current;
	m1 = mu + current;
#else
	neighborMin.xyz = neighborMax.xyz = mu;
	neighborMin.xyz = min(current, neighborMin.xyz);
	neighborMax.xyz = max(current, neighborMax.xyz);
#endif

	//[unroll]
	for (i = 0; i < NUM_NEIGHBORS; ++i)
	{
		gaussian += neighbors[i] * weights[i];

#if	_VARIANCE_AABB_
		m1 += neighbors[i];
		m2 += neighbors[i] * neighbors[i];
#else
		neighborMin.xyz = min(neighbors[i], neighborMin.xyz);
		neighborMax.xyz = max(neighbors[i], neighborMax.xyz);
#endif
	}

#if	_VARIANCE_AABB_
	mu /= NUM_SAMPLES + 1;
	const min16float3 sigma = sqrt(abs(m2 / (NUM_SAMPLES + 1) - mu * mu));
	const min16float3 gsigma = gamma * sigma;
	neighborMin.xyz = mu - gsigma;
	neighborMax.xyz = mu + gsigma;
	neighborMin.w = GET_LUMA(mu -sigma);
	neighborMax.w = GET_LUMA(mu + sigma);
#else
	neighborMin.w = GET_LUMA(neighborMin.xyz);
	neighborMax.w = GET_LUMA(neighborMax.xyz);
#endif

	gaussian /= 4.0;

	return gaussian;
}

//--------------------------------------------------------------------------------------
// Clip color
//--------------------------------------------------------------------------------------
min16float3 clipColor(min16float3 color, min16float3 minColor, min16float3 maxColor)
{
	const min16float3 cent = 0.5 * (maxColor + minColor);
	const min16float3 dist = 0.5 * (maxColor - minColor);

	const min16float3 disp = color - cent;
	const min16float3 dir = abs(disp / dist);
	const min16float maxComp = max(dir.x, max(dir.y, dir.z));

	if (maxComp > 1.0) return cent + disp / maxComp;
	else return color;
}

[numthreads(8, 8, 1)]
void main(uint2 DTid : SV_DispatchThreadID)
{
	float2 texSize;
	g_historyImage.GetDimensions(texSize.x, texSize.y);
	const float2 tex = (DTid + 0.5) / texSize;

	const min16float4 current = g_currentImage[uint3(DTid, 0)];
	const min16float4 mu = g_currentImage[uint3(DTid, 1)];
	const min16float4 velocity = VelocityMax(DTid);
	const float2 texBack = tex - velocity.xy;
	min16float4 history = g_historyImage.SampleLevel(g_sampler, texBack, 0);
	history.xyz *= history.xyz;
	
	min16float4 neighborMin, neighborMax;
	min16float3 filtered = NeighborMinMax(neighborMin, neighborMax, mu.xyz, current.xyz, DTid);

	const min16float speed = abs(velocity.x) + abs(velocity.y);
	history.w = speed > 0.0 ? 0.25 : history.w;

	if (speed > 0.0 || current.w <= 0.0 || mu.w <= 0.0)
		history.xyz = clipColor(history.xyz, neighborMin.xyz, neighborMax.xyz);

	const min16float alpha = history.w + 1.0 / 255.0;
	const min16float blend = history.w / alpha;
	min16float3 result = history.w < 1.0 ? lerp(current.xyz, history.xyz, blend) : history.xyz;

	RenderTarget[DTid] = min16float4(sqrt(result), alpha);
	//RenderTarget[DTid] = min16float4(result, alpha) * current.w;
	//RenderTarget[DTid] = g_currentImage[uint3(DTid, 1)];
	//RenderTarget[DTid] = float4(abs(velocity.x) > 1e-5 ? 1.0 : 0.0, abs(velocity.y) > 1e-5 ? 1.0 : 0.0, 0.0, alpha);
}
