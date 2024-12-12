#type vertex
#version 460 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_TexCoords;

out vec2 v_TexCoords;

void main() {
	gl_Position = vec4(a_Position, 1.0f);
	v_TexCoords = a_TexCoords;
}

#type fragment
#version 460 core
out vec4 out_color;
in vec2 v_TexCoords;

uniform sampler2D u_gImage;
uniform float u_ContrastThreshold = 0.05;
uniform float u_RelativeThreshold = 0.1;

float Luminance(vec4 color) {
    return dot(color.rgb, vec3(0.2126729, 0.7151522, 0.0721750));   
}

vec4 FXAAQuality() {
    vec2 UV = v_TexCoords;
    ivec2 texSize = textureSize(u_gImage, 0);
    vec2 texelSize = vec2(1.0f) / texSize;

    vec4 origin = texture(u_gImage, UV);
    float M  = Luminance(origin);
    float E  = Luminance(texture(u_gImage, UV + vec2( texelSize.x,          0.0)));
    float N  = Luminance(texture(u_gImage, UV + vec2(         0.0,  texelSize.y)));
    float W  = Luminance(texture(u_gImage, UV + vec2(-texelSize.x,          0.0)));
    float S  = Luminance(texture(u_gImage, UV + vec2(         0.0, -texelSize.y)));
    float NW = Luminance(texture(u_gImage, UV + vec2(-texelSize.x,  texelSize.y)));
    float NE = Luminance(texture(u_gImage, UV + vec2( texelSize.x,  texelSize.y)));
    float SW = Luminance(texture(u_gImage, UV + vec2(-texelSize.x, -texelSize.y)));
    float SE = Luminance(texture(u_gImage, UV + vec2( texelSize.x, -texelSize.y)));
    
    //计算出对比度的值
    float maxLuma = max(max(max(N, E), max(W, S)), M);
    float minLuma = min(min(min(N, E), min(W, S)), M);
    float contrast =  maxLuma - minLuma;

    //如果对比度值很小，认为不需要进行抗锯齿，直接跳过抗锯齿计算
    if (contrast < max(u_ContrastThreshold, maxLuma * u_RelativeThreshold)) {
        return origin;
    }

    // 先计算出锯齿的方向，是水平还是垂直方向
    float vertical    = abs(N + S - 2 * M) * 2 + abs(NE + SE - 2 * E) + abs(NW + SW - 2 * W);
    float horizontal  = abs(E + W - 2 * M) * 2 + abs(NE + NW - 2 * N) + abs(SE + SW - 2 * S);
    bool isHorizontal = vertical > horizontal;

    //混合的方向
    vec2 pixelStep = isHorizontal ? vec2(0, texelSize.y) : vec2(texelSize.x, 0);
    // 确定混合方向的正负值
    float positive = abs((isHorizontal ? N : E) - M);
    float negative = abs((isHorizontal ? S : W) - M);

    // if(positive < negative) pixelStep = -pixelStep;
    // 算出锯齿两侧的亮度变化的梯度值
    float gradient;
    float oppositeLuminance;
    if (positive > negative) {
        gradient = positive;
        oppositeLuminance = isHorizontal ? N : E;
    } else {
        pixelStep = -pixelStep;
        gradient = negative;
        oppositeLuminance = isHorizontal ? S : W;
    }

    // 这部分是基于亮度的混合系数计算
    float filter_ = 2.0 * (N + E + S + W) + NE + NW + SE + SW;
    filter_ = filter_ / 12.0;
    filter_ = abs(filter_ -  M);
    filter_ = clamp(filter_ / contrast, 0.0, 1.0);
    // 基于亮度的混合系数值
    float pixelBlend = smoothstep(0, 1, filter_);
    pixelBlend = pixelBlend * pixelBlend;
    
    // 下面是基于边界的混合系数计算
    vec2 UVEdge = UV;
    UVEdge += pixelStep * 0.5f;
    vec2 EdgeStep = isHorizontal ? vec2(texelSize.x, 0) : vec2(0, texelSize.y);

    // 这里是定义搜索的步长，步长越长，效果自然越好
    #define _SearchSteps 15.0
    // 未搜索到边界时，猜测的边界距离
    #define _Guess 8.0

    // 沿着锯齿边界两侧，进行搜索，找到锯齿的边界
    float EdgeLuminance = (M + oppositeLuminance) * 0.5f;
    float GradientThreshold = gradient * 0.25f;
    float PLuminanceDelta, NLuminanceDelta, PDistance, NDistance;
    int i;
    for (i = 1; i <= _SearchSteps; ++i) {
        PLuminanceDelta = Luminance(texture(u_gImage, UVEdge + i * EdgeStep)) - EdgeLuminance;
        if (abs(PLuminanceDelta) > GradientThreshold) {
            PDistance = i * (isHorizontal ? EdgeStep.x : EdgeStep.y);
            break;
        }
    }
    if (i == _SearchSteps + 1) {
        PDistance = (isHorizontal ? EdgeStep.x : EdgeStep.y) * _Guess;
    }
    for (i = 1; i <= _SearchSteps; ++i) {
        NLuminanceDelta = Luminance(texture(u_gImage, UVEdge - i * EdgeStep)) - EdgeLuminance;
        if (abs(NLuminanceDelta) > GradientThreshold) {
            NDistance = i * (isHorizontal ? EdgeStep.x : EdgeStep.y);
            break;
        }
    }
    if (i == _SearchSteps + 1) {
        NDistance = (isHorizontal ? EdgeStep.x : EdgeStep.y) * _Guess;
    }

    float EdgeBlend;
    // 这里是计算基于边界的混合系数，如果边界方向错误，直接设为0，如果方向正确，按照相对的距离来估算混合系数
    if (PDistance < NDistance) {
        if (sign(PLuminanceDelta) == sign(M - EdgeLuminance)) {
            EdgeBlend = 0;
        } else {
            EdgeBlend = 0.5f - PDistance / (PDistance + NDistance);
        }
    } else {
        if (sign(NLuminanceDelta) == sign(M - EdgeLuminance)) {
            EdgeBlend = 0;
        } else {
            EdgeBlend = 0.5f - NDistance / (PDistance + NDistance);
        }
    }

    //从两种混合系数中，取最大的那个
    float FinalBlend = max(pixelBlend, EdgeBlend);
    vec4 Result = texture(u_gImage, UV + pixelStep * FinalBlend);
    return Result;
}

 
void main() {
    out_color = FXAAQuality();
}