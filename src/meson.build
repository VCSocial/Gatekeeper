gatekeeper_sources = [
  'main.c',
  'gatekeeper-window.c',
]

gatekeeper_deps = [
  dependency('gio-2.0', version: '>= 2.50'),
  dependency('gtk+-3.0', version: '>= 3.22'),
]

gnome = import('gnome')

gatekeeper_sources += gnome.compile_resources('gatekeeper-resources',
  'gatekeeper.gresource.xml',
  c_name: 'gatekeeper'
)

executable('gatekeeper', gatekeeper_sources,
  dependencies: gatekeeper_deps,
  install: true,
)
