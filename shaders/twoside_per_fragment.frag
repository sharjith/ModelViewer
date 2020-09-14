#version 450 core

in vec3 g_position;
in vec3 g_normal;
in vec2 g_texCoord2d;
noperspective in vec3 g_edgeDistance;
in vec3 g_reflectionPosition;
in vec3 g_reflectionNormal;

in GS_OUT_SHADOW {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
    vec4 FragPosLightSpace;
    vec3 cameraPos;
    vec3 lightPos;
} fs_in_shadow;

uniform float alpha;
uniform bool texEnabled;
uniform sampler2D texUnit;
uniform sampler2D texture_diffuse;
uniform sampler2D texture_specular;
uniform samplerCube envMap;
uniform sampler2D shadowMap;
// IBL
uniform samplerCube irradianceMap;
uniform samplerCube prefilterMap;
uniform sampler2D brdfLUT;

uniform bool envMapEnabled;
uniform bool shadowsEnabled;
uniform float shadowSamples;
uniform vec3 cameraPos;
uniform mat4 viewMatrix;
uniform bool sectionActive;
uniform int displayMode;
uniform int renderingMode;
uniform bool selected;
uniform vec4 reflectColor;
uniform bool floorRendering;
uniform bool lockLightAndCamera = true;
uniform bool hasDiffuseTexture = false;
uniform bool hasSpecularTexture = false;

struct LineInfo
{
    float Width;
    vec4 Color;
};

uniform LineInfo Line;

struct LightSource
{
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    vec3 position;
};
uniform LightSource lightSource;

struct LightModel
{
    vec3 ambient;
};
uniform LightModel lightModel;

struct Material {
    vec3  emission;
    vec3  ambient;
    vec3  diffuse;
    vec3  specular;
    float shininess;
    bool  metallic;
};
uniform Material material;

struct PBRLighting {
    vec3 albedo;
    float metallic;
    float roughness;
    float ambientOcclusion;
};
uniform PBRLighting pbrLighting;

const float PI = 3.14159265359;

layout( location = 0 ) out vec4 fragColor;

float calculateShadow(vec4 fragPosLightSpace);
vec4  shadeBlinnPhong(LightSource source, LightModel model, Material mat, vec3 position, vec3 normal);
vec4  calculatePBRLighting(vec3 normal);

float DistributionGGX(vec3 N, vec3 H, float roughness);
float GeometrySchlickGGX(float NdotV, float roughness);
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness);
vec3  fresnelSchlick(float cosTheta, vec3 F0);
vec3  fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness);


void main()
{
    vec4 v_color_front;
    vec4 v_color_back;
    vec4 v_color;

    if(renderingMode == 0)
    {
        v_color_front = shadeBlinnPhong(lightSource, lightModel, material, g_position, g_normal);
        v_color_back  = shadeBlinnPhong(lightSource, lightModel, material, g_position, -g_normal);
    }
    else if(renderingMode == 1)
    {
        v_color_front = calculatePBRLighting(g_normal);
        v_color_back  = calculatePBRLighting(-g_normal);
    }

    if( gl_FrontFacing )
    {
        v_color = v_color_front;
    }
    else
    {
        if(sectionActive)
            v_color = v_color_back + 0.15f;
        else
            v_color = v_color_back;
    }

    if(displayMode == 0 || displayMode == 3) // shaded
    {
        if(texEnabled == true)
            fragColor = v_color * texture2D(texUnit, g_texCoord2d);
        else
            fragColor = v_color;
    }
    else if(displayMode == 1) // wireframe
    {
        fragColor = vec4(1.0f, 1.0f, 1.0f, 0.75f);
    }
    else // wireshaded
    {
        // Find the smallest distance
        float d = min(g_edgeDistance.x, g_edgeDistance.y );
        d = min( d, g_edgeDistance.z );

        float mixVal;
        if( d < Line.Width - 1.0f )
        {
            mixVal = 1.0f;
        } else if( d > Line.Width + 1.0f )
        {
            mixVal = 0.0f;
        }
        else
        {
            float x = d - (Line.Width - 1.0f);
            mixVal = exp2(-2.0f * (x*x));
        }

        if(texEnabled == true)
            fragColor = mix(v_color * texture2D(texUnit, g_texCoord2d), Line.Color, mixVal);
        else
            fragColor = mix(v_color, Line.Color, mixVal);
    }

    if(hasDiffuseTexture)
    {
        fragColor = vec4(texture2D(texture_diffuse, g_texCoord2d));
    }
    if(hasSpecularTexture)
    {
        fragColor = mix(vec4(texture2D(texture_diffuse, g_texCoord2d)), vec4(texture2D(texture_specular, g_texCoord2d)), 0.5);
    }
    
    if(selected)
    {
        fragColor = mix(fragColor, vec4(1.0f, .65f, 0.0f, 1.0f), 0.5f);
    }
}

// ----------------------------------------------------------------------------
vec4 shadeBlinnPhong(LightSource source, LightModel model, Material mat, vec3 position, vec3 normal)
{
    vec3 halfVector; // light half vector
    if(lockLightAndCamera)
        halfVector = normalize(source.position + vec3(0.0, 0.0, 0.0));
    else
        halfVector = normalize(source.position + cameraPos);
    float nDotVP    = dot(normal, normalize(source.position));                 // normal . light direction
    float nDotHV    = max(0.f, dot(normal,  halfVector));                      // normal . light half vector
    float pf        = mix(0.f, pow(nDotHV, mat.shininess), step(0.f, nDotVP)); // power factor

    vec3 ambient    = source.ambient;
    vec3 diffuse    = source.diffuse * nDotVP;
    vec3 specular   = source.specular * pf;
    vec3 sceneColor = mat.emission + mat.ambient * model.ambient;

    vec4 colorLinear;

    if(shadowsEnabled && displayMode == 3) // Shadow Mapping
    {
        float shadowFactor = calculateShadow(fs_in_shadow.FragPosLightSpace);
        colorLinear =  vec4(clamp(sceneColor +
                             (ambient  * mat.ambient + 1 - shadowFactor) *
                             (diffuse  * mat.diffuse +
                              specular * mat.specular), 0.f, 1.f ), alpha);
    }
    else
    {
        colorLinear =  vec4(clamp(sceneColor +
                             ambient  * mat.ambient +
                             diffuse  * mat.diffuse +
                             specular * mat.specular, 0.f, 1.f ), alpha);
    }
    if(envMapEnabled && displayMode == 3) // Environment mapping
    {

        if(alpha < 1.0f && !floorRendering) // Transparent - refract
        {
            vec4 colour = colorLinear;
            vec3 I = normalize(g_reflectionPosition - cameraPos);
            vec3 R = refract(I, normalize(g_reflectionNormal), 1.0f - alpha);
            if(texEnabled == true)
                colorLinear = mix(texture2D(texUnit, g_texCoord2d), vec4(texture(envMap, R).rgb, 1.0f - alpha), 1.0f - alpha);
            else
                colorLinear = vec4(texture(envMap, R).rgb, 1.0f - alpha);
            colorLinear = mix(colorLinear, colour, alpha/1.0f);
        }
        else // Opaque - Reflect
        {
            vec3 I = normalize(cameraPos - g_reflectionPosition);
            vec3 R = refract(-I, normalize(-g_reflectionNormal), 1.0f); // inverted refraction for reflection
            float factor =  material.metallic ? 1.0f : length(material.diffuse) * 1.5f;
            colorLinear = mix(colorLinear, vec4(texture(envMap, R).rgb, 1.0f), material.shininess/128 * (length(material.specular) * factor));
        }
    }

    return colorLinear;
}
// ----------------------------------------------------------------------------
float calculateShadow(vec4 fragPosLightSpace)
{
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(shadowMap, projCoords.xy).r;
    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;

    vec3 normal = normalize(fs_in_shadow.Normal);
    vec3 lightDir;
    if(lockLightAndCamera)
        lightDir = normalize(lightSource.position);
    else
        lightDir = normalize(fs_in_shadow.cameraPos - fs_in_shadow.FragPos);
    float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);

    // PCF - Percentage Closer Filtering
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += currentDepth - bias > pcfDepth  ? 1.0 : 0.0;
        }
    }

    shadow /= shadowSamples;

    // keep the shadow at 0.0 when outside the far_plane region of the light's frustum.
    if(projCoords.z > 1.0)
        shadow = 0.0;

    return shadow;
}

vec4 calculatePBRLighting(vec3 normal)
{
    vec3 N = normalize(normal);
    vec3 V = normalize(lightSource.position + vec3(0));

    // calculate reflectance at normal incidence; if dia-electric (like plastic) use F0
    // of 0.04 and if it's a metal, use the albedo color as F0 (metallic workflow)
    vec3 F0 = vec3(0.04);
    F0 = mix(F0, pbrLighting.albedo, pbrLighting.metallic);

    // reflectance equation
    vec3 Lo = vec3(0.0);

    // calculate per-light radiance
    vec3 L = normalize(lightSource.position + vec3(0));
    vec3 H = normalize(V + L);
    float distance = length(lightSource.position - vec3(0));
    float attenuation = 1.0 / (distance * distance);
    float lightIntensity = 1000.0f;
    vec3 lightColor = vec3(3.0f, 3.0f, 3.0f) * lightIntensity;
    //vec3 radiance = lightColor * attenuation;//(lightSource.ambient + lightSource.diffuse + lightSource.specular);
    vec3 radiance;
    if(shadowsEnabled && displayMode == 3)
    {
        float shadowFactor = calculateShadow(fs_in_shadow.FragPosLightSpace);
        radiance = (lightSource.ambient + 1- shadowFactor)  * (lightSource.diffuse + lightSource.specular);
    }
    else
    {
        radiance = (lightSource.ambient + lightSource.diffuse + lightSource.specular);
    }

    // Cook-Torrance BRDF
    float NDF = DistributionGGX(N, H, pbrLighting.roughness);
    float G   = GeometrySmith(N, V, L, pbrLighting.roughness);
    vec3 F    = fresnelSchlick(clamp(dot(H, V), 0.0, 1.0), F0);

    vec3 nominator    = NDF * G * F;
    float denominator = 4 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0);
    vec3 specular = nominator / max(denominator, 0.001); // prevent divide by zero for NdotV=0.0 or NdotL=0.0

    // kS is equal to Fresnel
    vec3 kS = F;
    // for energy conservation, the diffuse and specular light can't
    // be above 1.0 (unless the surface emits light); to preserve this
    // relationship the diffuse component (kD) should equal 1.0 - kS.
    vec3 kD = vec3(1.0) - kS;
    // multiply kD by the inverse metalness such that only non-metals
    // have diffuse lighting, or a linear blend if partly metal (pure metals
    // have no diffuse light).
    kD *= 1.0 - pbrLighting.metallic;

    // scale light by NdotL
    float NdotL = max(dot(N, L), 0.0);

    // add to outgoing radiance Lo
    Lo += (kD * pbrLighting.albedo / PI + specular) * radiance * NdotL;  // note that we already multiplied the BRDF by the Fresnel (kS) so we won't multiply by kS again

    // ambient lighting (note that the next IBL tutorial will replace
    // this ambient lighting with environment lighting).
    vec3 ambient;

    if(envMapEnabled && displayMode == 3)
    {
        // ambient lighting (we now use IBL as the ambient term)
        /*
        kS = fresnelSchlick(max(dot(N, V), 0.0), F0);
        kD = 1.0 - kS;
        kD *= 1.0 - pbrLighting.metallic;
        vec3 irradiance = texture(irradianceMap, N).rgb;
        vec3 diffuse      = irradiance * pbrLighting.albedo;
        ambient = (kD * diffuse) * pbrLighting.ambientOcclusion;
        */

        vec3 F = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, pbrLighting.roughness);

        kS = F;
        kD = 1.0 - kS;
        kD *= 1.0 - pbrLighting.metallic;

        vec3 irradiance = texture(irradianceMap, N).rgb;
        vec3 diffuse      = irradiance * pbrLighting.albedo;

        //N = normalize(normal);
        V = normalize(cameraPos - vec3(0));
        vec3 R = reflect(-V, N);
        // sample both the pre-filter map and the BRDF lut and combine them together as per the Split-Sum approximation to get the IBL specular part.
        const float MAX_REFLECTION_LOD = 4.0;
        vec3 prefilteredColor = textureLod(prefilterMap, R,  pbrLighting.roughness * MAX_REFLECTION_LOD).rgb;
        vec2 brdf  = texture(brdfLUT, vec2(max(dot(N, V), 0.0), pbrLighting.roughness)).rg;
        vec3 specular = prefilteredColor * (F * brdf.x + brdf.y);

        ambient = (kD * diffuse + specular) * pbrLighting.ambientOcclusion;
    }
    else
    {
        ambient = lightSource.ambient * pbrLighting.albedo * pbrLighting.ambientOcclusion;
    }

    vec3 color = ambient + Lo;

    // HDR tonemapping
    color = color / (color + vec3(1.0));
    // gamma correct
    color = pow(color, vec3(1.0/2.2));

    return vec4(color, alpha);
}

// ----------------------------------------------------------------------------
float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness*roughness;
    float a2 = a*a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;

    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / max(denom, 0.001); // prevent divide by zero for roughness=0.0 and NdotH=1.0
}
// ----------------------------------------------------------------------------
float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}
// ----------------------------------------------------------------------------
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}
// ----------------------------------------------------------------------------
vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}
// ----------------------------------------------------------------------------
vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(1.0 - cosTheta, 5.0);
}
