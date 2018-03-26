// render.h
#pragma once

#include "options.h"
#include "window.h"
#include "shader-input-layout.h"

#include "../../source/core/slang-result.h"
#include "../../source/core/smart-pointer.h"

namespace renderer_test {

// Declare opaque type
class Buffer: public Slang::RefObject
{
	public:
};
class InputLayout: public Slang::RefObject
{
	public:
};

class ShaderProgram: public Slang::RefObject
{
	public:
};

class BindingState: public Slang::RefObject
{
	public:
};

struct ShaderCompileRequest
{
    struct SourceInfo
    {
        char const* path;

        // The data may either be source text (in which
        // case it can be assumed to be nul-terminated with
        // `dataEnd` pointing at the terminator), or
        // raw binary data (in which case `dataEnd` points
        // at the end of the buffer).
        char const* dataBegin;
        char const* dataEnd;
    };

    struct EntryPoint
    {
        char const* name = nullptr;
        char const* profile = nullptr;

        SourceInfo  source;
    };

    SourceInfo source;
    EntryPoint vertexShader;
    EntryPoint fragmentShader;
    EntryPoint computeShader;
    Slang::List<Slang::String> entryPointTypeArguments;
};

class ShaderCompiler
{
public:
    virtual ShaderProgram* compileProgram(ShaderCompileRequest const& request) = 0;
};

enum class Format
{
    Unknown,
    RGB_Float32,
    RG_Float32,
};

enum class BufferFlavor
{
    Constant,
    Vertex
};

struct BufferDesc
{
    UInt            size        = 0;
    BufferFlavor    flavor      = BufferFlavor::Constant;
    void const*     initData    = nullptr;
};

struct InputElementDesc
{
    char const* semanticName;
    UInt        semanticIndex;
    Format      format;
    UInt        offset;
};

enum class MapFlavor
{
    HostRead,
    HostWrite,
    WriteDiscard,
};

enum class PrimitiveTopology
{
    TriangleList,
};

class Renderer: public Slang::RefObject
{
public:
    virtual SlangResult initialize(void* inWindowHandle) = 0;

    virtual void setClearColor(const float color[4]) = 0;
    virtual void clearFrame() = 0;

    virtual void presentFrame() = 0;

    virtual SlangResult captureScreenShot(const char* outputPath) = 0;
    virtual void serializeOutput(BindingState* state, const char* outputPath) = 0;
    virtual Buffer* createBuffer(const BufferDesc& desc) = 0;

    virtual InputLayout* createInputLayout(const InputElementDesc* inputElements, UInt inputElementCount) = 0;
    virtual BindingState* createBindingState(const ShaderInputLayout& shaderInput) = 0;
    virtual ShaderCompiler* getShaderCompiler() = 0;

    virtual void* map(Buffer* buffer, MapFlavor flavor) = 0;
    virtual void unmap(Buffer* buffer) = 0;

    virtual void setInputLayout(InputLayout* inputLayout) = 0;
    virtual void setPrimitiveTopology(PrimitiveTopology topology) = 0;
    virtual void setBindingState(BindingState* state) = 0;
    virtual void setVertexBuffers(UInt startSlot, UInt slotCount, Buffer*const* buffers, const UInt* strides, const UInt* offsets) = 0;

    inline void setVertexBuffer(UInt slot, Buffer* buffer, UInt stride, UInt offset = 0);

    virtual void setShaderProgram(ShaderProgram* program) = 0;

    virtual void setConstantBuffers(UInt startSlot, UInt slotCount, Buffer*const* buffers, const UInt* offsets) = 0;
    inline void setConstantBuffer(UInt slot, Buffer* buffer, UInt offset = 0);

    virtual void draw(UInt vertexCount, UInt startVertex = 0) = 0;
    virtual void dispatchCompute(int x, int y, int z) = 0;
};


// ----------------------------------------------------------------------------------------
inline void Renderer::setVertexBuffer(UInt slot, Buffer* buffer, UInt stride, UInt offset)
{
    setVertexBuffers(slot, 1, &buffer, &stride, &offset);
}
// ----------------------------------------------------------------------------------------
inline void Renderer::setConstantBuffer(UInt slot, Buffer* buffer, UInt offset)
{
    setConstantBuffers(slot, 1, &buffer, &offset);
}


} // renderer_test
