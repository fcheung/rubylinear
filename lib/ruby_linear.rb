require 'rubylinear_native'

module RubyLinear
  class Parameter
    attr_accessor :eps
    attr_accessor :c
    attr_accessor :weights
    attr_accessor :solver_type
    def initialize(solver_type)
      self.solver_type = solver_type;
      self.eps = 0.01
      self.c = 1
      self.weights = {}
    end
  end
end