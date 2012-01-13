module ProtobufClosureLibrary

class ProtocJs
  def self.compile proto_file, out_dir, *args
    if !File.exist? out_dir
      FileUtils.mkdir_p out_dir
    end

    ProtocJsCore.compile RUBY_BIN, proto_file, 
      @@static_proto_path | [
        "--js_out=#{out_dir}",
        "--proto_path=#{File.dirname proto_file}"
      ] | args
  end

private
  RUBY_BIN = File.join(
              RbConfig::CONFIG['bindir'],
              RbConfig::CONFIG['ruby_install_name']).sub(/.*\s.*/m, '"\&"')
  PROTOC_JS_CORE_DIR = File.join GEM_ROOT_DIR, 'ext', 'protoc_js_core'

  def self.add_static_proto_path path
    return if !File.directory?(path)
    @@static_proto_path |= ["--proto_path=#{path}"]
  end

  @@static_proto_path = []
  add_static_proto_path '/usr/include'
  add_static_proto_path '/usr/local/include'
  add_static_proto_path '/opt/include'
  add_static_proto_path PROTOC_JS_CORE_DIR
end

end
