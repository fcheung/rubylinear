require 'rubylinear_native'

module RubyLinear
  def self.validate_options(options)
    raise ArgumentError, "A solver must be specified" unless options[:solver]
    unknown_keys = options.keys - [:c, :solver, :eps, :weights]
    if unknown_keys.any?
      raise ArgumentError, "Unknown options: #{unknown_keys.inspect}"
    end
  end
end