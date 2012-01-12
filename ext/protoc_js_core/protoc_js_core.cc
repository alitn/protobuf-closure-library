#include <iostream>
using namespace std;

#pragma GCC diagnostic ignored "-Wconversion"
#include "rice/Class.hpp"
#include "rice/Array.hpp"
#include "rice/String.hpp"
#include "rice/Exception.hpp"
using namespace Rice;

#include <google/protobuf/compiler/plugin.h>
#include <google/protobuf/compiler/command_line_interface.h>
#include "code_generator.h"

class RuntimeException : public exception {
public:
  RuntimeException(string ss) : s(ss) {};
  virtual ~RuntimeException() throw() {};
  virtual const char* what() const throw() { return s.c_str(); };
private:
  string s;
};

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

  int result = protoc_cli.Run((int) argv.size(), &argv[0]);

  if (result != 0) {
    // Use ostringstream to avoid segfault.
    ostringstream oss;
    oss <<
      "An error occured while running protoc:\n"
      "-- Protoc output ---------------------\n\n"
      << cerr_buffer.str() << endl <<
      "--------------------------------------\n";
    throw RuntimeException(oss.str());
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
