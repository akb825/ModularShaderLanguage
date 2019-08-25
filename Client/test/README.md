# Generating test files

Run the latest compiled version with the following arguments:

* mslc -c %{sourceDir}/tools/mslc/test/spirv.conf -o %{sourceDir}/Client/test/CompleteShader.mslb %{sourceDir}/tools/mslc/test/shaders/CompleteShader.msl -O
* mslc -c %{sourceDir}/tools/mslc/test/spirv.conf -o %{sourceDir}/Client/test/ComputeLocalSize.mslb %{sourceDir}/Compile/test/inputs/ComputeLocalSize.msl
