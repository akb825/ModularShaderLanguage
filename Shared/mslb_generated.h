// automatically generated by the FlatBuffers compiler, do not modify

#ifndef FLATBUFFERS_GENERATED_MSLB_MSLB_H_
#define FLATBUFFERS_GENERATED_MSLB_MSLB_H_

#include "flatbuffers/flatbuffers.h"


namespace mslb {

struct Uniform;
struct UniformBlock;
struct Attribute;
struct Pipeline;
struct Shader;
struct Module;

enum class Type : uint8_t {
  Float = 0,
  Vec2 = 1,
  Vec3 = 2,
  Vec4 = 3,
  Double = 4,
  DVec2 = 5,
  DVec3 = 6,
  DVec4 = 7,
  Int = 8,
  IVec2 = 9,
  IVec3 = 10,
  IVec4 = 11,
  UInt = 12,
  UVec2 = 13,
  UVec3 = 14,
  UVec4 = 15,
  Bool = 16,
  BVec2 = 17,
  BVec3 = 18,
  BVec4 = 19,
  Mat2 = 20,
  Mat3 = 21,
  Mat4 = 22,
  Mat2x3 = 23,
  Mat2x4 = 24,
  Mat3x2 = 25,
  Mat3x4 = 26,
  Mat4x2 = 27,
  Mat4x3 = 28,
  DMat2 = 29,
  DMat3 = 30,
  DMat4 = 31,
  DMat2x3 = 32,
  DMat2x4 = 33,
  DMat3x2 = 34,
  DMat3x4 = 35,
  DMat4x2 = 36,
  DMat4x3 = 37,
  Sampler1D = 38,
  Sampler2D = 39,
  Sampler3D = 40,
  SamplerCube = 41,
  Sampler1DShadow = 42,
  Sampler2DShadow = 43,
  Sampler1DArray = 44,
  Sampler2DArray = 45,
  Sampler1DArrayShadow = 46,
  Sampler2DArrayShadow = 47,
  Sampler2DMS = 48,
  Sampler2DMSArray = 49,
  SamplerCubeShadow = 50,
  SamplerBuffer = 51,
  Sampler2DRect = 52,
  Sampler2DRectShadow = 53,
  ISampler1D = 54,
  ISampler2D = 55,
  ISampler3D = 56,
  ISamplerCube = 57,
  ISampler1DArray = 58,
  ISampler2DArray = 59,
  ISampler2DMS = 60,
  ISampler2DMSArray = 61,
  ISampler2DRect = 62,
  USampler1D = 63,
  USampler2D = 64,
  USampler3D = 65,
  USamplerCube = 66,
  USampler1DArray = 67,
  USampler2DArray = 68,
  USampler2DMS = 69,
  USampler2DMSArray = 70,
  USampler2DRect = 71,
  MIN = Float,
  MAX = USampler2DRect
};

inline const char **EnumNamesType() {
  static const char *names[] = { "Float", "Vec2", "Vec3", "Vec4", "Double", "DVec2", "DVec3", "DVec4", "Int", "IVec2", "IVec3", "IVec4", "UInt", "UVec2", "UVec3", "UVec4", "Bool", "BVec2", "BVec3", "BVec4", "Mat2", "Mat3", "Mat4", "Mat2x3", "Mat2x4", "Mat3x2", "Mat3x4", "Mat4x2", "Mat4x3", "DMat2", "DMat3", "DMat4", "DMat2x3", "DMat2x4", "DMat3x2", "DMat3x4", "DMat4x2", "DMat4x3", "Sampler1D", "Sampler2D", "Sampler3D", "SamplerCube", "Sampler1DShadow", "Sampler2DShadow", "Sampler1DArray", "Sampler2DArray", "Sampler1DArrayShadow", "Sampler2DArrayShadow", "Sampler2DMS", "Sampler2DMSArray", "SamplerCubeShadow", "SamplerBuffer", "Sampler2DRect", "Sampler2DRectShadow", "ISampler1D", "ISampler2D", "ISampler3D", "ISamplerCube", "ISampler1DArray", "ISampler2DArray", "ISampler2DMS", "ISampler2DMSArray", "ISampler2DRect", "USampler1D", "USampler2D", "USampler3D", "USamplerCube", "USampler1DArray", "USampler2DArray", "USampler2DMS", "USampler2DMSArray", "USampler2DRect", nullptr };
  return names;
}

inline const char *EnumNameType(Type e) { return EnumNamesType()[static_cast<int>(e)]; }

struct Uniform FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  enum {
    VT_NAME = 4,
    VT_TYPE = 6,
    VT_BLOCKINDEX = 8,
    VT_BUFFEROFFSET = 10,
    VT_ELEMENTS = 12
  };
  const flatbuffers::String *name() const { return GetPointer<const flatbuffers::String *>(VT_NAME); }
  Type type() const { return static_cast<Type>(GetField<uint8_t>(VT_TYPE, 0)); }
  uint32_t blockIndex() const { return GetField<uint32_t>(VT_BLOCKINDEX, 0); }
  uint32_t bufferOffset() const { return GetField<uint32_t>(VT_BUFFEROFFSET, 0); }
  uint32_t elements() const { return GetField<uint32_t>(VT_ELEMENTS, 0); }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyFieldRequired<flatbuffers::uoffset_t>(verifier, VT_NAME) &&
           verifier.Verify(name()) &&
           VerifyField<uint8_t>(verifier, VT_TYPE) &&
           VerifyField<uint32_t>(verifier, VT_BLOCKINDEX) &&
           VerifyField<uint32_t>(verifier, VT_BUFFEROFFSET) &&
           VerifyField<uint32_t>(verifier, VT_ELEMENTS) &&
           verifier.EndTable();
  }
};

struct UniformBuilder {
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_name(flatbuffers::Offset<flatbuffers::String> name) { fbb_.AddOffset(Uniform::VT_NAME, name); }
  void add_type(Type type) { fbb_.AddElement<uint8_t>(Uniform::VT_TYPE, static_cast<uint8_t>(type), 0); }
  void add_blockIndex(uint32_t blockIndex) { fbb_.AddElement<uint32_t>(Uniform::VT_BLOCKINDEX, blockIndex, 0); }
  void add_bufferOffset(uint32_t bufferOffset) { fbb_.AddElement<uint32_t>(Uniform::VT_BUFFEROFFSET, bufferOffset, 0); }
  void add_elements(uint32_t elements) { fbb_.AddElement<uint32_t>(Uniform::VT_ELEMENTS, elements, 0); }
  UniformBuilder(flatbuffers::FlatBufferBuilder &_fbb) : fbb_(_fbb) { start_ = fbb_.StartTable(); }
  UniformBuilder &operator=(const UniformBuilder &);
  flatbuffers::Offset<Uniform> Finish() {
    auto o = flatbuffers::Offset<Uniform>(fbb_.EndTable(start_, 5));
    fbb_.Required(o, Uniform::VT_NAME);  // name
    return o;
  }
};

inline flatbuffers::Offset<Uniform> CreateUniform(flatbuffers::FlatBufferBuilder &_fbb,
   flatbuffers::Offset<flatbuffers::String> name = 0,
   Type type = Type::Float,
   uint32_t blockIndex = 0,
   uint32_t bufferOffset = 0,
   uint32_t elements = 0) {
  UniformBuilder builder_(_fbb);
  builder_.add_elements(elements);
  builder_.add_bufferOffset(bufferOffset);
  builder_.add_blockIndex(blockIndex);
  builder_.add_name(name);
  builder_.add_type(type);
  return builder_.Finish();
}

struct UniformBlock FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  enum {
    VT_NAME = 4,
    VT_SIZE = 6
  };
  const flatbuffers::String *name() const { return GetPointer<const flatbuffers::String *>(VT_NAME); }
  uint32_t size() const { return GetField<uint32_t>(VT_SIZE, 0); }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyFieldRequired<flatbuffers::uoffset_t>(verifier, VT_NAME) &&
           verifier.Verify(name()) &&
           VerifyField<uint32_t>(verifier, VT_SIZE) &&
           verifier.EndTable();
  }
};

struct UniformBlockBuilder {
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_name(flatbuffers::Offset<flatbuffers::String> name) { fbb_.AddOffset(UniformBlock::VT_NAME, name); }
  void add_size(uint32_t size) { fbb_.AddElement<uint32_t>(UniformBlock::VT_SIZE, size, 0); }
  UniformBlockBuilder(flatbuffers::FlatBufferBuilder &_fbb) : fbb_(_fbb) { start_ = fbb_.StartTable(); }
  UniformBlockBuilder &operator=(const UniformBlockBuilder &);
  flatbuffers::Offset<UniformBlock> Finish() {
    auto o = flatbuffers::Offset<UniformBlock>(fbb_.EndTable(start_, 2));
    fbb_.Required(o, UniformBlock::VT_NAME);  // name
    return o;
  }
};

inline flatbuffers::Offset<UniformBlock> CreateUniformBlock(flatbuffers::FlatBufferBuilder &_fbb,
   flatbuffers::Offset<flatbuffers::String> name = 0,
   uint32_t size = 0) {
  UniformBlockBuilder builder_(_fbb);
  builder_.add_size(size);
  builder_.add_name(name);
  return builder_.Finish();
}

struct Attribute FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  enum {
    VT_NAME = 4,
    VT_TYPE = 6
  };
  const flatbuffers::String *name() const { return GetPointer<const flatbuffers::String *>(VT_NAME); }
  Type type() const { return static_cast<Type>(GetField<uint8_t>(VT_TYPE, 0)); }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyFieldRequired<flatbuffers::uoffset_t>(verifier, VT_NAME) &&
           verifier.Verify(name()) &&
           VerifyField<uint8_t>(verifier, VT_TYPE) &&
           verifier.EndTable();
  }
};

struct AttributeBuilder {
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_name(flatbuffers::Offset<flatbuffers::String> name) { fbb_.AddOffset(Attribute::VT_NAME, name); }
  void add_type(Type type) { fbb_.AddElement<uint8_t>(Attribute::VT_TYPE, static_cast<uint8_t>(type), 0); }
  AttributeBuilder(flatbuffers::FlatBufferBuilder &_fbb) : fbb_(_fbb) { start_ = fbb_.StartTable(); }
  AttributeBuilder &operator=(const AttributeBuilder &);
  flatbuffers::Offset<Attribute> Finish() {
    auto o = flatbuffers::Offset<Attribute>(fbb_.EndTable(start_, 2));
    fbb_.Required(o, Attribute::VT_NAME);  // name
    return o;
  }
};

inline flatbuffers::Offset<Attribute> CreateAttribute(flatbuffers::FlatBufferBuilder &_fbb,
   flatbuffers::Offset<flatbuffers::String> name = 0,
   Type type = Type::Float) {
  AttributeBuilder builder_(_fbb);
  builder_.add_name(name);
  builder_.add_type(type);
  return builder_.Finish();
}

struct Pipeline FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  enum {
    VT_NAME = 4,
    VT_VERTEX = 6,
    VT_TESSELLATIONCONTROL = 8,
    VT_TESSELLATIONEVALUATION = 10,
    VT_GEOMETRY = 12,
    VT_FRAGMENT = 14,
    VT_COMPUTE = 16,
    VT_UNIFORMS = 18,
    VT_UNIFORMBLOCKS = 20,
    VT_ATTRIBUTES = 22
  };
  const flatbuffers::String *name() const { return GetPointer<const flatbuffers::String *>(VT_NAME); }
  uint32_t vertex() const { return GetField<uint32_t>(VT_VERTEX, 0); }
  uint32_t tessellationControl() const { return GetField<uint32_t>(VT_TESSELLATIONCONTROL, 0); }
  uint32_t tessellationEvaluation() const { return GetField<uint32_t>(VT_TESSELLATIONEVALUATION, 0); }
  uint32_t geometry() const { return GetField<uint32_t>(VT_GEOMETRY, 0); }
  uint32_t fragment() const { return GetField<uint32_t>(VT_FRAGMENT, 0); }
  uint32_t compute() const { return GetField<uint32_t>(VT_COMPUTE, 0); }
  const flatbuffers::Vector<flatbuffers::Offset<Uniform>> *uniforms() const { return GetPointer<const flatbuffers::Vector<flatbuffers::Offset<Uniform>> *>(VT_UNIFORMS); }
  const flatbuffers::Vector<flatbuffers::Offset<UniformBlock>> *uniformBlocks() const { return GetPointer<const flatbuffers::Vector<flatbuffers::Offset<UniformBlock>> *>(VT_UNIFORMBLOCKS); }
  const flatbuffers::Vector<flatbuffers::Offset<Attribute>> *attributes() const { return GetPointer<const flatbuffers::Vector<flatbuffers::Offset<Attribute>> *>(VT_ATTRIBUTES); }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyFieldRequired<flatbuffers::uoffset_t>(verifier, VT_NAME) &&
           verifier.Verify(name()) &&
           VerifyField<uint32_t>(verifier, VT_VERTEX) &&
           VerifyField<uint32_t>(verifier, VT_TESSELLATIONCONTROL) &&
           VerifyField<uint32_t>(verifier, VT_TESSELLATIONEVALUATION) &&
           VerifyField<uint32_t>(verifier, VT_GEOMETRY) &&
           VerifyField<uint32_t>(verifier, VT_FRAGMENT) &&
           VerifyField<uint32_t>(verifier, VT_COMPUTE) &&
           VerifyFieldRequired<flatbuffers::uoffset_t>(verifier, VT_UNIFORMS) &&
           verifier.Verify(uniforms()) &&
           verifier.VerifyVectorOfTables(uniforms()) &&
           VerifyFieldRequired<flatbuffers::uoffset_t>(verifier, VT_UNIFORMBLOCKS) &&
           verifier.Verify(uniformBlocks()) &&
           verifier.VerifyVectorOfTables(uniformBlocks()) &&
           VerifyFieldRequired<flatbuffers::uoffset_t>(verifier, VT_ATTRIBUTES) &&
           verifier.Verify(attributes()) &&
           verifier.VerifyVectorOfTables(attributes()) &&
           verifier.EndTable();
  }
};

struct PipelineBuilder {
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_name(flatbuffers::Offset<flatbuffers::String> name) { fbb_.AddOffset(Pipeline::VT_NAME, name); }
  void add_vertex(uint32_t vertex) { fbb_.AddElement<uint32_t>(Pipeline::VT_VERTEX, vertex, 0); }
  void add_tessellationControl(uint32_t tessellationControl) { fbb_.AddElement<uint32_t>(Pipeline::VT_TESSELLATIONCONTROL, tessellationControl, 0); }
  void add_tessellationEvaluation(uint32_t tessellationEvaluation) { fbb_.AddElement<uint32_t>(Pipeline::VT_TESSELLATIONEVALUATION, tessellationEvaluation, 0); }
  void add_geometry(uint32_t geometry) { fbb_.AddElement<uint32_t>(Pipeline::VT_GEOMETRY, geometry, 0); }
  void add_fragment(uint32_t fragment) { fbb_.AddElement<uint32_t>(Pipeline::VT_FRAGMENT, fragment, 0); }
  void add_compute(uint32_t compute) { fbb_.AddElement<uint32_t>(Pipeline::VT_COMPUTE, compute, 0); }
  void add_uniforms(flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<Uniform>>> uniforms) { fbb_.AddOffset(Pipeline::VT_UNIFORMS, uniforms); }
  void add_uniformBlocks(flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<UniformBlock>>> uniformBlocks) { fbb_.AddOffset(Pipeline::VT_UNIFORMBLOCKS, uniformBlocks); }
  void add_attributes(flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<Attribute>>> attributes) { fbb_.AddOffset(Pipeline::VT_ATTRIBUTES, attributes); }
  PipelineBuilder(flatbuffers::FlatBufferBuilder &_fbb) : fbb_(_fbb) { start_ = fbb_.StartTable(); }
  PipelineBuilder &operator=(const PipelineBuilder &);
  flatbuffers::Offset<Pipeline> Finish() {
    auto o = flatbuffers::Offset<Pipeline>(fbb_.EndTable(start_, 10));
    fbb_.Required(o, Pipeline::VT_NAME);  // name
    fbb_.Required(o, Pipeline::VT_UNIFORMS);  // uniforms
    fbb_.Required(o, Pipeline::VT_UNIFORMBLOCKS);  // uniformBlocks
    fbb_.Required(o, Pipeline::VT_ATTRIBUTES);  // attributes
    return o;
  }
};

inline flatbuffers::Offset<Pipeline> CreatePipeline(flatbuffers::FlatBufferBuilder &_fbb,
   flatbuffers::Offset<flatbuffers::String> name = 0,
   uint32_t vertex = 0,
   uint32_t tessellationControl = 0,
   uint32_t tessellationEvaluation = 0,
   uint32_t geometry = 0,
   uint32_t fragment = 0,
   uint32_t compute = 0,
   flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<Uniform>>> uniforms = 0,
   flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<UniformBlock>>> uniformBlocks = 0,
   flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<Attribute>>> attributes = 0) {
  PipelineBuilder builder_(_fbb);
  builder_.add_attributes(attributes);
  builder_.add_uniformBlocks(uniformBlocks);
  builder_.add_uniforms(uniforms);
  builder_.add_compute(compute);
  builder_.add_fragment(fragment);
  builder_.add_geometry(geometry);
  builder_.add_tessellationEvaluation(tessellationEvaluation);
  builder_.add_tessellationControl(tessellationControl);
  builder_.add_vertex(vertex);
  builder_.add_name(name);
  return builder_.Finish();
}

struct Shader FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  enum {
    VT_DATA = 4
  };
  const flatbuffers::Vector<uint8_t> *data() const { return GetPointer<const flatbuffers::Vector<uint8_t> *>(VT_DATA); }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<flatbuffers::uoffset_t>(verifier, VT_DATA) &&
           verifier.Verify(data()) &&
           verifier.EndTable();
  }
};

struct ShaderBuilder {
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_data(flatbuffers::Offset<flatbuffers::Vector<uint8_t>> data) { fbb_.AddOffset(Shader::VT_DATA, data); }
  ShaderBuilder(flatbuffers::FlatBufferBuilder &_fbb) : fbb_(_fbb) { start_ = fbb_.StartTable(); }
  ShaderBuilder &operator=(const ShaderBuilder &);
  flatbuffers::Offset<Shader> Finish() {
    auto o = flatbuffers::Offset<Shader>(fbb_.EndTable(start_, 1));
    return o;
  }
};

inline flatbuffers::Offset<Shader> CreateShader(flatbuffers::FlatBufferBuilder &_fbb,
   flatbuffers::Offset<flatbuffers::Vector<uint8_t>> data = 0) {
  ShaderBuilder builder_(_fbb);
  builder_.add_data(data);
  return builder_.Finish();
}

struct Module FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  enum {
    VT_VERSION = 4,
    VT_TARGETID = 6,
    VT_TARGETVERSION = 8,
    VT_PIPELINES = 10,
    VT_SHADERS = 12,
    VT_SHAREDDATA = 14
  };
  uint32_t version() const { return GetField<uint32_t>(VT_VERSION, 0); }
  uint32_t targetId() const { return GetField<uint32_t>(VT_TARGETID, 0); }
  uint32_t targetVersion() const { return GetField<uint32_t>(VT_TARGETVERSION, 0); }
  const flatbuffers::Vector<flatbuffers::Offset<Pipeline>> *pipelines() const { return GetPointer<const flatbuffers::Vector<flatbuffers::Offset<Pipeline>> *>(VT_PIPELINES); }
  const flatbuffers::Vector<flatbuffers::Offset<Shader>> *shaders() const { return GetPointer<const flatbuffers::Vector<flatbuffers::Offset<Shader>> *>(VT_SHADERS); }
  const flatbuffers::Vector<uint8_t> *sharedData() const { return GetPointer<const flatbuffers::Vector<uint8_t> *>(VT_SHAREDDATA); }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<uint32_t>(verifier, VT_VERSION) &&
           VerifyField<uint32_t>(verifier, VT_TARGETID) &&
           VerifyField<uint32_t>(verifier, VT_TARGETVERSION) &&
           VerifyFieldRequired<flatbuffers::uoffset_t>(verifier, VT_PIPELINES) &&
           verifier.Verify(pipelines()) &&
           verifier.VerifyVectorOfTables(pipelines()) &&
           VerifyFieldRequired<flatbuffers::uoffset_t>(verifier, VT_SHADERS) &&
           verifier.Verify(shaders()) &&
           verifier.VerifyVectorOfTables(shaders()) &&
           VerifyFieldRequired<flatbuffers::uoffset_t>(verifier, VT_SHAREDDATA) &&
           verifier.Verify(sharedData()) &&
           verifier.EndTable();
  }
};

struct ModuleBuilder {
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_version(uint32_t version) { fbb_.AddElement<uint32_t>(Module::VT_VERSION, version, 0); }
  void add_targetId(uint32_t targetId) { fbb_.AddElement<uint32_t>(Module::VT_TARGETID, targetId, 0); }
  void add_targetVersion(uint32_t targetVersion) { fbb_.AddElement<uint32_t>(Module::VT_TARGETVERSION, targetVersion, 0); }
  void add_pipelines(flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<Pipeline>>> pipelines) { fbb_.AddOffset(Module::VT_PIPELINES, pipelines); }
  void add_shaders(flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<Shader>>> shaders) { fbb_.AddOffset(Module::VT_SHADERS, shaders); }
  void add_sharedData(flatbuffers::Offset<flatbuffers::Vector<uint8_t>> sharedData) { fbb_.AddOffset(Module::VT_SHAREDDATA, sharedData); }
  ModuleBuilder(flatbuffers::FlatBufferBuilder &_fbb) : fbb_(_fbb) { start_ = fbb_.StartTable(); }
  ModuleBuilder &operator=(const ModuleBuilder &);
  flatbuffers::Offset<Module> Finish() {
    auto o = flatbuffers::Offset<Module>(fbb_.EndTable(start_, 6));
    fbb_.Required(o, Module::VT_PIPELINES);  // pipelines
    fbb_.Required(o, Module::VT_SHADERS);  // shaders
    fbb_.Required(o, Module::VT_SHAREDDATA);  // sharedData
    return o;
  }
};

inline flatbuffers::Offset<Module> CreateModule(flatbuffers::FlatBufferBuilder &_fbb,
   uint32_t version = 0,
   uint32_t targetId = 0,
   uint32_t targetVersion = 0,
   flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<Pipeline>>> pipelines = 0,
   flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<Shader>>> shaders = 0,
   flatbuffers::Offset<flatbuffers::Vector<uint8_t>> sharedData = 0) {
  ModuleBuilder builder_(_fbb);
  builder_.add_sharedData(sharedData);
  builder_.add_shaders(shaders);
  builder_.add_pipelines(pipelines);
  builder_.add_targetVersion(targetVersion);
  builder_.add_targetId(targetId);
  builder_.add_version(version);
  return builder_.Finish();
}

inline const mslb::Module *GetModule(const void *buf) { return flatbuffers::GetRoot<mslb::Module>(buf); }

inline bool VerifyModuleBuffer(flatbuffers::Verifier &verifier) { return verifier.VerifyBuffer<mslb::Module>(); }

inline void FinishModuleBuffer(flatbuffers::FlatBufferBuilder &fbb, flatbuffers::Offset<mslb::Module> root) { fbb.Finish(root); }

}  // namespace mslb

#endif  // FLATBUFFERS_GENERATED_MSLB_MSLB_H_
