#define point2 vec2
#define point3 vec3

float distanceSquared(vec2 a, vec2 b) { a -= b; return dot(a, a); }

// Returns true if the ray hit something
bool traceScreenSpaceRay1(
 // Camera-space ray origin, which must be within the view volume
 point3 csOrig, 

 // Unit length camera-space ray direction
 vec3 csDir,

 // A projection matrix that maps to pixel coordinates (not [-1, +1]
 // normalized device coordinates)
 mat4x4 proj, 

 // The camera-space Z buffer (all negative values)
 sampler2D csZBuffer,

 // Dimensions of csZBuffer
 vec2 csZBufferSize,

 // Camera space thickness to ascribe to each pixel in the depth buffer
 float zThickness, 

 // (Negative number)
 float nearPlaneZ, 

 // Step in horizontal or vertical pixels between samples. This is a float
 // because integer math is slow on GPUs, but should be set to an integer >= 1
 float stride,

 // Number between 0 and 1 for how far to bump the ray in stride units
 // to conceal banding artifacts
 float jitter,

 // Maximum number of iterations. Higher gives better images but may be slow
 const float maxSteps, 

 // Maximum camera-space distance to trace before returning a miss
 float maxDistance, 

 // Pixel coordinates of the first intersection with the scene
 out point2 hitPixel, 

 // Camera space location of the ray hit
 out point3 hitPoint) {

    // Clip to the near plane    
    float rayLength = ((csOrig.z + csDir.z * maxDistance) > nearPlaneZ) ?
        (nearPlaneZ - csOrig.z) / csDir.z : maxDistance;
    point3 csEndPoint = csOrig + csDir * rayLength;

    // Project into homogeneous clip space
    vec4 H0 = proj * vec4(csOrig, 1.0);
    vec4 H1 = proj * vec4(csEndPoint, 1.0);
    float k0 = 1.0 / H0.w, k1 = 1.0 / H1.w;

    // The interpolated homogeneous version of the camera-space points  
    point3 Q0 = csOrig * k0, Q1 = csEndPoint * k1;

    // Screen-space endpoints
    point2 P0 = H0.xy * k0, P1 = H1.xy * k1;

    // If the line is degenerate, make it cover at least one pixel
    // to avoid handling zero-pixel extent as a special case later
    P1 += vec2((distanceSquared(P0, P1) < 0.0001) ? 0.01 : 0.0);
    vec2 delta = P1 - P0;

    // Permute so that the primary iteration is in x to collapse
    // all quadrant-specific DDA cases later
    bool permute = false;
    if (abs(delta.x) < abs(delta.y)) { 
        // This is a more-vertical line
        permute = true; delta = delta.yx; P0 = P0.yx; P1 = P1.yx; 
    }

    float stepDir = sign(delta.x);
    float invdx = stepDir / delta.x;

    // Track the derivatives of Q and k
    vec3  dQ = (Q1 - Q0) * invdx;
    float dk = (k1 - k0) * invdx;
    vec2  dP = vec2(stepDir, delta.y * invdx);

    // Scale derivatives by the desired pixel stride and then
    // offset the starting values by the jitter fraction
    dP *= stride; dQ *= stride; dk *= stride;
    P0 += dP * jitter; Q0 += dQ * jitter; k0 += dk * jitter;

    // Slide P from P0 to P1, (now-homogeneous) Q from Q0 to Q1, k from k0 to k1
    point3 Q = Q0; 

    // Adjust end condition for iteration direction
    float  end = P1.x * stepDir;

    float k = k0, stepCount = 0.0, prevZMaxEstimate = csOrig.z;
    float rayZMin = prevZMaxEstimate, rayZMax = prevZMaxEstimate;
    float sceneZMax = rayZMax + 100;
    for (point2 P = P0; 
         ((P.x * stepDir) <= end) && (stepCount < maxSteps) &&
         ((rayZMax < sceneZMax - zThickness) || (rayZMin > sceneZMax)) &&
          (sceneZMax != 0); 
         P += dP, Q.z += dQ.z, k += dk, ++stepCount) {
        
        rayZMin = prevZMaxEstimate;
        rayZMax = (dQ.z * 0.5 + Q.z) / (dk * 0.5 + k);
        prevZMaxEstimate = rayZMax;
        if (rayZMin > rayZMax) { 
           float t = rayZMin; rayZMin = rayZMax; rayZMax = t;
        }

        hitPixel = permute ? P.yx : P;
        // You may need hitPixel.y = csZBufferSize.y - hitPixel.y; here if your vertical axis
        // is different than ours in screen space
        sceneZMax = texelFetch(csZBuffer, int2(hitPixel), 0);
    }
    
    // Advance Q based on the number of steps
    Q.xy += dQ.xy * stepCount;
    hitPoint = Q * (1.0 / k);
    return (rayZMax >= sceneZMax - zThickness) && (rayZMin < sceneZMax);
}


/*
  (C) 2019 David Lettier
  lettier.com
*/

#version 150

uniform mat4 lensProjection;

uniform sampler2D positionTexture;
uniform sampler2D normalTexture;
uniform sampler2D maskTexture;

uniform vec2 enabled;

out vec4 fragColor;

void main() {
    float maxDistance = 8.0;
    float resolution  = 0.3;
    int   steps       = 5;
    float thickness   = 0.5;

    vec2 texSize  = textureSize(positionTexture, 0).xy;
    vec2 texCoord = gl_FragCoord.xy / texSize;

    vec4 uv = vec4(0.0);

    vec4 positionFrom = texture(positionTexture, texCoord);
    vec4 mask         = texture(maskTexture,     texCoord);

    if (positionFrom.w <= 0.0 || enabled.x != 1.0 || mask.r <= 0.0) {
        fragColor = uv;
        return;
    }

    vec3 unitPositionFrom = normalize(positionFrom.xyz);
    vec3 normal           = normalize(texture(normalTexture, texCoord).xyz);
    vec3 pivot            = normalize(reflect(unitPositionFrom, normal));

    vec4 positionTo = positionFrom;

    vec4 startView = vec4(positionFrom.xyz + (pivot * 0.0), 1.0);
    vec4 endView   = vec4(positionFrom.xyz + (pivot * maxDistance), 1.0);

    vec4 startFrag = startView;
    startFrag      = lensProjection * startFrag;
    startFrag.xyz  /= startFrag.w;
    startFrag.xy   = startFrag.xy * 0.5 + 0.5;
    startFrag.xy  *= texSize;

    vec4 endFrag = endView;
    endFrag      = lensProjection * endFrag;
    endFrag.xyz  /= endFrag.w;
    endFrag.xy   = endFrag.xy * 0.5 + 0.5;
    endFrag.xy  *= texSize;

    vec2 frag  = startFrag.xy;
    uv.xy      = frag / texSize;

    float deltaX    = endFrag.x - startFrag.x;
    float deltaY    = endFrag.y - startFrag.y;
    float useX      = abs(deltaX) >= abs(deltaY) ? 1.0 : 0.0;
    float delta     = mix(abs(deltaY), abs(deltaX), useX) * clamp(resolution, 0.0, 1.0);
    vec2  increment = vec2(deltaX, deltaY) / max(delta, 0.001);

    float search0 = 0.0;
    float search1 = 0.0;

    int hit0 = 0;
    int hit1 = 0;

    float viewDistance = startView.y;
    float depth        = thickness;

    float i = 0.0;

    for (i = 0; i < int(delta); ++i) {
        frag      += increment;
        uv.xy      = frag / texSize;
        positionTo = texture(positionTexture, uv.xy);

        search1 = mix(
            (frag.y - startFrag.y) / deltaY,
            (frag.x - startFrag.x) / deltaX,
            useX
        );

        search1 = clamp(search1, 0.0, 1.0);

        viewDistance = (startView.y * endView.y) / mix(endView.y, startView.y, search1);
        depth        = viewDistance - positionTo.y;

        if (depth > 0.0 && depth < thickness) {
            hit0 = 1;
            break;
        } else {
            search0 = search1;
        }
    }

    search1 = search0 + ((search1 - search0) / 2.0);

    steps *= hit0;

    for (i = 0; i < steps; ++i) {
        frag       = mix(startFrag.xy, endFrag.xy, search1);
        uv.xy      = frag / texSize;
        positionTo = texture(positionTexture, uv.xy);

        viewDistance = (startView.y * endView.y) / mix(endView.y, startView.y, search1);
        depth        = viewDistance - positionTo.y;

        if (depth > 0.0 && depth < thickness) {
            hit1 = 1;
            search1 = search0 + ((search1 - search0) / 2.0);
        } else {
            float temp = search1;
            search1 = search1 + ((search1 - search0) / 2.0);
            search0 = temp;
        }
    }

    float visibility =
        hit1 *
        positionTo.w *
        (1.0 - max(dot(-unitPositionFrom, pivot), 0.0)) *
        (1.0 - clamp(depth / thickness, 0.0, 1.0)) *
        (1.0 - clamp(length(positionTo - positionFrom) / maxDistance, 0.0, 1.0)) *
        (uv.x < 0.0 || uv.x > 1.0 ? 0.0 : 1.0) *
        (uv.y < 0.0 || uv.y > 1.0 ? 0.0 : 1.0);

    visibility = clamp(visibility, 0.0, 1.0);

    uv.ba = vec2(visibility);

    fragColor = uv;
}
