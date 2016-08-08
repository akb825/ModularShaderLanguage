/*
 * Copyright 2016 Aaron Barany
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <MSL/Compile/Target.h>
#include <gtest/gtest.h>
#include <locale>

namespace msl
{

namespace
{

class MockTarget : public Target
{
public:
	std::uint32_t getId() const override
	{
		return MSL_CREATE_ID('M', 'O', 'C', 'K');
	}

	std::uint32_t getVersion() const override
	{
		return 0;
	}

	bool featureSupported(Feature feature) const override
	{
		return feature == Feature::Integers;
	}

	std::vector<std::uint8_t> crossCompile(Output&, const std::vector<std::uint32_t>&,
		const std::string&, std::size_t, std::size_t) const override
	{
		return std::vector<std::uint8_t>();
	}
};

std::string generateMacroName(const std::string& str)
{
	std::string macro = "HAS_";
	for (std::size_t i = 0; i < str.size(); ++i)
	{
		if (i > 0 && std::isupper(str[i]) && !std::isdigit(str[i - 1]))
			macro.push_back('_');
		macro.push_back(static_cast<char>(std::toupper(str[i])));
	}
	return macro;
}

} // namespace

#define TEST_FEATURE(featureName, counter) \
	EXPECT_STREQ(#featureName, Target::getFeatureInfo(Target::Feature::featureName).name); \
	EXPECT_EQ(generateMacroName(#featureName), \
		Target::getFeatureInfo(Target::Feature::featureName).define); \
	++counter

TEST(TargetTest, FeatureList)
{
	unsigned int counter = 0;
	TEST_FEATURE(Integers, counter);
	TEST_FEATURE(Doubles, counter);
	TEST_FEATURE(NonSquareMatrices, counter);
	TEST_FEATURE(Texture3D, counter);
	TEST_FEATURE(TextureArray, counter);
	TEST_FEATURE(ShadowSamplers, counter);
	TEST_FEATURE(MultisampledTextures, counter);
	TEST_FEATURE(IntegerTextures, counter);
	TEST_FEATURE(Images, counter);
	TEST_FEATURE(UniformBuffers, counter);
	TEST_FEATURE(Buffers, counter);
	TEST_FEATURE(Std140, counter);
	TEST_FEATURE(Std430, counter);
	TEST_FEATURE(TessellationStages, counter);
	TEST_FEATURE(GeometryStage, counter);
	TEST_FEATURE(ComputeStage, counter);
	TEST_FEATURE(MultipleRenderTargets, counter);
	TEST_FEATURE(DualSourceBlending, counter);
	TEST_FEATURE(DepthHints, counter);
	TEST_FEATURE(Derivatives, counter);
	TEST_FEATURE(AdvancedDerivatives, counter);
	TEST_FEATURE(MemoryBarriers, counter);
	TEST_FEATURE(PrimitiveStreams, counter);
	TEST_FEATURE(InterpolationFunctions, counter);
	TEST_FEATURE(TextureGather, counter);
	TEST_FEATURE(TexelFetch, counter);
	TEST_FEATURE(TextureSize, counter);
	TEST_FEATURE(TextureQueryLod, counter);
	TEST_FEATURE(TextureQueryLevels, counter);
	TEST_FEATURE(TextureSamples, counter);
	TEST_FEATURE(BitFunctions, counter);
	TEST_FEATURE(PackingFunctions, counter);

	unsigned int featureCount = Target::featureCount;
	EXPECT_EQ(featureCount, counter);
}

TEST(TargetTest, FeatureEnabled)
{
	MockTarget target;
	EXPECT_TRUE(target.featureEnabled(Target::Feature::Integers));
	EXPECT_FALSE(target.featureEnabled(Target::Feature::Doubles));

	EXPECT_FALSE(target.isFeatureOverridden(Target::Feature::Integers));
	EXPECT_FALSE(target.isFeatureOverridden(Target::Feature::Doubles));

	target.overrideFeature(Target::Feature::Integers, false);
	target.overrideFeature(Target::Feature::Doubles, true);

	EXPECT_TRUE(target.isFeatureOverridden(Target::Feature::Integers));
	EXPECT_TRUE(target.isFeatureOverridden(Target::Feature::Doubles));

	EXPECT_FALSE(target.featureEnabled(Target::Feature::Integers));
	EXPECT_TRUE(target.featureEnabled(Target::Feature::Doubles));

	target.clearOverride(Target::Feature::Integers);
	target.clearOverride(Target::Feature::Doubles);

	EXPECT_FALSE(target.isFeatureOverridden(Target::Feature::Integers));
	EXPECT_FALSE(target.isFeatureOverridden(Target::Feature::Doubles));

	EXPECT_TRUE(target.featureEnabled(Target::Feature::Integers));
	EXPECT_FALSE(target.featureEnabled(Target::Feature::Doubles));
}

} // namespace msl
