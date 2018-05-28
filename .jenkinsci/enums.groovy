// types of tests provided by the developers (can be found at the CMakeLists.txt files)
enum TestTypes {
	module, integration, system, cmake, regression, benchmark, framework
	public TestTypes() {}
}

// list of agent labels (build platforms) to count coverage (the choice depends on the build parameters)
enum CoveragePlatforms {
  x86_64_aws_cov, mac, armv8, armv7
}

return this
