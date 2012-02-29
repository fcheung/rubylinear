require 'rubygems'
require 'bundler/setup'

$: << File.dirname(__FILE__) + '/../ext'
puts $:
require 'ruby_linear'


RSpec.configure do |config|
  
end