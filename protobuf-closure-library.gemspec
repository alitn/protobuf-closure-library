# -*- encoding: utf-8 -*-
$:.push File.expand_path("../lib", __FILE__)
require "protobuf-closure-library/version"

Gem::Specification.new do |s|
  s.name        = "protobuf-closure-library"
  s.version     = ProtobufClosureLibrary::VERSION
  s.authors     = ["alitn"]
  s.email       = [""]
  s.homepage    = "https://github.com/alitn/protobuf-closure-library"
  s.summary     = "Protocol buffer javascript compiler for closure library."
  s.description = <<-FIN
      A wrapper for native protocol buffer javascript compiler
      which generates closure library proto2 messages.
    FIN

  s.rubyforge_project = "protobuf-closure-library"

  s.files         = `git ls-files`.split("\n")
  s.test_files    = `git ls-files -- {test,spec,features}/*`.split("\n")
  s.executables   = `git ls-files -- bin/*`.split("\n").map{ |f| File.basename(f) }
  s.require_paths = ["lib"]

  s.add_dependency "rice"
  s.add_development_dependency "shoulda"
  s.add_development_dependency "rake-compiler"

  s.extensions    = ['ext/protoc_js_core/extconf.rb']
end
