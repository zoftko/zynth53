cmake = import('cmake')
opt_var = cmake.subproject_options()
opt_var.append_compile_args('cpp', '-w')
opt_var.append_link_args('-w')

gtest_proj = cmake.subproject('googletest', options: opt_var)
gtest = declare_dependency(
  dependencies: [
	dependency('threads'),
	gtest_proj.dependency('gtest'),
    gtest_proj.dependency('gtest_main')
  ]
)

test_si5351 = executable(
  'test_si5351',
  'test_si5351.cc',
  include_directories: inc,
  dependencies: [gtest],
  link_with: [si5351]
)

test('gtest_si5351', test_si5351)
