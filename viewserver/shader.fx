

Texture2D texZero : register(t0);// - ...(0,1)

Texture2D texOne : register(t1);// - ...(1,1)

SamplerState samLinear : register(s0);

cbuffer CONSTANT_MATRIX_BUFFER : register(b0)
{
	matrix world;
	matrix view;
	matrix projection;
	
	float4 offsets;
	float4 camPos; //<-- float3 will bite you in the butt! - ITS TRUE
	//REMEMBER TO PAD A FLOAT3 INTO A FLOAT4
	float4 offsetWave;
};
//VS AND PS WITH NORMS - WELCOME TO SEMANTIC BINDING!
struct VS_MODEL
{
	float4 Pos : POSITION;
	float2 Tex : TEXTURE;
	float3 Norm : NORMAL;
};
struct PS_MODEL
{
	float4 Pos : SV_Position;
	float2 Tex : TEXTURE;
	float3 Norm : NORMAL;
	float3 WorldPos : WORLD_POS;
};

//DEATHSTAR SHADERS
//FUTURE DEFAULT STRUCTURE FOR VS MODEL WITH NORMS
PS_MODEL vs_deathstar(VS_MODEL input)
{
	PS_MODEL output;

	output.Pos = mul(world, input.Pos);
	output.WorldPos = output.Pos.xyz;
	output.Pos = mul(view, output.Pos);
	output.Pos = mul(projection, output.Pos);
	output.Tex = input.Tex;
	output.Norm = mul(world, float4(input.Norm, 0));
	return output;
}


float4 ps_deathstar(PS_MODEL input) : SV_Target
{
	//ALWAYS correct norms after translation!!
	input.Norm = normalize(input.Norm);

	//our object'S texture
	float4 texture1color = texZero.Sample(samLinear, input.Tex);

	//lightsource
	float3 lightPos = float3(1000, 2000, -1000);

	//ambient
	float4 amb = (0.0005, 0.0005, 0.0005, 0.1) * texture1color;

	//diffuse light
	float3 lightDir = normalize(lightPos - input.WorldPos);
	float diffuse = saturate(dot(lightDir, input.Norm));//saturate(x) ... if(x<0)x=0;else if(x>1) x=1;

	//specular
	float specular = 0;
	//if (dot(lightDir, input.Norm) <= 0) {
		float3 r = reflect(-lightDir, input.Norm);
		float3 camdir = camPos - input.WorldPos;
		camdir = normalize(camdir);
		specular = dot(r, camdir);
		specular = saturate(specular);
		specular = pow(specular, 30);
	//}

	//light calculation:
	float4 SpecLightColor = float4(1, 1, 1, 1);//light color
	float4 color = texture1color * diffuse + SpecLightColor*specular + amb;
	color.a = 1;
	return color;
}

//VS AND PS WITH NO NORMS 
struct VS_INPUT
{
	float3 Pos : POSITION;
	float2 Tex : TEXTURE;
};
struct PS_INPUT
{
	float4 Pos : SV_Position;
	float2 Tex : TEXTURE;
	float3 ViewPos : VIEW_POS;
};

//TERRAIN SHADERS
PS_INPUT vs_terrain(VS_INPUT input)
{
	PS_INPUT output;

	float4 pos = float4(input.Pos, 1);//HERE'S A BITE

	pos = mul(world, pos);
	//PROVIDES THE MOTION FOR THE TEXTURE
	//NOTe: SAMPLELEVEL v. SAMPLE
	float4 color1 = texZero.SampleLevel(samLinear, input.Tex + offsets.xy, 0);
	pos.y += (color1.r) *17-20;//HEIGHTMAPPING AND LOWER

	pos = mul(view, pos);
	output.ViewPos = pos.xyz;
	pos = mul(projection, pos);
	output.Pos = pos;
	output.Tex = input.Tex;

	return output;
}

float4 ps_terrain(PS_INPUT input) : SV_Target
{	//ALLOWS COLOR TO MOVE WITH THE TEXTURE
	float4 color = texOne.Sample(samLinear, (input.Tex + offsets.xy) * 70);//multiply number of textures
	float h = texZero.Sample(samLinear, input.Tex + offsets.xy).r;
	h *= 1.5;//ADDS LIGHT AND DARKNESS BASED ON THE HEIGHT FROM RED
	color.rgb *= h;
	float dist = length(input.ViewPos.xz) / 220;//FAR BOUNDARY
	float4 far = float4(float3(1, 1, 1) * .2, 1);//FOG COLOR
	float4 finalColor = lerp(color, far, saturate(pow(dist,6)) ); //<-- saturate faster than min - WANTED TO RALLY PUSH THE 'FOG' BACK WITH A DARK BACKGROUND
	
	return finalColor;
}

//WATER SHADER
PS_INPUT vs_water(VS_INPUT input)
{
	PS_INPUT output;
	float4 pos = float4(input.Pos, 1);//HERE'S A BITE!

	pos = mul(world, pos);

	//USE THE COLOR AS A HEIGHT MULTIPLIER -  ALONG THE Y AXIS
	float4 color1 = texZero.SampleLevel(samLinear, input.Tex + float2(offsetWave.x, offsetWave.y)*2, 0);
	float4 color2 = texOne.SampleLevel(samLinear, input.Tex + float2(offsetWave.z, offsetWave.w)*2, 0);
	
	//using the red color of RGB for heightmapping and lower also
	pos.y += (color1.r + color2.r)*3-16; 

	pos = mul(view, pos);
	output.ViewPos = pos.xyz;
	pos = mul(projection, pos);
	output.Pos = pos;
	output.Tex = input.Tex;

	return output;
}

float4 ps_water(PS_INPUT input) : SV_Target
{
	float4 color = texZero.Sample(samLinear, (input.Tex + offsets.xy*2));
	float4 color2 = texOne.Sample(samLinear, (input.Tex + offsets.xy*2));
	//SLIME GREEN ALIEN WATER BASED ON TEXTURES
	color.r = (color.r + color2.r) / 10.0f;
	color.g = (color.g + color2.g) / 7.0f;
	color.b = (color.b + color2.b) / 14.0f;

	// 2 states: color and farcolor
	float dist = length(input.ViewPos.xz) / 220;
	float4 far = float4(float3(1, 1, 1) * .2, 1);
	//FOG WITH THE WATER IF THE SLIME WATER IS HIGHER THAN THE TERRAIN HEIGHTMAPPING
	float4 finalColor = lerp(color, far, saturate(pow(dist, 6)) ); //<-- saturate faster than min

	return finalColor;
}

//FUTURE DEFAULT STRUCTURE FOR VS MODEL WITH NO NORMS
PS_INPUT vs_skybox(VS_INPUT input)
{	
	PS_INPUT output;
	float4 pos = float4(input.Pos, 1);//YES ANOTHER BITE!

	pos = mul(world, pos);
	pos = mul(view, pos);
	pos = mul(projection, pos);
	output.Pos = pos;
	output.Tex = input.Tex;
	
	return output;
}


float4 ps_skybox(PS_INPUT input) : SV_Target
{
	//return float4(input.Tex.y,input.Tex.y,0,1);//USED FOR TESTING
	float4 color = texZero.Sample(samLinear, input.Tex);
	color.a = 1;
	return color;
}


PS_INPUT vs_billboard(VS_INPUT input)
{
	PS_INPUT output;
	float4 pos = float4(input.Pos, 1);//...AND ANOTHER...
	
	pos = mul(world, pos);
	pos = mul(view, pos);
	pos = mul(projection, pos);
	output.Pos = pos;
	output.Tex = input.Tex;
	
	return output;
}


float4 ps_billboard(PS_INPUT input) : SV_Target
{	
	//return float4(input.Tex.y,input.Tex.y,0,1);//USED FOR TESTING
	float4 color = texZero.Sample(samLinear, input.Tex);
	//color.a = 1;//REMEMBER TO DO THIS WITH TRANSPARERNT BACKGROUNDS
	return color;
}