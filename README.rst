Protocol buffer javascript compiler for closure library
-------------------------------------------------------

This gem is a wrapper for native protocol buffer javascript compiler which generates closure library proto2 messages.

At the time of writing this gem, google does not provide an official protocol buffer compiler for closure library. An almost identical open source alternative is `protobuf-plugin-closure <http://code.google.com/p/protobuf-plugin-closure/>`_ which this gem is based on.

Installation
++++++++++++

`Protobuf <http://code.google.com/p/protobuf/>`_ runtime compiler and development header and libraries must be present at gem installation time.

* Under linux, user the right development package, eg ``libprotobuf-dev`` for ubuntu.
* Under osx, use ``brew install protobuf``.
* under windows, use the `official protobuf binaries <http://code.google.com/p/protobuf/downloads/list>`_.

Then, install the gem:

::
  
  gem install protobuf-closure-library

Usage
+++++

::
  
  require 'protobuf-closure-library'
  ProtobufClosureLibrary::ProtocJs.compile input_proto_file,
                                           output_dir, {
                                             generator_options: {key: value},
                                             protoc_options: []
                                           }

This generated a ``.pb.js`` file correponded to ``input_proto_file`` in ``output_dir``. The output file contains a subclass of  `goog.proto2.Message <http://closure-library.googlecode.com/svn/docs/class_goog_proto2_Message.html>`_.

By default, the generated message classes are subclasses of ``goog.proto2.Message``. This can be overriden by using ``generator_options: {js_superclass: 'cusom_package.CustomClass'}`` which generates message classes with ``cusom_package.CustomClass`` as the superclass.

Other protoc options can be passed to the compiler by ``protoc_options`` key.
