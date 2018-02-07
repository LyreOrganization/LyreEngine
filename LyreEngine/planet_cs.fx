Texture2DArray<float4>	Terrain : register(t0);
RWTexture1D<float>		Result	: register(u0);

groupshared float	groupResults[64*64];
groupshared float4	plane;
groupshared float3	normals[2][2];
groupshared float3	corners[2][2];

float4 CreatePlaneFromPointAndNormal(float3 n, float3 p) {
	return float4(n, -dot(n, p));
}

float ComputeDistanceFromPlane(float4 plane, float3 position) {
	return dot(plane.xyz, position) + plane.w;
}

[numthreads(32, 32, 1)]
void CS( 
	uint3	Gid		: SV_GroupID,
	uint3	DTid	: SV_DispatchThreadID,
	uint3	GTid	: SV_GroupThreadID,
	uint	GI		: SV_GroupIndex 
) {
	bool isCornerThread = (GTid.x == 0 || GTid.x == 31) && (GTid.y == 0 || GTid.y == 31);
	bool isCorner[2][2] = {
		{
			GTid.x == 0 && GTid.y == 0,
			GTid.x == 0 && GTid.y == 31
		},
		{
			GTid.x == 31 && GTid.y == 0,
			GTid.x == 31 && GTid.y == 31
		}
	};

	if (isCornerThread) {
		groupResults[GI * 4]		= Terrain.Load(int4(GTid.xy * 2,					Gid.x, 0)).w;
		groupResults[GI * 4 + 1]	= Terrain.Load(int4(GTid.x * 2 + 1, GTid.y * 2,		Gid.x, 0)).w;
		groupResults[GI * 4 + 2]	= Terrain.Load(int4(GTid.x * 2,		GTid.y * 2 + 1, Gid.x, 0)).w;
		groupResults[GI * 4 + 3]	= Terrain.Load(int4(GTid.xy * 2 + 1,				Gid.x, 0)).w;

		if (isCorner[0][0]) {
			corners[0][0] = float3(0, groupResults[GI * 4], 0);
		}
		else if (isCorner[1][0]) {
			corners[1][0] = float3(1, groupResults[GI * 4 + 1], 0);
		}
		else if (isCorner[0][1]) {
			corners[0][1] = float3(0, groupResults[GI * 4 + 2], 1);
		}
		else {//(isCorner[1][1])
			corners[1][1] = float3(1, groupResults[GI * 4 + 3], 1);
		}
	}
	
	GroupMemoryBarrierWithGroupSync();

	if (isCorner[0][0]) {
		normals[0][0] = normalize(cross(
			corners[0][1] - corners[0][0],
			corners[1][0] - corners[0][0]
		));
	}
	else if (isCorner[1][0]) {
		normals[1][0] = normalize(cross(
			corners[0][0] - corners[1][0],
			corners[1][1] - corners[1][0]
		));
	}
	else if (isCorner[0][1]) {
		normals[0][1] = normalize(cross(
			corners[1][1] - corners[0][1],
			corners[0][0] - corners[0][1]
		));
	}
	else if (isCorner[1][1]) {
		normals[1][1] = normalize(cross(
			corners[1][0] - corners[1][1],
			corners[0][1] - corners[1][1]
		));
	}
	else {
		groupResults[GI * 4]		= Terrain.Load(int4(GTid.xy * 2,					Gid.x, 0)).w;
		groupResults[GI * 4 + 1]	= Terrain.Load(int4(GTid.x * 2 + 1, GTid.y * 2,		Gid.x, 0)).w;
		groupResults[GI * 4 + 2]	= Terrain.Load(int4(GTid.x * 2,		GTid.y * 2 + 1, Gid.x, 0)).w;
		groupResults[GI * 4 + 3]	= Terrain.Load(int4(GTid.xy * 2 + 1,				Gid.x, 0)).w;
	}

	GroupMemoryBarrierWithGroupSync();

	if (GI == 0) {
		float3 n = normalize(
			normals[0][0] + normals[1][0] + normals[0][1] + normals[1][1]
		);

		float3 p = float3(0.0f, corners[0][0].y, 0.0f);
		for (int i = 0; i < 2; ++i) {
			for (int j = 0; j < 2; ++j) {
				if (corners[i][j].y < p.y) {
					p = corners[i][j];
				}
			}
		}

		plane = CreatePlaneFromPointAndNormal(n, p);
	}

	GroupMemoryBarrierWithGroupSync();

	groupResults[GI * 4] = ComputeDistanceFromPlane(plane, float3(
		(float)GTid.x*2 / 63.0f,
		groupResults[GI * 4],
		(float)GTid.y*2 / 63.0f
	));
	groupResults[GI * 4 + 1] = ComputeDistanceFromPlane(plane, float3(
		(float)(GTid.x*2 + 1) / 63.0f,
		groupResults[GI * 4 + 1],
		(float)GTid.y*2 / 63.0f
	));
	groupResults[GI * 4 + 2] = ComputeDistanceFromPlane(plane, float3(
		(float)GTid.x*2 / 63.0f,
		groupResults[GI * 4 + 2],
		(float)(GTid.y*2 + 1) / 63.0f
	));
	groupResults[GI * 4 + 3] = ComputeDistanceFromPlane(plane, float3(
		(float)(GTid.x*2 + 1) / 63.0f,
		groupResults[GI * 4 + 3],
		(float)(GTid.y*2 + 1) / 63.0f
	));

	GroupMemoryBarrierWithGroupSync();

	if (GI == 0) {
		float deviation = 0.0f;
		for (int i = 0; i < 64 * 64; ++i)
			deviation += pow(groupResults[i], 2);
		deviation /= ((64.0f * 64.0f) - 1.0f);
		deviation = sqrt(deviation);

		Result[Gid.x] = deviation;
	}
}
