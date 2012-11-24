# rb-threadframe

rb-threadframe is a set of patches to Ruby MRI 1.9.2 and 1.9.3 that
adds introspection of call frames and adds other run-time support for debuggers and run-time introspection. 

For MRI 1.9.2, there are additional routines are in a C extension. For MRI 1.9.3, everthing is in a patched Ruby. Necessary patches and some simple patch code are found in this repository though. 

## Requirements

Source for Ruby MRI version 1.9.2 or 1.9.3 p327. Patches are provided
for these two versions only.

See the [wiki](http://github.com/rocky/rb-threadframe/wiki) for more information.
