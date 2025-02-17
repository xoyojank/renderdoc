/******************************************************************************
 * The MIT License (MIT)
 *
 * Copyright (c) 2022 Baldur Karlsson
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 ******************************************************************************/

#include "metal_command_buffer.h"
#include "metal_command_queue.h"
#include "metal_device.h"
#include "metal_function.h"
#include "metal_library.h"
#include "metal_manager.h"
#include "metal_render_pipeline_state.h"
#include "metal_resources.h"
#include "metal_texture.h"
#include "metal_types.h"

// serialisation of object handles via IDs.
template <class SerialiserType, class type>
void DoSerialiseViaResourceId(SerialiserType &ser, type &el)
{
  MetalResourceManager *rm = (MetalResourceManager *)ser.GetUserData();

  ResourceId id;

  if(ser.IsWriting() && rm)
    id = GetResID(el);
  if(ser.IsStructurising() && rm)
    id = rm->GetOriginalID(GetResID(el));

  DoSerialise(ser, id);

  if(ser.IsReading())
  {
    el = NULL;

    if(rm && !IsStructuredExporting(rm->GetState()))
    {
      if(id != ResourceId() && rm)
      {
        if(rm->HasLiveResource(id))
        {
          // we leave this wrapped.
          el = (type)rm->GetLiveResource(id);
        }
      }
    }
  }
}

#define IMPLEMENT_WRAPPED_TYPE_SERIALISE(CPPTYPE)                 \
  template <class SerialiserType>                                 \
  void DoSerialise(SerialiserType &ser, WrappedMTL##CPPTYPE *&el) \
  {                                                               \
    DoSerialiseViaResourceId(ser, el);                            \
  }                                                               \
  INSTANTIATE_SERIALISE_TYPE(WrappedMTL##CPPTYPE *);

METALCPP_WRAPPED_PROTOCOLS(IMPLEMENT_WRAPPED_TYPE_SERIALISE);
#undef IMPLEMENT_WRAPPED_TYPE_SERIALISE

template <typename SerialiserType>
void DoSerialise(SerialiserType &ser, NS::String *&el)
{
  rdcstr rdcStr;
  if(el)
  {
    rdcStr = el->utf8String();
  }
  DoSerialise(ser, rdcStr);

  if(ser.IsReading())
  {
    el = NS::String::string(rdcStr.data(), NS::UTF8StringEncoding);
  }
}

template <typename SerialiserType>
void DoSerialise(SerialiserType &ser, MTL::TextureSwizzleChannels &el)
{
  SERIALISE_MEMBER(red);
  SERIALISE_MEMBER(green);
  SERIALISE_MEMBER(blue);
  SERIALISE_MEMBER(alpha);
}

template <typename SerialiserType>
void DoSerialise(SerialiserType &ser, RDMTL::TextureDescriptor &el)
{
  SERIALISE_MEMBER(textureType);
  SERIALISE_MEMBER(pixelFormat);
  SERIALISE_MEMBER(width);
  SERIALISE_MEMBER(height);
  SERIALISE_MEMBER(depth);
  SERIALISE_MEMBER(mipmapLevelCount);
  SERIALISE_MEMBER(sampleCount);
  SERIALISE_MEMBER(arrayLength);
  SERIALISE_MEMBER(resourceOptions);
  SERIALISE_MEMBER(cpuCacheMode);
  SERIALISE_MEMBER(storageMode);
  SERIALISE_MEMBER(hazardTrackingMode);
  SERIALISE_MEMBER(usage);
  SERIALISE_MEMBER(allowGPUOptimizedContents);
  SERIALISE_MEMBER(swizzle);
}

template <typename SerialiserType>
void DoSerialise(SerialiserType &ser, RDMTL::RenderPipelineColorAttachmentDescriptor &el)
{
  SERIALISE_MEMBER(pixelFormat);
  SERIALISE_MEMBER(blendingEnabled);
  SERIALISE_MEMBER(sourceRGBBlendFactor);
  SERIALISE_MEMBER(destinationRGBBlendFactor);
  SERIALISE_MEMBER(rgbBlendOperation);
  SERIALISE_MEMBER(sourceAlphaBlendFactor);
  SERIALISE_MEMBER(destinationAlphaBlendFactor);
  SERIALISE_MEMBER(alphaBlendOperation);
  SERIALISE_MEMBER(writeMask);
}

template <typename SerialiserType>
void DoSerialise(SerialiserType &ser, RDMTL::PipelineBufferDescriptor &el)
{
  SERIALISE_MEMBER(mutability);
}

template <typename SerialiserType>
void DoSerialise(SerialiserType &ser, RDMTL::VertexAttributeDescriptor &el)
{
  SERIALISE_MEMBER(format);
  SERIALISE_MEMBER(offset);
  SERIALISE_MEMBER(bufferIndex);
}

template <typename SerialiserType>
void DoSerialise(SerialiserType &ser, RDMTL::VertexBufferLayoutDescriptor &el)
{
  SERIALISE_MEMBER(stride);
  SERIALISE_MEMBER(stepFunction);
  SERIALISE_MEMBER(stepRate);
}

template <typename SerialiserType>
void DoSerialise(SerialiserType &ser, RDMTL::VertexDescriptor &el)
{
  SERIALISE_MEMBER(layouts);
  SERIALISE_MEMBER(attributes);
}

template <typename SerialiserType>
void DoSerialise(SerialiserType &ser, RDMTL::FunctionGroup &el)
{
  SERIALISE_MEMBER(callsite);
  SERIALISE_MEMBER(functions);
}

template <typename SerialiserType>
void DoSerialise(SerialiserType &ser, RDMTL::LinkedFunctions &el)
{
  SERIALISE_MEMBER(functions);
  SERIALISE_MEMBER(binaryFunctions);
  SERIALISE_MEMBER(groups);
  SERIALISE_MEMBER(privateFunctions);
}

template <typename SerialiserType>
void DoSerialise(SerialiserType &ser, RDMTL::RenderPipelineDescriptor &el)
{
  SERIALISE_MEMBER(label);
  SERIALISE_MEMBER(vertexFunction);
  SERIALISE_MEMBER(fragmentFunction);
  SERIALISE_MEMBER(vertexDescriptor);
  SERIALISE_MEMBER(sampleCount);
  SERIALISE_MEMBER(rasterSampleCount);
  SERIALISE_MEMBER(alphaToCoverageEnabled);
  SERIALISE_MEMBER(alphaToOneEnabled);
  SERIALISE_MEMBER(rasterizationEnabled);
  SERIALISE_MEMBER(maxVertexAmplificationCount);
  SERIALISE_MEMBER(colorAttachments);
  SERIALISE_MEMBER(depthAttachmentPixelFormat);
  SERIALISE_MEMBER(stencilAttachmentPixelFormat);
  SERIALISE_MEMBER(inputPrimitiveTopology);
  SERIALISE_MEMBER(tessellationPartitionMode);
  SERIALISE_MEMBER(maxTessellationFactor);
  SERIALISE_MEMBER(tessellationFactorScaleEnabled);
  SERIALISE_MEMBER(tessellationFactorFormat);
  SERIALISE_MEMBER(tessellationControlPointIndexType);
  SERIALISE_MEMBER(tessellationFactorStepFunction);
  SERIALISE_MEMBER(tessellationOutputWindingOrder);
  SERIALISE_MEMBER(vertexBuffers);
  SERIALISE_MEMBER(fragmentBuffers);
  SERIALISE_MEMBER(supportIndirectCommandBuffers);
  // TODO: will MTL::BinaryArchive need to be a wrapped resource
  // SERIALISE_MEMBER(binaryArchives);
  // TODO: will MTL::DynamicLibrary need to be a wrapped resource
  // SERIALISE_MEMBER(vertexPreloadedLibraries);
  // SERIALISE_MEMBER(fragmentPreloadedLibraries);
  SERIALISE_MEMBER(vertexLinkedFunctions);
  SERIALISE_MEMBER(fragmentLinkedFunctions);
  SERIALISE_MEMBER(supportAddingVertexBinaryFunctions);
  SERIALISE_MEMBER(supportAddingFragmentBinaryFunctions);
  SERIALISE_MEMBER(maxVertexCallStackDepth);
  SERIALISE_MEMBER(maxFragmentCallStackDepth);
}

INSTANTIATE_SERIALISE_TYPE(NS::String *);
INSTANTIATE_SERIALISE_TYPE(MTL::TextureSwizzleChannels);
INSTANTIATE_SERIALISE_TYPE(RDMTL::TextureDescriptor);
INSTANTIATE_SERIALISE_TYPE(RDMTL::RenderPipelineColorAttachmentDescriptor);
INSTANTIATE_SERIALISE_TYPE(RDMTL::PipelineBufferDescriptor);
INSTANTIATE_SERIALISE_TYPE(RDMTL::VertexAttributeDescriptor);
INSTANTIATE_SERIALISE_TYPE(RDMTL::VertexBufferLayoutDescriptor);
INSTANTIATE_SERIALISE_TYPE(RDMTL::VertexDescriptor);
INSTANTIATE_SERIALISE_TYPE(RDMTL::FunctionGroup);
INSTANTIATE_SERIALISE_TYPE(RDMTL::LinkedFunctions);
INSTANTIATE_SERIALISE_TYPE(RDMTL::RenderPipelineDescriptor);
