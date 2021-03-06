
sampler mytex0 : register(s0) = sampler_state
{
	MinFilter = linear;
	MagFilter = linear;
};

sampler mytex1 : register(s1) = sampler_state
{
	MinFilter = linear;
	MagFilter = linear;
};

matrix matWorld;
matrix matWorldInv;
matrix matViewProj;

float4 lightPos = { -10, 10, -10, 1 };
float4 eyePos;

float heightScale = 0.2f;
float heightBias = 0.01f;

void vs_main(
	in out	float4 pos		: POSITION,
	in		float3 norm		: NORMAL,
	in		float3 tang		: TANGENT,
	in		float3 bin		: BINORMAL,
	in out	float2 tex		: TEXCOORD0,
	out		float3 vdir		: TEXCOORD1,
	out		float3 wtan		: TEXCOORD2,
	out		float3 wbin		: TEXCOORD3,
	out		float3 wnorm	: TEXCOORD4)
{
	wnorm = mul(matWorldInv, float4(norm, 0)).xyz;
	wtan = mul(float4(tang, 0), matWorld).xyz;
	wbin = mul(float4(bin, 0), matWorld).xyz;

	pos = mul(pos, matWorld);
	vdir = pos.xyz - eyePos.xyz;

	pos = mul(pos, matViewProj);
}

void ps_main(
	in	float2 tex		: TEXCOORD0,
	in	float3 vdir		: TEXCOORD1,
	in	float3 wtan		: TEXCOORD2,
	in	float3 wbin		: TEXCOORD3,
	in	float3 wnorm	: TEXCOORD4,
	out	float4 color	: COLOR0)
{
#ifdef KANEKO
	float3 t = normalize(wtan);
	float3 b = normalize(wbin);
	float3 n = normalize(wnorm);

	float2 newtex;
	float depth = tex2D(mytex1, tex).a;
	float tan1, tan2;

	float3 eu = normalize(vdir - dot(vdir, b) * b);
	float3 ev = normalize(vdir - dot(vdir, t) * t);

	tan1 = length(cross(eu, n)) / -dot(eu, n);
	tan2 = length(cross(ev, n)) / -dot(ev, n);

	newtex.x = tex.x + tan1 * depth;
	newtex.y = tex.y + tan2 * depth;

	// TODO: make it good...
	float3 norm = tex2D(mytex1, newtex) * 2 - 1;
	color = float4(norm, 1);
#else
	float3x3 tbn = { wtan, -wbin, wnorm };

	// view direction
	float3 p = vdir;
	float3 v = normalize(vdir);
	float3 s = normalize(mul(tbn, v));

	// calculate an offset from current height
	float curr = tex2D(mytex1, tex).a;
	float height = (1 - curr - heightBias) * heightScale;

	float x = length(s.xy);
	float off = (x * height) / s.z;

	float2 dir = normalize(float2(-s.x, s.y)) * off;

	tex += dir;
	p.xy += dir;

	// put light vector to tangent space
	float3 l = normalize(lightPos.xyz - p);
	l = normalize(mul(tbn, l));

	float3 n = tex2D(mytex1, tex) * 2 - 1;
	float3 h = normalize(l - s);

	// irradiance
	float diffuse = saturate(dot(n, l));
	float specular = saturate(dot(n, h));

	diffuse = diffuse * 0.8f + 0.2f;
	specular = pow(specular, 80);

	// final color
	color = tex2D(mytex0, tex) * diffuse + specular;
	color.a = 1;
#endif
}

technique prallax
{
	pass p0
	{
		vertexshader = compile vs_2_0 vs_main();
		pixelshader = compile ps_2_0 ps_main();
	}
}
