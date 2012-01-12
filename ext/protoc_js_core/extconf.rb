require 'mkmf-rice'

SOURCE_DIR = File.expand_path File.dirname(__FILE__)
COMPILED_PROTO_DIR = SOURCE_DIR #File.join(SOURCE_DIR, 'js')

HEADER_DIRS = ['/usr/local/include', '/usr/include']
LIB_DIRS = ['/usr/local/lib', '/usr/lib']

Dir::mkdir COMPILED_PROTO_DIR if !FileTest::directory?(COMPILED_PROTO_DIR)

def generate_proto_deps proto
  `protoc -I /usr/include -I /usr/local/include -I #{SOURCE_DIR}/js --cpp_out=#{COMPILED_PROTO_DIR} #{proto}`
end
generate_proto_deps "#{SOURCE_DIR}/js/javascript_package.proto"
generate_proto_deps "#{SOURCE_DIR}/js/int64_encoding.proto"

dir_config 'protoc_js_core', HEADER_DIRS, LIB_DIRS

$LIBS << ' -lprotobuf -lprotoc -lpthread'

have_library 'protobuf'
have_library 'protoc'
have_library 'pthread'

$warnflags.gsub! /-Wdeclaration-after-statement/, ''
$warnflags.gsub! /-Wimplicit-function-declaration/, ''

create_makefile 'protoc_js_core'
