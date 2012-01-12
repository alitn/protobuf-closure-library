Protocol buffer javascript compiler for closure library
-------------------------------------------------------

This gem is a wrapper for native protocol buffer javascript compiler which generates closure library proto2 messages.

At the time of writing this gem, google does not provide an official protocol buffer compiler for closure library. An almost identical open source alternative is `protobuf-plugin-closure <http://code.google.com/p/protobuf-plugin-closure/>`_ which this gem is based on.

Usage
+++++

::
  
  ProtobufClosureLibrary::ProtocJs.compile input_proto_file,
                                           output_dir, optional_protoc_arg, ...

This generated a ``.pb.js`` file correponded to ``input_proto_file`` in ``output_dir``. The output file contains a subclass of  `goog.proto2.Message <http://closure-library.googlecode.com/svn/docs/class_goog_proto2_Message.html>`_.
