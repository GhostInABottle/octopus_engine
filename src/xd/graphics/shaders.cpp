#include "shaders.hpp"
#include "vertex_traits.hpp"

xd::flat_shader::flat_shader()
{
    static const char *vertex_shader_src =
        "#version 110\n"
        "uniform mat4 mvpMatrix;"
        "attribute vec4 vVertex;"
        "void main(void)"
        "{"
        "    gl_Position = mvpMatrix * vVertex;"
        "}";

    static const char *fragment_shader_src =
        "#version 110\n"
        "uniform vec4 vColor;"
        "void main(void)"
        "{"
        "    gl_FragColor = vColor;"
        "}";

    attach(GL_VERTEX_SHADER, vertex_shader_src);
    attach(GL_FRAGMENT_SHADER, fragment_shader_src);
    bind_attribute("vVertex", xd::VERTEX_POSITION);
    link();
}

void xd::flat_shader::setup(const glm::mat4& mvp, const glm::vec4& color)
{
    use();
    bind_uniform("mvpMatrix", mvp);
    bind_uniform("vColor", color);
}

xd::text_shader::text_shader()
{
    static const char *vertex_shader_src =
        "#version 110\n"
        "uniform mat4 mvpMatrix;"
        "uniform vec2 vPosition;"
        "attribute vec4 vVertex;"
        "attribute vec2 vTexCoords;"
        "varying vec2 vVaryingTexCoords;"
        "void main(void)"
        "{"
        "    vVaryingTexCoords = vTexCoords;"
        "    gl_Position = mvpMatrix * (vVertex + vec4(vPosition.x, vPosition.y, 0, 0));"
        "}";

    static const char *fragment_shader_src =
        "#version 110\n"
        "uniform vec4 vColor;"
        "uniform sampler2D colorMap;"
        "varying vec2 vVaryingTexCoords;"
        "void main(void)"
        "{"
        "    gl_FragColor.rgb = vColor.rgb;"
        "    gl_FragColor.a = vColor.a * texture2D(colorMap, vVaryingTexCoords).r;"
        "}";

    attach(GL_VERTEX_SHADER, vertex_shader_src);
    attach(GL_FRAGMENT_SHADER, fragment_shader_src);
    bind_attribute("vVertex", xd::VERTEX_POSITION);
    bind_attribute("vTexCoords", xd::VERTEX_TEXTURE);
    link();
}

xd::sprite_shader::sprite_shader()
{
    static const char *vertex_shader_src =
        "#version 110\n"
        "uniform mat4 mvpMatrix;"
        "uniform vec4 vPosition;"
        "attribute vec4 vVertex;"
        "attribute vec2 vTexCoords;"
        "varying vec2 vVaryingTexCoords;"
        "void main(void)"
        "{"
        "    vVaryingTexCoords = vTexCoords;"
        "    gl_Position = mvpMatrix * (vPosition + vVertex);"
        "}";

    static const char *fragment_shader_src =
        "#version 110\n"
        "uniform vec4 vColor;"
        "uniform sampler2D colorMap;"
        "uniform vec4 vColorKey;"
        "varying vec2 vVaryingTexCoords;"
        "void main(void)"
        "{"
        "    vec4 vTexColor = texture2D(colorMap, vVaryingTexCoords);"
        "    if (vColorKey.a > 0.0 && vTexColor == vColorKey) vTexColor.a = 0.0;"
        "    gl_FragColor = vColor * vTexColor;"
        "}";

    attach(GL_VERTEX_SHADER, vertex_shader_src);
    attach(GL_FRAGMENT_SHADER, fragment_shader_src);
    bind_attribute("vVertex", xd::VERTEX_POSITION);
    bind_attribute("vTexCoords", xd::VERTEX_TEXTURE);
    link();
}

xd::sprite_outline_shader::sprite_outline_shader()
{
    static const char *vertex_shader_src =
        "#version 110\n"
        "uniform mat4 mvpMatrix;"
        "uniform vec4 vPosition;"
        "attribute vec4 vVertex;"
        "attribute vec2 vTexCoords;"
        "varying vec2 vVaryingTexCoords;"
        "void main(void)"
        "{"
        "    vVaryingTexCoords = vTexCoords;"
        "    gl_Position = mvpMatrix * (vPosition + vVertex);"
        "}";

    static const char *fragment_shader_src =
        "#version 110\n"
        "uniform vec4 vColor;"
        "uniform sampler2D colorMap;"
        "uniform vec4 vColorKey;"
        "uniform vec4 vOutlineColor;"
        "uniform vec2 vTexSize;"
        "varying vec2 vVaryingTexCoords;"
        ""
        "float neighborAlpha(vec2 neighbor) {"
        "    vec4 vTexColor = texture2D(colorMap, neighbor);"
        "    return vColorKey.a > 0.0 && vTexColor == vColorKey ? 0.0 : vTexColor.a;"
        "}"
        ""
        "void main(void)"
        "{"
        "    vec4 vTexColor = texture2D(colorMap, vVaryingTexCoords);"
        "    vec2 stepSize = 1.0 / vTexSize;"
        "    float alpha = 4.0 * (vColorKey.a > 0.0 && vTexColor == vColorKey ? 0.0 : vTexColor.a);"
        "    alpha -= neighborAlpha(vVaryingTexCoords + vec2(stepSize.x, 0.0));"
        "    alpha -= neighborAlpha(vVaryingTexCoords + vec2(-stepSize.x, 0.0));"
        "    alpha -= neighborAlpha(vVaryingTexCoords + vec2(0.0, stepSize.y));"
        "    alpha -= neighborAlpha(vVaryingTexCoords + vec2(0.0, -stepSize.y));"
        "    gl_FragColor = vec4(vOutlineColor.rgb, alpha > 0.0 ? vOutlineColor.a : 0.0);"
        "}";

    attach(GL_VERTEX_SHADER, vertex_shader_src);
    attach(GL_FRAGMENT_SHADER, fragment_shader_src);
    bind_attribute("vVertex", xd::VERTEX_POSITION);
    bind_attribute("vTexCoords", xd::VERTEX_TEXTURE);
    link();
}

xd::fullscreen_shader::fullscreen_shader()
{
    static const char* vertex_shader_src =
        "#version 110\n"
        "uniform mat4 mvpMatrix;"
        "uniform vec4 vPosition;"
        "attribute vec4 vVertex;"
        "attribute vec2 vTexCoords;"
        "varying vec2 vVaryingTexCoords;"
        "void main(void)"
        "{"
        "    vVaryingTexCoords = vTexCoords;"
        "    gl_Position = mvpMatrix * (vPosition + vVertex);"
        "}";

    static const char* fragment_shader_src =
        "#version 110\n"
        "const vec4 lumCoeff = vec4(0.2125, 0.7154, 0.0721, 0.0);"
        "uniform vec4 vColor;"
        "uniform vec4 vColorKey;"
        "uniform sampler2D colorMap;"
        "uniform float brightness;"
        "uniform float contrast;"
        "uniform float saturation;"
        "varying vec2 vVaryingTexCoords;"
        "void main(void) {"
        "    vec4 vTexColor = texture2D(colorMap, vVaryingTexCoords.st);"
        "    if (vColorKey.a > 0.0 && vTexColor == vColorKey) vTexColor.a = 0.0;"
        "    vec4 pixelColor = vColor * vTexColor;"
        "    pixelColor.rgb = (pixelColor.rgb - 0.5) * max(contrast, 0.0) + 0.5 + brightness;"
        "    vec4 saturationIntensity = vec4(dot(pixelColor, lumCoeff));"
        "    gl_FragColor = mix(saturationIntensity, pixelColor, saturation);"
        "}";

    attach(GL_VERTEX_SHADER, vertex_shader_src);
    attach(GL_FRAGMENT_SHADER, fragment_shader_src);
    bind_attribute("vVertex", xd::VERTEX_POSITION);
    bind_attribute("vTexCoords", xd::VERTEX_TEXTURE);
    link();
}
