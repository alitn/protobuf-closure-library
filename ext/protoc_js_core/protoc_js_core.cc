#include <iostream>
using namespace std;

#pragma GCC diagnostic ignored "-Wconversion"
#include "rice/Class.hpp"
#include "rice/Array.hpp"
#include "rice/String.hpp"
using namespace Rice;

#include <google/protobuf/compiler/plugin.h>
#include <google/protobuf/compiler/command_line_interface.h>
#include "code_generator.h"

void compile (Object ruby_bin, Object proto_file, Object args) {
  // Prepare protoc cli.
  sg::protobuf::js::CodeGenerator js_generator("js_plugin");
  google::protobuf::compiler::CommandLineInterface protoc_cli;
  protoc_cli.RegisterGenerator("--js_out", &js_generator,
    "Generate closure-library javascript code.");
  
  // Create args.
  vector<const char*> argv;
  argv.push_back(String(ruby_bin).c_str());
  Array args_array = Array(args);
  for(Array::iterator it = args_array.begin(); it != args_array.end(); ++it) {
    argv.push_back(String(*it).c_str());
  }
  argv.push_back(String(proto_file).c_str());

  // Redirect cerr.
  stringstream cerr_buffer;
  streambuf *original_cerr_buffer = cerr.rdbuf(cerr_buffer.rdbuf());

  // Run the compiler.
  int result = protoc_cli.Run((int) argv.size(), &argv[0]);

  // Check for errors.
  if (result != 0) {
    string err = cerr_buffer.str();
    rb_raise(rb_eRuntimeError,
      "\nAn error occured while running protoc:\n"
      "-- Protoc output ---------------------\n\n%s\n"
      "--------------------------------------\n",
      err.c_str()
    );
  }

  // De-redirect cerr.
  cerr.rdbuf(original_cerr_buffer);
}

extern "C"
void Init_protoc_js_core() {
  Module mProtobufClosureLibrary = define_module("ProtobufClosureLibrary");
  Class cProtocJs = define_class_under(mProtobufClosureLibrary, "ProtocJsCore");
  cProtocJs.define_singleton_method("compile", &compile);
}
