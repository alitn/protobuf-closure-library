#!/usr/bin/env rake
begin
  require 'bundler/setup'
rescue LoadError
  puts 'You must `gem install bundler` and `bundle install` to run rake tasks'
end

require 'bundler/gem_tasks'
require 'rake/extensiontask'
require 'rake/testtask'

def gemspec
  @gemspec ||= eval(File.read File.expand_path('../protobuf-closure-library.gemspec', __FILE__))
end

Rake::ExtensionTask.new :protoc_js_core, gemspec

Rake::TestTask.new :test  do |t|
  t.libs << 'lib'
  t.libs << 'test'
  t.pattern = 'test/**/*_test.rb'
  t.verbose = true
end
