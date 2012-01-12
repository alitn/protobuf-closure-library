require 'shoulda'
require 'protobuf-closure-library'

class ProtobufClosureLibraryTest < Test::Unit::TestCase
  include ProtobufClosureLibrary

  should 'check if the native extension is sane' do
    assert_equal Object, ProtocJsCore.superclass
  end

  should 'compile a .proto file to closure library javascript' do
    proto_file = File.join GEM_ROOT_DIR, 'test', 'proto', 'test.proto'
    out_dir = File.join GEM_ROOT_DIR, 'tmp'
    compiled_file = File.join out_dir, 'test.pb.js'

    ProtocJs.compile proto_file, out_dir

    assert File.exists?(compiled_file)
  end
end
