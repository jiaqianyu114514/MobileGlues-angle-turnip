#include "glsl_for_es.h"

#include <glslang/Public/ShaderLang.h>
#include <glslang/Include/Types.h>
#include <glslang/Public/ShaderLang.h>
#include <spirv_cross/spirv_cross_c.h>
#include <iostream>
#include <fstream>
#include "../log.h"
#include "glslang/SPIRV/GlslangToSpv.h"
#include <string>
#include <regex>
#include <strstream>

#define DEBUG 0

char* (*MesaConvertShader)(const char *src, unsigned int type, unsigned int glsl, unsigned int essl);

typedef std::vector<uint32_t> Spirv;

static TBuiltInResource InitResources()
{
    TBuiltInResource Resources;

    Resources.maxLights                                 = 32;
    Resources.maxClipPlanes                             = 6;
    Resources.maxTextureUnits                           = 32;
    Resources.maxTextureCoords                          = 32;
    Resources.maxVertexAttribs                          = 64;
    Resources.maxVertexUniformComponents                = 4096;
    Resources.maxVaryingFloats                          = 64;
    Resources.maxVertexTextureImageUnits                = 32;
    Resources.maxCombinedTextureImageUnits              = 80;
    Resources.maxTextureImageUnits                      = 32;
    Resources.maxFragmentUniformComponents              = 4096;
    Resources.maxDrawBuffers                            = 32;
    Resources.maxVertexUniformVectors                   = 128;
    Resources.maxVaryingVectors                         = 8;
    Resources.maxFragmentUniformVectors                 = 16;
    Resources.maxVertexOutputVectors                    = 16;
    Resources.maxFragmentInputVectors                   = 15;
    Resources.minProgramTexelOffset                     = -8;
    Resources.maxProgramTexelOffset                     = 7;
    Resources.maxClipDistances                          = 8;
    Resources.maxComputeWorkGroupCountX                 = 65535;
    Resources.maxComputeWorkGroupCountY                 = 65535;
    Resources.maxComputeWorkGroupCountZ                 = 65535;
    Resources.maxComputeWorkGroupSizeX                  = 1024;
    Resources.maxComputeWorkGroupSizeY                  = 1024;
    Resources.maxComputeWorkGroupSizeZ                  = 64;
    Resources.maxComputeUniformComponents               = 1024;
    Resources.maxComputeTextureImageUnits               = 16;
    Resources.maxComputeImageUniforms                   = 8;
    Resources.maxComputeAtomicCounters                  = 8;
    Resources.maxComputeAtomicCounterBuffers            = 1;
    Resources.maxVaryingComponents                      = 60;
    Resources.maxVertexOutputComponents                 = 64;
    Resources.maxGeometryInputComponents                = 64;
    Resources.maxGeometryOutputComponents               = 128;
    Resources.maxFragmentInputComponents                = 128;
    Resources.maxImageUnits                             = 8;
    Resources.maxCombinedImageUnitsAndFragmentOutputs   = 8;
    Resources.maxCombinedShaderOutputResources          = 8;
    Resources.maxImageSamples                           = 0;
    Resources.maxVertexImageUniforms                    = 0;
    Resources.maxTessControlImageUniforms               = 0;
    Resources.maxTessEvaluationImageUniforms            = 0;
    Resources.maxGeometryImageUniforms                  = 0;
    Resources.maxFragmentImageUniforms                  = 8;
    Resources.maxCombinedImageUniforms                  = 8;
    Resources.maxGeometryTextureImageUnits              = 16;
    Resources.maxGeometryOutputVertices                 = 256;
    Resources.maxGeometryTotalOutputComponents          = 1024;
    Resources.maxGeometryUniformComponents              = 1024;
    Resources.maxGeometryVaryingComponents              = 64;
    Resources.maxTessControlInputComponents             = 128;
    Resources.maxTessControlOutputComponents            = 128;
    Resources.maxTessControlTextureImageUnits           = 16;
    Resources.maxTessControlUniformComponents           = 1024;
    Resources.maxTessControlTotalOutputComponents       = 4096;
    Resources.maxTessEvaluationInputComponents          = 128;
    Resources.maxTessEvaluationOutputComponents         = 128;
    Resources.maxTessEvaluationTextureImageUnits        = 16;
    Resources.maxTessEvaluationUniformComponents        = 1024;
    Resources.maxTessPatchComponents                    = 120;
    Resources.maxPatchVertices                          = 32;
    Resources.maxTessGenLevel                           = 64;
    Resources.maxViewports                              = 16;
    Resources.maxVertexAtomicCounters                   = 0;
    Resources.maxTessControlAtomicCounters              = 0;
    Resources.maxTessEvaluationAtomicCounters           = 0;
    Resources.maxGeometryAtomicCounters                 = 0;
    Resources.maxFragmentAtomicCounters                 = 8;
    Resources.maxCombinedAtomicCounters                 = 8;
    Resources.maxAtomicCounterBindings                  = 1;
    Resources.maxVertexAtomicCounterBuffers             = 0;
    Resources.maxTessControlAtomicCounterBuffers        = 0;
    Resources.maxTessEvaluationAtomicCounterBuffers     = 0;
    Resources.maxGeometryAtomicCounterBuffers           = 0;
    Resources.maxFragmentAtomicCounterBuffers           = 1;
    Resources.maxCombinedAtomicCounterBuffers           = 1;
    Resources.maxAtomicCounterBufferSize                = 16384;
    Resources.maxTransformFeedbackBuffers               = 4;
    Resources.maxTransformFeedbackInterleavedComponents = 64;
    Resources.maxCullDistances                          = 8;
    Resources.maxCombinedClipAndCullDistances           = 8;
    Resources.maxSamples                                = 4;
    Resources.maxMeshOutputVerticesNV                   = 256;
    Resources.maxMeshOutputPrimitivesNV                 = 512;
    Resources.maxMeshWorkGroupSizeX_NV                  = 32;
    Resources.maxMeshWorkGroupSizeY_NV                  = 1;
    Resources.maxMeshWorkGroupSizeZ_NV                  = 1;
    Resources.maxTaskWorkGroupSizeX_NV                  = 32;
    Resources.maxTaskWorkGroupSizeY_NV                  = 1;
    Resources.maxTaskWorkGroupSizeZ_NV                  = 1;
    Resources.maxMeshViewCountNV                        = 4;

    Resources.limits.nonInductiveForLoops                 = 1;
    Resources.limits.whileLoops                           = 1;
    Resources.limits.doWhileLoops                         = 1;
    Resources.limits.generalUniformIndexing               = 1;
    Resources.limits.generalAttributeMatrixVectorIndexing = 1;
    Resources.limits.generalVaryingIndexing               = 1;
    Resources.limits.generalSamplerIndexing               = 1;
    Resources.limits.generalVariableIndexing              = 1;
    Resources.limits.generalConstantMatrixVectorIndexing  = 1;

    return Resources;
}

int getGLSLVersion(const char* glsl_code) {
    std::string code(glsl_code);
    std::regex version_pattern(R"(#version\s+(\d{3}))");
    std::smatch match;
    if (std::regex_search(code, match, version_pattern)) {
        return std::stoi(match[1].str());
    }

    return -1;
}

std::string removeSecondLine(std::string code) {
    size_t firstLineEnd = code.find('\n');
    if (firstLineEnd == std::string::npos) {
        return code;
    }
    size_t secondLineEnd = code.find('\n', firstLineEnd + 1);
    if (secondLineEnd == std::string::npos) {
        return code;
    }
    code.erase(firstLineEnd + 1, secondLineEnd - firstLineEnd);
    return code;
}

char* disable_GL_ARB_derivative_control(char* glslCode) {
    std::string code(glslCode);
    std::string target = "GL_ARB_derivative_control";
    size_t pos = code.find(target);

    if (pos != std::string::npos) {
        size_t ifdefPos = 0;
        while ((ifdefPos = code.find("#ifdef GL_ARB_derivative_control", ifdefPos)) != std::string::npos) {
            code.replace(ifdefPos, 32, "#if 0");
            ifdefPos += 4;
        }

        size_t ifndefPos = 0;
        while ((ifndefPos = code.find("#ifndef GL_ARB_derivative_control", ifndefPos)) != std::string::npos) {
            code.replace(ifndefPos, 33, "#if 1");
            ifndefPos += 4;
        }

        code = removeSecondLine(code);

        char* result = new char[code.length() + 1];
        std::strcpy(result, code.c_str());
        return result;
    }

    char* result = new char[code.length() + 1];
    std::strcpy(result, code.c_str());
    return result;
}

char* forceSupporterInput(char* glslCode) {
    // first
    const char* target = "const mat3 rotInverse = transpose(rot);";
    const char* replacement = "const mat3 rotInverse = mat3(rot[0][0], rot[1][0], rot[2][0], rot[0][1], rot[1][1], rot[2][1], rot[0][2], rot[1][2], rot[2][2]);";

    char* pos = strstr(glslCode, target);
    if (pos != nullptr) {
        size_t targetLen = strlen(target);
        size_t replacementLen = strlen(replacement);

        size_t newSize = strlen(glslCode) - targetLen + replacementLen + 1;
        char* modifiedCode = new char[newSize];

        strncpy(modifiedCode, glslCode, pos - glslCode);
        modifiedCode[pos - glslCode] = '\0';

        strcat(modifiedCode, replacement);

        strcat(modifiedCode, pos + targetLen);
        glslCode = new char[strlen(modifiedCode) + 1];
        std::strcpy(glslCode, modifiedCode);
        std::free(modifiedCode);
    }
    
    // second
    if (!std::strstr(glslCode, "deferredOutput2 = GI_TemporalFilter()")) {
        return glslCode;
    }

    if (std::strstr(glslCode, "vec4 GI_TemporalFilter()")) {
        return glslCode;
    }


    LOG_D("find GI_TemporalFilter()")
    
    const char* GI_TemporalFilter = R"(
vec4 GI_TemporalFilter() {
    vec2 uv = gl_FragCoord.xy / screenSize;
    uv += taaJitter * pixelSize;
    vec4 currentGI = texture(colortex0, uv);
    float depth = texture(depthtex0, uv).r;
    vec4 clipPos = vec4(uv * 2.0 - 1.0, depth, 1.0);
    vec4 viewPos = gbufferProjectionInverse * clipPos;
    viewPos /= viewPos.w;
    vec4 worldPos = gbufferModelViewInverse * viewPos;
    vec4 prevClipPos = gbufferPreviousProjection * (gbufferPreviousModelView * worldPos);
    prevClipPos /= prevClipPos.w;
    vec2 prevUV = prevClipPos.xy * 0.5 + 0.5;
    vec4 historyGI = texture(colortex1, prevUV);
    float difference = length(currentGI.rgb - historyGI.rgb);
    float thresholdValue = 0.1;
    float adaptiveBlend = mix(0.9, 0.0, smoothstep(thresholdValue, thresholdValue * 2.0, difference));
    vec4 filteredGI = mix(currentGI, historyGI, adaptiveBlend);
    if (difference > thresholdValue * 2.0) {
        filteredGI = currentGI;
    }
    return filteredGI;
}
)";

    char *mainPos = strstr(glslCode, "\nvoid main()");
    if (mainPos == NULL) {
        LOG_E("Error: 'void main()' not found in GLSL code.");
        return glslCode;
    }

    size_t prefixLength = mainPos - glslCode; 
    size_t originalLength = strlen(glslCode);
    size_t insertLength = strlen(GI_TemporalFilter);

    char *modifiedCode = (char *)malloc(originalLength + insertLength + 2);
    if (modifiedCode == NULL) {
        LOG_E("Memory allocation failed.");
        return glslCode;
    }

    strncpy(modifiedCode, glslCode, prefixLength); 
    modifiedCode[prefixLength] = '\0';

    strcat(modifiedCode, "\n");
    strcat(modifiedCode, GI_TemporalFilter);
    strcat(modifiedCode, "\n");

    strcat(modifiedCode, mainPos);

    free(glslCode);
    glslCode = modifiedCode;

    return glslCode;
}

std::string forceSupporterOutput(const std::string& glslCode) {
    bool hasPrecisionFloat = glslCode.find("precision ") != std::string::npos &&
                             glslCode.find("float;") != std::string::npos;
    bool hasPrecisionInt = glslCode.find("precision ") != std::string::npos &&
                           glslCode.find("int;") != std::string::npos;
    if (hasPrecisionFloat && hasPrecisionInt) {
        return glslCode;
    }
    std::string result = glslCode;
    std::string precisionFloat = hasPrecisionFloat ? "" : "precision highp float;\n";
    std::string precisionInt = hasPrecisionInt ? "" : "precision highp int;\n";
    size_t lastExtensionPos = result.rfind("#extension");
    size_t insertionPos = 0;
    if (lastExtensionPos != std::string::npos) {
        size_t nextNewline = result.find('\n', lastExtensionPos);
        if (nextNewline != std::string::npos) {
            insertionPos = nextNewline + 1;
        } else {
            insertionPos = result.length();
        }
    } else {
        size_t firstNewline = result.find('\n');
        if (firstNewline != std::string::npos) {
            insertionPos = firstNewline + 1;
        } else {
            result = precisionFloat + precisionInt + result;
            return result;
        }
    }
    result.insert(insertionPos, precisionFloat + precisionInt);
    return result;
}

std::string removeLayoutBinding(const std::string& glslCode) {
    std::regex bindingRegex(R"(layout\s*\(\s*binding\s*=\s*\d+\s*\)\s*)");
    std::string result = std::regex_replace(glslCode, bindingRegex, "");
    std::regex bindingRegex2(R"(layout\s*\(\s*binding\s*=\s*\d+\s*,)");
    result = std::regex_replace(result, bindingRegex2, "layout(");
    return result;
}
// TODO
std::string makeRGBWriteonly(const std::string& input) {
    std::regex pattern(R"(.*layout\([^)]*rgba[^)]*\).*?)");
    std::string result;
    std::string::size_type start = 0;
    std::string::size_type end;
    while ((end = input.find('\n', start)) != std::string::npos) {
        std::string line = input.substr(start, end - start);
        if (std::regex_search(line, pattern)) {
            result += "writeonly " + line + "\n";
        } else {
            result += line + "\n";
        }
        start = end + 1;
    }
    std::string lastLine = input.substr(start);
    if (std::regex_search(lastLine, pattern)) {
        result += "writeonly " + lastLine;
    } else {
        result += lastLine;
    }

    return result;
}

std::string removeLocationBinding(const std::string& glslCode) {
    std::regex locationRegex(R"(layout\s*\(\s*location\s*=\s*\d+\s*\)\s*)");
    std::string result = std::regex_replace(glslCode, locationRegex, "");
    return result;
}

char* removeLineDirective(char* glslCode) {
    char* cursor = glslCode;
    int modifiedCodeIndex = 0;
    size_t maxLength = 1024 * 10;
    char* modifiedGlslCode = (char*)malloc(maxLength * sizeof(char));
    if (!modifiedGlslCode) return NULL;

    while (*cursor) {
        if (strncmp(cursor, "\n#", 2) == 0) {
            modifiedGlslCode[modifiedCodeIndex++] = *cursor++;
            modifiedGlslCode[modifiedCodeIndex++] = *cursor++;
            char* last_cursor = cursor;
            while (cursor[0] != '\n') cursor++;
            char* line_feed_cursor = cursor;
            while (isspace(cursor[0])) cursor--;
            if (cursor[0] == '\\')
            {
                // find line directive, now remove it
                char* slash_cursor = cursor;
                cursor = last_cursor;
                while (cursor < slash_cursor - 1)
                    modifiedGlslCode[modifiedCodeIndex++] = *cursor++;
                modifiedGlslCode[modifiedCodeIndex++] = ' ';
                cursor = line_feed_cursor + 1;
                while (isspace(cursor[0])) cursor++;

                while (true) {
                    char* last_cursor2 = cursor;
                    while (cursor[0] != '\n') cursor++;
                    cursor -= 1;
                    while (isspace(cursor[0])) cursor--;
                    if (cursor[0] == '\\') {
                        char* slash_cursor2 = cursor;
                        cursor = last_cursor2;
                        while (cursor < slash_cursor2)
                            modifiedGlslCode[modifiedCodeIndex++] = *cursor++;
                        while (cursor[0] != '\n') cursor++;
                        cursor++;
                        while (isspace(cursor[0])) cursor++;
                    } else {
                        cursor = last_cursor2;
                        while (cursor[0] != '\n')
                            modifiedGlslCode[modifiedCodeIndex++] = *cursor++;
                        break;
                    }
                }
                cursor++;
            }
            else {
                cursor = last_cursor;
            }
        }
        else {
            modifiedGlslCode[modifiedCodeIndex++] = *cursor++;
        }

        if (modifiedCodeIndex >= maxLength - 1) {
            maxLength *= 2;
            modifiedGlslCode = (char*)realloc(modifiedGlslCode, maxLength);
            if (!modifiedGlslCode) return NULL;
        }
    }

    modifiedGlslCode[modifiedCodeIndex] = '\0';
    return modifiedGlslCode;
}

std::string replaceText(const std::string& input, const std::string& from, const std::string& to) {
    std::string result = input;
    size_t pos = 0;
    while ((pos = result.find(from, pos)) != std::string::npos) {
        result.replace(pos, from.length(), to);
        pos += to.length();
    }
    return result;
}

std::string addPrecisionToSampler2DShadow(const std::string& glslCode) {
    std::string result = glslCode;
    result = replaceText(result, " sampler2DShadow ", " highp sampler2DShadow ");
    result = replaceText(result, " mediump highp ", " mediump ");
    result = replaceText(result, " lowp highp ", " lowp ");
    result = replaceText(result, " highp highp ", " highp ");
    return result;
}

char* GLSLtoGLSLES_2(char* glsl_code, GLenum glsl_type, uint essl_version) {
    glslang::InitializeProcess();
    EShLanguage shader_language;
    switch (glsl_type) {
        case GL_VERTEX_SHADER:
            shader_language = EShLanguage::EShLangVertex;
            break;
        case GL_FRAGMENT_SHADER:
            shader_language = EShLanguage::EShLangFragment;
            break;
        case GL_COMPUTE_SHADER:
            shader_language = EShLanguage::EShLangCompute;
            break;
        case GL_TESS_CONTROL_SHADER:
            shader_language = EShLanguage::EShLangTessControl;
            break;
        case GL_TESS_EVALUATION_SHADER:
            shader_language = EShLanguage::EShLangTessEvaluation;
            break;
        case GL_GEOMETRY_SHADER:
            shader_language = EShLanguage::EShLangGeometry;
            break;
        default:
            LOG_D("GLSL type not supported!");
            return nullptr;
    }

    glslang::TShader shader(shader_language);

    char* correct_glsl = glsl_code;
    correct_glsl = removeLineDirective(correct_glsl);
    correct_glsl = disable_GL_ARB_derivative_control(correct_glsl);
    correct_glsl = forceSupporterInput(correct_glsl);
    LOG_D("Firstly converted GLSL:\n%s",correct_glsl)
    char *shader_source = correct_glsl;
    int glsl_version = getGLSLVersion(shader_source);
    if (glsl_version == -1) {
        glsl_version = 140;
        std::string shader_str(shader_source);
        shader_str.insert(0, "#version 140\n");
        std::strcpy(shader_source, shader_str.c_str());

    }
    LOG_D("GLSL version: %d",glsl_version);

    shader.setStrings(&shader_source, 1);

    using namespace glslang;
    shader.setEnvInput(EShSourceGlsl, shader_language, EShClientVulkan, glsl_version);
    shader.setEnvClient(EShClientOpenGL, EShTargetOpenGL_450);
    shader.setEnvTarget(EShTargetSpv, EShTargetSpv_1_6);
    shader.setAutoMapLocations(true);
    shader.setAutoMapBindings(true);

    TBuiltInResource TBuiltInResource_resources = InitResources();

    if (!shader.parse(&TBuiltInResource_resources, glsl_version, true, EShMsgDefault)) {
        LOG_D("GLSL Compiling ERROR: \n%s",shader.getInfoLog());
        return NULL;
    }
    LOG_D("GLSL Compiled.");

    glslang::TProgram program;
    program.addShader(&shader);

    if (!program.link(EShMsgDefault)) {
        LOG_D("Shader Linking ERROR: %s",program.getInfoLog());
        return nullptr;
    }
    LOG_D("Shader Linked." );
    std::vector<unsigned int> spirv_code;
    glslang::SpvOptions spvOptions;
    spvOptions.disableOptimizer = true;
    glslang::GlslangToSpv(*program.getIntermediate(shader_language), spirv_code, &spvOptions);

    std::string essl;

    const SpvId *spirv = spirv_code.data();
    size_t word_count = spirv_code.size();

    spvc_context context = NULL;
    spvc_parsed_ir ir = NULL;
    spvc_compiler compiler_glsl = NULL;
    spvc_compiler_options options = NULL;
    spvc_resources resources = NULL;
    const spvc_reflected_resource *list = NULL;
    const char *result = NULL;
    size_t count;
    size_t i;

    spvc_context_create(&context);
    spvc_context_parse_spirv(context, spirv, word_count, &ir);
    spvc_context_create_compiler(context, SPVC_BACKEND_GLSL, ir, SPVC_CAPTURE_MODE_TAKE_OWNERSHIP, &compiler_glsl);
    spvc_compiler_create_shader_resources(compiler_glsl, &resources);
    spvc_resources_get_resource_list_for_type(resources, SPVC_RESOURCE_TYPE_UNIFORM_BUFFER, &list, &count);
    spvc_compiler_create_compiler_options(compiler_glsl, &options);
    spvc_compiler_options_set_uint(options, SPVC_COMPILER_OPTION_GLSL_VERSION, essl_version >= 300 ? essl_version : 300);
    spvc_compiler_options_set_bool(options, SPVC_COMPILER_OPTION_GLSL_ES, SPVC_TRUE);
    spvc_compiler_install_compiler_options(compiler_glsl, options);
    spvc_compiler_compile(compiler_glsl, &result);
    essl=result;
    spvc_context_destroy(context);

    essl = removeLayoutBinding(essl);
    //essl = removeLocationBinding(essl);
    //essl = addPrecisionToSampler2DShadow(essl);
    essl = forceSupporterOutput(essl);
    essl = makeRGBWriteonly(essl);

    char* result_essl = new char[essl.length() + 1];
    std::strcpy(result_essl, essl.c_str());

    LOG_D("Originally GLSL to GLSL ES Complete: \n%s",result_essl)

    free(shader_source);
    glslang::FinalizeProcess();
    return result_essl;
}

char * GLSLtoGLSLES_1(char* glsl_code, GLenum glsl_type, unsigned int esversion) {
    LOG_W("Warning: use glsl optimizer to convert shader.")
    if (esversion < 300) esversion = 300;
    char * result = MesaConvertShader(glsl_code, glsl_type == GL_VERTEX_SHADER ? 35633 : 35632, 460LL, esversion);
    char * ret = (char*)malloc(sizeof(char) * strlen(result) + 1);
    strcpy(ret, result);
    return ret;
}
