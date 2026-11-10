struct VertexOutput {
    float4 position : SV_Position;
    float3 color : COLOR;
};

func generateVertex(vertexID: uint)->VertexOutput {
    static const float2 positions[] = { float2(0.0, -0.5), float2(0.5, 0.5), float2(-0.5, 0.5) };
    VertexOutput output;
    output.position = float4(positions[vertexID], 0.0, 1.0);
    output.color = float3(1.0, 0.0, 0.0);
    return output;
}

[shader("vertex")]
VertexOutput vertexMain(uint vertexID: SV_VertexID) {
    return generateVertex(vertexID);
}

func shadeFragment(input: VertexOutput)->float4 {
    return float4(input.color, 1.0);
}

[shader("fragment")]
float4 fragmentMain(VertexOutput input) : SV_Target {
    return shadeFragment(input);
}
