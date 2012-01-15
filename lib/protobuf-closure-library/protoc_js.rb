module ProtobufClosureLibrary

class ProtocJs
  def self.compile proto_file, out_dir, options
    options = {
      generator_options: {},
      protoc_options: []
    }.merge options

    if !File.exist? out_dir
      FileUtils.mkdir_p out_dir
    end

    generator_options = options[:generator_options].map{|k, v| "#{k}=#{v}" }.join(',')
    js_out_options = generator_options.empty? ? out_dir : "#{generator_options}:#{out_dir}"

    ProtocJsCore.compile RUBY_BIN, proto_file, 
      @@static_proto_path | [
        "--js_out=#{js_out_options}",
        "--proto_path=#{File.dirname proto_file}"
      ] | options[:protoc_options]
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
