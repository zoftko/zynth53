project(
  'sam',
  ['c', 'cpp'],
  version: '0.1',
  default_options: [
    'warning_level=3',
    'werror=true',
    'c_std=c17',
    'cpp_std=c++20'
    ]
)

inc = include_directories('include')
subdir('src')

if not meson.is_cross_build()
  subdir('tests')
endif
